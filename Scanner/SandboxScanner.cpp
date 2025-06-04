#include "SandboxScanner.h"
#include "Sandbox/SandboxManager.h"
#include "Sandbox/SandboxTypes.h"
#include <QFileDialog>
#include <QDebug>
#include <QUuid>
#include <QStandardPaths>
#include <QDir>

// Static constants
const QString SandboxScanner::SANDBOX_IMAGE_NAME = "avproject-sandbox:latest";
const int SandboxScanner::DEFAULT_TIMEOUT_SECONDS = 300; // 5 minutes
const int SandboxScanner::PROGRESS_CHECK_INTERVAL_MS = 5000; // 5 seconds

SandboxScanner::SandboxScanner()
    : m_dockerManager(std::make_unique<Docker::DockerManager>())
    , m_sandboxManager(std::make_unique<Sandbox::SandboxManager>())
    , m_status(ScanStatus::IDLE)
    , m_isScanning(false)
    , m_sandboxTimeout(DEFAULT_TIMEOUT_SECONDS)
    , m_monitoringLevel(1) // Standard monitoring by default
    , m_timeoutTimer(new QTimer(this))
    , m_progressTimer(new QTimer(this))
{
    // Setup timers
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &SandboxScanner::onSandboxTimeout);
    
    m_progressTimer->setSingleShot(false);
    connect(m_progressTimer, &QTimer::timeout, this, &SandboxScanner::checkSandboxProgress);
    
    // Initialize sandbox environment
    initializeSandboxEnvironment();
}

SandboxScanner::~SandboxScanner() {
    if (m_isScanning) {
        cancelScan();
    }
    cleanupSandboxEnvironment();
}

bool SandboxScanner::selectFile() {
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        tr("Select File for Sandbox Analysis"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("Executable Files (*.exe *.bat *.cmd *.com *.scr *.pif);;All Files (*)")
    );
    
    if (!filePath.isEmpty()) {
        setFile(filePath);
        return true;
    }
    return false;
}

bool SandboxScanner::scanFile(const QString& filePath) {
    if (m_isScanning) {
        m_lastError = tr("Sandbox analysis already in progress");
        return false;
    }
    
    if (filePath.isEmpty()) {
        m_lastError = tr("No file selected for analysis");
        return false;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        m_lastError = tr("Selected file does not exist or is not a regular file");
        return false;
    }
    
    if (!isValidExecutableFile(filePath)) {
        m_lastError = tr("Selected file is not a valid executable for sandbox analysis");
        return false;
    }
    
    m_selectedFile = fileInfo;
    m_results.clear();
    m_lastError.clear();
    m_analysisReport.clear();
    m_behaviorSummary.clear();
    
    updateScanStatus(ScanStatus::SCANNING);
    m_isScanning = true;
    
    appendToResults(tr("Starting sandbox analysis of: %1").arg(fileInfo.fileName()));
    appendToResults(tr("File size: %1 bytes").arg(fileInfo.size()));
    appendToResults(tr("File path: %1").arg(filePath));
    
    emit scanStarted();
    
    // Start the analysis process
    if (!prepareSandboxContainer()) {
        m_lastError = tr("Failed to prepare sandbox container");
        updateScanStatus(ScanStatus::ERROR);
        m_isScanning = false;
        emit scanError(m_lastError);
        return false;
    }
    
    if (!executeFileInSandbox(filePath)) {
        m_lastError = tr("Failed to execute file in sandbox");
        updateScanStatus(ScanStatus::ERROR);
        m_isScanning = false;
        cleanupSandboxEnvironment();
        emit scanError(m_lastError);
        return false;
    }
    
    // Start monitoring
    m_timeoutTimer->start(m_sandboxTimeout * 1000);
    m_progressTimer->start(PROGRESS_CHECK_INTERVAL_MS);
    
    appendToResults(tr("File submitted to sandbox for analysis..."));
    appendToResults(tr("Monitoring timeout: %1 seconds").arg(m_sandboxTimeout));
    
    return true;
}

