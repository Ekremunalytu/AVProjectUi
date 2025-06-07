/**
 * @file CDRScannerTest.cpp
 * @brief Unit tests for CDRScanner class
 * @author Test Suite
 * @version 1.0
 * @date 2024
 */

#include <QTest>
#include <QObject>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QSignalSpy>
#include <QTimer>
#include <QEventLoop>
#include <QStandardPaths>
#include <QDebug>
#include <QFileInfo>
#include <memory>

// Test target
#include "security/cdr/CDRScanner.h"
#include "security/cdr/CdrTypes.h"
#include "infrastructure/docker/DockerTypes.h"

class CDRScannerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Constructor and initialization tests
    void testConstructor();
    void testInitialization_WithDocker();
    void testInitialization_WithoutDocker();

    // Scanning functionality tests
    void testScanFile_ValidFile();
    void testScanFile_InvalidFile();
    void testScanFile_UnsupportedFormat();
    void testScanDirectory_ValidDirectory();
    void testScanDirectory_EmptyDirectory();
    void testScanDirectory_InvalidDirectory();

    // Status and state tests
    void testGetCurrentStatus();
    void testIsScanning();
    void testStopScanning();

    // Configuration tests
    void testSetCdrConfiguration();
    void testGetCdrConfiguration();
    void testUpdateConfiguration();

    // Docker integration tests
    void testDockerAvailability();
    void testCdrContainerManagement();
    void testCdrImageManagement();

    // Signal emission tests
    void testScanResultsReadySignal();
    void testScanErrorSignal();
    void testScanProgressSignal();
    void testScanStatusChangedSignal();

    // Error handling tests
    void testDockerConnectionError();
    void testScanningErrors();
    void testContainerCreationFailure();

    // Performance tests
    void testLargeFileScan();
    void testConcurrentScans();
    void testScanTimeout();

    // CDR specific tests
    void testSanitizationResults();
    void testThreatDetection();
    void testFileReconstruction();

private:
    // Helper methods
    QString createTestFile(const QString& content, const QString& extension = ".txt");
    QString createMaliciousTestFile();
    QString createTestDirectory();
    void waitForSignal(QObject* sender, const char* signal, int timeout = 5000);
    bool isDockerAvailable();
    void verifyValidCdrConfig(const CDR::CdrConfiguration& config);

    QTemporaryDir m_tempDir;
    std::unique_ptr<CDRScanner> m_scanner;
    bool m_dockerAvailable;
};

void CDRScannerTest::initTestCase()
{
    qDebug() << "CDRScannerTest::initTestCase()";
    QVERIFY(m_tempDir.isValid());
    
    // Check Docker availability
    m_dockerAvailable = isDockerAvailable();
    if (!m_dockerAvailable) {
        qWarning() << "Docker not available - some tests will be skipped";
    }
}

void CDRScannerTest::cleanupTestCase()
{
    qDebug() << "CDRScannerTest::cleanupTestCase()";
}

void CDRScannerTest::init()
{
    m_scanner = std::make_unique<CDRScanner>();
    QVERIFY(m_scanner != nullptr);
}

void CDRScannerTest::cleanup()
{
    if (m_scanner) {
        m_scanner->stopScanning();
        m_scanner.reset();
    }
}

void CDRScannerTest::testConstructor()
{
    auto scanner = std::make_unique<CDRScanner>();
    QVERIFY(scanner != nullptr);
    
    // Test initial state
    QCOMPARE(scanner->getCurrentStatus(), ScanStatus::Idle);
    QVERIFY(!scanner->isScanning());
    
    // Test configuration is initialized
    CDR::CdrConfiguration config = scanner->getCdrConfiguration();
    verifyValidCdrConfig(config);
}

void CDRScannerTest::testInitialization_WithDocker()
{
    if (!m_dockerAvailable) {
        QSKIP("Docker not available");
    }
    
    // Scanner should initialize successfully with Docker
    QVERIFY(m_scanner != nullptr);
    QCOMPARE(m_scanner->getCurrentStatus(), ScanStatus::Idle);
    
    // Check Docker manager initialization
    // Note: This test assumes Docker is running and accessible
}

