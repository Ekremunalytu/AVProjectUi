/**
 * @file BasicScanner.cpp
 * @brief Implementation of the BasicScanner class for scanning files against a malware database
 * @author Ekrem Ünal
 * @date 9.05.2025
 */

#include "BasicScanner.h"
#include "yara/YaraRuleManager.h"
#include "../../storage/database/DbManager/DbManager.h"
#include "../../storage/database/DatabaseService/DatabaseService.h"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
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
    constexpr auto YARA_NOT_INITIALIZED = "YARA engine is not initialized.";
    constexpr auto YARA_SCAN_FAILED = "YARA scan failed.";
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
      m_lastError(ScannerErrorCode::NoError),
      m_yaraManager(std::make_unique<YaraRuleManager>()),
      m_yaraInitialized(false),
      m_maxFileSize(10 * 1024 * 1024) // Default max file size: 10 MB
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
    
    // Initialize YARA with default rules path
    initializeYara();
}

/**
 * @brief Default destructor
 */
BasicScanner::~BasicScanner() = default;

/**
 * @brief Initializes YARA engine and loads rules
 * @param rulesPath Path to the YARA rules directory or file
 * @return True if initialization was successful, false otherwise
 */
bool BasicScanner::initializeYara(const QString& rulesPath)
{
    if (m_yaraInitialized) {
        return true; // Already initialized
    }
    
    // Initialize YARA engine
    auto initResult = m_yaraManager->initialize();
    if (initResult) {  // Check if error occurred (non-zero value)
        qDebug() << "Failed to initialize YARA engine:" << initResult.message().c_str();
        setLastError(ScannerErrorCode::YaraInitializationFailed, 
                    QString("YARA initialization failed: %1").arg(initResult.message().c_str()));
        return false;
    }
    
    // Determine rules path
    QString actualRulesPath = rulesPath;
    if (actualRulesPath.isEmpty()) {
        qDebug() << "No YARA rules path provided, searching for rules...";
        
        // Try different possible paths for YARA rules
        QStringList possiblePaths = {
            // Source directory relative to project root
            "/Volumes/Crucial/AVProjectUi/src/security/scanning/yara/rules",
            // Relative to current working directory (for source builds)
            QDir::currentPath() + "/src/security/scanning/yara/rules",
            // Relative to application directory (for release builds)
            QCoreApplication::applicationDirPath() + "/rules",
            QCoreApplication::applicationDirPath() + "/../rules",
            QCoreApplication::applicationDirPath() + "/../../src/security/scanning/yara/rules",
            QCoreApplication::applicationDirPath() + "/../../../src/security/scanning/yara/rules",
            QCoreApplication::applicationDirPath() + "/../../../../src/security/scanning/yara/rules",
            // Try relative to executable path for debug builds
            QCoreApplication::applicationDirPath() + "/../../../../../../../src/security/scanning/yara/rules"
        };
        
        qDebug() << "Current working directory:" << QDir::currentPath();
        qDebug() << "Application directory:" << QCoreApplication::applicationDirPath();
        qDebug() << "Searching for YARA rules in the following paths:";
        
        // Find the first existing path
        for (const QString& path : possiblePaths) {
            qDebug() << "  Checking:" << path;
            QDir dir(path);
            if (dir.exists()) {
                qDebug() << "    Directory exists!";
                // Check if there are actually .yar files in this directory
                QStringList yarFiles = dir.entryList(QStringList() << "*.yar", QDir::Files, QDir::Name);
                qDebug() << "    Found .yar files:" << yarFiles;
                if (!yarFiles.isEmpty()) {
                    actualRulesPath = path;
                    qDebug() << "Found YARA rules directory with" << yarFiles.size() << "rule files at:" << actualRulesPath;
                    break;
                }
            } else {
                qDebug() << "    Directory does not exist";
            }
        }
        
        // If no directory found, fall back to Qt resources
        if (actualRulesPath.isEmpty()) {
            actualRulesPath = ":/rules"; // Qt resource path
            qDebug() << "No local YARA rules found, trying Qt resources:" << actualRulesPath;
        }
    }
    
    // Load YARA rules
    auto loadResult = m_yaraManager->loadRules(actualRulesPath.toStdString());
    if (loadResult) {  // Check if error occurred (non-zero value)
        qDebug() << "Failed to load YARA rules from:" << actualRulesPath;
        qDebug() << "Error:" << loadResult.message().c_str();
        setLastError(ScannerErrorCode::YaraRulesLoadFailed,
                    QString("Failed to load YARA rules from %1: %2")
                    .arg(actualRulesPath)
                    .arg(loadResult.message().c_str()));
        return false;
    }
    
    m_yaraInitialized = true;
    qDebug() << "YARA engine initialized successfully with rules from:" << actualRulesPath;
    return true;
}

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
 * This method now performs comprehensive scanning by delegating to scanFileComprehensive.
 * 
 * @param filePath Path to the file to scan. If empty, uses previously selected file
 * @return True if scan was initiated successfully, false if an error occurred
 */
