/**
 * @file BasicScanner.cpp
 * @brief Implementation of the BasicScanner class for scanning files against a malware database
 * @author Ekrem Ünal
 * @date 9.05.2025
 */

#include "BasicScanner.h"
#include "Database/DbManager/DbManager.h"
#include "Database/DatabaseService/DatabaseService.h"
#include <QFile>
#include <QFileDialog> // Added for QFileDialog
#include <QCryptographicHash> // Added for QCryptographicHash
#include <QDirIterator> // Added for directory scanning
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
      m_isScanning(false), // General flag, might need more granular control
      m_dbManager(dbManager),
      m_lastError(ScannerErrorCode::NoError),
      m_totalFilesToScan(0),
      m_processedFilesCount(0),
      m_threatsFoundInDirectory(0),
      m_isDirectoryScanActive(false),
      m_recursiveScan(true)
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
    setLastError(ScannerErrorCode::NoError);

    if (m_isScanning || m_isDirectoryScanActive) { // Prevent concurrent scans
        setLastError(ScannerErrorCode::ScanInProgress);
        emit scanError(getLastErrorCode(), getLastErrorMessage());
        return false;
    }

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        setLastError(ScannerErrorCode::FileNotFound,
                    tr("Invalid file selected: %1").arg(filePath));
        m_results = tr("Invalid file selected.");
        emit scanResultsReady(m_results, false); 
        emit scanError(getLastErrorCode(), getLastErrorMessage());
        return false;
    }

    if (!m_dbManager) {
        setLastError(ScannerErrorCode::DatabaseNotConnected,
                    tr(StandardText::DB_NOT_AVAILABLE));
        m_results = tr("Error: %1").arg(StandardText::DB_NOT_AVAILABLE);
        emit scanResultsReady(m_results, false);
        emit scanError(getLastErrorCode(), getLastErrorMessage());
        return false;
    }

    m_isScanning = true; // Indicate a scan is active

    // Process single file directly without queue mechanism
    QMetaObject::invokeMethod(this, "processSingleFile", Qt::QueuedConnection, Q_ARG(QString, filePath));

    return true;
}

bool BasicScanner::startDirectoryScan(const QString& directoryPath, bool recursive)
{
    setLastError(ScannerErrorCode::NoError);

    if (m_isScanning || m_isDirectoryScanActive) { // Prevent concurrent scans
        setLastError(ScannerErrorCode::ScanInProgress);
        emit scanError(getLastErrorCode(), tr("Another scan operation is already in progress."));
        return false;
    }

    QDir dir(directoryPath);
    if (!dir.exists()) {
        setLastError(ScannerErrorCode::FileNotFound, tr("Directory not found: %1").arg(directoryPath));
        emit scanError(getLastErrorCode(), getLastErrorMessage());
        return false;
    }

    m_isDirectoryScanActive = true;
    m_isScanning = true; // General flag
    m_currentDirectoryPath = directoryPath;
    m_recursiveScan = recursive;
    m_scanQueue.clear();
    m_processedFilesCount = 0;
    m_threatsFoundInDirectory = 0;

    emit directoryScanStarted(m_currentDirectoryPath);

    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags;
    if (m_recursiveScan) {
        flags |= QDirIterator::Subdirectories;
    }

    QDirIterator it(directoryPath, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, flags);
    while (it.hasNext()) {
        m_scanQueue.enqueue(it.next());
    }
    m_totalFilesToScan = m_scanQueue.size();

    if (m_totalFilesToScan == 0) {
        m_isDirectoryScanActive = false;
        m_isScanning = false;
        emit directoryScanFinished(m_currentDirectoryPath, 0, 0);
        return true; // No files to scan
    }

    // Start processing the queue
    QMetaObject::invokeMethod(this, "processNextFileInQueue", Qt::QueuedConnection);
    return true;
}

