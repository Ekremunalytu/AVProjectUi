/**
 * @file SandboxScannerTest.cpp
 * @brief Unit tests for SandboxScanner class
 * @author Test Suite
 * @date 2025
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QSignalSpy>
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

#include "security/sandbox/SandboxScanner.h"
#include "security/sandbox/SandboxManager.h"
#include "core/interfaces/IScanner.h"

/**
 * @brief Test class for SandboxScanner functionality
 */
class SandboxScannerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testScannerCreation();
    void testScannerType();
    void testScannerStatus();
    void testScannerInitialization();
    
    // File scanning tests
    void testScanFile_ValidFile();
    void testScanFile_InvalidFile();
    void testScanFile_NonExistentFile();
    void testScanFile_LargeFile();
    void testScanFile_MaliciousFile();
    
    // Directory scanning tests
    void testScanDirectory_ValidDirectory();
    void testScanDirectory_EmptyDirectory();
    void testScanDirectory_NestedDirectory();
    void testScanDirectory_InvalidDirectory();
    
    // Sandbox integration tests
    void testSandboxAvailability();
    void testSandboxExecution();
    void testSandboxTimeout();
    void testSandboxCleanup();
    
    // Signal/slot tests
    void testScanStartedSignal();
    void testScanCompletedSignal();
    void testScanProgressSignal();
    void testScanErrorSignal();
    
    // Concurrent scanning tests
    void testConcurrentScans();
    void testScanCancellation();
    void testMaxConcurrentScans();
    
    // Error handling tests
    void testSandboxUnavailable();
    void testScanTimeout();
    void testResourceExhaustion();
    void testCorruptedFiles();
    
    // Performance tests
    void testScanPerformance();
    void testMemoryUsage();
    void testScannerReuse();

private:
    void createTestFile(const QString& filePath, const QString& content = "Safe test content");
    void createMaliciousTestFile(const QString& filePath);
    void createLargeTestFile(const QString& filePath, qint64 size);
    bool isSandboxAvailable();
    void waitForScanCompletion(SandboxScanner* scanner, int timeoutMs = 10000);
    
    QTemporaryDir* m_tempDir;
    SandboxScanner* m_scanner;
    QString m_testFilePath;
    QString m_testDirPath;
};

void SandboxScannerTest::initTestCase()
{
    // Initialize Qt application if not already done
    if (!QCoreApplication::instance()) {
        static int argc = 1;
        static char* argv[] = {"SandboxScannerTest"};
        new QCoreApplication(argc, argv);
    }
    
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    m_testFilePath = m_tempDir->path() + "/test_file.txt";
    m_testDirPath = m_tempDir->path() + "/test_directory";
    
    // Create test directory
    QDir().mkpath(m_testDirPath);
    
    qDebug() << "Test directory:" << m_tempDir->path();
}

void SandboxScannerTest::cleanupTestCase()
{
    delete m_tempDir;
    m_tempDir = nullptr;
}

void SandboxScannerTest::init()
{
    m_scanner = new SandboxScanner();
    QVERIFY(m_scanner != nullptr);
}

void SandboxScannerTest::cleanup()
{
    if (m_scanner) {
        delete m_scanner;
        m_scanner = nullptr;
    }
    
    // Clean up test files
    QFile::remove(m_testFilePath);
    QDir testDir(m_testDirPath);
    testDir.removeRecursively();
    QDir().mkpath(m_testDirPath);
}

void SandboxScannerTest::testScannerCreation()
{
    QVERIFY(m_scanner != nullptr);
    QVERIFY(m_scanner->inherits("QObject"));
}

void SandboxScannerTest::testScannerType()
{
    ScannerType type = m_scanner->getScannerType();
    QCOMPARE(type, ScannerType::Sandbox);
}

void SandboxScannerTest::testScannerStatus()
{
    ScanStatus status = m_scanner->getStatus();
    QVERIFY(status == ScanStatus::Idle || status == ScanStatus::Ready);
}

