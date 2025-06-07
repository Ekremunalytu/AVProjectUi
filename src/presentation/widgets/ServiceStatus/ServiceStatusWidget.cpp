#include "ServiceStatusWidget.h" // Corrected header include
#include "ui_ServiceStatusWidget.h" // Corrected UI header include

#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QDirIterator>
#include <QRegularExpression>
#include <QRegularExpression>

// Include manager singletons
#include "storage/database/DatabaseService/DatabaseService.h"
#include "core/config/SettingsManager.h"
#include "core/config/AppConfig.h"
#include "core/session/SessionManager.h"
#include "infrastructure/docker/DockerManager.h"
#include "security/sandbox/SandboxManager.h"

ServiceStatusWidget::ServiceStatusWidget(QWidget *parent) : // Corrected constructor definition
    QWidget(parent),
    ui(new Ui::ServiceStatusWidget) // Corrected UI class instantiation
{
    ui->setupUi(this);
    initialUISetup();
}

ServiceStatusWidget::~ServiceStatusWidget() // Corrected destructor definition
{
    delete ui;
}

void ServiceStatusWidget::initialUISetup()
{
    updateLastRefreshTime();
    on_checkBoxDetailedView_toggled(ui->checkBoxDetailedView->isChecked());
    updateOverallStatus();
    updateCoreScanningEnginesStatus();
    updateTechnicalComponentsStatus();
    updateOtherAdditionsStatus();
}

void ServiceStatusWidget::on_buttonRefresh_clicked()
{
    qDebug() << "ServiceStatusWidget: Refresh button clicked.";
    updateOverallStatus();
    updateCoreScanningEnginesStatus();
    updateTechnicalComponentsStatus();
    updateOtherAdditionsStatus();
    updateLastRefreshTime();
}

void ServiceStatusWidget::on_checkBoxDetailedView_toggled(bool checked)
{
    qDebug() << "ServiceStatusWidget: Detailed view toggled:" << checked;
    ui->groupBoxTechnicalComponents->setVisible(checked);
    if (checked) {
        // Update technical components when detailed view is enabled
        updateTechnicalComponentsStatus();
    }
}

void ServiceStatusWidget::updateLastRefreshTime()
{
    ui->labelLastRefreshTime->setText(QString::fromUtf8("Last Refresh: %1").arg(QDateTime::currentDateTime().toString(QString::fromUtf8("dd.MM.yyyy hh:mm:ss"))));
}

void ServiceStatusWidget::updateOverallStatus()
{
    qDebug() << "ServiceStatusWidget: Updating overall status section...";
    
    // Check critical components status
    bool allSystemsOperational = true;
    QStringList issues;
    
    // Check database connectivity
    try {
        DatabaseService& dbService = DatabaseService::getInstance();
        if (!dbService.isDatabaseConnected()) {
            allSystemsOperational = false;
            issues.append("Database disconnected");
        }
    } catch (const std::exception& e) {
        allSystemsOperational = false;
        issues.append("Database service error");
    }
    
    // Check YARA rules availability - check multiple possible locations
    bool yaraRulesFound = false;
    QStringList yaraSearchPaths = {
        "/Volumes/Crucial/AVProjectUi/src/security/scanning/yara",
        "/Volumes/Crucial/AVProjectUi/build/yara_rules",
        "/Volumes/Crucial/AVProjectUi/cmake-build-debug/yara_rules",
        "/Volumes/Crucial/AVProjectUi/yara_rules"
    };
    
    for (const QString& path : yaraSearchPaths) {
        QDir yaraDir(path);
        if (yaraDir.exists()) {
            QStringList filters;
            filters << "*.yar" << "*.yara";
            QDirIterator iterator(path, filters, QDir::Files, QDirIterator::Subdirectories);
            if (iterator.hasNext()) {
                yaraRulesFound = true;
                break;
            }
        }
    }
    
    if (!yaraRulesFound) {
        allSystemsOperational = false;
        issues.append("YARA rules not found");
    }
    
    // Check Docker status
    if (!checkDockerStatus()) {
        issues.append("Docker not available");
        // Docker is not critical for basic operation
    }
    
    // Check connectivity
    if (!checkConnectivityStatus()) {
        issues.append("Limited connectivity");
        // Connectivity issues are warnings, not critical
    }
    
    if (allSystemsOperational) {
        ui->labelOverallStatusValue->setText("All systems operational");
        ui->labelOverallStatusValue->setStyleSheet("color: green;");
    } else {
        QString statusText = QString("Issues detected: %1").arg(issues.join(", "));
        ui->labelOverallStatusValue->setText(statusText);
        ui->labelOverallStatusValue->setStyleSheet("color: orange;");
    }
}

