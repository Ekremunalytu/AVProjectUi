#ifndef BASICSCANNER_H
#define BASICSCANNER_H

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QFileDialog>
#include <memory>
#include <system_error>
#include "Interface/IScanner.h"
#include "Interface/ScannerTypes.h"

// Add Qt String Literal namespace for Qt 6 compatibility
using namespace Qt::StringLiterals;

// Forward declaration
class DbManager;

/**
 * @brief Error codes for BasicScanner operations
 */
enum class ScannerErrorCode {
    NoError = 0,
    DatabaseNotConnected,
    FileNotFound,
    FileNotReadable,
    ScanInProgress,
    HashCalculationFailed,
    DatabaseQueryFailed,
    InvalidInput,
    MaliciousFileDetected,
    Unknown
};

/**
 * @brief The BasicScanner class provides a simple implementation for scanning files
 *        against a local database of known malware signatures.
 */
class BasicScanner : public QObject, public IScanner {
    Q_OBJECT

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
     * @brief Initiates a scan on the specified file.
     * @param filePath The path to the file to scan.
     * @return True if the scan was initiated, false otherwise.
     */
    bool scanFile(const QString& filePath) override;
    
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
     * @brief Signal emitted when scan results are available.
     * @param results The scan results as a formatted string.
     */
    void scanResultsReady(const QString& results);
    
    /**
     * @brief Signal emitted when an error occurs during scanning.
     * @param errorCode The error code.
     * @param errorMessage The error message.
     */
    void scanError(ScannerErrorCode errorCode, const QString& errorMessage);

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

    QFileInfo m_selectedFile; ///< Information about the currently selected file
    QString m_results; ///< Formatted results from the last scan operation
    bool m_isScanning; ///< Flag indicating if a scan is currently in progress
    DbManager* m_dbManager; ///< Pointer to database manager (not owned by this class)
    ScannerErrorCode m_lastError; ///< Last error code that occurred during scanning
    QString m_lastErrorMessage; ///< Human-readable message for the last error
};

#endif //BASICSCANNER_H
