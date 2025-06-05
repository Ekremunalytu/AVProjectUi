/**
 * @file SettingsManager.h
 * @brief Settings management for the application
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QSettings>
#include <QObject>
#include <QString>
#include <QVariant>
#include <memory>

/**
 * @brief Manages application settings and configuration
 * 
 * This class provides a centralized way to manage application settings
 * including scanning preferences, API keys, UI themes, and other configuration options.
 */
class SettingsManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Get the singleton instance of SettingsManager
     * @return Reference to the SettingsManager instance
     */
    static SettingsManager& getInstance();

    /**
     * @brief Destructor
     */
    ~SettingsManager();

    // General Settings
    void setValue(const QString& key, const QVariant& value);
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;
    bool contains(const QString& key) const;
    void remove(const QString& key);

    // Specific setting getters/setters
    QString getVirusTotalApiKey() const;
    void setVirusTotalApiKey(const QString& apiKey);

    QString getLanguage() const;
    void setLanguage(const QString& language);

    QString getTheme() const;
    void setTheme(const QString& theme);

    bool getAutoScanEnabled() const;
    void setAutoScanEnabled(bool enabled);

    int getScanTimeout() const;
    void setScanTimeout(int timeoutSeconds);

    bool getNetworkMonitoringEnabled() const;
    void setNetworkMonitoringEnabled(bool enabled);

    QString getQuarantinePath() const;
    void setQuarantinePath(const QString& path);

    // Database settings
    QString getDatabasePath() const;
    void setDatabasePath(const QString& path);

    // Advanced settings
    bool getDebugModeEnabled() const;
    void setDebugModeEnabled(bool enabled);

    int getMaxScanThreads() const;
    void setMaxScanThreads(int threads);

signals:
    /**
     * @brief Emitted when a setting value changes
     * @param key The setting key that changed
     * @param value The new value
     */
    void settingChanged(const QString& key, const QVariant& value);

public slots:
    /**
     * @brief Reset all settings to default values
     */
    void resetToDefaults();

    /**
     * @brief Save all settings to persistent storage
     */
    void sync();

private:
    explicit SettingsManager(QObject* parent = nullptr);
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    void loadDefaults();

    std::unique_ptr<QSettings> m_settings;

    // Default values
    static const QString DEFAULT_LANGUAGE;
    static const QString DEFAULT_THEME;
    static const bool DEFAULT_AUTO_SCAN;
    static const int DEFAULT_SCAN_TIMEOUT;
    static const bool DEFAULT_NETWORK_MONITORING;
    static const bool DEFAULT_DEBUG_MODE;
    static const int DEFAULT_MAX_THREADS;
};

#endif // SETTINGSMANAGER_H
