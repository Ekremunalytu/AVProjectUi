/**
 * @file BasicScanner.cpp
 * @brief Implementation of the BasicScanner class for scanning files against a malware database
 * @author Ekrem Ünal
 * @date 9.05.2025
 */

#include "BasicScanner.h"
#include "../../storage/database/DbManager/DbManager.h"
#include "../../storage/database/DatabaseService/DatabaseService.h"
#include <QFile>
#include <QDebug>
#include <system_error>

// Add Qt String Literal namespace for Qt 6 compatibility
using namespace Qt::StringLiterals;

/**
 * @brief Standard text constants for UI display
 */
namespace StandardText {
    // File statuses
    constexpr auto MALICIOUS = "MALICIOUS";
    constexpr auto CLEAN = "CLEAN";
    
    // Result templates
    constexpr auto FILE_LABEL = "File: %1\n";
    constexpr auto HASH_LABEL = "SHA256: %1\n";
    constexpr auto STATUS_MALICIOUS = "Status: MALICIOUS - Found in database";
    constexpr auto STATUS_CLEAN = "Status: CLEAN - Not found in database";
    
    // Error messages
    constexpr auto DB_NOT_AVAILABLE = "Database connection is not available.";
    constexpr auto SCAN_CANCELED = "Scan canceled.";
}

/**
 * @brief Constructs a BasicScanner with database access
 * 
 * If no DbManager is provided, it will attempt to get one from the DatabaseService.
 * 
 * @param parent The parent QObject for memory management
 * @param dbManager Pointer to database manager (not owned by this class)
 */
BasicScanner::BasicScanner(QObject* parent, DbManager* dbManager)
    : QObject(parent),
      m_isScanning(false),
      m_dbManager(dbManager),
      m_lastError(ScannerErrorCode::NoError)
{
    // If no DbManager provided, get from DatabaseService
    if (!m_dbManager) {
        auto& dbService = DatabaseService::getInstance();
        if (dbService.isDatabaseConnected()) {
            m_dbManager = dbService.getDbManager();
        } else {
            setLastError(ScannerErrorCode::DatabaseNotConnected);
        }
    }
}

/**
 * @brief Default destructor
 */
BasicScanner::~BasicScanner() = default;

/**
 * @brief Opens a file dialog for the user to select a file for scanning
 * 
 * Validates that the selected file exists and is readable.
 * 
 * @return True if file was successfully selected, false otherwise
 */
bool BasicScanner::selectFile()
{
    if (m_isScanning) {
        setLastError(ScannerErrorCode::ScanInProgress);
        return false;
    }
    
    // Reset error state
    setLastError(ScannerErrorCode::NoError);
    
    QString filePath = QFileDialog::getOpenFileName(nullptr, 
        tr("Select File to Scan"), 
        QDir::homePath(), 
        tr("All Files (*.*)"));
    
    if (filePath.isEmpty()) {
        return false;  // User canceled, not an error
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        setLastError(ScannerErrorCode::FileNotFound, 
                    tr("The file '%1' does not exist.").arg(filePath));
        return false;
    }
    
    if (!fileInfo.isReadable()) {
        setLastError(ScannerErrorCode::FileNotReadable,
                    tr("The file '%1' is not readable.").arg(filePath));
        return false;
    }
    
    m_selectedFile = fileInfo;
    return true;
}

/**
 * @brief Initiates a scan on the specified file path or the previously selected file
 * 
 * Checks file validity, calculates its SHA256 hash, and compares against the malware database.
 * Emits scanResultsReady when complete or scanError if an error occurs.
 * 
 * @param filePath Path to the file to scan. If empty, uses previously selected file
 * @return True if scan was initiated successfully, false if an error occurred
 */
