// Scanner/CDRScanner.cpp
#include "CDRScanner.h"
#include "../../core/interfaces/ScannerTypes.h"
#include "../../infrastructure/docker/DockerTypes.h"
#include "../../infrastructure/docker/DockerExceptions.h"
#include "CdrManager.h"
#include "CdrTypes.h"

#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QThread>

CDRScanner::CDRScanner() : currentStatus(ScanStatus::Idle), 
                           threatsDetected(false),
                           wasSanitized(false),
                           cdrContainerName(QStringLiteral("cdr_processor_container")),
                           cdrImageName(QStringLiteral("ubuntu:latest")) {
    try {
        dockerManager = std::make_unique<Docker::DockerManager>();
        cdrManager = std::make_unique<CDR::CdrManager>();
          // Initialize CDR configuration with sensible defaults
        cdrConfig.securityLevel = CDR::CdrConfiguration::SecurityLevel::MEDIUM;
        cdrConfig.inputDirectory = "/tmp/cdr_input";
        cdrConfig.outputDirectory = "/tmp/cdr_output";
        cdrConfig.quarantineDirectory = "/tmp/cdr_quarantine";
        cdrConfig.analysisType = CDR::AnalysisType::COMPREHENSIVE_SCAN;
        cdrConfig.preserveOriginal = true;
        cdrConfig.autoSanitize = true;
        cdrConfig.blockExecutables = true;
        cdrConfig.blockAllScripts = true;
        cdrConfig.maxFileSizeMB = 100;
        cdrConfig.timeoutSeconds = 300;
        
        initializeCdrComponents();
        if (!isContainerReady()) {
            lastError = QStringLiteral("CDR Docker container is not ready or not running.");
            qWarning() << lastError;
            // Try to ensure the container is running
            if (!ensureCdrContainerRunning()) {
                currentStatus = ScanStatus::Error;
            }
        }
    } catch (const Docker::DaemonException& e) {
        lastError = QStringLiteral("Failed to connect to Docker daemon: %1").arg(e.what());
        currentStatus = ScanStatus::Error;
        qCritical() << lastError;
    } catch (const CDR::CdrBaseException& e) {
        lastError = QStringLiteral("CDR initialization error: %1").arg(e.what());
        currentStatus = ScanStatus::Error;
        qCritical() << lastError;
    } catch (const std::exception& e) {
        lastError = QStringLiteral("CDRScanner initialization error: %1").arg(e.what());
        currentStatus = ScanStatus::Error;
        qCritical() << lastError;
    }
}

void CDRScanner::initializeCdrComponents() {
    // Check if Docker daemon is running
    if (!dockerManager->isDaemonRunning()) {
        throw Docker::DaemonException("Docker daemon is not running. Please start Docker service.");
    }
    
    // For simplicity, we'll use a basic image that's commonly available
    // In a real CDR implementation, you would use a specialized CDR image
    if (!dockerManager->imageExists(cdrImageName.toStdString())) {
        qInfo() << "CDR base image" << cdrImageName << "not found locally. Attempting to pull...";
        try {
            dockerManager->pullImage(cdrImageName.toStdString());
            qInfo() << "Successfully pulled CDR base image:" << cdrImageName;
        } catch (const Docker::ImageNotFoundException& e) {
            qWarning() << "Failed to pull CDR image:" << cdrImageName << "Error:" << e.what();
            qInfo() << "Will attempt to create container anyway with existing images.";
        }
    }
    
    qInfo() << "CDRScanner components initialized successfully.";
}

