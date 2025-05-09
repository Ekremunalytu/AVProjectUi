#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>
#include <QSettings>
#include <QDebug>
#include <QDir>

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
    void setDatabasePath(const QString& path) {
        m_databasePath = path;
        m_settings.setValue(u"Database/Path"_qs, path);
    }
    
    /**
     * @brief Load configuration from settings file.
     */
    void loadConfig() {
        m_databasePath = m_settings.value(u"Database/Path"_qs, u"./MalwareHashes/identifier.sqlite"_qs).toString();
        
        // Ensure the database path exists
        QDir dbDir = QFileInfo(m_databasePath).dir();
        if (!dbDir.exists()) {
            qDebug() << "Creating database directory:" << dbDir.path();
            if (!dbDir.mkpath(u"."_qs)) {
                qWarning() << "Failed to create database directory:" << dbDir.path();
            }
        }
    }

private:
    // Private constructor for singleton pattern
    AppConfig()
        : m_settings(u"AVProject"_qs, u"AvProjectUi"_qs)
    {
        loadConfig();
    }
    
    // Delete copy and move constructors/operators to enforce singleton
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;
    AppConfig(AppConfig&&) = delete;
    AppConfig& operator=(AppConfig&&) = delete;
    
    QSettings m_settings;
    QString m_databasePath;
};

#endif // APPCONFIG_H 