bool BasicScanner::scanFile(const QString& filePath)
{
    return scanFileComprehensive(filePath);
}

/**
 * @brief Performs a comprehensive scan combining hash check and YARA analysis
 * 
 * This method first performs a hash-based scan against the malware database,
 * then if the file is not found in the database, performs YARA rule scanning.
 * 
 * @param filePath Path to the file to be scanned
 * @return True if scan completed successfully, false on error
 */
bool BasicScanner::scanFileComprehensive(const QString& filePath)
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
    
    // Step 1: Calculate file hash
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
    
    // Step 2: Check hash in database
    ScannerErrorCode dbError = ScannerErrorCode::NoError;
    bool hashFound = checkHashInDatabase(fileHash, &dbError);
    
    qDebug() << "Hash found in database:" << hashFound;
    
    if (dbError != ScannerErrorCode::NoError) {
        m_isScanning = false;
        // Error is already set by checkHashInDatabase
        m_results = tr("Error checking database: %1").arg(getLastErrorMessage());
        emit scanResultsReady(m_results);
        emit scanError(getLastErrorCode(), getLastErrorMessage());
        return false;
    }
    
    // Prepare initial results
    m_results = tr(StandardText::FILE_LABEL).arg(m_selectedFile.fileName());
    m_results += tr(StandardText::HASH_LABEL).arg(fileHash);
    
    if (hashFound) {
        // File found in malware database - it's definitely malicious
        m_results += tr(StandardText::STATUS_MALICIOUS);
        m_results += tr("\n\n=== YARA Analysis ===\n");
        m_results += tr("Skipped: File already identified as malicious in database\n");
        
        m_isScanning = false;
        emit scanResultsReady(m_results);
        emit scanError(ScannerErrorCode::MaliciousFileDetected, tr("Malicious file detected in database!"));
        return true;
    } else {
        // Hash not found in database - proceed with YARA analysis
        m_results += tr("Status: Not found in database - Proceeding with YARA analysis...\n");
        m_results += tr("\n=== YARA Analysis ===\n");
        
        // Step 3: Perform YARA scanning
        if (m_yaraInitialized) {
            std::vector<std::string> yaraMatches;
            auto scanResult = m_yaraManager->scanFile(m_selectedFile.filePath().toStdString(), yaraMatches);
            
            if (scanResult) {  // Check if error occurred (non-zero value)
                // YARA scan failed
                setLastError(ScannerErrorCode::YaraScanFailed,
                            QString("YARA scan failed: %1").arg(scanResult.message().c_str()));
                m_results += tr("YARA scan failed: %1\n").arg(scanResult.message().c_str());
                m_results += tr("Final Status: UNKNOWN - Database clean, YARA scan failed\n");
            } else if (!yaraMatches.empty()) {
                // YARA rules matched - file is potentially malicious
                m_results += tr("YARA Rules Matched (%1):\n").arg(yaraMatches.size());
                for (const auto& match : yaraMatches) {
                    m_results += tr("  - %1\n").arg(QString::fromStdString(match));
                }
                m_results += tr("Final Status: SUSPICIOUS/MALICIOUS - YARA rules detected potential threats\n");
                
                m_isScanning = false;
                emit scanResultsReady(m_results);
                emit scanError(ScannerErrorCode::MaliciousFileDetected, tr("Malicious patterns detected by YARA!"));
                return true;
            } else {
                // No YARA matches - file appears clean
                m_results += tr("No YARA rules matched\n");
                m_results += tr("Final Status: CLEAN - Not found in database and no YARA matches\n");
            }
        } else {
            // YARA not initialized
            m_results += tr("YARA engine not initialized - skipping rule-based analysis\n");
            m_results += tr("Final Status: CLEAN (Database only) - Not found in database\n");
        }
    }
    
    m_isScanning = false;
    emit scanResultsReady(m_results);
    return true;
}