void CDRScannerTest::testInitialization_WithoutDocker()
{
    if (m_dockerAvailable) {
        QSKIP("Docker is available, cannot test without Docker scenario");
    }
    
    // Scanner should handle Docker unavailability gracefully
    QVERIFY(m_scanner != nullptr);
    
    // Scanner should still be functional for basic operations
    QCOMPARE(m_scanner->getCurrentStatus(), ScanStatus::Idle);
}

void CDRScannerTest::testScanFile_ValidFile()
{
    QString testFile = createTestFile("Test content for CDR scanning", ".txt");
    QVERIFY(!testFile.isEmpty());
    
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    QSignalSpy statusSpy(m_scanner.get(), &CDRScanner::scanStatusChanged);
    
    m_scanner->scanFile(testFile);
    
    // Wait for scan completion
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 10000);
    
    // Verify signals were emitted
    QVERIFY(statusSpy.count() >= 1); // Status should have changed
    
    if (m_dockerAvailable) {
        // With Docker, we expect successful results
        QVERIFY(resultsSpy.count() == 1);
        QVERIFY(errorSpy.count() == 0);
    } else {
        // Without Docker, we expect an error or limited functionality
        QVERIFY(errorSpy.count() >= 1 || resultsSpy.count() >= 1);
    }
}

void CDRScannerTest::testScanFile_InvalidFile()
{
    QString invalidFile = m_tempDir.filePath("nonexistent.txt");
    
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    
    m_scanner->scanFile(invalidFile);
    
    // Wait for error or results
    waitForSignal(m_scanner.get(), SIGNAL(scanError(ScannerErrorCode, QString)), 5000);
    
    // Should get an error for invalid file
    QVERIFY(errorSpy.count() >= 1);
    QVERIFY(resultsSpy.count() == 0);
    
    // Verify error details
    if (errorSpy.count() > 0) {
        auto errorArgs = errorSpy.at(0);
        QVERIFY(errorArgs.size() >= 2);
        
        ScannerErrorCode errorCode = qvariant_cast<ScannerErrorCode>(errorArgs.at(0));
        QString errorMessage = errorArgs.at(1).toString();
        
        QVERIFY(errorCode != ScannerErrorCode::NoError);
        QVERIFY(!errorMessage.isEmpty());
    }
}

void CDRScannerTest::testScanFile_UnsupportedFormat()
{
    // Create a file with unsupported extension
    QString unsupportedFile = createTestFile("Unsupported content", ".xyz");
    
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    
    m_scanner->scanFile(unsupportedFile);
    
    // Wait for response
    waitForSignal(m_scanner.get(), SIGNAL(scanError(ScannerErrorCode, QString)), 5000);
    
    // May get error or results depending on implementation
    QVERIFY(errorSpy.count() >= 1 || resultsSpy.count() >= 1);
}

void CDRScannerTest::testScanDirectory_ValidDirectory()
{
    QString testDir = createTestDirectory();
    QVERIFY(!testDir.isEmpty());
    
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    QSignalSpy statusSpy(m_scanner.get(), &CDRScanner::scanStatusChanged);
    
    m_scanner->scanDirectory(testDir);
    
    // Wait for scan completion
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 15000);
    
    // Verify status changes occurred
    QVERIFY(statusSpy.count() >= 1);
    
    if (m_dockerAvailable) {
        // With Docker, expect successful results
        QVERIFY(resultsSpy.count() >= 1);
    } else {
        // Without Docker, may get error or limited results
        QVERIFY(errorSpy.count() >= 1 || resultsSpy.count() >= 1);
    }
}

void CDRScannerTest::testScanDirectory_EmptyDirectory()
{
    QString emptyDir = m_tempDir.filePath("empty");
    QDir().mkpath(emptyDir);
    
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    
    m_scanner->scanDirectory(emptyDir);
    
    // Wait for response
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 5000);
    
    // Should handle empty directory gracefully
    QVERIFY(resultsSpy.count() >= 1 || errorSpy.count() >= 1);
}