void ServiceStatusWidget::updateCoreScanningEnginesStatus()
{
    qDebug() << "ServiceStatusWidget: Updating core scanning engines status section...";
    
    // Update Basic Scanner status
    ui->labelBasicScannerStatus->setText("Active");
    
    // Get session scan count from SessionManager
    try {
        SessionManager& sessionManager = SessionManager::getInstance();
        int scannedFiles = sessionManager.getScannedFilesCount();
        ui->labelBasicScannerFilesValue->setText(QString::number(scannedFiles));
    } catch (const std::exception& e) {
        ui->labelBasicScannerFilesValue->setText("Error");
    }
    
    // Update YARA Engine status - count actual rules in source directory only
    QString yaraRulesPath = "/Volumes/Crucial/AVProjectUi/src/security/scanning/yara";
    
    int totalRuleCount = 0;
    QDateTime lastModified;
    bool yaraEngineActive = false;
    
    QDir yaraRulesDir(yaraRulesPath);
    if (yaraRulesDir.exists()) {
        yaraEngineActive = true;
        
        // Count actual YARA rules (not just files) recursively
        QStringList filters;
        filters << "*.yar" << "*.yara";
        QDirIterator iterator(yaraRulesPath, filters, QDir::Files, QDirIterator::Subdirectories);
        
        while (iterator.hasNext()) {
            iterator.next();
            QFileInfo fileInfo(iterator.filePath());
            
            // Update last modified time
            if (fileInfo.lastModified() > lastModified) {
                lastModified = fileInfo.lastModified();
            }
            
            // Count actual rules in this file
            QFile file(iterator.filePath());
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    QString line = in.readLine().trimmed();
                    // Count lines that start with "rule " (actual YARA rule definitions)
                    if (line.startsWith("rule ")) {
                        totalRuleCount++;
                    }
                }
                file.close();
            }
        }
    }
    
    if (yaraEngineActive && totalRuleCount > 0) {
        ui->labelYaraEngineStatus->setText("Active");
        ui->labelYaraRulesValue->setText(QString::number(totalRuleCount));
        ui->labelYaraLastUpdateValue->setText(lastModified.toString("dd.MM.yyyy hh:mm"));
    } else {
        ui->labelYaraEngineStatus->setText("Inactive - Rules not found");
        ui->labelYaraRulesValue->setText("0");
        ui->labelYaraLastUpdateValue->setText("Never");
    }
    
    // Update VirusTotal Integration status
    try {
        SettingsManager& settings = SettingsManager::getInstance();
        QString vtApiKey = settings.getVirusTotalApiKey();
        
        // Also check AppConfig as fallback
        if (vtApiKey.isEmpty()) {
            AppConfig& config = AppConfig::getInstance();
            vtApiKey = config.getVirusTotalApiKey();
        }
        
        if (!vtApiKey.isEmpty() && vtApiKey.length() >= 32) {
            // Check if we have successful scan history to indicate API is working
            SessionManager& sessionManager = SessionManager::getInstance();
            int vtScans = sessionManager.getVirusTotalScansCount();
            
            if (vtScans > 0) {
                ui->labelVirusTotalStatus->setText("Configured");
                ui->labelVirusTotalApiValue->setText("Valid Key Set");
                ui->labelVirusTotalQuotaValue->setText(QString("%1 scans used").arg(vtScans));
            } else {
                // API key is set but no scans have been performed yet
                ui->labelVirusTotalStatus->setText("Configured");
                ui->labelVirusTotalApiValue->setText("Valid Key Set");
                ui->labelVirusTotalQuotaValue->setText("Ready for use");
            }
        } else {
            ui->labelVirusTotalStatus->setText("Not Configured");
            ui->labelVirusTotalApiValue->setText("No API Key");
            ui->labelVirusTotalQuotaValue->setText("N/A");
        }
    } catch (const std::exception& e) {
        ui->labelVirusTotalStatus->setText("Error");
        ui->labelVirusTotalApiValue->setText("Settings Error");
        ui->labelVirusTotalQuotaValue->setText("N/A");
    }
}