QFileInfo SandboxScanner::getSelectedFile() const {
    return m_selectedFile;
}

QString SandboxScanner::getResults() const {
    return m_results;
}

bool SandboxScanner::isScanning() const {
    return m_isScanning;
}

bool SandboxScanner::cancelScan() {
    if (!m_isScanning) {
        return true;
    }
    
    m_timeoutTimer->stop();
    m_progressTimer->stop();
    
    try {
        if (!m_containerId.isEmpty() && m_sandboxManager) {
            m_sandboxManager->terminateSandbox(m_containerId.toStdString(), true);
        }
    } catch (const std::exception& e) {
        qWarning() << "Error during sandbox cleanup:" << e.what();
    }
    
    cleanupSandboxEnvironment();
    updateScanStatus(ScanStatus::CANCELLED);
    m_isScanning = false;
    
    appendToResults(tr("Sandbox analysis cancelled by user"));
    
    return true;
}

QString SandboxScanner::getLastError() const {
    return m_lastError;
}

ScannerType SandboxScanner::getType() const {
    return ScannerType::SANDBOX_SCANNER;
}

ScanStatus SandboxScanner::getStatus() const {
    return m_status;
}

void SandboxScanner::setFile(const QString& filePath) {
    if (!m_isScanning) {
        m_selectedFile = QFileInfo(filePath);
    }
}

QString SandboxScanner::getFile() const {
    return m_selectedFile.absoluteFilePath();
}

bool SandboxScanner::submitToContainer(const QString& containerName) {
    if (m_selectedFile.absoluteFilePath().isEmpty()) {
        m_lastError = tr("No file selected for submission");
        return false;
    }
    
    return scanFile(m_selectedFile.absoluteFilePath());
}

bool SandboxScanner::isContainerReady() const {
    if (m_containerId.isEmpty() || !m_sandboxManager) {
        return false;
    }
    
    try {
        return m_sandboxManager->isSandboxReady(m_containerId.toStdString());
    } catch (const std::exception& e) {
        qWarning() << "Error checking container status:" << e.what();
        return false;
    }
}

QString SandboxScanner::getContainerStatus() const {
    if (m_containerId.isEmpty() || !m_sandboxManager) {
        return tr("No container");
    }
    
    try {
        auto sandboxStatus = m_sandboxManager->getSandboxStatus(m_containerId.toStdString());
        return QString::fromStdString(sandboxStatus.status);
    } catch (const std::exception& e) {
        qWarning() << "Error getting container status:" << e.what();
        return tr("Error");
    }
}

void SandboxScanner::setSandboxTimeout(int seconds) {
    if (seconds > 0 && seconds <= 3600) { // Maximum 1 hour
        m_sandboxTimeout = seconds;
    }
}

void SandboxScanner::setMonitoringLevel(int level) {
    if (level >= 0 && level <= 2) {
        m_monitoringLevel = level;
    }
}

QString SandboxScanner::getAnalysisReport() const {
    return m_analysisReport;
}

QString SandboxScanner::getBehaviorSummary() const {
    return m_behaviorSummary;
}

void SandboxScanner::onSandboxTimeout() {
    appendToResults(tr("Sandbox analysis timeout reached"));
    
    // Perform final analysis
    if (monitorSandboxExecution()) {
        generateThreatReport();
    }
    
    // Cleanup and finish
    cleanupSandboxEnvironment();
    updateScanStatus(ScanStatus::COMPLETED);
    m_isScanning = false;
    m_progressTimer->stop();
    
    // Emit completion signal based on analysis results
    bool hasMaliciousBehavior = m_analysisReport.contains("MALICIOUS", Qt::CaseInsensitive) ||
                               m_analysisReport.contains("THREAT", Qt::CaseInsensitive) ||
                               m_analysisReport.contains("SUSPICIOUS", Qt::CaseInsensitive);
    
    emit scanCompleted(hasMaliciousBehavior);
}

