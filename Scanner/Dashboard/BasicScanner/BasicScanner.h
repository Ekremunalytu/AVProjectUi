#ifndef BASICSCANNER_H
#define BASICSCANNER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDirIterator>
#include <QQueue> // Added for QQueue
#include <QElapsedTimer>
#include <QDebug>
#include <QRegularExpression>
#include <QThread> // For QThread::msleep, consider alternatives for non-blocking delays

#include "Interface/IScanner.h" // Ensure this path is correct and IScanner is a Qt Interface
#include "Interface/ScannerTypes.h"


// Add Qt String Literal namespace for Qt 6 compatibility
using namespace Qt::StringLiterals;

// Forward declaration
class DbManager;

/**
 * @brief The BasicScanner class provides a simple implementation for scanning files
 *        against a local database of known malware signatures.
 */
class BasicScanner : public QObject, public IScanner {
    Q_OBJECT
    Q_INTERFACES(IScanner) // Added for Qt interface system

public:
    /**
     * @brief Constructs a BasicScanner with the given parent.
     * @param parent The parent QObject.
     * @param dbManager Pointer to the database manager (not owned by this class).
     */
    explicit BasicScanner(QObject* parent = nullptr, DbManager* dbManager = nullptr);
    
    /**
     * @brief Destructor for BasicScanner.
     */
    ~BasicScanner() override;
    
    /**
     * @brief Opens a file dialog for the user to select a file.
     * @return True if a file was successfully selected, false otherwise.
     */
    bool selectFile() override;

    /**
     * @brief Initiates a scan on the specified single file.
     * This will use the asynchronous scanning mechanism.
     * @param filePath The path to the file to scan.
     * @return True if the scan was initiated, false otherwise.
     */
    bool scanFile(const QString& filePath) override;

    /**
     * @brief Initiates a scan on all files within the specified directory.
     * @param directoryPath The path to the directory to scan.
     * @param recursive If true, subdirectories will also be scanned.
     * @return True if the directory scan was initiated, false otherwise.
     */
    Q_INVOKABLE bool startDirectoryScan(const QString& directoryPath, bool recursive = true);
    
    /**
     * @brief Retrieves information about the currently selected file.
     * @return QFileInfo for the currently selected file.
     */
    QFileInfo getSelectedFile() const override;
    
    /**
     * @brief Retrieves the scan results.
     * @return A formatted string containing the scan results.
     */
    QString getResults() const override;
    
    /**
     * @brief Checks if a scan is currently in progress.
     * @return True if a scan is in progress, false otherwise.
     */
    bool isScanning() const override;
    
    /**
     * @brief Cancels the current scan operation.
     * @return True if the scan was canceled, false otherwise.
     */
    bool cancelScan() override;
    
    /**
     * @brief Calculates the SHA256 hash of the specified file.
     * @param filePath The path to the file to hash.
     * @param errorCode Optional pointer to receive error code.
     * @return The SHA256 hash as a hex-encoded string, or an empty string if an error occurred.
     */
    QString calculateSha256(const QString& filePath, ScannerErrorCode* errorCode = nullptr);
    
    /**
     * @brief Checks if the given hash exists in the local database.
     * @param hash The SHA256 hash to check.
     * @param errorCode Optional pointer to receive error code.
     * @return True if the hash exists in the database, false otherwise.
     */
    bool checkHashInDatabase(const QString& hash, ScannerErrorCode* errorCode = nullptr);
    
    /**
     * @brief Gets a human-readable description of the last error.
     * @return Error message as a QString.
     */
    QString getLastError() const override;
    
    /**
     * @brief Sets the file path for scanning.
     * @param filePath The path to the file.
     */
    void setFile(const QString& filePath) override;
    
    /**
     * @brief Gets the currently set file path.
     * @return The file path as a QString.
     */
    QString getFile() const override;
    
    /**
     * @brief Gets the last error code that occurred during scanning.
     * @return The error code for the last error.
     */
    ScannerErrorCode getLastErrorCode() const;
    
    /**
     * @brief Gets a human-readable description of the last error.
     * @return Error message as a QString.
     */
    QString getLastErrorMessage() const;

signals:
    /**
     * @brief Signal emitted when scan results for a single file (from scanFile) are available.
     * @param results The scan results as a formatted string.
     * @param isMalicious True if the file was identified as malicious.
     */
    void scanResultsReady(const QString& results, bool isMalicious);
    
    /**
     * @brief Signal emitted when an error occurs during scanning.
     * @param errorCode The error code.
     * @param errorMessage The error message.
     */
    void scanError(ScannerErrorCode errorCode, const QString& errorMessage);

    /**
     * @brief Signal emitted when a directory scan operation starts.
     * @param directoryPath The path of the directory being scanned.
     */
    void directoryScanStarted(const QString& directoryPath);

    /**
     * @brief Signal emitted after each file in a directory scan is processed.
     * @param filePath The path of the file that was processed.
     * @param result A string indicating the outcome (e.g., "CLEAN", "MALICIOUS", "ERROR").
     * @param isMalicious True if the file was identified as malicious.
     * @param progressValue An integer from 0 to 100 indicating the overall progress of the directory scan.
     */
    void fileProcessed(const QString& filePath, const QString& result, bool isMalicious, int progressValue);

    /**
     * @brief Signal emitted when a directory scan operation finishes or is canceled.
     * @param directoryPath The path of the directory that was scanned.
     * @param filesScanned The total number of files scanned in the directory.
     * @param threatsFound The number of threats found. Value is -1 if scan was canceled.
     */
    void directoryScanFinished(const QString& directoryPath, int filesScanned, int threatsFound);

private slots:
    /**
     * @brief Processes the next file in the scan queue during a directory scan.
     */
    void processNextFileInQueue();
    
    /**
     * @brief Processes a single file scan asynchronously.
     * @param filePath The path to the file to scan.
     */
    void processSingleFile(const QString& filePath);

private:
    /**
     * @brief Sets the last error that occurred.
     * @param code The error code.
     * @param message The error message. If empty, a default message will be used.
     */
    void setLastError(ScannerErrorCode code, const QString& message = QString());
    
    /**
     * @brief Gets a default error message for the given error code.
     * @param code The error code.
     * @return A default error message.
     */
    QString getDefaultErrorMessage(ScannerErrorCode code) const;

    /**
     * @brief Performs the actual scanning logic for a single file.
     * @param filePath The path to the file to scan.
     * @param outResultString Formatted string with scan details.
     * @param outIsMalicious True if the file is determined to be malicious.
     * @return ScannerErrorCode indicating the outcome or error.
     */
    ScannerErrorCode performSingleFileScan(const QString& filePath, QString& outResultString, bool& outIsMalicious);

    QFileInfo m_selectedFile;
    QString m_results; // For single file scan results
    bool m_isScanning; // General scanning flag, might need refinement for concurrent scan types
    DbManager* m_dbManager; // Not owned by this class
    ScannerErrorCode m_lastError;
    QString m_lastErrorMessage;

    // Members for directory scanning
    QQueue<QString> m_scanQueue;
    int m_totalFilesToScan;
    int m_processedFilesCount;
    int m_threatsFoundInDirectory;
    bool m_isDirectoryScanActive;
    QString m_currentDirectoryPath;
    bool m_recursiveScan;
};

#endif //BASICSCANNER_H
