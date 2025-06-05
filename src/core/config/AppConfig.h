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
        m_databasePath = m_settings.value(QStringLiteral("Database/Path"), getDefaultDatabasePath()).toString();
        
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
};

#endif // APPCONFIG_H