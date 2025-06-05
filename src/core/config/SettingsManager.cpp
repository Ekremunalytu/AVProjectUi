/**
 * @file SettingsManager.cpp
 * @brief Implementation of SettingsManager class
 */

#include "SettingsManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

// Default values
const QString SettingsManager::DEFAULT_LANGUAGE = "tr_TR";
const QString SettingsManager::DEFAULT_THEME = "dark";
const bool SettingsManager::DEFAULT_AUTO_SCAN = true;
const int SettingsManager::DEFAULT_SCAN_TIMEOUT = 30;
const bool SettingsManager::DEFAULT_NETWORK_MONITORING = false;
const bool SettingsManager::DEFAULT_DEBUG_MODE = false;
const int SettingsManager::DEFAULT_MAX_THREADS = 4;

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
{
    // Create settings with organization and application name
    QCoreApplication::setOrganizationName("AVProjectUi");
    QCoreApplication::setApplicationName("AVProjectUi");
    
    m_settings = std::make_unique<QSettings>(QSettings::IniFormat, QSettings::UserScope, 
                                           "AVProjectUi", "AVProjectUi");
    
    // Ensure config directory exists
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir().mkpath(configDir + "/AVProjectUi");
    
    loadDefaults();
}

SettingsManager::~SettingsManager()
{
    sync();
}

SettingsManager& SettingsManager::getInstance()
{
    static SettingsManager instance;
    return instance;
}

void SettingsManager::setValue(const QString& key, const QVariant& value)
{
    QVariant oldValue = getValue(key);
    m_settings->setValue(key, value);
    
    if (oldValue != value) {
        emit settingChanged(key, value);
    }
}

QVariant SettingsManager::getValue(const QString& key, const QVariant& defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

bool SettingsManager::contains(const QString& key) const
{
    return m_settings->contains(key);
}

void SettingsManager::remove(const QString& key)
{
    m_settings->remove(key);
    emit settingChanged(key, QVariant());
}

QString SettingsManager::getVirusTotalApiKey() const
{
    return getValue("VirusTotal/ApiKey", "").toString();
}

void SettingsManager::setVirusTotalApiKey(const QString& apiKey)
{
    setValue("VirusTotal/ApiKey", apiKey);
}

QString SettingsManager::getLanguage() const
{
    return getValue("General/Language", DEFAULT_LANGUAGE).toString();
}

void SettingsManager::setLanguage(const QString& language)
{
    setValue("General/Language", language);
}

QString SettingsManager::getTheme() const
{
    return getValue("UI/Theme", DEFAULT_THEME).toString();
}

void SettingsManager::setTheme(const QString& theme)
{
    setValue("UI/Theme", theme);
}

bool SettingsManager::getAutoScanEnabled() const
{
    return getValue("Scanning/AutoScanEnabled", DEFAULT_AUTO_SCAN).toBool();
}

void SettingsManager::setAutoScanEnabled(bool enabled)
{
    setValue("Scanning/AutoScanEnabled", enabled);
}

int SettingsManager::getScanTimeout() const
{
    return getValue("Scanning/TimeoutSeconds", DEFAULT_SCAN_TIMEOUT).toInt();
}

void SettingsManager::setScanTimeout(int timeoutSeconds)
{
    setValue("Scanning/TimeoutSeconds", timeoutSeconds);
}

bool SettingsManager::getNetworkMonitoringEnabled() const
{
    return getValue("Network/MonitoringEnabled", DEFAULT_NETWORK_MONITORING).toBool();
}

void SettingsManager::setNetworkMonitoringEnabled(bool enabled)
{
    setValue("Network/MonitoringEnabled", enabled);
}

QString SettingsManager::getQuarantinePath() const
{
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/AVProjectUi/Quarantine";
    return getValue("Scanning/QuarantinePath", defaultPath).toString();
}

void SettingsManager::setQuarantinePath(const QString& path)
{
    setValue("Scanning/QuarantinePath", path);
}

QString SettingsManager::getDatabasePath() const
{
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/database.db";
    return getValue("Database/Path", defaultPath).toString();
}

void SettingsManager::setDatabasePath(const QString& path)
{
    setValue("Database/Path", path);
}

bool SettingsManager::getDebugModeEnabled() const
{
    return getValue("Advanced/DebugMode", DEFAULT_DEBUG_MODE).toBool();
}

void SettingsManager::setDebugModeEnabled(bool enabled)
{
    setValue("Advanced/DebugMode", enabled);
}

int SettingsManager::getMaxScanThreads() const
{
    return getValue("Advanced/MaxThreads", DEFAULT_MAX_THREADS).toInt();
}

void SettingsManager::setMaxScanThreads(int threads)
{
    setValue("Advanced/MaxThreads", threads);
}

void SettingsManager::resetToDefaults()
{
    m_settings->clear();
    loadDefaults();
    emit settingChanged("", QVariant()); // Signal that all settings changed
}

void SettingsManager::sync()
{
    m_settings->sync();
}

void SettingsManager::loadDefaults()
{
    // Only set defaults if they don't exist
    if (!contains("General/Language")) {
        setValue("General/Language", DEFAULT_LANGUAGE);
    }
    if (!contains("UI/Theme")) {
        setValue("UI/Theme", DEFAULT_THEME);
    }
    if (!contains("Scanning/AutoScanEnabled")) {
        setValue("Scanning/AutoScanEnabled", DEFAULT_AUTO_SCAN);
    }
    if (!contains("Scanning/TimeoutSeconds")) {
        setValue("Scanning/TimeoutSeconds", DEFAULT_SCAN_TIMEOUT);
    }
    if (!contains("Network/MonitoringEnabled")) {
        setValue("Network/MonitoringEnabled", DEFAULT_NETWORK_MONITORING);
    }
    if (!contains("Advanced/DebugMode")) {
        setValue("Advanced/DebugMode", DEFAULT_DEBUG_MODE);
    }
    if (!contains("Advanced/MaxThreads")) {
        setValue("Advanced/MaxThreads", DEFAULT_MAX_THREADS);
    }
    
    // Load VirusTotal API key from config.ini if it exists and not set
    if (!contains("VirusTotal/ApiKey")) {
        QSettings iniFile(":/core/config/config.ini", QSettings::IniFormat);
        QString apiKey = iniFile.value("VirusTotal/ApiKey", "").toString();
        if (!apiKey.isEmpty()) {
            setValue("VirusTotal/ApiKey", apiKey);
        }
    }
}