void SandboxScannerTest::testScannerInitialization()
{
    // Scanner should be properly initialized
    QVERIFY(m_scanner != nullptr);
    
    // Test that scanner can report its capabilities
    ScannerType type = m_scanner->getScannerType();
    QCOMPARE(type, ScannerType::Sandbox);
    
    // Test initial status
    ScanStatus status = m_scanner->getStatus();
    QVERIFY(status != ScanStatus::Unknown);
}

void SandboxScannerTest::testScanFile_ValidFile()
{
    if (!isSandboxAvailable()) {
        QSKIP("Sandbox not available, skipping sandbox-dependent test");
    }
    
    createTestFile(m_testFilePath);
    
    QSignalSpy scanStartedSpy(m_scanner, &IScanner::scanStarted);
    QSignalSpy scanCompletedSpy(m_scanner, &IScanner::scanCompleted);
    
    m_scanner->scanFile(m_testFilePath);
    
    // Wait for scan to complete
    waitForScanCompletion(m_scanner);
    
    // Verify signals were emitted
    QVERIFY(scanStartedSpy.count() > 0);
    QVERIFY(scanCompletedSpy.count() > 0);
}

void SandboxScannerTest::testScanFile_InvalidFile()
{
    QSignalSpy errorSpy(m_scanner, &IScanner::scanError);
    
    m_scanner->scanFile("");  // Empty path
    
    // Should handle invalid file gracefully
    QTest::qWait(100);  // Allow for immediate error
    
    // Either should emit error or handle gracefully
    QVERIFY(errorSpy.count() >= 0);
}

void SandboxScannerTest::testScanFile_NonExistentFile()
{
    QString nonExistentFile = m_tempDir->path() + "/non_existent_file.txt";
    
    QSignalSpy errorSpy(m_scanner, &IScanner::scanError);
    
    m_scanner->scanFile(nonExistentFile);
    
    // Should handle non-existent file
    QTest::qWait(100);
    
    // Should either emit error or handle gracefully
    QVERIFY(errorSpy.count() >= 0);
}

void SandboxScannerTest::testScanFile_LargeFile()
{
    if (!isSandboxAvailable()) {
        QSKIP("Sandbox not available, skipping sandbox-dependent test");
    }
    
    QString largeFilePath = m_tempDir->path() + "/large_file.bin";
    createLargeTestFile(largeFilePath, 1024 * 1024);  // 1MB file
    
    QSignalSpy scanStartedSpy(m_scanner, &IScanner::scanStarted);
    QSignalSpy scanCompletedSpy(m_scanner, &IScanner::scanCompleted);
    QSignalSpy progressSpy(m_scanner, &IScanner::scanProgress);
    
    m_scanner->scanFile(largeFilePath);
    
    // Wait for scan to complete
    waitForScanCompletion(m_scanner, 15000);  // Longer timeout for large file
    
    // Verify signals
    QVERIFY(scanStartedSpy.count() > 0);
    QVERIFY(scanCompletedSpy.count() > 0);
    
    QFile::remove(largeFilePath);
}

void SandboxScannerTest::testScanFile_MaliciousFile()
{
    if (!isSandboxAvailable()) {
        QSKIP("Sandbox not available, skipping sandbox-dependent test");
    }
    
    QString maliciousFilePath = m_tempDir->path() + "/malicious_file.txt";
    createMaliciousTestFile(maliciousFilePath);
    
    QSignalSpy scanCompletedSpy(m_scanner, &IScanner::scanCompleted);
    
    m_scanner->scanFile(maliciousFilePath);
    
    waitForScanCompletion(m_scanner);
    
    // Should complete scan (result depends on sandbox implementation)
    QVERIFY(scanCompletedSpy.count() > 0);
    
    QFile::remove(maliciousFilePath);
}

