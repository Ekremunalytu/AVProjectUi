#ifndef FILESYSTEMWATCHER_H
#define FILESYSTEMWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QDir>
#include <QStringList>
#include <memory>
#include "../scanning/BasicScanner.h"

/**
 * @brief Real-time file system monitoring with automatic YARA scanning
 * 
 * This class monitors critical directories for file changes and automatically
 * triggers YARA scanning on new or modified files.
 */
class FileSystemWatcher : public QObject
{
    Q_OBJECT

public:
    explicit FileSystemWatcher(QObject* parent = nullptr);
    ~FileSystemWatcher();

    /**
     * @brief Start monitoring specified directories
     * @param directories List of directories to monitor
     * @return True if monitoring started successfully
     */
    bool startMonitoring(const QStringList& directories = {});
    
    /**
     * @brief Stop file system monitoring
     */
    void stopMonitoring();
    
    /**
     * @brief Check if monitoring is currently active
     */
    bool isMonitoring() const;
    
    /**
     * @brief Add a directory to monitor
     * @param path Directory path to monitor
     */
    void addWatchPath(const QString& path);
    
    /**
     * @brief Remove a directory from monitoring
     * @param path Directory path to remove
     */
    void removeWatchPath(const QString& path);
    
    /**
     * @brief Set file types to monitor (e.g., "*.exe", "*.dll")
     * @param patterns List of file patterns to monitor
     */
    void setFilePatterns(const QStringList& patterns);

signals:
    /**
     * @brief Emitted when a suspicious file is detected
     * @param filePath Path to the suspicious file
     * @param matches List of YARA rules that matched
     */
    void suspiciousFileDetected(const QString& filePath, const QStringList& matches);
    
    /**
     * @brief Emitted when monitoring status changes
     * @param isActive True if monitoring is active
     */
    void monitoringStatusChanged(bool isActive);
    
    /**
     * @brief Emitted for file activity notifications
     * @param message Activity description
     */
    void fileActivityDetected(const QString& message);

private slots:
    void onFileChanged(const QString& path);
    void onDirectoryChanged(const QString& path);
    void scanQueuedFiles();

private:
    QFileSystemWatcher* m_fsWatcher;
    std::unique_ptr<BasicScanner> m_scanner;
    QTimer* m_scanTimer;
    QStringList m_scanQueue;
    QStringList m_filePatterns;
    QStringList m_monitoredDirs;
    bool m_isMonitoring;
    
    // Default critical directories to monitor
    QStringList getDefaultWatchPaths() const;
    bool shouldScanFile(const QString& filePath) const;
    void processNewFile(const QString& filePath);
};

#endif // FILESYSTEMWATCHER_H