// IScanner interface methods
bool CDRScanner::scanFile(const QString& filePath) {
    if (!dockerManager || !cdrManager) {
        lastError = QStringLiteral("DockerManager or CdrManager not initialized.");
        currentStatus = ScanStatus::Error;
        return false;
    }
    if (!isContainerReady()) {
        lastError = QStringLiteral("CDR Docker container is not ready.");
        currentStatus = ScanStatus::Error;
        return false;
    }

    setFile(filePath);
    currentStatus = ScanStatus::Scanning;
    lastError.clear();
    sanitizedFilePath.clear();
    threatsDetected = false;
    wasSanitized = false;
    analysisDetails.clear();

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        lastError = QStringLiteral("File does not exist or is not a regular file: %1").arg(filePath);
        currentStatus = ScanStatus::Error;
        return false;
    }

    try {
        // 1. Ensure the CDR container is running
        if (!ensureCdrContainerRunning()) {
            lastError = QStringLiteral("Failed to ensure CDR container is running: %1").arg(lastError);
            currentStatus = ScanStatus::Error;
            return false;
        }

        // 2. Detect file type using proper CDR method
        CDR::FileType detectedType = CDR::CdrManager::detectFileTypeByContent(filePath.toStdString());
        qInfo() << "Detected file type:" << QString::fromStdString(CDR::getFileTypeName(detectedType));
        
        // 3. Check if file type is supported for CDR processing
        if (!CDR::CdrManager::isFileTypeSupported(filePath.toStdString())) {
            analysisDetails = QStringLiteral("File type not supported for CDR processing");
            threatsDetected = false;
            wasSanitized = false;
            
            // Copy file as-is to output directory with _verified suffix
            QFileInfo originalFileInfo(filePath);
            QString outputBaseName = originalFileInfo.baseName();
            QString outputExtension = originalFileInfo.suffix();
            QString outputDir = QString::fromStdString(cdrConfig.outputDirectory);
              // Ensure output directory exists
            QDir().mkpath(outputDir);
            
            sanitizedFilePath = outputDir + QStringLiteral("/") + outputBaseName + QStringLiteral("_verified.") + outputExtension;
            
            if (QFile::copy(filePath, sanitizedFilePath)) {
                qInfo() << "Unsupported file type copied without modification to:" << sanitizedFilePath;
                currentStatus = ScanStatus::Completed;
                return true;
            } else {
                lastError = QStringLiteral("Failed to copy unsupported file to output directory");
                currentStatus = ScanStatus::Error;
                return false;
            }
        }

        // 4. First scan for active content and threats (CRITICAL: Detection before sanitization)
        std::vector<std::string> activeContentTypes = cdrManager->detectActiveContent(filePath.toStdString());
        
        bool hasActiveContent = !activeContentTypes.empty();
        bool requiresSanitization = false;
        QString threatDetails;
        
        // Analyze detected active content
        for (const auto& contentType : activeContentTypes) {            if (contentType.find("macro") != std::string::npos ||
                contentType.find("script") != std::string::npos ||
                contentType.find("executable") != std::string::npos ||
                contentType.find("embedded") != std::string::npos) {
                requiresSanitization = true;
                threatDetails += QString::fromStdString(contentType) + QStringLiteral("; ");
            }
        }
        
        // Update threat detection status
        threatsDetected = hasActiveContent;
        if (hasActiveContent) {
            analysisDetails = QStringLiteral("Active content detected: ") + threatDetails;
            qInfo() << "Threats detected in file:" << filePath << "Details:" << threatDetails;
        } else {
            analysisDetails = QStringLiteral("No active content or threats detected");
            qInfo() << "File analysis complete - no threats detected:" << filePath;
        }

        // 5. Determine output path in configured directory (FIXED: Use proper output location)
        QFileInfo originalFileInfo(filePath);
        QString outputBaseName = originalFileInfo.baseName();
        QString outputExtension = originalFileInfo.suffix();
        QString outputDir = QString::fromStdString(cdrConfig.outputDirectory);
        
        // Ensure output directory exists
        QDir().mkpath(outputDir);
        
        // 6. Process file based on threat analysis
        CDR::SanitizationResult result;
          if (requiresSanitization) {
            // File has threats - perform sanitization
            wasSanitized = true;
            sanitizedFilePath = outputDir + QStringLiteral("/") + outputBaseName + QStringLiteral("_sanitized.") + outputExtension;            qInfo() << "Performing sanitization for file with threats:" << filePath;
            
            // Call appropriate sanitization method based on detected file type
            switch (detectedType) {
                case CDR::FileType::OFFICE_DOCUMENT:
                    result = cdrManager->sanitizeOfficeDocument(filePath.toStdString(), sanitizedFilePath.toStdString(), cdrConfig);
                    break;
                case CDR::FileType::PDF_DOCUMENT:
                    result = cdrManager->sanitizePdfDocument(filePath.toStdString(), sanitizedFilePath.toStdString(), cdrConfig);
                    break;
                case CDR::FileType::HTML_DOCUMENT:
                    result = cdrManager->sanitizeHtmlDocument(filePath.toStdString(), sanitizedFilePath.toStdString(), cdrConfig);
                    break;
                case CDR::FileType::ARCHIVE_FILE:
                    result = cdrManager->sanitizeArchiveFile(filePath.toStdString(), sanitizedFilePath.toStdString(), cdrConfig);
                    break;
                case CDR::FileType::SCRIPT_FILE:
                    result = cdrManager->sanitizeScriptFile(filePath.toStdString(), sanitizedFilePath.toStdString(), cdrConfig);
                    break;
                default:
                    // For other file types, use a generic approach or create a fallback result
                    result.success = false;
                    result.errorMessage = "File type not supported for sanitization: " + CDR::getFileTypeName(detectedType);
                    result.requiresQuarantine = true;
                    result.quarantineReason = "Unsupported file type for CDR processing";
                    break;
            }
            
            if (result.success && !result.requiresQuarantine) {
                analysisDetails += QStringLiteral(" - File successfully sanitized");
                qInfo() << "File sanitized successfully:" << sanitizedFilePath;
            } else if (result.requiresQuarantine) {
                // File requires quarantine - move to quarantine directory
                QString quarantineDir = QString::fromStdString(cdrConfig.quarantineDirectory);
                QDir().mkpath(quarantineDir);
                  sanitizedFilePath = quarantineDir + QStringLiteral("/") + outputBaseName + QStringLiteral("_quarantined.") + outputExtension;
                
                if (cdrManager->quarantineFile(filePath.toStdString(), sanitizedFilePath.toStdString())) {
                    analysisDetails += QStringLiteral(" - File quarantined: ") + QString::fromStdString(result.quarantineReason);
                    qWarning() << "File quarantined due to high risk:" << sanitizedFilePath;
                } else {
                    lastError = QStringLiteral("Failed to quarantine high-risk file");
                    currentStatus = ScanStatus::Error;
                    return false;
                }
            } else {
                lastError = QStringLiteral("Sanitization failed: %1").arg(QString::fromStdString(result.errorMessage));
                currentStatus = ScanStatus::Error;
                return false;
            }        } else {
            // File is clean - copy as verified
            wasSanitized = false;
            sanitizedFilePath = outputDir + QStringLiteral("/") + outputBaseName + QStringLiteral("_verified.") + outputExtension;
            
            if (QFile::copy(filePath, sanitizedFilePath)) {
                analysisDetails += QStringLiteral(" - File verified as safe (no sanitization needed)");
                qInfo() << "Clean file copied without modification:" << sanitizedFilePath;
            } else {
                lastError = QStringLiteral("Failed to copy clean file to output directory");
                currentStatus = ScanStatus::Error;
                return false;
            }
        }

        currentStatus = ScanStatus::Completed;
        return true;

    } catch (const CDR::CdrBaseException& e) {
        lastError = QStringLiteral("CDR processing error: %1").arg(e.what());
    } catch (const Docker::ContainerNotFoundException& e) {
        lastError = QStringLiteral("CDR container not found: %1").arg(e.what());
    } catch (const Docker::OperationException& e) {
        lastError = QStringLiteral("Docker operation failed: %1").arg(e.what());
    } catch (const std::exception& e) {
        lastError = QStringLiteral("Unexpected error during CDR processing: %1").arg(e.what());
    }

    currentStatus = ScanStatus::Error;
    qWarning() << "CDR Scan failed for" << filePath << ":" << lastError;
    return false;
}

