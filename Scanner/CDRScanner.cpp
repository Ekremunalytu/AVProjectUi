// Scanner/CDRScanner.cpp
#include "CDRScanner.h"
#include "Interface/ScannerTypes.h"
#include "Docker/include/docker/DockerTypes.h" // For Docker types if needed
#include "Docker/include/docker/DockerExceptions.h" // For Docker exceptions
// Potentially include headers from Docker/src/cdr/ if CdrManager or CdrSanitizer are used directly
// #include "Docker/src/cdr/CdrManager.h" 
// #include "Docker/src/cdr/CdrSanitizer.h"

#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QThread>

CDRScanner::CDRScanner() : currentStatus(ScanStatus::Idle), 
                           cdrContainerName(QStringLiteral("cdr_processor_container")),
                           cdrImageName(QStringLiteral("ubuntu:latest")) {
    try {
        dockerManager = std::make_unique<Docker::DockerManager>();
        // Initialize CDR specific components if necessary
        // cdrManager = std::make_unique<Cdr::CdrManager>();
        // cdrSanitizer = std::make_unique<Cdr::CdrSanitizer>();
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
    if (!dockerManager) {
        lastError = QStringLiteral("DockerManager not initialized.");
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

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        lastError = QStringLiteral("File does not exist or is not a regular file: %1").arg(filePath);
        currentStatus = ScanStatus::Error;
        return false;
    }

    // Define the CDR container name/ID (this might come from config)
    // For now, let's assume a fixed name, e.g., "cdr_processor_container"
    // This container should be pre-configured to perform CDR on files copied to a specific input volume
    // and place sanitized files in an output volume.
    const std::string cdrContainerName = this->cdrContainerName.toStdString(); // Use class member
    const std::string inputPathInContainer = "/input/" + fileInfo.fileName().toStdString();
    const std::string outputPathInContainer = "/output/" + fileInfo.fileName().toStdString(); // Expected sanitized file path

    try {
        // 1. Ensure the CDR container is running
        if (!ensureCdrContainerRunning()) {
            lastError = QStringLiteral("Failed to ensure CDR container is running: %1").arg(lastError);
            currentStatus = ScanStatus::Error;
            return false;
        }

        // 2. Copy the file to the container's input volume
        dockerManager->copyFileToContainer(cdrContainerName, filePath.toStdString(), inputPathInContainer);
        qInfo() << "File" << filePath << "copied to container" << QString::fromStdString(cdrContainerName) << "at" << QString::fromStdString(inputPathInContainer);

        // 3. Trigger CDR processing in the container
        // Example CDR command - this would depend on your actual CDR tool
        std::vector<std::string> cdrCommand = {
            "sh", "-c", 
            "cd /input && cp " + fileInfo.fileName().toStdString() + " /output/" + fileInfo.fileName().toStdString() + ".sanitized"
        };
        
        auto commandOutput = dockerManager->executeCommandInContainer(cdrContainerName, cdrCommand);
        qInfo() << "CDR processing command executed. Output:" << QString::fromStdString(commandOutput);

        // 4. Wait a bit for processing to complete (in a real implementation, you'd poll for completion)
        QThread::msleep(2000); // Wait 2 seconds - replace with proper polling

        // 5. Copy the sanitized file back from the container's output volume
        QString localSanitizedDir = QStringLiteral("./sanitized_files/"); // Should be configurable
        QDir().mkpath(localSanitizedDir);
        sanitizedFilePath = localSanitizedDir + fileInfo.fileName() + QStringLiteral(".sanitized");
        
        QString sanitizedOutputPath = QStringLiteral("/output/") + fileInfo.fileName() + QStringLiteral(".sanitized");
        dockerManager->copyFileFromContainer(cdrContainerName, sanitizedOutputPath.toStdString(), sanitizedFilePath.toStdString());
        qInfo() << "Sanitized file copied from container to" << sanitizedFilePath;

        // 6. (Optional) Clean up the input file in the container if needed
        // dockerManager->executeCommandInContainer(cdrContainerName, {"rm", inputPathInContainer});

        currentStatus = ScanStatus::Completed;
        return true;

    } catch (const Docker::ContainerNotFoundException& e) {
        lastError = QStringLiteral("CDR container '%1' not found: %2").arg(QString::fromStdString(cdrContainerName)).arg(e.what());
    } catch (const Docker::OperationException& e) {
        lastError = QStringLiteral("Operation failed on CDR container '%1': %2").arg(QString::fromStdString(cdrContainerName)).arg(e.what());
    } catch (const Docker::CommandFailureException& e) {
        lastError = QStringLiteral("Docker command failed for CDR: %1").arg(e.what());
    } catch (const std::exception& e) {
        lastError = QStringLiteral("An unexpected error occurred during CDR processing: %1").arg(e.what());
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
        return QStringLiteral("CDR Processing completed. Sanitized file: %1").arg(sanitizedFilePath);
    } else if (currentStatus == ScanStatus::Error) {
        return QStringLiteral("CDR Processing failed: %1").arg(lastError);
    } else if (currentStatus == ScanStatus::Scanning) {
        return QStringLiteral("CDR Processing in progress...");
    } else {
        return QStringLiteral("CDR Scanner ready");
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

