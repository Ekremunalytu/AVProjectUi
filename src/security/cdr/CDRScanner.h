/**
 * @file CDRScanner.h
 * @brief CDR (Content Disarm and Reconstruction) Scanner implementation
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 */

#ifndef CDR_SCANNER_H
#define CDR_SCANNER_H

#include "../../core/interfaces/IDockerScanner.h"
#include "../../core/interfaces/ScannerTypes.h"
#include "../../infrastructure/docker/DockerManager.h" // Assuming DockerManager.h is the main header for your Docker library
#include <QObject>
#include <QString>
#include <memory> // For std::unique_ptr
#include <QFileInfo>

// Forward declaration
enum class ScannerErrorCode;

// Include CDR specific headers
#include "CdrManager.h"
#include "CdrTypes.h"

/**
 * @brief CDR Scanner implementation for content disarm and reconstruction
 * 
 * The CDRScanner class provides functionality to scan and sanitize files using
 * Content Disarm and Reconstruction (CDR) technology. It runs in a Docker container
 * to provide isolation and security during the scanning process.
 * 
 * @details CDR scanning involves:
 * - Extracting active content from files
 * - Removing potentially malicious elements
 * - Reconstructing safe versions of the files
 * - Providing detailed analysis reports
 * 
 * @note This scanner requires Docker to be installed and running on the system.
 */
class CDRScanner : public QObject, public IDockerScanner {
    Q_OBJECT
    
public:
    /**
     * @brief Constructor for CDRScanner
     * 
     * Initializes the CDR scanner with default settings and prepares
     * the Docker environment for CDR operations.
     * 
     * @param parent The parent QObject (optional)
     */
    explicit CDRScanner(QObject *parent = nullptr);
    
    /**
     * @brief Destructor for CDRScanner
     * 
     * Cleans up resources and ensures proper shutdown of Docker containers.
     */
    ~CDRScanner() override = default;

    // IScanner interface methods
    
    /**
     * @brief Select a file for CDR scanning
     * 
     * Opens a file dialog to allow the user to select a file for scanning.
     * Supports various document formats including Office documents, PDFs, etc.
     * 
     * @return true if file selection was successful, false otherwise
     */
    bool selectFile() override;
    
    /**
     * @brief Scan a file using CDR technology
     * 
     * Performs Content Disarm and Reconstruction on the specified file.
     * The process involves extracting content, removing threats, and 
     * reconstructing a safe version of the file.
     * 
     * @param filePath Path to the file to be scanned
     * @return true if scanning completed successfully, false if failed
     */
    bool scanFile(const QString& filePath) override;
    
    /**
     * @brief Get information about the selected file
     * 
     * @return QFileInfo object containing details about the selected file
     */
    QFileInfo getSelectedFile() const override;
    
    /**
     * @brief Get the results of the last scan operation
     * 
     * Returns detailed information about the CDR process including:
     * - Threats found and removed
     * - File reconstruction status
     * - Performance metrics
     * 
     * @return Formatted string containing scan results
     */
    QString getResults() const override;
    
    /**
     * @brief Check if a scan operation is currently in progress
     * 
     * @return true if scanning is active, false otherwise
     */
    bool isScanning() const override;
    
    /**
     * @brief Cancel the current scan operation
     * 
     * Attempts to stop the ongoing CDR process and clean up resources.
     * 
     * @return true if cancellation was successful, false otherwise
     */
    bool cancelScan() override;
    
    /**
     * @brief Get the last error message
     * 
     * @return String containing the most recent error message, empty if no error
     */
    QString getLastError() const override;
    
    /**
     * @brief Get the scanner type
     * 
     * @return ScannerType enum value indicating this is a CDR scanner
     */
    ScannerType getType() const;
    
    /**
     * @brief Get the current scan status
     * 
     * @return ScanStatus enum indicating the current state of the scanner
     */
    ScanStatus getStatus() const;
    
    /**
     * @brief Set the file to be scanned
     * 
     * @param filePath Path to the file that should be scanned
     */
    void setFile(const QString& filePath) override;
    QString getFile() const override;