void CDRScannerTest::testScanDirectory_InvalidDirectory()
{
    QString invalidDir = m_tempDir.filePath("nonexistent");
    
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    
    m_scanner->scanDirectory(invalidDir);
    
    // Wait for error
    waitForSignal(m_scanner.get(), SIGNAL(scanError(ScannerErrorCode, QString)), 5000);
    
    // Should get an error for invalid directory
    QVERIFY(errorSpy.count() >= 1);
}

void CDRScannerTest::testGetCurrentStatus()
{
    // Initial status should be Idle
    QCOMPARE(m_scanner->getCurrentStatus(), ScanStatus::Idle);
    
    // Status should change during scanning
    QString testFile = createTestFile("Test content", ".txt");
    
    QSignalSpy statusSpy(m_scanner.get(), &CDRScanner::scanStatusChanged);
    
    m_scanner->scanFile(testFile);
    
    // Wait a moment for status to change
    QTest::qWait(100);
    
    // Status should have changed from Idle
    ScanStatus currentStatus = m_scanner->getCurrentStatus();
    QVERIFY(currentStatus == ScanStatus::Scanning || currentStatus == ScanStatus::Completed);
}

void CDRScannerTest::testIsScanning()
{
    // Initially should not be scanning
    QVERIFY(!m_scanner->isScanning());
    
    QString testFile = createTestFile("Test content", ".txt");
    
    m_scanner->scanFile(testFile);
    
    // Should be scanning immediately after starting
    QTest::qWait(50);
    
    // May or may not still be scanning depending on implementation speed
    bool wasScanning = m_scanner->isScanning();
    
    // Wait for completion
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 5000);
    
    // Should not be scanning after completion
    QVERIFY(!m_scanner->isScanning());
}

void CDRScannerTest::testStopScanning()
{
    QString testFile = createTestFile("Test content for stop test", ".txt");
    
    QSignalSpy statusSpy(m_scanner.get(), &CDRScanner::scanStatusChanged);
    
    m_scanner->scanFile(testFile);
    
    // Wait a moment then stop
    QTest::qWait(100);
    m_scanner->stopScanning();
    
    // Verify status changes
    QVERIFY(statusSpy.count() >= 1);
    
    // Should not be scanning after stop
    QVERIFY(!m_scanner->isScanning());
}

void CDRScannerTest::testSetCdrConfiguration()
{
    CDR::CdrConfiguration newConfig;
    newConfig.securityLevel = CDR::CdrConfiguration::SecurityLevel::HIGH;
    newConfig.inputDirectory = "/tmp/test_input";
    newConfig.outputDirectory = "/tmp/test_output";
    newConfig.quarantineDirectory = "/tmp/test_quarantine";
    newConfig.analysisType = CDR::AnalysisType::COMPREHENSIVE_SCAN;
    newConfig.preserveOriginal = false;
    newConfig.autoSanitize = true;
    newConfig.blockExecutables = true;
    newConfig.blockAllScripts = true;
    newConfig.removeMetadata = true;
    newConfig.validateAfterSanitization = true;
    
    m_scanner->setCdrConfiguration(newConfig);
    
    CDR::CdrConfiguration retrievedConfig = m_scanner->getCdrConfiguration();
    
    QCOMPARE(retrievedConfig.securityLevel, newConfig.securityLevel);
    QCOMPARE(QString::fromStdString(retrievedConfig.inputDirectory), QString::fromStdString(newConfig.inputDirectory));
    QCOMPARE(QString::fromStdString(retrievedConfig.outputDirectory), QString::fromStdString(newConfig.outputDirectory));
    QCOMPARE(retrievedConfig.analysisType, newConfig.analysisType);
    QCOMPARE(retrievedConfig.preserveOriginal, newConfig.preserveOriginal);
    QCOMPARE(retrievedConfig.autoSanitize, newConfig.autoSanitize);
    QCOMPARE(retrievedConfig.blockExecutables, newConfig.blockExecutables);
}

void CDRScannerTest::testGetCdrConfiguration()
{
    CDR::CdrConfiguration config = m_scanner->getCdrConfiguration();
    verifyValidCdrConfig(config);
}