QString CDRScanner::getLastError() const {
    return lastError;
}

ScannerType CDRScanner::getType() const {
    return ScannerType::CDR;
}

ScanStatus CDRScanner::getStatus() const {
    return currentStatus;
}

void CDRScanner::setFile(const QString& filePath) {
    currentFilePath = filePath;
}

QString CDRScanner::getFile() const {
    return currentFilePath;
}

// IDockerScanner interface methods
bool CDRScanner::submitToContainer(const QString& containerName) {
    // This method might be used if the submission process is more generic
    // For CDR, scanFile is more specific. We can adapt this if needed.
    if (!dockerManager) return false;
    qInfo() << "Submitting file to container:" << containerName; // Placeholder
    // Actual implementation would involve dockerManager->copyFileToContainer or similar
    // For now, let's consider scanFile as the primary way to interact for CDR.
    lastError = QStringLiteral("submitToContainer not fully implemented for CDRScanner; use scanFile.");
    return false; 
}

bool CDRScanner::isContainerReady() const {
    if (!dockerManager) return false;
    try {
        // Check if Docker daemon is running
        if (!dockerManager->isDaemonRunning()) {
            return false;
        }
        
        // Check if the specific CDR container is running
        return isCdrContainerRunning();

    } catch (const Docker::DaemonException& e) {
        qWarning() << "Docker daemon error in isContainerReady:" << e.what();
        return false;
    } catch (const std::exception& e) {
        qWarning() << "Error checking container readiness:" << e.what();
        return false;
    }
}