/**
 * @brief Performs YARA rule scanning on the specified file
 * @param filePath Path to the file to be scanned with YARA rules
 * @return True if scan completed successfully, false on error
 */
bool BasicScanner::scanFileWithYara(const QString& filePath)
{
    if (!m_yaraInitialized) {
        setLastError(ScannerErrorCode::YaraInitializationFailed, tr(StandardText::YARA_NOT_INITIALIZED));
        return false;
    }
    
    if (filePath.isEmpty()) {
        setLastError(ScannerErrorCode::InvalidInput, tr("File path is empty"));
        return false;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        setLastError(ScannerErrorCode::FileNotFound, tr("File does not exist: %1").arg(filePath));
        return false;
    }
    
    // Perform YARA scan
    std::vector<std::string> matches;
    auto result = m_yaraManager->scanFile(filePath.toStdString(), matches);
    
    if (result) {  // Check if error occurred (non-zero value)
        setLastError(ScannerErrorCode::YaraScanFailed,
                    QString("YARA scan failed: %1").arg(result.message().c_str()));
        return false;
    }
    
    // Process results
    QString yaraResults = tr("=== YARA Scan Results ===\n");
    yaraResults += tr("File: %1\n").arg(fileInfo.fileName());
    
    if (matches.empty()) {
        yaraResults += tr("Status: CLEAN - No YARA rules matched\n");
    } else {
        yaraResults += tr("Status: SUSPICIOUS/MALICIOUS - %1 YARA rule(s) matched\n").arg(matches.size());
        yaraResults += tr("Matched Rules:\n");
        for (const auto& match : matches) {
            yaraResults += tr("  - %1\n").arg(QString::fromStdString(match));
        }
    }
    
    m_results = yaraResults;
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
        case ScannerErrorCode::MaliciousFileDetected:
            return tr("Malicious file detected");
        case ScannerErrorCode::YaraInitializationFailed:
            return tr("YARA engine initialization failed");
        case ScannerErrorCode::YaraRulesLoadFailed:
            return tr("Failed to load YARA rules");
        case ScannerErrorCode::YaraScanFailed:
            return tr("YARA scan operation failed");
        case ScannerErrorCode::Unknown:
        default:
            return tr("Unknown error");
    }
}

/**
 * @brief Performs asynchronous YARA scanning
 * @param filePath Path to the file to be scanned
 * @return Future that will contain the scan results
 */
QFuture<QString> BasicScanner::scanFileAsync(const QString& filePath)
{
    return QtConcurrent::run([this, filePath]() -> QString {
        std::vector<std::string> matches;
        auto result = m_yaraManager->scanFile(filePath.toStdString(), matches);
        
        if (result) {
            return QString("YARA scan failed: %1").arg(result.message().c_str());
        }
        
        QString results = QString("=== Async YARA Scan Results ===\n");
        results += QString("File: %1\n").arg(QFileInfo(filePath).fileName());
        
        if (matches.empty()) {
            results += "Status: CLEAN - No YARA rules matched\n";
        } else {
            results += QString("Status: SUSPICIOUS/MALICIOUS - %1 rule(s) matched\n").arg(matches.size());
            results += "Matched Rules:\n";
            for (const auto& match : matches) {
                results += QString("  - %1\n").arg(QString::fromStdString(match));
            }
        }
        
        return results;
    });
}

/**
 * @brief Sets maximum file size for scanning
 * @param maxSize Maximum file size in bytes
 */
void BasicScanner::setMaxFileSize(qint64 maxSize)
{
    m_maxFileSize = maxSize;
}

/**
 * @brief Checks if file size is within limits
 * @param filePath Path to the file to check
 * @return True if file size is acceptable, false otherwise
 */
bool BasicScanner::isFileSizeAcceptable(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.size() <= m_maxFileSize;
}
