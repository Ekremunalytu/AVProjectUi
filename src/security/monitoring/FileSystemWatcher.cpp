#include "FileSystemWatcher.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>

FileSystemWatcher::FileSystemWatcher(QObject* parent)
    : QObject(parent)
    , m_fsWatcher(new QFileSystemWatcher(this))
    , m_scanner(std::make_unique<BasicScanner>(this))
    , m_scanTimer(new QTimer(this))
    , m_isMonitoring(false)
{
    // Connect file system watcher signals
    connect(m_fsWatcher, &QFileSystemWatcher::fileChanged,
            this, &FileSystemWatcher::onFileChanged);
    connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged,
            this, &FileSystemWatcher::onDirectoryChanged);
    
    // Setup scan timer (batch process files every 2 seconds)
    m_scanTimer->setInterval(2000);
    m_scanTimer->setSingleShot(false);
    connect(m_scanTimer, &QTimer::timeout,
            this, &FileSystemWatcher::scanQueuedFiles);
    
    // Initialize YARA scanner
    m_scanner->initializeYara();
    
    // Connect scanner signals
    connect(m_scanner.get(), &BasicScanner::scanResultsReady,
            this, [this](const QString& results) {
                if (results.contains("SUSPICIOUS") || results.contains("MALICIOUS")) {
                    // Extract file path and matches from results
                    // This would need proper parsing in real implementation
                    QString filePath = ""; // Extract from results
                    QStringList matches; // Extract from results
                    emit suspiciousFileDetected(filePath, matches);
                }
            });
    
    // Set default file patterns to monitor
    m_filePatterns = {"*.exe", "*.dll", "*.bat", "*.cmd", "*.scr", "*.pif", 
                      "*.com", "*.jar", "*.zip", "*.rar", "*.7z", "*.tar.gz"};
}

FileSystemWatcher::~FileSystemWatcher()
{
    stopMonitoring();
}

bool FileSystemWatcher::startMonitoring(const QStringList& directories)
{
    if (m_isMonitoring) {
        qDebug() << "FileSystemWatcher: Already monitoring";
        return true;
    }
    
    QStringList dirsToMonitor = directories.isEmpty() ? getDefaultWatchPaths() : directories;
    
    for (const QString& dir : dirsToMonitor) {
        QFileInfo dirInfo(dir);
        if (dirInfo.exists() && dirInfo.isDir()) {
            m_fsWatcher->addPath(dir);
            m_monitoredDirs.append(dir);
            qDebug() << "FileSystemWatcher: Added watch path:" << dir;
        } else {
            qDebug() << "FileSystemWatcher: Invalid directory:" << dir;
        }
    }
    
    if (!m_monitoredDirs.isEmpty()) {
        m_isMonitoring = true;
        m_scanTimer->start();
        emit monitoringStatusChanged(true);
        qDebug() << "FileSystemWatcher: Monitoring started for" << m_monitoredDirs.size() << "directories";
        return true;
    }
    
    return false;
}

void FileSystemWatcher::stopMonitoring()
{
    if (!m_isMonitoring) {
        return;
    }
    
    m_fsWatcher->removePaths(m_monitoredDirs);
    m_monitoredDirs.clear();
    m_scanQueue.clear();
    m_scanTimer->stop();
    m_isMonitoring = false;
    
    emit monitoringStatusChanged(false);
    qDebug() << "FileSystemWatcher: Monitoring stopped";
}

bool FileSystemWatcher::isMonitoring() const
{
    return m_isMonitoring;
}

void FileSystemWatcher::addWatchPath(const QString& path)
{
    QFileInfo pathInfo(path);
    if (pathInfo.exists() && pathInfo.isDir() && !m_monitoredDirs.contains(path)) {
        m_fsWatcher->addPath(path);
        m_monitoredDirs.append(path);
        qDebug() << "FileSystemWatcher: Added new watch path:" << path;
    }
}