QString CDRScanner::getContainerStatus() const {
    if (!dockerManager) return QStringLiteral("DockerManager not initialized");
    
    try {
        if (!dockerManager->isDaemonRunning()) {
            return QStringLiteral("Docker daemon is not running");
        }
        
        // Get detailed status of the CDR container
        auto containers = dockerManager->listContainers(true); // Include all containers
        
        for (const auto& container : containers) {
            if (container.containerName == cdrContainerName.toStdString()) {
                return QStringLiteral("CDR Container '%1': %2")
                       .arg(cdrContainerName)
                       .arg(QString::fromStdString(container.containerStatus));
            }
        }
        
        return QStringLiteral("CDR Container '%1' not found").arg(cdrContainerName);
        
    } catch (const Docker::DockerException& e) {
        return QStringLiteral("Error checking container status: %1").arg(e.what());
    } catch (const std::exception& e) {
        return QStringLiteral("Unexpected error: %1").arg(e.what());
    }
}

// CDRScanner specific methods
QString CDRScanner::getSanitizedFilePath() const {
    return sanitizedFilePath;
}

// IScanner interface methods implementation
bool CDRScanner::selectFile() {
    // This could open a file dialog, but for now we'll return false
    // indicating this functionality should be handled at the UI level
    return false;
}

QFileInfo CDRScanner::getSelectedFile() const {
    return QFileInfo(currentFilePath);
}

QString CDRScanner::getResults() const {
    if (currentStatus == ScanStatus::Completed && !sanitizedFilePath.isEmpty()) {
        QFileInfo originalFile(currentFilePath);
        QFileInfo processedFile(sanitizedFilePath);
        
        QString result = QStringLiteral("🔍 CDR Analysis Completed\n\n");
        result += QStringLiteral("📄 Original file: %1 (%2 bytes)\n").arg(originalFile.fileName()).arg(originalFile.size());
        result += QStringLiteral("📊 Analysis: %1\n").arg(analysisDetails);
        
        if (processedFile.exists()) {
            result += QStringLiteral("✅ Processed file: %1 (%2 bytes)\n").arg(processedFile.fileName()).arg(processedFile.size());            result += QStringLiteral("📁 Output location: %1\n\n").arg(processedFile.absoluteFilePath());
            
            if (sanitizedFilePath.contains(QStringLiteral("_quarantined"))) {
                result += QStringLiteral("⚠️  Status: File QUARANTINED (high-risk threats detected)\n");
                result += QStringLiteral("🔒 Action: File isolated for security - DO NOT USE\n");
                result += QStringLiteral("🛡️  Threats: Detected and contained\n");
            } else if (wasSanitized) {
                result += QStringLiteral("🛡️  Status: File SANITIZED (threats detected and neutralized)\n");
                result += QStringLiteral("✅ Action: Malicious content removed/cleaned\n");
                result += QStringLiteral("⚠️  Threats: Detected and successfully handled\n");
            } else {
                result += QStringLiteral("✅ Status: File VERIFIED as safe (no threats detected)\n");
                result += QStringLiteral("🔒 Action: File confirmed clean, no modifications needed\n");
                result += QStringLiteral("✅ Threats: None detected\n");
            }
        } else {
            result += QStringLiteral("❌ Warning: Processed file not found at expected location\n");
        }
        
        return result;
    } else if (currentStatus == ScanStatus::Error) {
        return QStringLiteral("❌ CDR Processing failed: %1").arg(lastError);
    } else if (currentStatus == ScanStatus::Scanning) {
        return QStringLiteral("🔍 CDR Analysis in progress...\n🔎 Detecting active content and threats...\n⏳ Please wait...");
    } else {
        return QStringLiteral("🛡️  CDR Scanner ready\n📁 Select a file to analyze for threats and active content");
    }
}

bool CDRScanner::isScanning() const {
    return currentStatus == ScanStatus::Scanning;
}