    // IDockerScanner interface methods
    bool submitToContainer(const QString& containerName) override;
    bool isContainerReady() const override;
    QString getContainerStatus() const override;
    
    // CDRScanner specific methods
    /**
     * @brief Get the path to the sanitized file after CDR processing
     * 
     * @return Path to the processed and sanitized file, empty if no file has been processed
     */
    QString getSanitizedFilePath() const; // Get the path of the processed file
    
    /**
     * @brief Check if threats were detected during the last scan
     * 
     * @return true if threats were found and removed, false otherwise
     */
    bool getThreatsDetected() const { return threatsDetected; }
    
    /**
     * @brief Check if the file was sanitized during the last scan
     * 
     * @return true if the file was modified/sanitized, false if no changes were made
     */
    bool getWasSanitized() const { return wasSanitized; }
    
    /**
     * @brief Get detailed analysis information from the CDR process
     * 
     * @return String containing detailed analysis results and statistics
     */
    QString getAnalysisDetails() const { return analysisDetails; }

    // CDR configuration methods
    /**
     * @brief Set the CDR configuration parameters
     * 
     * @param config CDR configuration object containing scan settings
     */
    void setCdrConfiguration(const CDR::CdrConfiguration& config) { cdrConfig = config; }
    
    /**
     * @brief Get the current CDR configuration
     * 
     * @return Current CDR configuration object
     */
    CDR::CdrConfiguration getCdrConfiguration() const { return cdrConfig; }
    
    /**
     * @brief Set the output directory for sanitized files
     * 
     * @param outputDir Path to the directory where sanitized files will be saved
     */
    void setOutputDirectory(const QString& outputDir) { cdrConfig.outputDirectory = outputDir.toStdString(); }
    
    /**
     * @brief Set the quarantine directory for dangerous files
     * 
     * @param quarantineDir Path to the directory where quarantined files will be stored
     */
    void setQuarantineDirectory(const QString& quarantineDir) { cdrConfig.quarantineDirectory = quarantineDir.toStdString(); }

private:
    std::unique_ptr<Docker::DockerManager> dockerManager; ///< Docker management instance for container operations
    std::unique_ptr<CDR::CdrManager> cdrManager; ///< CDR manager for proper CDR operations
    
    QString currentFilePath; ///< Path to the currently selected file for scanning
    QString lastError; ///< Most recent error message from scan operations
    ScanStatus currentStatus; ///< Current status of the scanner
    QString sanitizedFilePath; ///< Store path to the sanitized file
    QString cdrContainerName; ///< Name of the CDR Docker container
    QString cdrImageName; ///< Name of the CDR Docker image
    
    // CDR configuration
    CDR::CdrConfiguration cdrConfig; ///< Configuration settings for CDR operations
    
    // CDR analysis results
    bool threatsDetected = false; ///< Flag indicating if threats were detected in last scan
    bool wasSanitized = false; ///< Flag indicating if file was sanitized in last scan
    QString analysisDetails; ///< Detailed analysis results from CDR process
    
    // Helper methods
    /**
     * @brief Initialize CDR components and configurations
     * 
     * Sets up the CDR manager and Docker components needed for scanning operations.
     */
    void initializeCdrComponents();
    
    /**
     * @brief Ensure the CDR container is running and ready
     * 
     * @return true if container is running or successfully started, false otherwise
     */
    bool ensureCdrContainerRunning();
    
    /**
     * @brief Create a new CDR container instance
     * 
     * @return true if container creation was successful, false otherwise
     */
    bool createCdrContainer();
    
    /**
     * @brief Check if the CDR container is currently running
     * 
     * @return true if the CDR container is active and running, false otherwise
     */
    bool isCdrContainerRunning() const;

signals:
    /**
     * @brief Signal emitted when CDR scan results are available.
     * @param results The scan results as a formatted string.
     */
    void scanResultsReady(const QString& results);
    
    /**
     * @brief Signal emitted when an error occurs during CDR scanning.
     * @param errorCode The error code.
     * @param errorMessage The error message.
     */
    void scanError(ScannerErrorCode errorCode, const QString& errorMessage);
};

#endif // CDR_SCANNER_H
