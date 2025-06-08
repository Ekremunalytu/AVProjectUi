#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QApplication>
#include <QtCore/QtGlobal>

/**
 * @brief The AppConfig class provides application-wide configuration settings.
 *
 * This singleton class manages configuration settings for the application,
 * including file paths and other configurable parameters.
 */
class AppConfig {
public:
    /**
     * @brief Get the singleton instance of AppConfig.
     * @return Reference to the singleton AppConfig instance.
     */
    static AppConfig& getInstance() {
        static AppConfig instance;
        return instance;
    }
    
    /**
     * @brief Get the database file path.
     * @return Database file path as a QString.
     */
    QString getDatabasePath() const {
        return m_databasePath;
    }

    /**
     * @brief Set the database file path.
     * @param path The new database file path.
     */
    void setDatabasePath(const QString &path) {
        m_databasePath = path;
        m_settings.setValue(QStringLiteral("Database/Path"), path);
    }
    
    /**
     * @brief Get the VirusTotal API key.
     * @return VirusTotal API key as a QString.
     */
    QString getVirusTotalApiKey() const {
        return m_virusTotalApiKey;
    }
    
    /**
     * @brief Set the VirusTotal API key.
     * @param apiKey The new VirusTotal API key.
     */
    void setVirusTotalApiKey(const QString& apiKey) {
        m_virusTotalApiKey = apiKey;
        
        // Write to config.ini file using m_settings
        m_settings.setValue(QStringLiteral("VirusTotal/ApiKey"), apiKey);
    }
    
    /**
     * @brief Get the VirusTotal API endpoints
     * @return API URL for file submissions
     */
    QString getVirusTotalFilesUrl() const {
        return QStringLiteral("https://www.virustotal.com/api/v3/files");
    }

    /**
     * @brief Get the VirusTotal analyses endpoint template
     * @return API URL template for analyses queries (use with QString.arg())
     */
    QString getVirusTotalAnalysesUrl() const {
        return QStringLiteral("https://www.virustotal.com/api/v3/analyses/%1");
    }
    
    /**
     * @brief Load configuration from settings file.
     */
    void loadConfig() {
        // Load Database Path from m_settings (which points to config.ini)
        QString rawDbPath = m_settings.value(QStringLiteral("Database/Path"), getDefaultDatabasePath()).toString();
        
        // Convert relative paths to absolute paths relative to config.ini's directory
        if (QDir::isRelativePath(rawDbPath)) {
            QDir configDir = QFileInfo(getActualConfigIniPath()).absoluteDir();
            m_databasePath = QDir::cleanPath(configDir.absoluteFilePath(rawDbPath));
        } else {
            m_databasePath = rawDbPath;
        }
        
        // Smart database path resolution: If the resolved path doesn't exist or is too small,
        // try to find the actual production database
        m_databasePath = findBestDatabasePath(m_databasePath);
        
        // Load VirusTotal API key from m_settings (which points to config.ini)
        m_virusTotalApiKey = m_settings.value(QStringLiteral("VirusTotal/ApiKey"), QString()).toString();
    }

    /**
     * @brief Check and create database path if it does not exist.
     */
    void checkAndCreateDbPath() {
        // Use the loaded m_databasePath
        QFileInfo dbFileInfo(m_databasePath);
        QDir dbDir = dbFileInfo.absoluteDir();

        if (!dbDir.exists()) {
            if (!dbDir.mkpath(QStringLiteral("."))) { // Use . for current directory within dbDir path
                qWarning() << "Could not create database directory:" << dbDir.absolutePath();
            }
        }
    }

private:
    // Private constructor for singleton pattern
    AppConfig()
        : m_settings(getActualConfigIniPath(), QSettings::IniFormat) // Initialize m_settings to point to config.ini
    {
        loadConfig();
        checkAndCreateDbPath();
    }
    
    // Delete copy and move constructors/operators to enforce singleton
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;
    AppConfig(AppConfig&&) = delete;
    AppConfig& operator=(AppConfig&&) = delete;
    
private: // m_settings is now private
    QSettings m_settings;
    QString m_databasePath;
    QString m_virusTotalApiKey;