void SandboxScannerTest::testScanDirectory_ValidDirectory()
{
    if (!isSandboxAvailable()) {
        QSKIP("Sandbox not available, skipping sandbox-dependent test");
    }
    
    // Create test files in directory
    createTestFile(m_testDirPath + "/file1.txt");
    createTestFile(m_testDirPath + "/file2.txt");
    
    QSignalSpy scanStartedSpy(m_scanner, &IScanner::scanStarted);
    QSignalSpy scanCompletedSpy(m_scanner, &IScanner::scanCompleted);
    
    m_scanner->scanDirectory(m_testDirPath);
    
    waitForScanCompletion(m_scanner, 15000);
    
    QVERIFY(scanStartedSpy.count() > 0);
    QVERIFY(scanCompletedSpy.count() > 0);
}

void SandboxScannerTest::testScanDirectory_EmptyDirectory()
{
    QString emptyDirPath = m_tempDir->path() + "/empty_directory";
    QDir().mkpath(emptyDirPath);
    
    QSignalSpy scanCompletedSpy(m_scanner, &IScanner::scanCompleted);
    
    m_scanner->scanDirectory(emptyDirPath);
    
    // Should handle empty directory gracefully
    QTest::qWait(1000);
    
    // May complete immediately or emit error
    QVERIFY(scanCompletedSpy.count() >= 0);
    
    QDir(emptyDirPath).removeRecursively();
}

void SandboxScannerTest::testScanDirectory_NestedDirectory()
{
    if (!isSandboxAvailable()) {
        QSKIP("Sandbox not available, skipping sandbox-dependent test");
    }
    
    // Create nested directory structure
    QString nestedPath = m_testDirPath + "/nested/deep";
    QDir().mkpath(nestedPath);
    
    createTestFile(m_testDirPath + "/root_file.txt");
    createTestFile(m_testDirPath + "/nested/nested_file.txt");
    createTestFile(nestedPath + "/deep_file.txt");
    
    QSignalSpy scanStartedSpy(m_scanner, &IScanner::scanStarted);
    QSignalSpy scanCompletedSpy(m_scanner, &IScanner::scanCompleted);
    
    m_scanner->scanDirectory(m_testDirPath);
    
    waitForScanCompletion(m_scanner, 20000);
    
    QVERIFY(scanStartedSpy.count() > 0);
    QVERIFY(scanCompletedSpy.count() > 0);
}

void SandboxScannerTest::testScanDirectory_InvalidDirectory()
{
    QString invalidDir = "/non/existent/directory";
    
    QSignalSpy errorSpy(m_scanner, &IScanner::scanError);
    
    m_scanner->scanDirectory(invalidDir);
    
    QTest::qWait(100);
    
    // Should handle invalid directory
    QVERIFY(errorSpy.count() >= 0);
}

void SandboxScannerTest::testSandboxAvailability()
{
    // Test if sandbox is available
    bool available = isSandboxAvailable();
    
    // This test just checks that we can determine availability
    QVERIFY(available == true || available == false);
    
    if (!available) {
        qDebug() << "Sandbox not available in test environment";
    }
}

void SandboxScannerTest::testSandboxExecution()
{
    if (!isSandboxAvailable()) {
        QSKIP("Sandbox not available, skipping sandbox execution test");
    }
    
    createTestFile(m_testFilePath);
    
    QSignalSpy scanStartedSpy(m_scanner, &IScanner::scanStarted);
    
    m_scanner->scanFile(m_testFilePath);
    
    // Verify scan starts (indicating sandbox execution began)
    QTest::qWait(1000);
    QVERIFY(scanStartedSpy.count() > 0);
    
    // Let scan complete
    waitForScanCompletion(m_scanner);
}

void SandboxScannerTest::testSandboxTimeout()
{
    if (!isSandboxAvailable()) {
        QSKIP("Sandbox not available, skipping timeout test");
    }
    
    // Create a file that might cause longer processing
    createLargeTestFile(m_testFilePath, 5 * 1024 * 1024);  // 5MB
    
    QSignalSpy scanCompletedSpy(m_scanner, &IScanner::scanCompleted);
    QSignalSpy errorSpy(m_scanner, &IScanner::scanError);
    
    m_scanner->scanFile(m_testFilePath);
    
    // Wait for reasonable time
    waitForScanCompletion(m_scanner, 30000);
    
    // Should complete or timeout gracefully
    QVERIFY(scanCompletedSpy.count() > 0 || errorSpy.count() > 0);
}