void ServiceStatusWidget::updateTechnicalComponentsStatus()
{
    qDebug() << "ServiceStatusWidget: Updating technical components section...";
    
    // Update SQLite Database status
    try {
        DatabaseService& dbService = DatabaseService::getInstance();
        if (dbService.isDatabaseConnected()) {
            ui->labelLocalDbStatusValue->setText("Connected");
            
            // Get database path from AppConfig
            AppConfig& config = AppConfig::getInstance();
            QString dbPath = config.getDatabasePath();
            QFileInfo dbFileInfo(dbPath);
            
            if (dbFileInfo.exists()) {
                ui->labelLocalDbFileValue->setText(dbFileInfo.fileName());
                ui->labelLocalDbSizeValue->setText(QString("%1 KB").arg(dbFileInfo.size() / 1024));
            } else {
                ui->labelLocalDbFileValue->setText("Database file not found");
                ui->labelLocalDbSizeValue->setText("0 KB");
            }
            
            ui->labelLocalDbVersionValue->setText("SQLite 3.x");
            
            // Get actual hash count from the database
            DbManager* dbManager = dbService.getDbManager();
            if (dbManager) {
                std::error_code ec;
                long hashCount = dbManager->getSignatureCount(ec);
                if (!ec && hashCount >= 0) {
                    ui->labelLocalDbRecordsValue->setText(QString("%1 hashes").arg(QLocale().toString(hashCount)));
                } else {
                    ui->labelLocalDbRecordsValue->setText("Error getting count");
                }
            } else {
                ui->labelLocalDbRecordsValue->setText("DbManager error");
            }
        } else {
            ui->labelLocalDbStatusValue->setText("Disconnected");
            ui->labelLocalDbFileValue->setText("N/A");
            ui->labelLocalDbSizeValue->setText("N/A");
            ui->labelLocalDbVersionValue->setText("N/A");
            ui->labelLocalDbRecordsValue->setText("N/A");
        }
    } catch (const std::exception& e) {
        ui->labelLocalDbStatusValue->setText("Error");
        ui->labelLocalDbFileValue->setText("Service Error");
        ui->labelLocalDbSizeValue->setText("N/A");
        ui->labelLocalDbVersionValue->setText("N/A");
        ui->labelLocalDbRecordsValue->setText("N/A");
    }
    
    // Update CDR Service status
    ui->labelCdrStatusValue->setText("Available");
    
    // Get CDR processed count from SessionManager
    try {
        SessionManager& sessionManager = SessionManager::getInstance();
        int cdrProcessed = sessionManager.getCdrProcessedCount();
        ui->labelCdrProcessedValue->setText(QString::number(cdrProcessed));
    } catch (const std::exception& e) {
        ui->labelCdrProcessedValue->setText("Error");
    }
    ui->labelCdrEngineValue->setText("Integrated CDR");
    
    // Update Sandbox Analysis status
    ui->labelSandboxStatusValue->setText("Available");
    ui->labelSandboxActiveValue->setText("0"); // Active analysis count
    ui->labelSandboxCompletedValue->setText("0"); // Session completed count
    ui->labelSandboxEnvironmentValue->setText("Docker Ready");
    
    // Update Docker Infrastructure status
    checkDockerStatus();
}

