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
#include "../../core/session/SessionManager.h"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QDateTime>
#include <QMap>
#include <system_error>
#include <chrono>

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
      m_lastErrorMessage(),  // Explicitly initialize as empty
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
        qDebug() << "No YARA rules path provided, attempting to load built-in rules...";
        
        // Try to load built-in rules first
        auto builtinResult = m_yaraManager->loadBuiltinRules();
        if (!builtinResult) {  // Success (error_code success is 0)
            qDebug() << "Successfully loaded built-in YARA rules";
            m_yaraInitialized = true;
            return true;
        }
        
        qDebug() << "Failed to load built-in rules, trying manual search...";
        
        // Try different possible paths for YARA rules
        QStringList possiblePaths = {
            // Build directory paths
            QCoreApplication::applicationDirPath() + "/yara_rules",
            QCoreApplication::applicationDirPath() + "/../yara_rules", 
            QCoreApplication::applicationDirPath() + "/../../yara_rules",
            QCoreApplication::applicationDirPath() + "/Contents/Resources/yara_rules",  // macOS app bundle
            // Source directory relative to project root
            "/Volumes/Crucial/AVProjectUi/src/security/scanning/yara/rules",
            "/Volumes/Crucial/AVProjectUi/src/security/scanning/yara",
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
                // Check if there are actually .yar files in this directory or subdirectories
                QStringList yarFiles = dir.entryList(QStringList() << "*.yar", QDir::Files, QDir::Name);
                bool hasSubdirs = dir.exists("rules") || dir.exists("maldocs");
                qDebug() << "    Found .yar files:" << yarFiles.size();
                qDebug() << "    Has rule subdirectories:" << hasSubdirs;
                if (!yarFiles.isEmpty() || hasSubdirs) {
                    actualRulesPath = path;
                    qDebug() << "Found YARA rules directory at:" << actualRulesPath;
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
    
    // Increment scanned files counter
    SessionManager::getInstance().incrementScannedFiles();
    
    // Start scan timing
    auto scanStartTime = std::chrono::high_resolution_clock::now();
    
    // Generate comprehensive scan header
    m_results = generateScanHeader();
    
    // Step 1: Calculate file hash with timing
    auto hashStartTime = std::chrono::high_resolution_clock::now();
    ScannerErrorCode hashError = ScannerErrorCode::NoError;
    QString fileHash = calculateSha256(m_selectedFile.filePath(), &hashError);
    auto hashEndTime = std::chrono::high_resolution_clock::now();
    auto hashDuration = std::chrono::duration_cast<std::chrono::milliseconds>(hashEndTime - hashStartTime);
    
    qDebug() << "Calculated hash:" << fileHash;
    
    if (fileHash.isEmpty()) {
        m_isScanning = false;
        // Error is already set by calculateSha256
        m_results += tr("\n❌ SCAN FAILED\n");
        m_results += tr("Failed to calculate hash for file: %1").arg(m_selectedFile.fileName());
        emit scanResultsReady(m_results);
        emit scanError(getLastErrorCode(), getLastErrorMessage());
        return false;
    }
    
    // Step 2: Check hash in database with timing
    auto dbStartTime = std::chrono::high_resolution_clock::now();
    ScannerErrorCode dbError = ScannerErrorCode::NoError;
    bool hashFound = checkHashInDatabase(fileHash, &dbError);
    auto dbEndTime = std::chrono::high_resolution_clock::now();
    auto dbDuration = std::chrono::duration_cast<std::chrono::milliseconds>(dbEndTime - dbStartTime);
    
    qDebug() << "Hash found in database:" << hashFound;
    
    if (dbError != ScannerErrorCode::NoError) {
        m_isScanning = false;
        // Error is already set by checkHashInDatabase
        m_results += tr("\n❌ DATABASE ERROR\n");
        m_results += tr("Error checking database: %1").arg(getLastErrorMessage());
        emit scanResultsReady(m_results);
        emit scanError(getLastErrorCode(), getLastErrorMessage());
        return false;
    }
    
    // Add database analysis results
    m_results += generateDatabaseAnalysisResult(fileHash, hashFound, hashDuration, dbDuration);
    
    if (hashFound) {
        // File found in malware database - it's definitely malicious
        m_results += generateYaraSkippedResult();
        m_results += generateFinalResult(true, true, 0, scanStartTime);
        
        m_isScanning = false;
        emit scanResultsReady(m_results);
        emit scanError(ScannerErrorCode::MaliciousFileDetected, tr("Malicious file detected in database!"));
        return true;
    } else {
        // Hash not found in database - proceed with YARA analysis
        m_results += generateYaraAnalysisResult(scanStartTime);
        
        // Step 3: Perform YARA scanning
        if (m_yaraInitialized) {
            auto yaraStartTime = std::chrono::high_resolution_clock::now();
            std::vector<std::string> yaraMatches;
            auto scanResult = m_yaraManager->scanFile(m_selectedFile.filePath().toStdString(), yaraMatches);
            auto yaraEndTime = std::chrono::high_resolution_clock::now();
            auto yaraDuration = std::chrono::duration_cast<std::chrono::milliseconds>(yaraEndTime - yaraStartTime);
            
            if (scanResult) {  // Check if error occurred (non-zero value)
                // YARA scan failed
                setLastError(ScannerErrorCode::YaraScanFailed,
                            QString("YARA scan failed: %1").arg(scanResult.message().c_str()));
                m_results += tr("❌ YARA scan failed: %1\n").arg(scanResult.message().c_str());
                m_results += tr("Scan time: %1 ms\n").arg(yaraDuration.count());
                m_results += generateFinalResult(false, false, 0, scanStartTime, true);
            } else if (!yaraMatches.empty()) {
                // YARA rules matched - file is potentially malicious
                SessionManager::getInstance().incrementYaraMatches();
                m_results += generateYaraMatchResult(yaraMatches, yaraDuration);
                m_results += generateFinalResult(true, false, yaraMatches.size(), scanStartTime);
                
                m_isScanning = false;
                emit scanResultsReady(m_results);
                emit scanError(ScannerErrorCode::MaliciousFileDetected, tr("Malicious patterns detected by YARA!"));
                return true;
            } else {
                // No YARA matches - file appears clean
                m_results += generateYaraCleanResult(yaraDuration);
                m_results += generateFinalResult(false, false, 0, scanStartTime);
            }
        } else {
            // YARA not initialized
            m_results += tr("⚠️  YARA engine not initialized - skipping rule-based analysis\n");
            m_results += generateFinalResult(false, false, 0, scanStartTime, false, true);
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
    
    // Set the file info for header generation
    m_selectedFile = fileInfo;
    
    // Start scan timing
    auto scanStartTime = std::chrono::high_resolution_clock::now();
    
    // Generate YARA-only scan header
    m_results = tr("╔══════════════════════════════════════════════════════════════════════════════╗\n");
    m_results += tr("║                          🔬 YARA RULE ENGINE SCAN                             ║\n");
    m_results += tr("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    
    // File information section
    m_results += tr("📁 FILE INFORMATION:\n");
    m_results += tr("   Name: %1\n").arg(fileInfo.fileName());
    m_results += tr("   Path: %1\n").arg(filePath);
    m_results += tr("   Size: %1 bytes (%2)\n").arg(fileInfo.size()).arg(formatFileSize(fileInfo.size()));
    m_results += tr("   Modified: %1\n").arg(fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
    
    // YARA engine information
    if (m_yaraInitialized) {
        auto stats = m_yaraManager->getStatistics();
        m_results += tr("\n⚙️  YARA ENGINE:\n");
        m_results += tr("   Rules loaded: %1\n").arg(stats.totalRulesLoaded);
        m_results += tr("   Engine status: ✅ Initialized\n");
    }
    
    m_results += tr("\n🔍 SCANNING...\n");
    m_results += tr("═══════════════════════════════════════════════════════════════════════════════\n\n");
    
    // Perform YARA scan with timing
    auto yaraStartTime = std::chrono::high_resolution_clock::now();
    std::vector<std::string> matches;
    auto result = m_yaraManager->scanFile(filePath.toStdString(), matches);
    auto yaraEndTime = std::chrono::high_resolution_clock::now();
    auto yaraDuration = std::chrono::duration_cast<std::chrono::milliseconds>(yaraEndTime - yaraStartTime);
    
    if (result) {  // Check if error occurred (non-zero value)
        setLastError(ScannerErrorCode::YaraScanFailed,
                    QString("YARA scan failed: %1").arg(result.message().c_str()));
        m_results += tr("❌ YARA scan failed: %1\n").arg(result.message().c_str());
        m_results += tr("Scan time: %1 ms\n").arg(yaraDuration.count());
        return false;
    }
    
    // Process results
    if (matches.empty()) {
        m_results += generateYaraCleanResult(yaraDuration);
        m_results += generateFinalResult(false, false, 0, scanStartTime);
    } else {
        m_results += generateYaraMatchResult(matches, yaraDuration);
        m_results += generateFinalResult(true, false, matches.size(), scanStartTime);
    }
    
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
    // If the filePath is empty, clear the selected file and reset error state
    if (filePath.isEmpty()) {
        m_selectedFile = QFileInfo();
        setLastError(ScannerErrorCode::NoError);
        return;
    }
    
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
    
    if (code == ScannerErrorCode::NoError) {
        // Clear error message when no error
        m_lastErrorMessage.clear();
    } else if (message.isEmpty()) {
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
        auto scanStartTime = std::chrono::high_resolution_clock::now();
        QFileInfo fileInfo(filePath);
        
        QString results;
        results += tr("╔══════════════════════════════════════════════════════════════════════════════╗\n");
        results += tr("║                      🔬 ASYNC YARA RULE ENGINE SCAN                          ║\n");
        results += tr("╠══════════════════════════════════════════════════════════════════════════════╣\n");
        
        // File information
        results += tr("📁 FILE INFORMATION:\n");
        results += tr("   Name: %1\n").arg(fileInfo.fileName());
        results += tr("   Path: %1\n").arg(filePath);
        results += tr("   Size: %1 bytes (%2)\n").arg(fileInfo.size()).arg(formatFileSize(fileInfo.size()));
        results += tr("   Scan type: Asynchronous YARA analysis\n");
        
        // YARA engine status
        if (m_yaraInitialized) {
            auto stats = m_yaraManager->getStatistics();
            results += tr("\n⚙️  YARA ENGINE:\n");
            results += tr("   Rules loaded: %1\n").arg(stats.totalRulesLoaded);
            results += tr("   Engine status: ✅ Initialized\n");
        }
        
        results += tr("\n🔍 ASYNC SCANNING...\n");
        results += tr("═══════════════════════════════════════════════════════════════════════════════\n\n");
        
        // Perform scan with timing
        auto yaraStartTime = std::chrono::high_resolution_clock::now();
        std::vector<std::string> matches;
        auto result = m_yaraManager->scanFile(filePath.toStdString(), matches);
        auto yaraEndTime = std::chrono::high_resolution_clock::now();
        auto yaraDuration = std::chrono::duration_cast<std::chrono::milliseconds>(yaraEndTime - yaraStartTime);
        
        if (result) {
            results += tr("❌ YARA scan failed: %1\n").arg(result.message().c_str());
            results += tr("Scan time: %1 ms\n").arg(yaraDuration.count());
            results += tr("\n📊 ASYNC SCAN SUMMARY:\n");
            results += tr("   Status: ❌ FAILED\n");
            results += tr("   Error: Scan operation failed\n");
        } else if (matches.empty()) {
            results += tr("Scan completed in: %1 ms\n").arg(yaraDuration.count());
            results += tr("Result: ✅ Clean - No YARA rules matched\n");
            results += tr("  • No malicious patterns detected\n");
            results += tr("  • No suspicious behaviors identified\n");
            results += tr("  • File appears safe based on rule analysis\n\n");
            
            results += tr("📊 ASYNC SCAN SUMMARY:\n");
            results += tr("   Status: ✅ CLEAN\n");
            results += tr("   Confidence: High\n");
        } else {
            results += tr("Scan completed in: %1 ms\n").arg(yaraDuration.count());
            results += tr("Result: ❌ SUSPICIOUS/MALICIOUS - %1 rule(s) matched\n\n").arg(matches.size());
            
            // Categorize matches
            QMap<QString, QStringList> categorizedMatches;
            for (const auto& match : matches) {
                QString rule = QString::fromStdString(match);
                if (rule.startsWith("APT_")) categorizedMatches["🎯 Advanced Threats"].append(rule);
                else if (rule.startsWith("MALW_")) categorizedMatches["🦠 Malware"].append(rule);
                else if (rule.startsWith("RANSOM_")) categorizedMatches["🔒 Ransomware"].append(rule);
                else if (rule.startsWith("RAT_")) categorizedMatches["🕵️ RATs"].append(rule);
                else if (rule.startsWith("TOOLKIT_")) categorizedMatches["🧰 Toolkits"].append(rule);
                else if (rule.contains("Maldoc")) categorizedMatches["📄 Maldocs"].append(rule);
                else categorizedMatches["🔍 Other"].append(rule);
            }
            
            results += tr("Matched Rules by Category:\n");
            for (auto it = categorizedMatches.constBegin(); it != categorizedMatches.constEnd(); ++it) {
                results += tr("  %1 (%2):\n").arg(it.key()).arg(it.value().size());
                for (const QString& rule : it.value()) {
                    results += tr("    • %1\n").arg(rule);
                }
            }
            
            results += tr("\n📊 ASYNC SCAN SUMMARY:\n");
            results += tr("   Status: ❌ THREATS DETECTED\n");
            results += tr("   Matched rules: %1\n").arg(matches.size());
            results += tr("   Confidence: High\n");
        }
        
        auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - scanStartTime);
        results += tr("   Total time: %1 ms\n").arg(totalTime.count());
        results += tr("   Timestamp: %1\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        
        results += tr("\n╚══════════════════════════════════════════════════════════════════════════════╝\n");
        
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

/**
 * @brief Generates a comprehensive scan header with file details
 * @return Formatted header string
 */
QString BasicScanner::generateScanHeader() const
{
    QString header;
    header += tr("╔══════════════════════════════════════════════════════════════════════════════╗\n");
    header += tr("║                        📋 COMPREHENSIVE FILE SCAN REPORT                      ║\n");
    header += tr("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    
    // File information section
    header += tr("📁 FILE INFORMATION:\n");
    header += tr("   Name: %1\n").arg(m_selectedFile.fileName());
    header += tr("   Path: %1\n").arg(m_selectedFile.filePath());
    header += tr("   Size: %1 bytes (%2)\n").arg(m_selectedFile.size()).arg(formatFileSize(m_selectedFile.size()));
    header += tr("   Modified: %1\n").arg(m_selectedFile.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
    header += tr("   Created: %1\n").arg(m_selectedFile.birthTime().toString("yyyy-MM-dd hh:mm:ss"));
    header += tr("   Type: %1\n").arg(m_selectedFile.suffix().isEmpty() ? "Unknown" : m_selectedFile.suffix().toUpper());
    header += tr("   Permissions: %1\n").arg(formatFilePermissions());
    
    // Scan configuration
    header += tr("\n⚙️  SCAN CONFIGURATION:\n");
    header += tr("   Max file size: %1\n").arg(formatFileSize(m_maxFileSize));
    header += tr("   YARA engine: %1\n").arg(m_yaraInitialized ? "✅ Initialized" : "❌ Not initialized");
    if (m_yaraInitialized) {
        auto stats = m_yaraManager->getStatistics();
        header += tr("   YARA rules loaded: %1\n").arg(stats.totalRulesLoaded);
        header += tr("   Rules last updated: %1\n").arg(formatDateTime(stats.lastRuleUpdate));
    }
    header += tr("   Database: %1\n").arg(m_dbManager ? "✅ Connected" : "❌ Not connected");
    
    header += tr("\n🔍 SCAN PROGRESS:\n");
    header += tr("═══════════════════════════════════════════════════════════════════════════════\n\n");
    
    return header;
}

/**
 * @brief Generates database analysis results
 */
QString BasicScanner::generateDatabaseAnalysisResult(const QString& hash, bool found, 
                                                    std::chrono::milliseconds hashTime, 
                                                    std::chrono::milliseconds dbTime) const
{
    QString result;
    result += tr("🏛️ DATABASE ANALYSIS:\n");
    result += tr("═══════════════════\n");
    result += tr("Hash calculation: %1 ms\n").arg(hashTime.count());
    result += tr("SHA256: %1\n").arg(hash);
    result += tr("Database lookup: %1 ms\n").arg(dbTime.count());
    
    if (found) {
        result += tr("Status: ❌ MALICIOUS - Hash found in malware database\n");
        result += tr("⚠️  This file is a known malware sample!\n");
    } else {
        result += tr("Status: ✅ Clean - Hash not found in malware database\n");
    }
    
    result += tr("\n");
    return result;
}

/**
 * @brief Generates YARA analysis header
 */
QString BasicScanner::generateYaraAnalysisResult(std::chrono::high_resolution_clock::time_point scanStart) const
{
    QString result;
    result += tr("🔬 YARA RULE ENGINE ANALYSIS:\n");
    result += tr("═══════════════════════════════\n");
    
    if (m_yaraInitialized) {
        auto stats = m_yaraManager->getStatistics();
        auto ruleNames = m_yaraManager->getLoadedRuleNames();
        
        // Categorize rules
        QMap<QString, int> ruleCategories;
        for (const auto& ruleName : ruleNames) {
            QString rule = QString::fromStdString(ruleName);
            if (rule.startsWith("APT_")) ruleCategories["APT (Advanced Threats)"]++;
            else if (rule.startsWith("MALW_")) ruleCategories["Malware Detection"]++;
            else if (rule.startsWith("RANSOM_")) ruleCategories["Ransomware"]++;
            else if (rule.startsWith("RAT_")) ruleCategories["Remote Access Trojans"]++;
            else if (rule.startsWith("TOOLKIT_")) ruleCategories["Attacker Toolkits"]++;
            else if (rule.contains("Maldoc")) ruleCategories["Malicious Documents"]++;
            else ruleCategories["Other/Custom"]++;
        }
        
        result += tr("Engine Statistics:\n");
        result += tr("  • Total rules loaded: %1\n").arg(stats.totalRulesLoaded);
        result += tr("  • Files previously scanned: %1\n").arg(stats.filesScanned);
        result += tr("  • Total matches found: %1\n").arg(stats.matchesFound);
        if (stats.averageScanTime.count() > 0) {
            result += tr("  • Average scan time: %1 ms\n").arg(stats.averageScanTime.count());
        }
        
        result += tr("\nRule Categories:\n");
        for (auto it = ruleCategories.constBegin(); it != ruleCategories.constEnd(); ++it) {
            result += tr("  • %1: %2 rules\n").arg(it.key()).arg(it.value());
        }
        
        result += tr("\nScanning file...\n");
    } else {
        result += tr("❌ YARA engine not initialized\n");
    }
    
    return result;
}

/**
 * @brief Generates results when YARA scanning is skipped
 */
QString BasicScanner::generateYaraSkippedResult() const
{
    QString result;
    result += tr("\n🔬 YARA RULE ENGINE ANALYSIS:\n");
    result += tr("═══════════════════════════════\n");
    result += tr("⏭️  Skipped: File already identified as malicious in database\n");
    result += tr("   (YARA analysis unnecessary for known malware)\n\n");
    return result;
}

/**
 * @brief Generates YARA match results
 */
QString BasicScanner::generateYaraMatchResult(const std::vector<std::string>& matches, 
                                             std::chrono::milliseconds scanTime) const
{
    QString result;
    result += tr("Scan completed in: %1 ms\n").arg(scanTime.count());
    result += tr("Result: ❌ SUSPICIOUS/MALICIOUS - %1 rule(s) matched\n\n").arg(matches.size());
    
    // Categorize matches
    QMap<QString, QStringList> categorizedMatches;
    for (const auto& match : matches) {
        QString rule = QString::fromStdString(match);
        if (rule.startsWith("APT_")) categorizedMatches["🎯 Advanced Persistent Threats (APT)"].append(rule);
        else if (rule.startsWith("MALW_")) categorizedMatches["🦠 Malware Detection"].append(rule);
        else if (rule.startsWith("RANSOM_")) categorizedMatches["🔒 Ransomware"].append(rule);
        else if (rule.startsWith("RAT_")) categorizedMatches["🕵️ Remote Access Trojans"].append(rule);
        else if (rule.startsWith("TOOLKIT_")) categorizedMatches["🧰 Attacker Toolkits"].append(rule);
        else if (rule.contains("Maldoc")) categorizedMatches["📄 Malicious Documents"].append(rule);
        else categorizedMatches["🔍 Other Threats"].append(rule);
    }
    
    result += tr("Matched Rules by Category:\n");
    for (auto it = categorizedMatches.constBegin(); it != categorizedMatches.constEnd(); ++it) {
        result += tr("  %1 (%2 matches):\n").arg(it.key()).arg(it.value().size());
        for (const QString& rule : it.value()) {
            result += tr("    • %1\n").arg(rule);
        }
        result += tr("\n");
    }
    
    return result;
}

/**
 * @brief Generates YARA clean results
 */
QString BasicScanner::generateYaraCleanResult(std::chrono::milliseconds scanTime) const
{
    QString result;
    result += tr("Scan completed in: %1 ms\n").arg(scanTime.count());
    result += tr("Result: ✅ Clean - No YARA rules matched\n");
    result += tr("  • No malicious patterns detected\n");
    result += tr("  • No suspicious behaviors identified\n");
    result += tr("  • File appears safe based on rule analysis\n\n");
    return result;
}

/**
 * @brief Generates the final scan result summary
 */
QString BasicScanner::generateFinalResult(bool isMalicious, bool foundInDb, int yaraMatches,
                                         std::chrono::high_resolution_clock::time_point scanStart,
                                         bool yaraFailed, bool yaraNotInitialized) const
{
    auto scanEnd = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(scanEnd - scanStart);
    
    QString result;
    result += tr("╔══════════════════════════════════════════════════════════════════════════════╗\n");
    result += tr("║                                📊 FINAL ASSESSMENT                            ║\n");
    result += tr("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    
    if (isMalicious) {
        result += tr("🚨 THREAT DETECTED:\n");
        if (foundInDb) {
            result += tr("   • Classification: KNOWN MALWARE\n");
            result += tr("   • Source: Malware database\n");
            result += tr("   • Confidence: 100% (Known malicious hash)\n");
        } else {
            result += tr("   • Classification: SUSPICIOUS/MALICIOUS\n");
            result += tr("   • Source: YARA rule analysis\n");
            result += tr("   • Matched rules: %1\n").arg(yaraMatches);
            result += tr("   • Confidence: High (Pattern-based detection)\n");
        }
        result += tr("   • ⚠️  RECOMMENDATION: DO NOT EXECUTE - Quarantine immediately\n");
    } else {
        if (yaraFailed) {
            result += tr("❓ SCAN INCOMPLETE:\n");
            result += tr("   • Database check: ✅ Clean\n");
            result += tr("   • YARA analysis: ❌ Failed\n");
            result += tr("   • Status: UNKNOWN (Partial scan only)\n");
            result += tr("   • ⚠️  RECOMMENDATION: Retry scan or use alternative scanner\n");
        } else if (yaraNotInitialized) {
            result += tr("⚠️  LIMITED SCAN:\n");
            result += tr("   • Database check: ✅ Clean\n");
            result += tr("   • YARA analysis: ⏸️  Not available\n");
            result += tr("   • Status: CLEAN (Database only)\n");
            result += tr("   • ℹ️  Note: Full scan requires YARA engine initialization\n");
        } else {
            result += tr("✅ FILE APPEARS CLEAN:\n");
            result += tr("   • Database check: ✅ No known malware hash\n");
            result += tr("   • YARA analysis: ✅ No suspicious patterns\n");
            result += tr("   • Status: CLEAN\n");
            result += tr("   • ✅ RECOMMENDATION: File appears safe to use\n");
        }
    }
    
    result += tr("\n📈 SCAN PERFORMANCE:\n");
    result += tr("   • Total scan time: %1 ms\n").arg(totalDuration.count());
    result += tr("   • Timestamp: %1\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    
    if (m_yaraInitialized) {
        auto stats = m_yaraManager->getStatistics();
        result += tr("   • Rules analyzed: %1\n").arg(stats.totalRulesLoaded);
    }
    
    result += tr("\n╚══════════════════════════════════════════════════════════════════════════════╝\n");
    
    return result;
}

/**
 * @brief Formats file size in human-readable format
 */
QString BasicScanner::formatFileSize(qint64 size) const
{
    const QStringList units = {"B", "KB", "MB", "GB", "TB"};
    double fileSize = static_cast<double>(size);
    int unitIndex = 0;
    
    while (fileSize >= 1024.0 && unitIndex < units.size() - 1) {
        fileSize /= 1024.0;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(QString::number(fileSize, 'f', 2)).arg(units[unitIndex]);
}

/**
 * @brief Formats file permissions
 */
QString BasicScanner::formatFilePermissions() const
{
    QString perms;
    if (m_selectedFile.isReadable()) perms += "R";
    if (m_selectedFile.isWritable()) perms += "W";
    if (m_selectedFile.isExecutable()) perms += "X";
    return perms.isEmpty() ? "None" : perms;
}

/**
 * @brief Formats datetime from chrono time_point
 */
QString BasicScanner::formatDateTime(const std::chrono::system_clock::time_point& timePoint) const
{
    auto time_t = std::chrono::system_clock::to_time_t(timePoint);
    return QDateTime::fromSecsSinceEpoch(time_t).toString("yyyy-MM-dd hh:mm:ss");
}