bool BasicScanner::scanFile(const QString& filePath)
{
    // Reset error state
    setLastError(ScannerErrorCode::NoError);
    
    if (m_isScanning) {
        setLastError(ScannerErrorCode::ScanInProgress);
        return false;
    }
    
    if (!filePath.isEmpty()) {
        m_selectedFile = QFileInfo(filePath);
    }
    
    if (!m_selectedFile.exists() || !m_selectedFile.isFile()) {
        setLastError(ScannerErrorCode::FileNotFound,
                    tr("Invalid file selected: %1").arg(m_selectedFile.filePath()));
        m_results = tr("Invalid file selected.");
        emit scanResultsReady(m_results);
        emit scanError(getLastErrorCode(), getLastErrorMessage());
        return false;
    }
    
    if (!m_dbManager) {
        setLastError(ScannerErrorCode::DatabaseNotConnected,
                    tr(StandardText::DB_NOT_AVAILABLE));
        m_results = tr("Error: %1").arg(StandardText::DB_NOT_AVAILABLE);
        emit scanResultsReady(m_results);
        emit scanError(getLastErrorCode(), getLastErrorMessage());
        return false;
    }
    
    m_isScanning = true;
    
    // Calculate file hash
    ScannerErrorCode hashError = ScannerErrorCode::NoError;
    QString fileHash = calculateSha256(m_selectedFile.filePath(), &hashError);
    
    qDebug() << "Calculated hash:" << fileHash;
    
    if (fileHash.isEmpty()) {
        m_isScanning = false;
        // Error is already set by calculateSha256
        m_results = tr("Failed to calculate hash for file: %1").arg(m_selectedFile.fileName());
        emit scanResultsReady(m_results);
        emit scanError(getLastErrorCode(), getLastErrorMessage());
        return false;
    }
    
    // Check if hash exists in database
    ScannerErrorCode dbError = ScannerErrorCode::NoError;
    bool hashFound = checkHashInDatabase(fileHash, &dbError);
    
    qDebug() << "Hash found in database:" << hashFound;
    qDebug() << "Database error:" << (dbError != ScannerErrorCode::NoError ? "Yes" : "No");
    
    if (dbError != ScannerErrorCode::NoError) {
        m_isScanning = false;
        // Error is already set by checkHashInDatabase
        m_results = tr("Error checking database: %1").arg(getLastErrorMessage());
        emit scanResultsReady(m_results);
        emit scanError(getLastErrorCode(), getLastErrorMessage());
        return false;
    }
    
    // Prepare results
    m_results = tr(StandardText::FILE_LABEL).arg(m_selectedFile.fileName());
    m_results += tr(StandardText::HASH_LABEL).arg(fileHash);
    
    if (hashFound) {
        m_results += tr(StandardText::STATUS_MALICIOUS);
        emit scanError(ScannerErrorCode::MaliciousFileDetected, tr("Malicious file detected!"));
    } else {
        m_results += tr(StandardText::STATUS_CLEAN);
    }
    
    m_isScanning = false;
    emit scanResultsReady(m_results);
    return true;
}

/**
 * @brief Returns information about the currently selected file
 * @return QFileInfo object for the selected file
 */
QFileInfo BasicScanner::getSelectedFile() const
{
    return m_selectedFile;
}

/**
 * @brief Returns the scan results as a formatted string
 * @return Scan results string
 */
QString BasicScanner::getResults() const
{
    return m_results;
}

/**
 * @brief Checks if a scan is currently in progress
 * @return True if scanning, false otherwise
 */
bool BasicScanner::isScanning() const
{
    return m_isScanning;
}

/**
 * @brief Cancels an ongoing scan operation
 * @return True if scan was canceled, false if no scan was in progress
 */
bool BasicScanner::cancelScan()
{
    if (!m_isScanning) {
        return false;
    }
    
    m_isScanning = false;
    m_results = tr(StandardText::SCAN_CANCELED);
    emit scanResultsReady(m_results);
    return true;
}

/**
 * @brief Calculates the SHA256 hash of a file
 * 
 * Processes the file in chunks to handle large files efficiently.
 * 
 * @param filePath Path to the file to hash
 * @param errorCode Pointer to receive error code if an error occurs
 * @return SHA256 hash as hex string, or empty string on error
 */
QString BasicScanner::calculateSha256(const QString& filePath, ScannerErrorCode* errorCode)
{
    QFile file(filePath);
    if (!file.exists()) {
        setLastError(ScannerErrorCode::FileNotFound, 
                    tr("File does not exist: %1").arg(filePath));
        if (errorCode) *errorCode = getLastErrorCode();
        return QString();
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        setLastError(ScannerErrorCode::FileNotReadable,
                    tr("Failed to open file for hash calculation: %1").arg(file.errorString()));
        qWarning() << getLastErrorMessage();
        if (errorCode) *errorCode = getLastErrorCode();
        return QString();
    }
    
    QCryptographicHash hasher(QCryptographicHash::Sha256);
    
    // Process file in chunks to handle large files
    constexpr qint64 bufferSize = 8192; // 8 KB
    QByteArray buffer;
    
    while (!file.atEnd()) {
        buffer = file.read(bufferSize);
        if (buffer.isEmpty() && file.error() != QFile::NoError) {
            setLastError(ScannerErrorCode::HashCalculationFailed,
                        tr("Error reading file during hash calculation: %1").arg(file.errorString()));
            file.close();
            if (errorCode) *errorCode = getLastErrorCode();
            return QString();
        }
        hasher.addData(buffer);
    }
    
    file.close();
    
    if (errorCode) *errorCode = ScannerErrorCode::NoError;
    return QString::fromLatin1(hasher.result().toHex());
}