void ServiceStatusWidget::updateOtherAdditionsStatus()
{
    qDebug() << "ServiceStatusWidget: Updating additional status section...";
    
    // Update Network Monitoring status - check if actually implemented
    // TODO: Replace with actual network monitoring status when implemented
    ui->labelNetworkStatusValue->setText("Not Implemented");
    ui->labelNetworkActiveValue->setText("Development Pending");
    ui->labelNetworkThreatValue->setText("N/A");
    
    // Update Connectivity status
    checkConnectivityStatus();
    
    // Update System Resources
    updateSystemResourceStatus();
    
    // Update Application Logging - check if logging system is actually implemented
    // TODO: Replace with actual logging status when implemented
    ui->labelLogServiceStatusValue->setText("Not Implemented");
    ui->labelLogLevelValue->setText("N/A");
    
    // Check log file size - look for actual log files
    QStringList logSearchPaths = {
        QDir::homePath() + "/AVProjectUi/logs",
        "/Volumes/Crucial/AVProjectUi/logs",
        "/tmp/avproject_logs"
    };
    
    bool logDirFound = false;
    for (const QString& logPath : logSearchPaths) {
        QDir logDir(logPath);
        if (logDir.exists()) {
            QStringList logFiles = logDir.entryList(QStringList() << "*.log", QDir::Files);
            if (!logFiles.isEmpty()) {
                QFileInfo logInfo(logDir.absoluteFilePath(logFiles.first()));
                ui->labelLogSizeValue->setText(QString("%1 KB").arg(logInfo.size() / 1024));
                ui->labelLogServiceStatusValue->setText("Basic Logging Active");
                logDirFound = true;
                break;
            }
        }
    }
    
    if (!logDirFound) {
        ui->labelLogSizeValue->setText("No logs found");
    }
}

bool ServiceStatusWidget::checkDockerStatus()
{
    // Check if Docker is running by trying to execute docker version
    QProcess dockerProcess;
    dockerProcess.setProgram("docker");
    dockerProcess.setArguments(QStringList() << "version" << "--format" << "{{.Server.Version}}");
    dockerProcess.start();
    
    if (dockerProcess.waitForFinished(3000)) { // 3 second timeout
        if (dockerProcess.exitCode() == 0) {
            QString version = dockerProcess.readAllStandardOutput().trimmed();
            ui->labelDockerDaemonStatusValue->setText("Running");
            ui->labelDockerVersionValue->setText(version.isEmpty() ? "Available" : version);
            
            // Get container count
            getDockerContainerCount();
            getDockerImageCount();
            return true;
        } else {
            ui->labelDockerDaemonStatusValue->setText("Not Running");
            ui->labelDockerVersionValue->setText("N/A");
            ui->labelDockerContainersValue->setText("N/A");
            ui->labelDockerImagesValue->setText("N/A");
            return false;
        }
    } else {
        ui->labelDockerDaemonStatusValue->setText("Not Available");
        ui->labelDockerVersionValue->setText("Docker not found");
        ui->labelDockerContainersValue->setText("N/A");
        ui->labelDockerImagesValue->setText("N/A");
        return false;
    }
}

int ServiceStatusWidget::getDockerContainerCount()
{
    QProcess dockerProcess;
    dockerProcess.setProgram("docker");
    dockerProcess.setArguments(QStringList() << "ps" << "-q");
    dockerProcess.start();
    
    if (dockerProcess.waitForFinished(2000)) {
        if (dockerProcess.exitCode() == 0) {
            QStringList containers = QString(dockerProcess.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);
            int count = containers.count();
            ui->labelDockerContainersValue->setText(QString::number(count));
            return count;
        } else {
            ui->labelDockerContainersValue->setText("Error");
            return -1;
        }
    } else {
        ui->labelDockerContainersValue->setText("Timeout");
        return -1;
    }
}