bool CDRScanner::cancelScan() {
    if (currentStatus == ScanStatus::Scanning) {
        currentStatus = ScanStatus::Cancelled;
        lastError = QStringLiteral("Scan cancelled by user");
        return true;
    }
    return false;
}

// Helper methods for CDR container management
bool CDRScanner::ensureCdrContainerRunning() {
    if (!dockerManager) {
        lastError = QStringLiteral("DockerManager not initialized.");
        return false;
    }
    
    try {
        // First check if container exists and is running
        if (isCdrContainerRunning()) {
            return true;
        }
        
        // Check if container exists but is stopped
        auto containers = dockerManager->listContainers(true); // Include all containers
        bool containerExists = false;
        
        for (const auto& container : containers) {
            if (container.containerName == cdrContainerName.toStdString()) {
                containerExists = true;
                if (container.containerStatus.find("Up") == std::string::npos) {
                    // Container exists but is not running, start it
                    qInfo() << "Starting existing CDR container:" << cdrContainerName;
                    dockerManager->startContainer(cdrContainerName.toStdString());
                    return true;
                }
                break;
            }
        }
        
        // If container doesn't exist, create and run it
        if (!containerExists) {
            return createCdrContainer();
        }
        
        return true;
        
    } catch (const Docker::DockerException& e) {
        lastError = QStringLiteral("Failed to ensure CDR container is running: %1").arg(e.what());
        qWarning() << lastError;
        return false;
    } catch (const std::exception& e) {
        lastError = QStringLiteral("Unexpected error ensuring CDR container: %1").arg(e.what());
        qWarning() << lastError;
        return false;
    }
}

bool CDRScanner::createCdrContainer() {
    if (!dockerManager) {
        lastError = QStringLiteral("DockerManager not initialized.");
        return false;
    }
    
    try {
        qInfo() << "Creating new CDR container:" << cdrContainerName;
        
        // Configure the container
        Docker::ContainerRunConfiguration config;
        config.imageName = cdrImageName.toStdString();
        config.containerName = cdrContainerName.toStdString();
        
        // Add volumes for input and output
        Docker::MountPoint inputMount;
        inputMount.mountType = "bind";
        inputMount.source = "/tmp/cdr_input";
        inputMount.destination = "/input";
        inputMount.readOnly = false;
        
        Docker::MountPoint outputMount;
        outputMount.mountType = "bind";
        outputMount.source = "/tmp/cdr_output"; 
        outputMount.destination = "/output";
        outputMount.readOnly = false;
        
        config.hostConfig.mounts = {inputMount, outputMount};
        
        // Keep container running with a long-running command
        config.command = {"sleep", "infinity"};
        
        // Run the container
        auto result = dockerManager->runNewContainer(config);
        
        if (result.status == "Running" || result.status == "Exited") {
            qInfo() << "CDR container created successfully with ID:" << QString::fromStdString(result.containerId);
            
            // Create the input/output directories on host if they don't exist
            QDir().mkpath(QStringLiteral("/tmp/cdr_input"));
            QDir().mkpath(QStringLiteral("/tmp/cdr_output"));
            
            return true;
        } else {
            lastError = QStringLiteral("Failed to create CDR container. Status: %1, Error: %2")
                        .arg(QString::fromStdString(result.status))
                        .arg(QString::fromStdString(result.errorMessage));
            qWarning() << lastError;
            return false;
        }
        
    } catch (const Docker::DockerException& e) {
        lastError = QStringLiteral("Failed to create CDR container: %1").arg(e.what());
        qWarning() << lastError;
        return false;
    } catch (const std::exception& e) {
        lastError = QStringLiteral("Unexpected error creating CDR container: %1").arg(e.what());
        qWarning() << lastError;
        return false;
    }
}

bool CDRScanner::isCdrContainerRunning() const {
    if (!dockerManager) {
        return false;
    }
    
    try {
        auto containers = dockerManager->listContainers(false); // Only running containers
        
        for (const auto& container : containers) {
            if (container.containerName == cdrContainerName.toStdString()) {
                return container.containerStatus.find("Up") != std::string::npos;
            }
        }
        
        return false;
        
    } catch (const Docker::DockerException& e) {
        qWarning() << "Failed to check CDR container status:" << e.what();
        return false;
    } catch (const std::exception& e) {
        qWarning() << "Unexpected error checking CDR container status:" << e.what();
        return false;
    }
}