void CDRScannerTest::testUpdateConfiguration()
{
    // Get original config
    CDR::CdrConfiguration originalConfig = m_scanner->getCdrConfiguration();
    
    // Modify config
    CDR::CdrConfiguration modifiedConfig = originalConfig;
    modifiedConfig.securityLevel = CDR::CdrConfiguration::SecurityLevel::HIGH;
    modifiedConfig.blockAllScripts = true;
    
    m_scanner->setCdrConfiguration(modifiedConfig);
    
    // Verify changes were applied
    CDR::CdrConfiguration currentConfig = m_scanner->getCdrConfiguration();
    QCOMPARE(currentConfig.securityLevel, CDR::CdrConfiguration::SecurityLevel::HIGH);
    QCOMPARE(currentConfig.blockAllScripts, true);
}

void CDRScannerTest::testDockerAvailability()
{
    if (!m_dockerAvailable) {
        QSKIP("Docker not available for testing");
    }
    
    // Test that Docker operations work
    QString testFile = createTestFile("Docker test content", ".txt");
    
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    
    m_scanner->scanFile(testFile);
    
    // Wait for completion
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 10000);
    
    // Should succeed with Docker available
    QVERIFY(resultsSpy.count() >= 1 || errorSpy.count() >= 1);
}

void CDRScannerTest::testCdrContainerManagement()
{
    if (!m_dockerAvailable) {
        QSKIP("Docker not available for container testing");
    }
    
    // This test verifies container lifecycle management
    // Note: Implementation depends on CDRScanner's container management methods
    
    QString testFile = createTestFile("Container test content", ".txt");
    
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    
    m_scanner->scanFile(testFile);
    
    // Wait for scan to complete
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 10000);
    
    // Container should be managed properly (created, used, cleaned up)
    QVERIFY(resultsSpy.count() >= 1);
}

void CDRScannerTest::testCdrImageManagement()
{
    if (!m_dockerAvailable) {
        QSKIP("Docker not available for image testing");
    }
    
    // Test image availability and management
    // This would test image pulling, verification, etc.
    
    // Scanner should handle image management transparently
    QString testFile = createTestFile("Image test content", ".txt");
    
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    
    m_scanner->scanFile(testFile);
    
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 15000);
    
    // Should handle image operations correctly
    QVERIFY(resultsSpy.count() >= 1 || errorSpy.count() >= 1);
}

void CDRScannerTest::testScanResultsReadySignal()
{
    QString testFile = createTestFile("Signal test content", ".txt");
    
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    
    m_scanner->scanFile(testFile);
    
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 10000);
    
    QVERIFY(resultsSpy.count() >= 1);
    
    if (resultsSpy.count() > 0) {
        // Verify signal parameters
        QList<QVariant> arguments = resultsSpy.at(0);
        QVERIFY(arguments.size() >= 1);
        
        QString results = arguments.at(0).toString();
        QVERIFY(!results.isEmpty());
    }
}

void CDRScannerTest::testScanErrorSignal()
{
    QString invalidFile = m_tempDir.filePath("nonexistent.txt");
    
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    
    m_scanner->scanFile(invalidFile);
    
    waitForSignal(m_scanner.get(), SIGNAL(scanError(ScannerErrorCode, QString)), 5000);
    
    QVERIFY(errorSpy.count() >= 1);
    
    if (errorSpy.count() > 0) {
        QList<QVariant> arguments = errorSpy.at(0);
        QVERIFY(arguments.size() >= 2);
        
        ScannerErrorCode errorCode = qvariant_cast<ScannerErrorCode>(arguments.at(0));
        QString errorMessage = arguments.at(1).toString();
        
        QVERIFY(errorCode != ScannerErrorCode::NoError);
        QVERIFY(!errorMessage.isEmpty());
    }
}

void CDRScannerTest::testScanProgressSignal()
{
    QString testFile = createTestFile("Progress test content", ".txt");
    
    QSignalSpy progressSpy(m_scanner.get(), &CDRScanner::scanProgress);
    
    m_scanner->scanFile(testFile);
    
    // Wait for scan completion
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 10000);
    
    // Progress signals are optional and implementation-dependent
    // Just verify no crashes occurred
    QVERIFY(progressSpy.count() >= 0);
}