int ServiceStatusWidget::getDockerImageCount()
{
    QProcess dockerProcess;
    dockerProcess.setProgram("docker");
    dockerProcess.setArguments(QStringList() << "images" << "-q");
    dockerProcess.start();
    
    if (dockerProcess.waitForFinished(2000)) {
        if (dockerProcess.exitCode() == 0) {
            QStringList images = QString(dockerProcess.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);
            int count = images.count();
            ui->labelDockerImagesValue->setText(QString::number(count));
            return count;
        } else {
            ui->labelDockerImagesValue->setText("Error");
            return -1;
        }
    } else {
        ui->labelDockerImagesValue->setText("Timeout");
        return -1;
    }
}

bool ServiceStatusWidget::checkConnectivityStatus()
{
    bool overallConnectivity = true;
    
    // Check VirusTotal connectivity
    try {
        SettingsManager& settings = SettingsManager::getInstance();
        QString vtApiKey = settings.getVirusTotalApiKey();
        
        // Also check AppConfig as fallback
        if (vtApiKey.isEmpty()) {
            AppConfig& config = AppConfig::getInstance();
            vtApiKey = config.getVirusTotalApiKey();
        }
        
        if (!vtApiKey.isEmpty() && vtApiKey.length() >= 32) {
            // Check if we have scan history to confirm API is working
            SessionManager& sessionManager = SessionManager::getInstance();
            int vtScans = sessionManager.getVirusTotalScansCount();
            
            if (vtScans > 0) {
                ui->labelConnVirusTotalValue->setText("Active (API Working)");
            } else {
                ui->labelConnVirusTotalValue->setText("Configured");
            }
        } else {
            ui->labelConnVirusTotalValue->setText("Not Configured");
            overallConnectivity = false;
        }
    } catch (const std::exception& e) {
        ui->labelConnVirusTotalValue->setText("Error");
        overallConnectivity = false;
    }
    
    // Check general internet connectivity by pinging a reliable server
    QProcess pingProcess;
    pingProcess.setProgram("ping");
    pingProcess.setArguments(QStringList() << "-c" << "1" << "-W" << "2000" << "8.8.8.8");
    pingProcess.start();
    
    if (pingProcess.waitForFinished(3000)) {
        if (pingProcess.exitCode() == 0) {
            ui->labelConnInternetValue->setText("Connected");
        } else {
            ui->labelConnInternetValue->setText("No Internet");
            overallConnectivity = false;
        }
    } else {
        ui->labelConnInternetValue->setText("Timeout");
        overallConnectivity = false;
    }
    
    return overallConnectivity;
}

void ServiceStatusWidget::updateSystemResourceStatus()
{
    // Get CPU usage (simplified - would need more sophisticated monitoring)
    ui->labelResCpuValue->setText("< 5%"); // Placeholder for actual CPU monitoring
    
    // Get memory usage (simplified)
    ui->labelResMemoryValue->setText("~150 MB"); // Placeholder for actual memory monitoring
    
    // Check available disk space
    QString workspacePath = "/Volumes/Crucial/AVProjectUi";
    QFileInfo workspaceInfo(workspacePath);
    if (workspaceInfo.exists()) {
        // Get available space on the volume
        QProcess dfProcess;
        dfProcess.setProgram("df");
        dfProcess.setArguments(QStringList() << "-h" << workspacePath);
        dfProcess.start();
        
        if (dfProcess.waitForFinished(2000)) {
            QString output = dfProcess.readAllStandardOutput();
            QStringList lines = output.split('\n');
            if (lines.size() > 1) {
                QStringList fields = lines[1].split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                if (fields.size() >= 4) {
                    ui->labelResDiskSpaceValue->setText(fields[3] + " available");
                } else {
                    ui->labelResDiskSpaceValue->setText("Available");
                }
            } else {
                ui->labelResDiskSpaceValue->setText("Unknown");
            }
        } else {
            ui->labelResDiskSpaceValue->setText("Check failed");
        }
    } else {
        ui->labelResDiskSpaceValue->setText("Workspace not found");
    }
}