void BasicScanner::processNextFileInQueue()
{
    if (!m_isDirectoryScanActive || m_scanQueue.isEmpty()) {
        bool wasSingleFileScan = (m_totalFilesToScan == 1 && m_processedFilesCount == 1 && !m_currentDirectoryPath.isEmpty() && m_scanQueue.isEmpty());

        if (wasSingleFileScan && m_isScanning) { // Check m_isScanning to ensure it was the scanFile context
             // The old scanResultsReady is now effectively handled by fileProcessed for the single file.
             // We can emit the original scanResultsReady here if strict backward compatibility for its exact signature is needed.
             // For now, assuming the new fileProcessed is sufficient for UI updates.
        }
        
        if (m_isDirectoryScanActive) { // Ensure this is for a directory scan or the adapted single file scan
             emit directoryScanFinished(m_currentDirectoryPath, m_processedFilesCount, m_threatsFoundInDirectory);
        }
        m_isDirectoryScanActive = false;
        m_isScanning = false; // Reset general flag
        return;
    }

    QString filePath = m_scanQueue.dequeue();
    m_processedFilesCount++;

    QString resultString;
    bool isMalicious = false;
    
    // Perform the actual scan for the current file
    ScannerErrorCode fileScanError = performSingleFileScan(filePath, resultString, isMalicious);

    if (isMalicious) {
        m_threatsFoundInDirectory++;
    }

    int progress = 0;
    if (m_totalFilesToScan > 0) {
        progress = static_cast<int>((static_cast<double>(m_processedFilesCount) / m_totalFilesToScan) * 100.0);
    }
    
    // Determine result category string for fileProcessed signal
    QString resultCategory;
    if (fileScanError == ScannerErrorCode::NoError) {
        resultCategory = QString::fromUtf8(isMalicious ? StandardText::MALICIOUS : StandardText::CLEAN);
    } else if (fileScanError == ScannerErrorCode::MaliciousFileDetected) { // This case is covered by isMalicious
         resultCategory = QString::fromUtf8(StandardText::MALICIOUS);
    }
    else {
        resultCategory = tr("ERROR: %1").arg(getDefaultErrorMessage(fileScanError));
    }


    emit fileProcessed(filePath, resultCategory, isMalicious, progress);
    
    // If it was a single file scan initiated via scanFile(), also emit the original signal for compatibility
    if (m_totalFilesToScan == 1 && m_processedFilesCount == 1 && m_isScanning && !m_currentDirectoryPath.isEmpty() && m_scanQueue.isEmpty()) {
        // The resultString from performSingleFileScan contains the detailed report
        emit scanResultsReady(resultString, isMalicious);
    }


    if (m_isDirectoryScanActive && !m_scanQueue.isEmpty()) {
        QMetaObject::invokeMethod(this, "processNextFileInQueue", Qt::QueuedConnection);
    } else if (m_isDirectoryScanActive && m_scanQueue.isEmpty()) { // All files processed
        emit directoryScanFinished(m_currentDirectoryPath, m_processedFilesCount, m_threatsFoundInDirectory);
        m_isDirectoryScanActive = false;
        m_isScanning = false;
    }
}