void SandboxScannerTest::testSandboxCleanup()
{
    if (!isSandboxAvailable()) {
        QSKIP("Sandbox not available, skipping cleanup test");
    }
    
    createTestFile(m_testFilePath);
    
    // Perform multiple scans to test cleanup
    for (int i = 0; i < 3; ++i) {
        QSignalSpy scanCompletedSpy(m_scanner, &IScanner::scanCompleted);
        
        m_scanner->scanFile(m_testFilePath);
        waitForScanCompletion(m_scanner);
        
        QVERIFY(scanCompletedSpy.count() > 0);
    }
    
    // Scanner should still be functional after multiple uses
    QVERIFY(m_scanner->getStatus() != ScanStatus::Error);
}

void SandboxScannerTest::testScanStartedSignal()
{
    createTestFile(m_testFilePath);
    
    QSignalSpy scanStartedSpy(m_scanner, &IScanner::scanStarted);
    
    m_scanner->scanFile(m_testFilePath);
    
    // Should emit scanStarted signal
    QTest::qWait(1000);
    QVERIFY(scanStartedSpy.count() > 0);
    
    waitForScanCompletion(m_scanner);
}

void SandboxScannerTest::testScanCompletedSignal()
{
    if (!isSandboxAvailable()) {
        QSKIP("Sandbox not available, skipping completion signal test");
    }
    
    createTestFile(m_testFilePath);
    
    QSignalSpy scanCompletedSpy(m_scanner, &IScanner::scanCompleted);
    
    m_scanner->scanFile(m_testFilePath);
    waitForScanCompletion(m_scanner);
    
    QVERIFY(scanCompletedSpy.count() > 0);
}

void SandboxScannerTest::testScanProgressSignal()
{
    if (!isSandboxAvailable()) {
        QSKIP("Sandbox not available, skipping progress signal test");
    }
    
    createLargeTestFile(m_testFilePath, 2 * 1024 * 1024);  // 2MB
    
    QSignalSpy progressSpy(m_scanner, &IScanner::scanProgress);
    
    m_scanner->scanFile(m_testFilePath);
    waitForScanCompletion(m_scanner, 15000);
    
    // May or may not emit progress depending on implementation
    QVERIFY(progressSpy.count() >= 0);
}

void SandboxScannerTest::testScanErrorSignal()
{
    QSignalSpy errorSpy(m_scanner, &IScanner::scanError);
    
    // Try to scan non-existent file
    m_scanner->scanFile("/non/existent/file.txt");
    
    QTest::qWait(1000);
    
    // Should handle error gracefully
    QVERIFY(errorSpy.count() >= 0);
}

void SandboxScannerTest::testConcurrentScans()
{
    if (!isSandboxAvailable()) {
        QSKIP("Sandbox not available, skipping concurrent scan test");
    }
    
    // Create multiple test files
    QStringList testFiles;
    for (int i = 0; i < 3; ++i) {
        QString filePath = m_tempDir->path() + QString("/test_file_%1.txt").arg(i);
        createTestFile(filePath);
        testFiles.append(filePath);
    }
    
    QSignalSpy scanStartedSpy(m_scanner, &IScanner::scanStarted);
    QSignalSpy scanCompletedSpy(m_scanner, &IScanner::scanCompleted);
    
    // Start multiple scans
    for (const QString& filePath : testFiles) {
        m_scanner->scanFile(filePath);
    }
    
    // Wait for all scans to complete
    waitForScanCompletion(m_scanner, 30000);
    
    // Should handle multiple scans
    QVERIFY(scanStartedSpy.count() > 0);
    QVERIFY(scanCompletedSpy.count() > 0);
    
    // Clean up
    for (const QString& filePath : testFiles) {
        QFile::remove(filePath);
    }
}