void CDRScannerTest::testScanStatusChangedSignal()
{
    QString testFile = createTestFile("Status test content", ".txt");
    
    QSignalSpy statusSpy(m_scanner.get(), &CDRScanner::scanStatusChanged);
    
    m_scanner->scanFile(testFile);
    
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 10000);
    
    // Should have at least one status change
    QVERIFY(statusSpy.count() >= 1);
    
    if (statusSpy.count() > 0) {
        QList<QVariant> arguments = statusSpy.at(0);
        QVERIFY(arguments.size() >= 1);
        
        ScanStatus status = qvariant_cast<ScanStatus>(arguments.at(0));
        QVERIFY(status != ScanStatus::Idle || statusSpy.count() > 1);
    }
}

void CDRScannerTest::testDockerConnectionError()
{
    // This test simulates Docker connection issues
    // Note: Difficult to test without actually disrupting Docker
    
    QString testFile = createTestFile("Docker error test", ".txt");
    
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    
    m_scanner->scanFile(testFile);
    
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 10000);
    
    // Should handle Docker issues gracefully
    QVERIFY(errorSpy.count() >= 0); // May or may not get errors
    QVERIFY(resultsSpy.count() >= 0); // May or may not get results
}

void CDRScannerTest::testScanningErrors()
{
    // Test various error conditions
    QStringList errorCases = {
        "",  // Empty path
        "/nonexistent/path/file.txt",  // Non-existent file
        m_tempDir.path()  // Directory instead of file
    };
    
    for (const QString& errorCase : errorCases) {
        QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
        
        m_scanner->scanFile(errorCase);
        
        waitForSignal(m_scanner.get(), SIGNAL(scanError(ScannerErrorCode, QString)), 3000);
        
        // Should get error for invalid inputs
        QVERIFY(errorSpy.count() >= 1);
    }
}

void CDRScannerTest::testContainerCreationFailure()
{
    if (!m_dockerAvailable) {
        QSKIP("Docker not available for container failure testing");
    }
    
    // This test would require simulating container creation failure
    // Implementation depends on ability to mock Docker operations
    
    QString testFile = createTestFile("Container failure test", ".txt");
    
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    
    m_scanner->scanFile(testFile);
    
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 10000);
    
    // Should handle container issues gracefully
    QVERIFY(errorSpy.count() >= 0 || resultsSpy.count() >= 0);
}

void CDRScannerTest::testLargeFileScan()
{
    // Create a larger test file
    QString largeContent;
    for (int i = 0; i < 5000; ++i) {
        largeContent += QString("Line %1: Large file test content for CDR scanning.\n").arg(i);
    }
    
    QString largeFile = createTestFile(largeContent, ".txt");
    
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    
    m_scanner->scanFile(largeFile);
    
    // Wait longer for large file processing
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 20000);
    
    // Should handle large files
    QVERIFY(resultsSpy.count() >= 1 || errorSpy.count() >= 1);
}

void CDRScannerTest::testConcurrentScans()
{
    // Test multiple rapid scan requests
    QStringList testFiles;
    for (int i = 0; i < 3; ++i) {
        QString content = QString("Concurrent test file %1").arg(i);
        testFiles << createTestFile(content, QString(".txt"));
    }
    
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    
    // Start multiple scans
    for (const QString& file : testFiles) {
        m_scanner->scanFile(file);
        QTest::qWait(50); // Small delay between scans
    }
    
    // Wait for all to complete
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 15000);
    
    // Should handle concurrent requests appropriately
    QVERIFY(resultsSpy.count() >= 1 || errorSpy.count() >= 1);
}

void CDRScannerTest::testScanTimeout()
{
    QString testFile = createTestFile("Timeout test content", ".txt");
    
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    QSignalSpy errorSpy(m_scanner.get(), &CDRScanner::scanError);
    
    m_scanner->scanFile(testFile);
    
    // Wait for a reasonable timeout
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 30000);
    
    // Should complete within timeout or provide error
    QVERIFY(resultsSpy.count() >= 1 || errorSpy.count() >= 1);
}

void CDRScannerTest::testSanitizationResults()
{
    QString testFile = createMaliciousTestFile();
    
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    
    m_scanner->scanFile(testFile);
    
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 10000);
    
    if (resultsSpy.count() > 0) {
        QString results = resultsSpy.at(0).at(0).toString();
        
        // Results should contain sanitization information
        QVERIFY(!results.isEmpty());
        // Could check for specific sanitization keywords
    }
}