void FileSystemWatcher::removeWatchPath(const QString& path)
{
    if (m_monitoredDirs.contains(path)) {
        m_fsWatcher->removePath(path);
        m_monitoredDirs.removeAll(path);
        qDebug() << "FileSystemWatcher: Removed watch path:" << path;
    }
}

void FileSystemWatcher::setFilePatterns(const QStringList& patterns)
{
    m_filePatterns = patterns;
    qDebug() << "FileSystemWatcher: Updated file patterns:" << patterns;
}

void FileSystemWatcher::onFileChanged(const QString& path)
{
    QFileInfo fileInfo(path);
    if (fileInfo.exists() && shouldScanFile(path)) {
        processNewFile(path);
    }
}

void FileSystemWatcher::onDirectoryChanged(const QString& path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }
    
    // Check for new files in the directory
    QStringList filters;
    for (const QString& pattern : m_filePatterns) {
        filters << pattern;
    }
    
    QFileInfoList newFiles = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& fileInfo : newFiles) {
        if (shouldScanFile(fileInfo.absoluteFilePath())) {
            processNewFile(fileInfo.absoluteFilePath());
        }
    }
    
    emit fileActivityDetected(QString("Directory activity detected: %1").arg(path));
}

void FileSystemWatcher::scanQueuedFiles()
{
    if (m_scanQueue.isEmpty()) {
        return;
    }
    
    // Process up to 5 files per timer interval to avoid overload
    int filesToProcess = qMin(5, m_scanQueue.size());
    
    for (int i = 0; i < filesToProcess; ++i) {
        QString filePath = m_scanQueue.takeFirst();
        
        QFileInfo fileInfo(filePath);
        if (fileInfo.exists() && fileInfo.isFile()) {
            qDebug() << "FileSystemWatcher: Scanning file:" << filePath;
            
            // Perform YARA scan
            if (m_scanner->scanFileWithYara(filePath)) {
                emit fileActivityDetected(QString("Scanned file: %1").arg(fileInfo.fileName()));
            }
        }
    }
}

QStringList FileSystemWatcher::getDefaultWatchPaths() const
{
    QStringList paths;
    
#ifdef Q_OS_WIN
    // Windows critical directories
    paths << "C:/Windows/System32"
          << "C:/Windows/SysWOW64"
          << "C:/Users/" + qgetenv("USERNAME") + "/AppData/Roaming"
          << "C:/Users/" + qgetenv("USERNAME") + "/AppData/Local"
          << "C:/Users/" + qgetenv("USERNAME") + "/Downloads"
          << "C:/Users/" + qgetenv("USERNAME") + "/Desktop"
          << "C:/Temp"
          << "C:/Windows/Temp";
#elif defined(Q_OS_MACOS)
    // macOS critical directories
    QString homeDir = QDir::homePath();
    paths << "/Applications"
          << "/System/Library"
          << "/usr/local/bin"
          << homeDir + "/Downloads"
          << homeDir + "/Desktop"
          << homeDir + "/Documents"
          << "/tmp"
          << "/var/tmp";
#else
    // Linux critical directories
    QString homeDir = QDir::homePath();
    paths << "/usr/bin"
          << "/usr/local/bin"
          << "/opt"
          << homeDir + "/Downloads"
          << homeDir + "/Desktop"
          << homeDir + "/Documents"
          << "/tmp"
          << "/var/tmp";
#endif
    
    return paths;
}

bool FileSystemWatcher::shouldScanFile(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    
    // Check if file extension matches our patterns
    for (const QString& pattern : m_filePatterns) {
        QString extension = "*." + fileInfo.suffix().toLower();
        if (pattern.toLower() == extension) {
            return true;
        }
    }
    
    // Always scan executable files
    if (fileInfo.isExecutable()) {
        return true;
    }
    
    return false;
}

void FileSystemWatcher::processNewFile(const QString& filePath)
{
    if (!m_scanQueue.contains(filePath)) {
        m_scanQueue.append(filePath);
        qDebug() << "FileSystemWatcher: Queued file for scanning:" << QFileInfo(filePath).fileName();
    }
}