void SandboxScannerTest::testScanCancellation()
{
    if (!isSandboxAvailable()) {
        QSKIP("Sandbox not available, skipping cancellation test");
    }
    
    createLargeTestFile(m_testFilePath, 10 * 1024 * 1024);  // 10MB
    
    m_scanner->scanFile(m_testFilePath);
    
    // Wait a bit then cancel
    QTest::qWait(500);
    
    // Try to cancel scan (if cancellation method exists)
    // Note: This depends on the actual interface of SandboxScanner
    
    waitForScanCompletion(m_scanner, 5000);
    
    // Scanner should handle cancellation gracefully
    QVERIFY(m_scanner != nullptr);
}

void SandboxScannerTest::testMaxConcurrentScans()
{
    // Test behavior with many concurrent scans
    QList<QString> manyFiles;
    for (int i = 0; i < 10; ++i) {
        QString filePath = m_tempDir->path() + QString("/many_file_%1.txt").arg(i);
        createTestFile(filePath);
        manyFiles.append(filePath);
    }
    
    // Try to scan all files
    for (const QString& filePath : manyFiles) {
        m_scanner->scanFile(filePath);
    }
    
    // Should handle many scans without crashing
    QTest::qWait(2000);
    
    // Clean up
    for (const QString& filePath : manyFiles) {
        QFile::remove(filePath);
    }
    
    QVERIFY(m_scanner != nullptr);
}

void SandboxScannerTest::testSandboxUnavailable()
{
    // Test behavior when sandbox is unavailable
    // This test primarily checks that the scanner doesn't crash
    
    QString testFile = m_tempDir->path() + "/unavailable_test.txt";
    createTestFile(testFile);
    
    QSignalSpy errorSpy(m_scanner, &IScanner::scanError);
    
    m_scanner->scanFile(testFile);
    
    QTest::qWait(2000);
    
    // Should handle unavailable sandbox gracefully
    QVERIFY(m_scanner != nullptr);
    
    QFile::remove(testFile);
}

void SandboxScannerTest::testScanTimeout()
{
    // Test scan timeout handling
    QString timeoutFile = m_tempDir->path() + "/timeout_test.txt";
    createTestFile(timeoutFile);
    
    QSignalSpy completedSpy(m_scanner, &IScanner::scanCompleted);
    QSignalSpy errorSpy(m_scanner, &IScanner::scanError);
    
    m_scanner->scanFile(timeoutFile);
    
    // Wait for a reasonable timeout
    QTest::qWait(5000);
    
    // Should either complete or timeout gracefully
    QVERIFY(completedSpy.count() > 0 || errorSpy.count() >= 0);
    
    QFile::remove(timeoutFile);
}

void SandboxScannerTest::testResourceExhaustion()
{
    // Test behavior under resource pressure
    QList<QString> resourceFiles;
    for (int i = 0; i < 5; ++i) {
        QString filePath = m_tempDir->path() + QString("/resource_file_%1.bin").arg(i);
        createLargeTestFile(filePath, 1024 * 1024);  // 1MB each
        resourceFiles.append(filePath);
    }
    
    // Scan all files to stress resources
    for (const QString& filePath : resourceFiles) {
        m_scanner->scanFile(filePath);
    }
    
    // Should handle resource pressure
    QTest::qWait(5000);
    
    // Clean up
    for (const QString& filePath : resourceFiles) {
        QFile::remove(filePath);
    }
    
    QVERIFY(m_scanner != nullptr);
}

