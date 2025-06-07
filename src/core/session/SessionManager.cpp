#include "SessionManager.h"
#include <QMutexLocker>

SessionManager& SessionManager::getInstance()
{
    static SessionManager instance;
    return instance;
}

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
{
    // Initialize counters to 0 (already done by atomic initialization)
}

int SessionManager::getScannedFilesCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_scannedFilesCount.load();
}

int SessionManager::getCdrProcessedCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_cdrProcessedCount.load();
}

int SessionManager::getVirusTotalScansCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_virusTotalScansCount.load();
}

int SessionManager::getYaraMatchesCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_yaraMatchesCount.load();
}

void SessionManager::incrementScannedFiles()
{
    m_scannedFilesCount.fetch_add(1);
}

void SessionManager::incrementCdrProcessed()
{
    m_cdrProcessedCount.fetch_add(1);
}

void SessionManager::incrementVirusTotalScans()
{
    m_virusTotalScansCount.fetch_add(1);
}

void SessionManager::incrementYaraMatches()
{
    m_yaraMatchesCount.fetch_add(1);
}

void SessionManager::resetCounters()
{
    QMutexLocker locker(&m_mutex);
    m_scannedFilesCount.store(0);
    m_cdrProcessedCount.store(0);
    m_virusTotalScansCount.store(0);
    m_yaraMatchesCount.store(0);
}