ScannerErrorCode BasicScanner::performSingleFileScan(const QString& filePath, QString& outResultString, bool& outIsMalicious)
{
    outIsMalicious = false;
    QFileInfo currentFile(filePath);

    if (!currentFile.exists() || !currentFile.isFile()) {
        outResultString = tr("File not found or is not a file: %1").arg(filePath);
        return ScannerErrorCode::FileNotFound;
    }
     if (!currentFile.isReadable()) {
        outResultString = tr("File is not readable: %1").arg(filePath);
        return ScannerErrorCode::FileNotReadable;
    }


    // Calculate file hash
    ScannerErrorCode hashError = ScannerErrorCode::NoError;
    QString fileHash = calculateSha256(filePath, &hashError);

    if (fileHash.isEmpty()) {
        // Error is set by calculateSha256
        outResultString = tr("Failed to calculate hash for file: %1. Error: %2").arg(currentFile.fileName()).arg(getLastErrorMessage());
        return getLastErrorCode(); // Return the error code set by calculateSha256
    }

    // Check if hash exists in database
    ScannerErrorCode dbError = ScannerErrorCode::NoError;
    bool hashFound = checkHashInDatabase(fileHash, &dbError);

    if (dbError != ScannerErrorCode::NoError) {
        // Error is set by checkHashInDatabase
        outResultString = tr("Error checking database for file: %1. Error: %2").arg(currentFile.fileName()).arg(getLastErrorMessage());
        return getLastErrorCode(); // Return the error code set by checkHashInDatabase
    }

    // Prepare results string
    outResultString = tr(StandardText::FILE_LABEL).arg(currentFile.fileName());
    outResultString += tr(StandardText::HASH_LABEL).arg(fileHash);

    if (hashFound) {
        outResultString += tr(StandardText::STATUS_MALICIOUS);
        outIsMalicious = true;
        // Set last error specifically for malicious detection if not already an error
        setLastError(ScannerErrorCode::MaliciousFileDetected, tr("Malicious file detected: %1").arg(currentFile.fileName()));
        return ScannerErrorCode::MaliciousFileDetected; 
    } else {
        outResultString += tr(StandardText::STATUS_CLEAN);
        return ScannerErrorCode::NoError;
    }
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
    if (!m_isScanning && !m_isDirectoryScanActive) {
        return false;
    }
    
    bool wasActive = m_isDirectoryScanActive;
    m_isDirectoryScanActive = false; // Stop processing queue
    m_isScanning = false; // Reset general flag
    m_scanQueue.clear();
    
    m_results = tr(StandardText::SCAN_CANCELED); // For compatibility with old getResults()

    if (wasActive) {
        // Emit directoryScanFinished with -1 for threatsFound to indicate cancellation
        emit directoryScanFinished(m_currentDirectoryPath, m_processedFilesCount, -1);
    } else {
        // If it was a single file scan (now also using the queue),
        // we might need a specific signal or rely on the UI seeing no more fileProcessed signals.
        // For now, the directoryScanFinished with -1 might be generic enough if scanFile also sets m_currentDirectoryPath.
        // Let's ensure scanFile sets m_currentDirectoryPath. (Done in scanFile method)
         if (!m_currentDirectoryPath.isEmpty()){ // Check if a path context was set
            emit directoryScanFinished(m_currentDirectoryPath, m_processedFilesCount, -1);
         }
    }
    // The old scanResultsReady for single file scan cancellation:
    // emit scanResultsReady(m_results, false); // Emitting this might be confusing with new signals.
                                            // Let's rely on directoryScanFinished.

    qDebug() << "Scan canceled. Processed files:" << m_processedFilesCount << "out of" << m_totalFilesToScan;
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
                    tr("Error checking hash in database: %1").arg(ec.message().c_str()));
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
        case ScannerErrorCode::UnknownError: // Changed from Unknown to UnknownError
        default:
            return tr("Unknown error");
    }
}

/**
 * @brief Processes a single file scan asynchronously
 * 
 * This method handles single file scanning directly without using the directory scan queue.
 * It performs the actual scan and emits the appropriate signals.
 * 
 * @param filePath Path to the file to scan
 */
void BasicScanner::processSingleFile(const QString& filePath)
{
    QString resultString;
    bool isMalicious = false;
    
    // Perform the actual scan
    ScannerErrorCode errorCode = performSingleFileScan(filePath, resultString, isMalicious);
    
    // Store results for getResults() compatibility
    m_results = resultString;
    
    // Reset scanning flag
    m_isScanning = false;
    
    // Emit results
    if (errorCode == ScannerErrorCode::NoError || errorCode == ScannerErrorCode::MaliciousFileDetected) {
        emit scanResultsReady(resultString, isMalicious);
    } else {
        // Emit error signal for other error types
        emit scanError(errorCode, getLastErrorMessage());
    }
}