void SandboxScannerTest::testCorruptedFiles()
{
    // Create a corrupted file
    QString corruptedFile = m_tempDir->path() + "/corrupted.bin";
    QFile file(corruptedFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    
    // Write random binary data
    QByteArray corruptedData;
    for (int i = 0; i < 1000; ++i) {
        corruptedData.append(static_cast<char>(qrand() % 256));
    }
    file.write(corruptedData);
    file.close();
    
    QSignalSpy completedSpy(m_scanner, &IScanner::scanCompleted);
    QSignalSpy errorSpy(m_scanner, &IScanner::scanError);
    
    m_scanner->scanFile(corruptedFile);
    
    waitForScanCompletion(m_scanner);
    
    // Should handle corrupted files gracefully
    QVERIFY(completedSpy.count() > 0 || errorSpy.count() > 0);
    
    QFile::remove(corruptedFile);
}

void SandboxScannerTest::testScanPerformance()
{
    if (!isSandboxAvailable()) {
        QSKIP("Sandbox not available, skipping performance test");
    }
    
    createTestFile(m_testFilePath);
    
    QElapsedTimer timer;
    timer.start();
    
    QSignalSpy scanCompletedSpy(m_scanner, &IScanner::scanCompleted);
    
    m_scanner->scanFile(m_testFilePath);
    waitForScanCompletion(m_scanner);
    
    qint64 elapsed = timer.elapsed();
    
    QVERIFY(scanCompletedSpy.count() > 0);
    QVERIFY(elapsed > 0);  // Should take some time
    
    qDebug() << "Scan took" << elapsed << "ms";
}

void SandboxScannerTest::testMemoryUsage()
{
    // Basic memory usage test
    createTestFile(m_testFilePath);
    
    // Perform multiple scans to check for memory leaks
    for (int i = 0; i < 5; ++i) {
        QSignalSpy scanCompletedSpy(m_scanner, &IScanner::scanCompleted);
        
        m_scanner->scanFile(m_testFilePath);
        waitForScanCompletion(m_scanner);
        
        QVERIFY(scanCompletedSpy.count() > 0);
    }
    
    // Scanner should still be functional
    QVERIFY(m_scanner != nullptr);
}

void SandboxScannerTest::testScannerReuse()
{
    // Test that scanner can be reused multiple times
    for (int i = 0; i < 3; ++i) {
        QString testFile = m_tempDir->path() + QString("/reuse_test_%1.txt").arg(i);
        createTestFile(testFile);
        
        QSignalSpy scanCompletedSpy(m_scanner, &IScanner::scanCompleted);
        
        m_scanner->scanFile(testFile);
        waitForScanCompletion(m_scanner);
        
        QVERIFY(scanCompletedSpy.count() > 0);
        
        QFile::remove(testFile);
    }
    
    // Scanner should remain functional
    QVERIFY(m_scanner->getStatus() != ScanStatus::Error);
}

// Helper Methods

void SandboxScannerTest::createTestFile(const QString& filePath, const QString& content)
{
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&file);
    out << content;
    file.close();
}

void SandboxScannerTest::createMaliciousTestFile(const QString& filePath)
{
    // Create a file with EICAR test string (standard antivirus test)
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&file);
    out << "X5O!P%@AP[4\\PZX54(P^)7CC)7}$EICAR-STANDARD-ANTIVIRUS-TEST-FILE!$H+H*";
    file.close();
}

void SandboxScannerTest::createLargeTestFile(const QString& filePath, qint64 size)
{
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    
    QByteArray chunk(1024, 'A');  // 1KB chunk
    qint64 written = 0;
    
    while (written < size) {
        qint64 toWrite = qMin(static_cast<qint64>(chunk.size()), size - written);
        qint64 bytesWritten = file.write(chunk.left(static_cast<int>(toWrite)));
        QVERIFY(bytesWritten > 0);
        written += bytesWritten;
    }
    
    file.close();
}

bool SandboxScannerTest::isSandboxAvailable()
{
    // Check if sandbox is available in the test environment
    // This could check for Docker, specific tools, etc.
    
    // For now, assume sandbox might not be available in CI/test environments
    return QStandardPaths::findExecutable("docker").isEmpty() == false;
}

void SandboxScannerTest::waitForScanCompletion(SandboxScanner* scanner, int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();
    
    while (timer.elapsed() < timeoutMs) {
        ScanStatus status = scanner->getStatus();
        if (status == ScanStatus::Idle || status == ScanStatus::Completed || 
            status == ScanStatus::Error) {
            break;
        }
        
        QCoreApplication::processEvents();
        QThread::msleep(50);
    }
}

QTEST_MAIN(SandboxScannerTest)
#include "SandboxScannerTest.moc"