    // Helper function to get the actual path to config.ini
    QString getActualConfigIniPath() const {
        QString appPath = QApplication::applicationDirPath();
        
        // Try multiple possible locations for config.ini
        QStringList possiblePaths = {
            // 1. Same directory as executable (for deployed applications)
            QDir::cleanPath(appPath + QStringLiteral("/config.ini")),
            
            // 2. Core subdirectory relative to executable
            QDir::cleanPath(appPath + QStringLiteral("/Core/config.ini")),
            
            // 3. One level up and then Core (for build directories)
            QDir::cleanPath(appPath + QStringLiteral("/../Core/config.ini")),
            
            // 4. For macOS .app bundles - go up to project root
            QDir::cleanPath(appPath + QStringLiteral("/../../../../Core/config.ini")),
            
            // 5. For Windows debug/release builds
            QDir::cleanPath(appPath + QStringLiteral("/../../Core/config.ini")),
            
            // 6. Alternative paths for different build configurations
            QDir::cleanPath(appPath + QStringLiteral("/../../../Core/config.ini"))
        };
        
        // Return the first existing config file
        for (const QString& path : possiblePaths) {
            if (QFileInfo::exists(path)) {
                qDebug() << "Found config.ini at:" << path;
                return path;
            }
        }
        
        // If no config file found, return default path and log warning
        QString defaultPath = QDir::cleanPath(appPath + QStringLiteral("/../Core/config.ini"));
        qWarning() << "config.ini not found in any expected location. Using default path:" << defaultPath;
        qWarning() << "Searched paths:";
        for (const QString& path : possiblePaths) {
            qWarning() << "  -" << path;
        }
        
        return defaultPath;
    }

    // Helper function to get the default database path relative to config.ini's directory
    QString getDefaultDatabasePath() const {
        QDir configDir = QFileInfo(getActualConfigIniPath()).absoluteDir();
        return configDir.filePath(QStringLiteral("../MalwareHashes/identifier.sqlite"));
    }

    // Helper function to find the best available database path
    QString findBestDatabasePath(const QString& configuredPath) const {
        // Define potential database locations to search
        QStringList candidatePaths = {
            configuredPath, // First try the configured path
            
            // Production database locations (project root)
            "/Volumes/Crucial/AVProjectUi/resources/data/identifier.sqlite",
            
            // Build directory locations
            "/Volumes/Crucial/AVProjectUi/build/resources/data/identifier.sqlite",
            "/Volumes/Crucial/AVProjectUi/cmake-build-debug/resources/data/identifier.sqlite",
            "/Volumes/Crucial/AVProjectUi/cmake-build-release/resources/data/identifier.sqlite",
            
            // App bundle locations
            "/Volumes/Crucial/AVProjectUi/build/AVProjectUi.app/Contents/MalwareHashes/identifier.sqlite",
            "/Volumes/Crucial/AVProjectUi/cmake-build-debug/AVProjectUi.app/Contents/MalwareHashes/identifier.sqlite",
            
            // Alternative naming patterns
            "/Volumes/Crucial/AVProjectUi/MalwareHashes/identifier.sqlite",
            "/Volumes/Crucial/AVProjectUi/data/identifier.sqlite"
        };
        
        QString bestPath;
        qint64 bestSize = 0;
        const qint64 MIN_PRODUCTION_DB_SIZE = 100 * 1024 * 1024; // 100MB minimum for production DB
        
        qDebug() << "AppConfig: Searching for best database among candidates...";
        
        for (const QString& candidatePath : candidatePaths) {
            QFileInfo fileInfo(candidatePath);
            
            if (fileInfo.exists() && fileInfo.isFile()) {
                qint64 size = fileInfo.size();
                qDebug() << "AppConfig: Found database at" << candidatePath << "Size:" << size << "bytes";
                
                // If this database is significantly larger, prefer it
                if (size > bestSize) {
                    bestPath = candidatePath;
                    bestSize = size;
                }
                
                // If we found a database larger than minimum production size, and it's much larger than current best
                if (size >= MIN_PRODUCTION_DB_SIZE && size > (bestSize * 1.5)) {
                    bestPath = candidatePath;
                    bestSize = size;
                    qDebug() << "AppConfig: Selected large production database:" << candidatePath;
                    break; // This is likely our production database
                }
            } else {
                qDebug() << "AppConfig: Database not found at" << candidatePath;
            }
        }
        
        if (bestPath.isEmpty()) {
            qWarning() << "AppConfig: No database found in any candidate location. Using configured path:" << configuredPath;
            return configuredPath;
        }
        
        if (bestPath != configuredPath) {
            qDebug() << "AppConfig: Auto-selected database:" << bestPath << "instead of configured:" << configuredPath;
        }
        
        return bestPath;
    }
};

#endif // APPCONFIG_H