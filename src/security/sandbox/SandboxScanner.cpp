#include "SandboxScanner.h"
#include "SandboxManager.h"
#include "SandboxTypes.h"
#include <QFileDialog>
#include <QDebug>
#include <QUuid>
#include <QStandardPaths>
#include <QDir>
#include <QThread>

// Static constants
const QString SandboxScanner::SANDBOX_IMAGE_NAME = "avproject-sandbox:latest";
const int SandboxScanner::DEFAULT_TIMEOUT_SECONDS = 300; // 5 minutes
const int SandboxScanner::PROGRESS_CHECK_INTERVAL_MS = 5000; // 5 seconds

SandboxScanner::SandboxScanner()
    : m_sandboxManager(std::make_unique<Sandbox::SandboxManager>())
    , m_status(ScanStatus::Idle)
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
    
    updateScanStatus(ScanStatus::Scanning);
    m_isScanning = true;
    
    appendToResults(tr("Starting sandbox analysis of: %1").arg(fileInfo.fileName()));
    appendToResults(tr("File size: %1 bytes").arg(fileInfo.size()));
    appendToResults(tr("File path: %1").arg(filePath));
    
    emit scanStarted();
    
    // Start the analysis process
    if (!prepareSandboxContainer()) {
        m_lastError = tr("Failed to prepare sandbox container");
        updateScanStatus(ScanStatus::Error);
        m_isScanning = false;
        emit scanError(m_lastError);
        return false;
    }
    
    if (!executeFileInSandbox(filePath)) {
        m_lastError = tr("Failed to execute file in sandbox");
        updateScanStatus(ScanStatus::Error);
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
            m_sandboxManager->destroySandbox(m_containerId.toStdString());
        }
    } catch (const std::exception& e) {
        qWarning() << "Error during sandbox cleanup:" << e.what();
    }
    
    cleanupSandboxEnvironment();
    updateScanStatus(ScanStatus::Cancelled);
    m_isScanning = false;
    
    appendToResults(tr("Sandbox analysis cancelled by user"));
    
    return true;
}

QString SandboxScanner::getLastError() const {
    return m_lastError;
}

ScannerType SandboxScanner::getType() const {
    return ScannerType::Sandbox;
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
        // Use Docker manager to check container status
        auto& dockerMgr = m_sandboxManager->getDockerManager();
        auto containerInfo = dockerMgr.getContainerDetails(m_containerId.toStdString());
        return containerInfo.containerStatus == "running";
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
        auto& dockerMgr = m_sandboxManager->getDockerManager();
        auto containerInfo = dockerMgr.getContainerDetails(m_containerId.toStdString());
        return QString::fromStdString(containerInfo.containerStatus);
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
    updateScanStatus(ScanStatus::Completed);
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
        if (!m_sandboxManager->isDaemonRunning()) {
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
        config.level = static_cast<Sandbox::MonitoringLevel>(m_monitoringLevel);
        config.timeoutSeconds = m_sandboxTimeout;
        config.enableNetworkMonitoring = true;
        config.enableFileSystemMonitoring = true;
        config.enableProcessMonitoring = true;
        config.enableBehaviorAnalysis = true;
        config.sandboxPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation).toStdString() + "/sandbox";
        config.tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation).toStdString() + "/sandbox_temp";
        
        // Validate configuration before creating container
        if (!m_sandboxManager->validateSandboxConfiguration(config)) {
            m_lastError = tr("Invalid sandbox configuration");
            return false;
        }
        
        std::string containerId = m_sandboxManager->createSandbox(config);
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
        
        // Copy file to container using Docker manager
        auto& dockerMgr = m_sandboxManager->getDockerManager();
        std::string containerPath = "/sandbox/target" + 
            QFileInfo(filePath).suffix().prepend(".").toStdString();
        
        // Copy file using docker cp equivalent functionality
        try {
            dockerMgr.copyFileToContainer(m_containerId.toStdString(), 
                                        filePath.toStdString(), 
                                        containerPath);
        } catch (const std::exception& e) {
            m_lastError = tr("Failed to copy file to container: %1").arg(e.what());
            return false;
        }
        
        // Execute file in sandbox
        bool started = m_sandboxManager->executeFileInSandbox(m_containerId.toStdString(), containerPath);
        
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
        auto analysisResult = m_sandboxManager->collectAnalysisResults(m_containerId.toStdString());
        
        // Check for new behaviors
        for (const auto& threat : analysisResult.detectedThreats) {
            QString behaviorStr = QString::fromStdString(threat.description);
            if (!m_behaviorSummary.contains(behaviorStr)) {
                m_behaviorSummary += behaviorStr + "\n";
                emit behaviourDetected(behaviorStr);
            }
        }
        
        // Update analysis report
        m_analysisReport = QString::fromStdString(analysisResult.errorMessage);
        
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
            m_sandboxManager->destroySandbox(m_containerId.toStdString());
            
            appendToResults(tr("Sandbox environment cleaned up"));
        }
    } catch (const std::exception& e) {
        qWarning() << "Error during sandbox cleanup:" << e.what();
    }
    
    m_containerId.clear();
    m_containerName.clear();
}

bool SandboxScanner::generateThreatReport() {
    try {
        if (m_containerId.isEmpty()) {
            return false;
        }
        
        auto finalReport = m_sandboxManager->collectAnalysisResults(m_containerId.toStdString());
        
        appendToResults(tr("=== Sandbox Analysis Report ==="));
        
        // Generate summary based on threat level
        QString summary;
        switch (finalReport.threatLevel) {
            case Sandbox::ThreatLevel::NONE:
                summary = tr("File appears to be safe - no threats detected");
                break;
            case Sandbox::ThreatLevel::LOW:
                summary = tr("File shows minimal suspicious activity");
                break;
            case Sandbox::ThreatLevel::MEDIUM:
                summary = tr("File exhibits moderately suspicious behavior");
                break;
            case Sandbox::ThreatLevel::HIGH:
                summary = tr("File demonstrates highly suspicious behavior");
                break;
            case Sandbox::ThreatLevel::CRITICAL:
                summary = tr("File identified as malicious with critical threats");
                break;
        }
        appendToResults(summary);
        
        if (!finalReport.threats.empty()) {
            appendToResults(tr("=== Detected Threats ==="));
            for (const auto& threat : finalReport.threats) {
                appendToResults(tr("- %1: %2").arg(static_cast<int>(threat.type))
                                            .arg(QString::fromStdString(threat.description)));
            }
        }
        
        appendToResults(tr("=== Analysis Complete ==="));
        appendToResults(tr("Overall Safety: %1").arg(finalReport.isSafe ? tr("SAFE") : tr("UNSAFE")));
        
        m_analysisReport = QString::fromStdString(finalReport.errorMessage);
        
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