void SandboxScanner::checkSandboxProgress() {
    if (!isContainerReady()) {
        // Container might have finished or crashed
        onSandboxTimeout();
        return;
    }
    
    // Update progress and perform intermediate monitoring
    static int progressCounter = 0;
    progressCounter++;
    
    int totalChecks = m_sandboxTimeout / (PROGRESS_CHECK_INTERVAL_MS / 1000);
    int progress = (progressCounter * 100) / totalChecks;
    progress = qMin(progress, 95); // Cap at 95% until final analysis
    
    emit scanProgress(progress);
    
    // Perform intermediate monitoring
    monitorSandboxExecution();
    
    appendToResults(tr("Monitoring progress: %1%").arg(progress));
    
    // Check for early termination conditions
    if (m_analysisReport.contains("CRITICAL_THREAT", Qt::CaseInsensitive)) {
        appendToResults(tr("Critical threat detected - terminating analysis early"));
        onSandboxTimeout();
    }
}

bool SandboxScanner::initializeSandboxEnvironment() {
    try {
        if (!m_dockerManager->isDaemonRunning()) {
            m_lastError = tr("Docker daemon is not running");
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        m_lastError = tr("Failed to initialize sandbox environment: %1").arg(e.what());
        return false;
    }
}

bool SandboxScanner::prepareSandboxContainer() {
    try {
        m_containerName = generateContainerName();
        
        Sandbox::SandboxConfiguration config;
        config.containerName = m_containerName.toStdString();
        config.imageName = SANDBOX_IMAGE_NAME.toStdString();
        config.timeoutSeconds = m_sandboxTimeout;
        config.monitoringLevel = static_cast<Sandbox::MonitoringLevel>(m_monitoringLevel);
        config.networkIsolation = true;
        config.fileSystemIsolation = true;
        config.recordScreenshots = (m_monitoringLevel >= 2);
        config.memoryLimitMB = 512; // 512MB memory limit
        config.cpuLimit = 0.5; // 50% CPU limit
        config.processLimit = 64;
        
        // Validate configuration before creating container
        if (!m_sandboxManager->validateSandboxConfiguration(config)) {
            m_lastError = tr("Invalid sandbox configuration");
            return false;
        }
        
        std::string containerId = m_sandboxManager->createSandboxEnvironment(config);
        m_containerId = QString::fromStdString(containerId);
        
        appendToResults(tr("Sandbox container created: %1").arg(m_containerName));
        appendToResults(tr("Container ID: %1").arg(m_containerId.left(12)));
        appendToResults(tr("Monitoring level: %1").arg(m_monitoringLevel));
        
        // Wait a moment for container to be fully ready
        QThread::msleep(2000);
        
        if (!isContainerReady()) {
            m_lastError = tr("Sandbox container failed to start properly");
            return false;
        }
        
        return !m_containerId.isEmpty();
    } catch (const std::exception& e) {
        m_lastError = tr("Failed to prepare sandbox container: %1").arg(e.what());
        return false;
    }
}

bool SandboxScanner::executeFileInSandbox(const QString& filePath) {
    try {
        if (m_containerId.isEmpty()) {
            return false;
        }
        
        // Copy file to container
        std::string containerPath = "/sandbox/target" + 
            QFileInfo(filePath).suffix().prepend(".").toStdString();
        
        m_sandboxManager->copyFileToSandbox(m_containerId.toStdString(), 
                                          filePath.toStdString(), 
                                          containerPath);
        
        // Start execution monitoring
        Sandbox::ExecutionConfig execConfig;
        execConfig.targetFile = containerPath;
        execConfig.monitorNetwork = true;
        execConfig.monitorFileSystem = true;
        execConfig.monitorProcesses = true;
        execConfig.recordScreenshots = (m_monitoringLevel >= 2);
        execConfig.executionTimeoutSeconds = m_sandboxTimeout;
        
        bool started = m_sandboxManager->executeFileInSandbox(m_containerId.toStdString(), execConfig);
        
        if (started) {
            appendToResults(tr("File execution started in sandbox"));
            appendToResults(tr("Monitoring network activity, file changes, and process behavior"));
        }
        
        return started;
    } catch (const std::exception& e) {
        m_lastError = tr("Failed to execute file in sandbox: %1").arg(e.what());
        return false;
    }
}

bool SandboxScanner::monitorSandboxExecution() {
    try {
        if (m_containerId.isEmpty()) {
            return false;
        }
        
        // Get current analysis results
        auto analysisResult = m_sandboxManager->getSandboxAnalysisResults(m_containerId.toStdString());
        
        // Check for new behaviors
        for (const auto& behavior : analysisResult.detectedBehaviors) {
            QString behaviorStr = QString::fromStdString(behavior);
            if (!m_behaviorSummary.contains(behaviorStr)) {
                m_behaviorSummary += behaviorStr + "\n";
                emit behaviourDetected(behaviorStr);
            }
        }
        
        // Update analysis report
        m_analysisReport = QString::fromStdString(analysisResult.report);
        
        // Analyze specific aspects
        analyzeNetworkActivity();
        analyzeFileSystemChanges();
        analyzeSystemCalls();
        analyzeProcessBehavior();
        
        return true;
    } catch (const std::exception& e) {
        qWarning() << "Error monitoring sandbox execution:" << e.what();
        return false;
    }
}

void SandboxScanner::cleanupSandboxEnvironment() {
    try {
        if (!m_containerId.isEmpty() && m_sandboxManager) {
            // Terminate sandbox environment
            m_sandboxManager->terminateSandbox(m_containerId.toStdString(), true);
            
            appendToResults(tr("Sandbox environment cleaned up"));
        }
    } catch (const std::exception& e) {
        qWarning() << "Error during sandbox cleanup:" << e.what();
    }
    
    m_containerId.clear();
    m_containerName.clear();
}

bool SandboxScanner::analyzeNetworkActivity() {
    try {
        if (m_containerId.isEmpty() || !m_sandboxManager) {
            return false;
        }

        appendToResults(tr("Analyzing network connections and DNS queries..."));
        
        // Get network monitoring data from sandbox
        auto networkData = m_sandboxManager->getNetworkActivity(m_containerId.toStdString());
        
        bool suspiciousActivity = false;
        
        // Analyze outbound connections
        if (!networkData.outboundConnections.empty()) {
            appendToResults(tr("=== Outbound Network Connections ==="));
            for (const auto& connection : networkData.outboundConnections) {
                QString connInfo = tr("- %1:%2 (%3)").arg(QString::fromStdString(connection.destinationIP))
                                                     .arg(connection.destinationPort)
                                                     .arg(QString::fromStdString(connection.protocol));
                appendToResults(connInfo);
                
                // Check for suspicious IPs or ports
                if (connection.destinationPort == 4444 || connection.destinationPort == 6666 || 
                    connection.destinationPort == 1337 || connection.destinationPort == 31337) {
                    appendToResults(tr("  ⚠️  Suspicious port detected: %1").arg(connection.destinationPort));
                    suspiciousActivity = true;
                }
            }
        }
        
        // Analyze DNS queries
        if (!networkData.dnsQueries.empty()) {
            appendToResults(tr("=== DNS Queries ==="));
            for (const auto& query : networkData.dnsQueries) {
                QString queryInfo = tr("- %1 -> %2").arg(QString::fromStdString(query.domain))
                                                   .arg(QString::fromStdString(query.resolvedIP));
                appendToResults(queryInfo);
                
                // Check for suspicious domains
                QString domain = QString::fromStdString(query.domain).toLower();
                if (domain.contains("malware") || domain.contains("botnet") || domain.contains(".tk") ||
                    domain.contains("pastebin") || domain.contains("bit.ly")) {
                    appendToResults(tr("  ⚠️  Suspicious domain detected: %1").arg(domain));
                    suspiciousActivity = true;
                }
            }
        }
        
        // Check for data exfiltration patterns
        if (networkData.totalBytesTransmitted > 1048576) { // 1MB threshold
            appendToResults(tr("⚠️  Large data transmission detected: %1 bytes")
                          .arg(networkData.totalBytesTransmitted));
            suspiciousActivity = true;
        }
        
        if (suspiciousActivity) {
            appendToResults(tr("❌ Suspicious network activity detected!"));
        } else {
            appendToResults(tr("✅ No suspicious network activity detected"));
        }
        
        return true;
    } catch (const std::exception& e) {
        appendToResults(tr("Error analyzing network activity: %1").arg(e.what()));
        return false;
    }
}

bool SandboxScanner::analyzeFileSystemChanges() {
    try {
        if (m_containerId.isEmpty() || !m_sandboxManager) {
            return false;
        }

        appendToResults(tr("Analyzing file system modifications..."));
        
        // Get file system monitoring data from sandbox
        auto fsData = m_sandboxManager->getFileSystemActivity(m_containerId.toStdString());
        
        bool suspiciousActivity = false;
        
        // Analyze file creations
        if (!fsData.filesCreated.empty()) {
            appendToResults(tr("=== Files Created ==="));
            for (const auto& file : fsData.filesCreated) {
                QString fileInfo = tr("- %1").arg(QString::fromStdString(file.path));
                appendToResults(fileInfo);
                
                // Check for suspicious file locations or extensions
                QString path = QString::fromStdString(file.path).toLower();
                if (path.contains("/tmp/") || path.contains("startup") || path.contains("autorun") ||
                    path.endsWith(".exe") || path.endsWith(".dll") || path.endsWith(".bat") ||
                    path.endsWith(".vbs") || path.endsWith(".ps1")) {
                    appendToResults(tr("  ⚠️  Suspicious file creation: %1").arg(path));
                    suspiciousActivity = true;
                }
            }
        }
        
        // Analyze file modifications
        if (!fsData.filesModified.empty()) {
            appendToResults(tr("=== Files Modified ==="));
            for (const auto& file : fsData.filesModified) {
                QString fileInfo = tr("- %1").arg(QString::fromStdString(file.path));
                appendToResults(fileInfo);
                
                // Check for system file modifications
                QString path = QString::fromStdString(file.path).toLower();
                if (path.contains("/etc/") || path.contains("/bin/") || path.contains("/usr/bin/") ||
                    path.contains("hosts") || path.contains("passwd") || path.contains("shadow")) {
                    appendToResults(tr("  ⚠️  System file modification: %1").arg(path));
                    suspiciousActivity = true;
                }
            }
        }
        
        // Analyze file deletions
        if (!fsData.filesDeleted.empty()) {
            appendToResults(tr("=== Files Deleted ==="));
            for (const auto& file : fsData.filesDeleted) {
                QString fileInfo = tr("- %1").arg(QString::fromStdString(file.path));
                appendToResults(fileInfo);
                
                // Check for log file deletions (evidence destruction)
                QString path = QString::fromStdString(file.path).toLower();
                if (path.contains("log") || path.contains("history") || path.contains("audit")) {
                    appendToResults(tr("  ⚠️  Log file deletion detected: %1").arg(path));
                    suspiciousActivity = true;
                }
            }
        }
        
        // Check for encryption/ransomware patterns
        if (fsData.filesCreated.size() > 100 && fsData.filesDeleted.size() > 100) {
            appendToResults(tr("⚠️  Mass file creation/deletion pattern detected (possible ransomware)"));
            suspiciousActivity = true;
        }
        
        if (suspiciousActivity) {
            appendToResults(tr("❌ Suspicious file system activity detected!"));
        } else {
            appendToResults(tr("✅ No suspicious file system activity detected"));
        }
        
        return true;
    } catch (const std::exception& e) {
        appendToResults(tr("Error analyzing file system changes: %1").arg(e.what()));
        return false;
    }
}

bool SandboxScanner::analyzeSystemCalls() {
    try {
        if (m_containerId.isEmpty() || !m_sandboxManager) {
            return false;
        }

        appendToResults(tr("Analyzing system call patterns..."));
        
        // Get system call monitoring data from sandbox
        auto syscallData = m_sandboxManager->getSystemCallActivity(m_containerId.toStdString());
        
        bool suspiciousActivity = false;
        
        // Analyze high-frequency system calls
        if (!syscallData.syscallFrequency.empty()) {
            appendToResults(tr("=== System Call Analysis ==="));
            
            for (const auto& [syscall, count] : syscallData.syscallFrequency) {
                if (count > 1000) { // High frequency threshold
                    QString syscallInfo = tr("- %1: %2 calls").arg(QString::fromStdString(syscall)).arg(count);
                    appendToResults(syscallInfo);
                    
                    // Check for suspicious system calls
                    if (syscall == "ptrace" || syscall == "fork" || syscall == "clone" ||
                        syscall == "execve" || syscall == "mmap" || syscall == "mprotect") {
                        appendToResults(tr("  ⚠️  Suspicious system call pattern: %1").arg(QString::fromStdString(syscall)));
                        suspiciousActivity = true;
                    }
                }
            }
        }
        
        // Analyze privilege escalation attempts
        if (!syscallData.privilegeEscalationAttempts.empty()) {
            appendToResults(tr("=== Privilege Escalation Attempts ==="));
            for (const auto& attempt : syscallData.privilegeEscalationAttempts) {
                QString attemptInfo = tr("- %1 at %2").arg(QString::fromStdString(attempt.syscall))
                                                     .arg(QString::fromStdString(attempt.timestamp));
                appendToResults(attemptInfo);
                suspiciousActivity = true;
            }
        }
        
        // Analyze anti-debugging attempts
        if (!syscallData.antiDebuggingAttempts.empty()) {
            appendToResults(tr("=== Anti-Debugging Attempts ==="));
            for (const auto& attempt : syscallData.antiDebuggingAttempts) {
                QString attemptInfo = tr("- %1").arg(QString::fromStdString(attempt.technique));
                appendToResults(attemptInfo);
                suspiciousActivity = true;
            }
        }
        
        // Check for code injection patterns
        if (syscallData.syscallFrequency.count("mmap") > 0 && 
            syscallData.syscallFrequency.count("mprotect") > 0 &&
            syscallData.syscallFrequency.count("write") > 100) {
            appendToResults(tr("⚠️  Code injection pattern detected (mmap + mprotect + write)"));
            suspiciousActivity = true;
        }
        
        if (suspiciousActivity) {
            appendToResults(tr("❌ Suspicious system call activity detected!"));
        } else {
            appendToResults(tr("✅ No suspicious system call activity detected"));
        }
        
        return true;
    } catch (const std::exception& e) {
        appendToResults(tr("Error analyzing system calls: %1").arg(e.what()));
        return false;
    }
}

bool SandboxScanner::analyzeProcessBehavior() {
    try {
        if (m_containerId.isEmpty() || !m_sandboxManager) {
            return false;
        }

        appendToResults(tr("Analyzing process creation and injection..."));
        
        // Get process monitoring data from sandbox
        auto processData = m_sandboxManager->getProcessActivity(m_containerId.toStdString());
        
        bool suspiciousActivity = false;
        
        // Analyze process spawning
        if (!processData.processesCreated.empty()) {
            appendToResults(tr("=== Processes Created ==="));
            for (const auto& process : processData.processesCreated) {
                QString processInfo = tr("- PID %1: %2").arg(process.pid)
                                                       .arg(QString::fromStdString(process.commandLine));
                appendToResults(processInfo);
                
                // Check for suspicious process names or commands
                QString command = QString::fromStdString(process.commandLine).toLower();
                if (command.contains("powershell") || command.contains("cmd.exe") || 
                    command.contains("bash") || command.contains("sh") ||
                    command.contains("wget") || command.contains("curl") ||
                    command.contains("nc") || command.contains("netcat")) {
                    appendToResults(tr("  ⚠️  Suspicious process execution: %1").arg(command));
                    suspiciousActivity = true;
                }
            }
        }
        
        // Analyze process injection attempts
        if (!processData.injectionAttempts.empty()) {
            appendToResults(tr("=== Process Injection Attempts ==="));
            for (const auto& injection : processData.injectionAttempts) {
                QString injectionInfo = tr("- Source PID %1 -> Target PID %2 (%3)")
                                      .arg(injection.sourcePid)
                                      .arg(injection.targetPid)
                                      .arg(QString::fromStdString(injection.technique));
                appendToResults(injectionInfo);
                suspiciousActivity = true;
            }
        }
        
        // Analyze process hollowing attempts
        if (!processData.hollowingAttempts.empty()) {
            appendToResults(tr("=== Process Hollowing Attempts ==="));
            for (const auto& hollowing : processData.hollowingAttempts) {
                QString hollowingInfo = tr("- Target: %1 (PID %2)")
                                      .arg(QString::fromStdString(hollowing.targetProcess))
                                      .arg(hollowing.targetPid);
                appendToResults(hollowingInfo);
                suspiciousActivity = true;
            }
        }
        
        // Analyze parent-child process relationships
        if (processData.processesCreated.size() > 10) {
            appendToResults(tr("⚠️  High process creation activity detected (%1 processes)")
                          .arg(processData.processesCreated.size()));
            suspiciousActivity = true;
        }
        
        // Check for persistence mechanisms
        if (!processData.persistenceMechanisms.empty()) {
            appendToResults(tr("=== Persistence Mechanisms ==="));
            for (const auto& mechanism : processData.persistenceMechanisms) {
                QString mechInfo = tr("- %1: %2").arg(QString::fromStdString(mechanism.type))
                                               .arg(QString::fromStdString(mechanism.details));
                appendToResults(mechInfo);
                suspiciousActivity = true;
            }
        }
        
        if (suspiciousActivity) {
            appendToResults(tr("❌ Suspicious process behavior detected!"));
        } else {
            appendToResults(tr("✅ No suspicious process behavior detected"));
        }
        
        return true;
    } catch (const std::exception& e) {
        appendToResults(tr("Error analyzing process behavior: %1").arg(e.what()));
        return false;
    }
}

bool SandboxScanner::generateThreatReport() {
    try {
        if (m_containerId.isEmpty()) {
            return false;
        }
        
        auto finalReport = m_sandboxManager->generateThreatReport(m_containerId.toStdString());
        
        appendToResults(tr("=== Sandbox Analysis Report ==="));
        appendToResults(QString::fromStdString(finalReport.summary));
        
        if (!finalReport.threats.empty()) {
            appendToResults(tr("=== Detected Threats ==="));
            for (const auto& threat : finalReport.threats) {
                appendToResults(tr("- %1: %2").arg(QString::fromStdString(threat.type))
                                            .arg(QString::fromStdString(threat.description)));
            }
        }
        
        if (!finalReport.indicators.empty()) {
            appendToResults(tr("=== Indicators of Compromise ==="));
            for (const auto& indicator : finalReport.indicators) {
                appendToResults(tr("- %1").arg(QString::fromStdString(indicator)));
            }
        }
        
        appendToResults(tr("=== Analysis Complete ==="));
        
        m_analysisReport = QString::fromStdString(finalReport.fullReport);
        
        emit scanProgress(100);
        
        return true;
    } catch (const std::exception& e) {
        m_lastError = tr("Failed to generate threat report: %1").arg(e.what());
        return false;
    }
}

bool SandboxScanner::isValidExecutableFile(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    
    // Common executable file extensions for Windows
    QStringList executableExtensions = {
        "exe", "bat", "cmd", "com", "scr", "pif", "msi", "dll", 
        "jar", "ps1", "vbs", "js", "py", "pl", "sh"
    };
    
    return executableExtensions.contains(suffix) || fileInfo.isExecutable();
}

QString SandboxScanner::generateContainerName() const {
    QString uuid = QUuid::createUuid().toString().mid(1, 8); // Remove braces and take first 8 chars
    return QString("sandbox-analysis-%1").arg(uuid);
}

void SandboxScanner::updateScanStatus(ScanStatus status) {
    m_status = status;
}

void SandboxScanner::appendToResults(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_results += QString("[%1] %2\n").arg(timestamp, message);
}

#include "SandboxScanner.moc"