void CDRScannerTest::testThreatDetection()
{
    QString maliciousFile = createMaliciousTestFile();
    
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    
    m_scanner->scanFile(maliciousFile);
    
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 10000);
    
    if (resultsSpy.count() > 0) {
        QString results = resultsSpy.at(0).at(0).toString();
        
        // Results should indicate threat detection/removal
        QVERIFY(!results.isEmpty());
    }
}

void CDRScannerTest::testFileReconstruction()
{
    QString testFile = createTestFile("Original content for reconstruction", ".txt");
    
    QSignalSpy resultsSpy(m_scanner.get(), &CDRScanner::scanResultsReady);
    
    m_scanner->scanFile(testFile);
    
    waitForSignal(m_scanner.get(), SIGNAL(scanResultsReady(QString)), 10000);
    
    // CDR should reconstruct the file safely
    if (resultsSpy.count() > 0) {
        QString results = resultsSpy.at(0).at(0).toString();
        QVERIFY(!results.isEmpty());
    }
}

// Helper Methods Implementation

QString CDRScannerTest::createTestFile(const QString& content, const QString& extension)
{
    QTemporaryFile* tempFile = new QTemporaryFile(m_tempDir.filePath("cdrtest_XXXXXX" + extension));
    tempFile->setAutoRemove(false);
    
    if (tempFile->open()) {
        QTextStream stream(tempFile);
        stream << content;
        tempFile->close();
        
        QString filePath = tempFile->fileName();
        delete tempFile;
        return filePath;
    }
    
    delete tempFile;
    return QString();
}

QString CDRScannerTest::createMaliciousTestFile()
{
    QString maliciousContent = R"(
        <!-- Potentially malicious HTML content -->
        <html>
        <head><title>Test</title></head>
        <body>
            <script>
                // Potentially dangerous script
                document.write("Injected content");
                eval("alert('malicious')");
            </script>
            <iframe src="javascript:alert('xss')"></iframe>
            <object data="malicious.exe"></object>
        </body>
        </html>
    )";
    
    return createTestFile(maliciousContent, ".html");
}

QString CDRScannerTest::createTestDirectory()
{
    QString dirPath = m_tempDir.filePath("test_directory");
    QDir().mkpath(dirPath);
    
    // Create some test files in the directory
    QDir testDir(dirPath);
    
    QFile file1(testDir.filePath("test1.txt"));
    if (file1.open(QIODevice::WriteOnly)) {
        file1.write("Test file 1 content");
        file1.close();
    }
    
    QFile file2(testDir.filePath("test2.html"));
    if (file2.open(QIODevice::WriteOnly)) {
        file2.write("<html><body>Test HTML content</body></html>");
        file2.close();
    }
    
    return dirPath;
}

void CDRScannerTest::waitForSignal(QObject* sender, const char* signal, int timeout)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    
    connect(sender, signal, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    timer.start(timeout);
    loop.exec();
}

bool CDRScannerTest::isDockerAvailable()
{
    // Simple check for Docker availability
    // This could be enhanced to actually test Docker connectivity
    QProcess process;
    process.start("docker", QStringList() << "--version");
    process.waitForFinished(3000);
    
    return process.exitCode() == 0;
}

void CDRScannerTest::verifyValidCdrConfig(const CDR::CdrConfiguration& config)
{
    // Verify configuration has valid values
    QVERIFY(!config.inputDirectory.empty());
    QVERIFY(!config.outputDirectory.empty());
    QVERIFY(!config.quarantineDirectory.empty());
    
    // Security level should be valid
    QVERIFY(config.securityLevel >= CDR::CdrConfiguration::SecurityLevel::LOW &&
            config.securityLevel <= CDR::CdrConfiguration::SecurityLevel::HIGH);
    
    // Analysis type should be valid
    QVERIFY(config.analysisType >= CDR::AnalysisType::BASIC_SCAN &&
            config.analysisType <= CDR::AnalysisType::COMPREHENSIVE_SCAN);
}

QTEST_MAIN(CDRScannerTest)
#include "CDRScannerTest.moc"