/**
 * @brief Checks if a hash exists in the malware database
 * 
 * @param hash SHA256 hash to check
 * @param errorCode Pointer to receive error code if an error occurs
 * @return True if hash found in database, false otherwise
 */
bool BasicScanner::checkHashInDatabase(const QString& hash, ScannerErrorCode* errorCode)
{
    if (!m_dbManager) {
        setLastError(ScannerErrorCode::DatabaseNotConnected,
                    tr("No database manager available when checking hash"));
        qWarning() << getLastErrorMessage();
        if (errorCode) *errorCode = getLastErrorCode();
        return false;
    }
    
    std::error_code ec;
    bool exists = m_dbManager->isSha256Exists(hash, ec);
    
    if (ec) {
        setLastError(ScannerErrorCode::DatabaseQueryFailed,
                    tr("Error checking hash in database: %1 (code: %2, category: %3)")
                        .arg(ec.message().c_str())
                        .arg(ec.value())
                        .arg(QString::fromStdString(ec.category().name())));
        qWarning() << getLastErrorMessage();
        if (errorCode) *errorCode = getLastErrorCode();
        return false;
    }
    
    if (errorCode) *errorCode = ScannerErrorCode::NoError;
    return exists;
}

/**
 * @brief Returns a human-readable message for the last error
 * 
 * If a custom error message was set, returns that. Otherwise,
 * returns a default message for the error code.
 * 
 * @return Error message string
 */
QString BasicScanner::getLastError() const
{
    if (!m_lastErrorMessage.isEmpty()) {
        return m_lastErrorMessage;
    }
    
    return getDefaultErrorMessage(m_lastError);
}

/**
 * @brief Returns the error code for the last error that occurred
 * @return Error code
 */
ScannerErrorCode BasicScanner::getLastErrorCode() const
{
    return m_lastError;
}

/**
 * @brief Sets the file to be scanned
 * @param filePath Path to the file to set for scanning
 */
void BasicScanner::setFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        setLastError(ScannerErrorCode::FileNotFound,
                    tr("File does not exist or is not a valid file: %1").arg(filePath));
        return;
    }
    
    if (!fileInfo.isReadable()) {
        setLastError(ScannerErrorCode::FileNotReadable,
                    tr("The file '%1' is not readable.").arg(filePath));
        return;
    }
    
    m_selectedFile = fileInfo;
    setLastError(ScannerErrorCode::NoError);
}

/**
 * @brief Gets the currently set file path
 * @return Path to the currently set file, or empty string if no file is set
 */
QString BasicScanner::getFile() const
{
    return m_selectedFile.filePath();
}

/**
 * @brief Returns a human-readable message for the last error
 * 
 * If a custom error message was set, returns that. Otherwise,
 * returns a default message for the error code.
 * 
 * @return Error message string
 */
QString BasicScanner::getLastErrorMessage() const
{
    if (!m_lastErrorMessage.isEmpty()) {
        return m_lastErrorMessage;
    }
    
    return getDefaultErrorMessage(m_lastError);
}

/**
 * @brief Sets the last error code and optional custom message
 * 
 * @param code Error code to set
 * @param message Optional custom error message
 */
void BasicScanner::setLastError(ScannerErrorCode code, const QString& message)
{
    m_lastError = code;
    m_lastErrorMessage = message;
    
    if (code != ScannerErrorCode::NoError && message.isEmpty()) {
        m_lastErrorMessage = getDefaultErrorMessage(code);
    }
}

/**
 * @brief Returns a default error message for the given error code
 * 
 * @param code Error code to get message for
 * @return Default error message for the code
 */
QString BasicScanner::getDefaultErrorMessage(ScannerErrorCode code) const
{
    switch (code) {
        case ScannerErrorCode::NoError:
            return tr("No error");
        case ScannerErrorCode::DatabaseNotConnected:
            return tr("Database is not connected");
        case ScannerErrorCode::FileNotFound:
            return tr("File not found");
        case ScannerErrorCode::FileNotReadable:
            return tr("File is not readable");
        case ScannerErrorCode::ScanInProgress:
            return tr("Scan is already in progress");
        case ScannerErrorCode::HashCalculationFailed:
            return tr("Failed to calculate file hash");
        case ScannerErrorCode::DatabaseQueryFailed:
            return tr("Database query failed");
        case ScannerErrorCode::InvalidInput:
            return tr("Invalid input provided");
        case ScannerErrorCode::Unknown:
        default:
            return tr("Unknown error");
    }
}
