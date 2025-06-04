/**
 * @file VirusTotalManager.h
 * @brief VirusTotal API integration for malware detection
 * @author Ekrem Ünal
 * @version 1.0
 * @date 10.05.2025
 */

#ifndef VIRUSTOTALMANAGER_H
#define VIRUSTOTALMANAGER_H

#include <QString>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject> // Added for QObject inheritance
#include <atomic>
#include "../../Interface/IVirusTotalScanner.h"
#include "../../Core/AppConfig.h"

/**
 * @brief VirusTotal API integration manager for cloud-based malware detection
 * 
 * @details The VirusTotalManager class provides comprehensive integration with the
 * VirusTotal public API service, enabling users to submit files for analysis by
 * multiple antivirus engines simultaneously.
 * 
 * Key features:
 * - File submission to VirusTotal cloud service
 * - Real-time scanning status monitoring
 * - Comprehensive analysis results from 70+ antivirus engines
 * - Hash-based analysis for faster results
 * - Rate limiting and quota management
 * - Automatic API key management
 * 
 * @note Requires a valid VirusTotal API key for operation.
 *       Free tier has rate limits and usage quotas.
 * 
 * @warning Files submitted to VirusTotal are stored on their servers
 *          and may be shared with security researchers.
 */
class VirusTotalManager : public QObject, public IVirusTotalScanner { // Inherit from QObject
    Q_OBJECT // Add Q_OBJECT macro
public:
    /**
     * @brief Constructor for VirusTotalManager
     * 
     * Initializes the VirusTotal manager with the specified API key.
     * If no API key is provided, it attempts to load one from configuration.
     * 
     * @param apiKey VirusTotal API key for authentication. If empty, will attempt
     *               to load from configuration file or environment variables.
     */
    explicit VirusTotalManager(const QString& apiKey = QString());
    
    /**
     * @brief Destructor for VirusTotalManager
     * 
     * Cleans up network resources and cancels any pending requests.
     */
    virtual ~VirusTotalManager() = default;

    // IVirusTotalScanner interface implementation
    
    /**
     * @brief Submit the selected file to VirusTotal cloud service
     * 
     * Uploads the file to VirusTotal for analysis by multiple antivirus engines.
     * For files already known to VirusTotal (based on hash), returns existing results.
     * 
     * @param apiKey Optional API key to use for this submission. If empty, uses
     *               the API key configured during construction.
     * @return true if submission was successful, false if failed
     */
    bool submitToRemoteService(const QString& apiKey = QString()) override;
    
    /**
     * @brief Get the current status of the VirusTotal submission
     * 
     * Returns the current state of the file analysis including:
     * - Submission status (pending, analyzing, completed)
     * - Number of engines that have completed analysis
     * - Estimated completion time
     * 
     * @return String describing the current submission status
     */
    QString getSubmissionStatus() const override;
    
    // IScanner interface implementation
    
    /**
     * @brief Select a file for VirusTotal analysis
     * 
     * Opens a file dialog allowing users to select files for cloud-based scanning.
     * Supports all file types that VirusTotal can analyze.
     * 
     * @return true if file selection was successful, false otherwise
     */
    bool selectFile() override;
    
    /**
     * @brief Submit and analyze a file using VirusTotal
     * 
     * Initiates the cloud-based analysis process:
     * 1. Calculates file hash to check for existing results
     * 2. Uploads file if not previously analyzed
     * 3. Monitors analysis progress across multiple AV engines
     * 4. Retrieves comprehensive results
     * 
     * @param filePath Path to the file to be analyzed
     * @return true if analysis was initiated successfully, false if failed
     */
    bool scanFile(const QString& filePath) override;
    
    /**
     * @brief Get information about the selected file
     * 
     * @return QFileInfo object containing metadata about the file to be analyzed
     */
    QFileInfo getSelectedFile() const override;
    
    /**
     * @brief Get comprehensive analysis results from VirusTotal
     * 
     * Returns detailed analysis results including:
     * - Detection results from all antivirus engines
     * - File reputation and community feedback
     * - Behavioral analysis information
     * - Threat classifications and family names
     * 
     * @return Formatted string containing complete analysis results
     */
    QString getResults() const override;
    
    /**
     * @brief Check if a VirusTotal analysis is currently in progress
     * 
     * @return true if file is being analyzed by VirusTotal, false otherwise
     */
    bool isScanning() const override;
    
    /**
     * @brief Cancel the current VirusTotal analysis
     * 
     * Stops monitoring the current analysis and cancels any pending network requests.
     * 
     * @return true if cancellation was successful, false otherwise
     */
    bool cancelScan() override;
    
    /**
     * @brief Get the last error message from VirusTotal operations
     * 
     * @return String containing the most recent error, empty if no error occurred
     */
    QString getLastError() const override;
    
    /**
     * @brief Set the file to be analyzed by VirusTotal
     * 
     * @param filePath Path to the file that should be submitted for analysis
     */
    void setFile(const QString& filePath) override;
    
    /**
     * @brief Get the path of the currently set file
     * 
     * @return Path to the file currently configured for analysis
     */
    QString getFile() const override;
    
    /**
     * @brief Initiate file analysis using current configuration
     * 
     * Starts the VirusTotal analysis process for the currently selected file
     * using the configured API key and settings.
     * 
     * @return true if analysis was initiated successfully, false if failed
     */
    bool scan(); // Not in interface, so no override

    // Original methods
    /**
     * @brief Get detailed analysis report for a specific analysis ID
     * 
     * Retrieves the complete analysis report from VirusTotal using the
     * provided analysis identifier.
     * 
     * @param analysisId Unique identifier for the VirusTotal analysis
     * @return Formatted analysis report string
     */
    QString getAnalysisReport(const QString& analysisId);
    
    /**
     * @brief Start polling for analysis results
     * 
     * Begins periodic polling of VirusTotal API to check for analysis completion
     * and retrieve results when ready.
     * 
     * @param analysisId Unique identifier for the analysis to monitor
     */
    void startPollingForResults(const QString& analysisId);

signals:
    /**
     * @brief Signal emitted when VirusTotal analysis results are ready
     * 
     * This signal is emitted when the analysis has completed and comprehensive
     * results are available from VirusTotal's multiple antivirus engines.
     * 
     * @param results Formatted string containing the complete analysis results
     */
    void analysisResultsReady(const QString& results);

private:
    /**
     * @brief Validate the format of a VirusTotal API key
     * 
     * Checks if the provided API key matches the expected VirusTotal format
     * and basic validation criteria.
     * 
     * @param apiKey API key string to validate
     * @return true if the API key format is valid, false otherwise
     */
    bool isValidApiKeyFormat(const QString& apiKey) const;
    
    QString m_apiKey; ///< VirusTotal API key for authentication
    QString m_lastAnalysisId; ///< ID of the most recent analysis submission
    QString m_lastSubmissionStatus; ///< Status of the last submission attempt
    QString m_lastResults; ///< Cached results from the last completed analysis
    QString m_lastError; ///< Most recent error message from VirusTotal operations
    QFileInfo m_selectedFile; ///< Information about the currently selected file
    std::atomic<bool> m_isScanning; ///< Thread-safe flag indicating if scanning is in progress
    QNetworkAccessManager m_networkManager; ///< Network manager for HTTP requests to VirusTotal API
    QNetworkReply* m_currentReply = nullptr; ///< Current active network request
    QString m_currentPollingAnalysisId; ///< Analysis ID currently being polled for results
    int m_pollingAttempt = 0; ///< Current polling attempt counter for rate limiting
};

#endif //VIRUSTOTALMANAGER_H
