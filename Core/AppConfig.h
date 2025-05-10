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
    static QString getDatabasePath() {
        QSettings configIni(QDir::cleanPath(QApplication::applicationDirPath() + QStringLiteral("/../Core/config.ini")),
                            QSettings::IniFormat);
        return configIni.value(QStringLiteral("Database/Path"), QStringLiteral("./MalwareHashes/identifier.sqlite")).toString();
    }

    /**
     * @brief Set the database file path.
     * @param path The new database file path.
     */
    static void setDatabasePath(const QString &path) {
        QSettings configIni(QDir::cleanPath(QApplication::applicationDirPath() + QStringLiteral("/../Core/config.ini")),
                            QSettings::IniFormat);
        configIni.setValue(QStringLiteral("Database/Path"), path);
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
        
        // Write to config.ini file
        QSettings configIni(QDir::cleanPath(QApplication::applicationDirPath() + QStringLiteral("/../Core/config.ini")), 
                            QSettings::IniFormat);
        configIni.setValue(QStringLiteral("VirusTotal/ApiKey"), apiKey);
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
        m_databasePath = m_settings.value(QStringLiteral("Database/Path"), QStringLiteral("./MalwareHashes/identifier.sqlite")).toString();
        
        // Load VirusTotal API key from config.ini
        QSettings configIni(QDir::cleanPath(QApplication::applicationDirPath() + QStringLiteral("/../Core/config.ini")), 
                            QSettings::IniFormat);
        m_virusTotalApiKey = configIni.value(QStringLiteral("VirusTotal/ApiKey"), QString()).toString();
    }

    /**
     * @brief Check and create database path if it does not exist.
     */
    void checkAndCreateDbPath() {
        QSettings configIni(QDir::cleanPath(QApplication::applicationDirPath() + QStringLiteral("/../Core/config.ini")),
                            QSettings::IniFormat);
        QString dbPath = configIni.value(QStringLiteral("Database/Path"), QStringLiteral("./MalwareHashes/identifier.sqlite")).toString();

        QFileInfo dbFileInfo(dbPath);
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
        : m_settings(QStringLiteral("AVProject"), QStringLiteral("AvProjectUi"))
    {
        loadConfig();
        checkAndCreateDbPath();
    }
    
    // Delete copy and move constructors/operators to enforce singleton
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;
    AppConfig(AppConfig&&) = delete;
    AppConfig& operator=(AppConfig&&) = delete;
    
public: // Making m_settings public for simplicity in your code
    QSettings m_settings;
    
private:
    QString m_databasePath;
    QString m_virusTotalApiKey;
};

#endif // APPCONFIG_H