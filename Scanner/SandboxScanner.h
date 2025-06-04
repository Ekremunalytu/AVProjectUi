#ifndef SANDBOX_SCANNER_H
#define SANDBOX_SCANNER_H

#include "Interface/IDockerScanner.h"
#include "Interface/ScannerTypes.h"
#include "Docker/include/docker/DockerManager.h"
#include <QString>
#include <memory>
#include <QFileInfo>
#include <QTimer>
#include <QObject>

namespace Sandbox {
    struct SandboxConfiguration;
    struct SandboxAnalysisResult;
    class SandboxManager;
}

/**
 * @brief The SandboxScanner class provides dynamic malware analysis capabilities
 *        using isolated Docker containers for safe file execution and monitoring.
 * 
 * This scanner executes files in a controlled environment, monitoring their behavior,
 * network activity, system calls, and file modifications to detect malicious patterns.
 */
class SandboxScanner : public QObject, public IDockerScanner {
    Q_OBJECT

public:
    SandboxScanner();
    ~SandboxScanner() override;

    // IScanner interface methods
    bool selectFile() override;
    bool scanFile(const QString& filePath) override;
    QFileInfo getSelectedFile() const override;
    QString getResults() const override;
    bool isScanning() const override;
    bool cancelScan() override;
    QString getLastError() const override;
    ScannerType getType() const override;
    ScanStatus getStatus() const override;
    void setFile(const QString& filePath) override;
    QString getFile() const override;

    // IDockerScanner interface methods
    bool submitToContainer(const QString& containerName) override;
    bool isContainerReady() const override;
    QString getContainerStatus() const override;

    // SandboxScanner specific methods
    void setSandboxTimeout(int seconds);
    void setMonitoringLevel(int level); // 0=Basic, 1=Standard, 2=Deep
    QString getAnalysisReport() const;
    QString getBehaviorSummary() const;
    
signals:
    void scanStarted();
    void scanProgress(int percentage);
    void scanCompleted(bool hasMaliciousBehavior);
    void scanError(const QString& error);
    void behaviourDetected(const QString& behaviour);

private slots:
    void onSandboxTimeout();
    void checkSandboxProgress();

private:
    // Core functionality
    bool initializeSandboxEnvironment();
    bool prepareSandboxContainer();
    bool executeFileInSandbox(const QString& filePath);
    bool monitorSandboxExecution();
    void cleanupSandboxEnvironment();
    
    // Analysis methods
    bool analyzeNetworkActivity();
    bool analyzeFileSystemChanges();
    bool analyzeSystemCalls();
    bool analyzeProcessBehavior();
    bool generateThreatReport();
    
    // Helper methods
    bool isValidExecutableFile(const QString& filePath) const;
    QString generateContainerName() const;
    void updateScanStatus(ScanStatus status);
    void appendToResults(const QString& message);

private:
    std::unique_ptr<Sandbox::SandboxManager> m_sandboxManager; // SandboxManager includes Docker functionality
    
    // Scan state
    QFileInfo m_selectedFile;
    QString m_results;
    QString m_lastError;
    ScanStatus m_status;
    bool m_isScanning;
    
    // Sandbox configuration
    QString m_containerName;
    QString m_containerId;
    int m_sandboxTimeout; // seconds
    int m_monitoringLevel;
    
    // Monitoring
    QTimer* m_timeoutTimer;
    QTimer* m_progressTimer;
    QString m_analysisReport;
    QString m_behaviorSummary;
    
    // Constants
    static const QString SANDBOX_IMAGE_NAME;
    static const int DEFAULT_TIMEOUT_SECONDS;
    static const int PROGRESS_CHECK_INTERVAL_MS;
};

#endif // SANDBOX_SCANNER_H
