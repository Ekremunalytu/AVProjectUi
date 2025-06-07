#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QMutex>
#include <atomic>

/**
 * @brief The SessionManager class manages session-specific counters and statistics
 * 
 * This singleton class tracks various session statistics like scanned files,
 * processed files, CDR operations, etc.
 */
class SessionManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the SessionManager instance
     */
    static SessionManager& getInstance();

    /**
     * @brief Get the number of files scanned in current session
     * @return Number of scanned files
     */
    int getScannedFilesCount() const;

    /**
     * @brief Get the number of files processed by CDR in current session
     * @return Number of CDR processed files
     */
    int getCdrProcessedCount() const;

    /**
     * @brief Get the number of VirusTotal scans in current session
     * @return Number of VirusTotal scans
     */
    int getVirusTotalScansCount() const;

    /**
     * @brief Get the number of YARA matches in current session
     * @return Number of YARA matches
     */
    int getYaraMatchesCount() const;

    /**
     * @brief Increment scanned files counter
     */
    void incrementScannedFiles();

    /**
     * @brief Increment CDR processed files counter
     */
    void incrementCdrProcessed();

    /**
     * @brief Increment VirusTotal scans counter
     */
    void incrementVirusTotalScans();

    /**
     * @brief Increment YARA matches counter
     */
    void incrementYaraMatches();

    /**
     * @brief Reset all session counters
     */
    void resetCounters();

private:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager() = default;

    // Delete copy constructor and assignment operator
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    std::atomic<int> m_scannedFilesCount{0};
    std::atomic<int> m_cdrProcessedCount{0};
    std::atomic<int> m_virusTotalScansCount{0};
    std::atomic<int> m_yaraMatchesCount{0};

    mutable QMutex m_mutex;
};

#endif // SESSIONMANAGER_H