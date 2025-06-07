/**
 * @file ScannerInterfaceTest.cpp
 * @brief Unit tests for IScanner interface implementations
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 */

#include <QtTest>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QTextStream>
#include "../../../src/core/interfaces/IScanner.h"

/**
 * @brief Mock implementation of IScanner for testing
 */
class MockScanner : public IScanner {
private:
    QString m_filePath;
    QString m_results;
    QString m_lastError;
    bool m_isScanning;
    bool m_scanResult;

public:
    MockScanner() : m_isScanning(false), m_scanResult(true) {}

    bool selectFile() override {
        // Simulate file selection
        m_filePath = "/tmp/test_file.txt";
        return !m_filePath.isEmpty();
    }

    bool scanFile(const QString& filePath) override {
        if (m_isScanning) {
            m_lastError = "Scanner is already running";
            return false;
        }
        
        if (filePath.isEmpty()) {
            m_lastError = "Invalid file path";
            return false;
        }
        
        m_filePath = filePath;
        m_isScanning = true;
        
        // Simulate scan results
        if (filePath.contains("malicious")) {
            m_results = "THREAT DETECTED: Malicious content found";
        } else if (filePath.contains("error")) {
            m_lastError = "Scan failed: File corrupted";
            m_isScanning = false;
            return false;
        } else {
            m_results = "CLEAN: No threats detected";
        }
        
        m_isScanning = false;
        return m_scanResult;
    }

    QFileInfo getSelectedFile() const override {
        return QFileInfo(m_filePath);
    }

    QString getResults() const override {
        return m_results;
    }

    bool isScanning() const override {
        return m_isScanning;
    }

    bool cancelScan() override {
        if (m_isScanning) {
            m_isScanning = false;
            m_results = "Scan cancelled";
            return true;
        }
        return false;
    }

    QString getLastError() const override {
        return m_lastError;
    }

    void setFile(const QString& filePath) override {
        m_filePath = filePath;
    }

    QString getFile() const override {
        return m_filePath;
    }

    // Test helpers
    void setScanning(bool scanning) { m_isScanning = scanning; }
    void setScanResult(bool result) { m_scanResult = result; }
};

/**
 * @brief Test class for IScanner interface
 */
class ScannerInterfaceTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Interface method tests
    void testSelectFile();
    void testScanFileValid();
    void testScanFileInvalid();
    void testScanFileMalicious();
    void testScanFileError();
    void testGetSelectedFile();
    void testGetResults();
    void testIsScanning();
    void testCancelScan();
    void testCancelScanWhenNotScanning();
    void testGetLastError();
    void testSetAndGetFile();
    
    // Edge cases
    void testScanWhileAlreadyScanning();
    void testEmptyFilePath();
    void testNonExistentFile();
    void testResultsBeforeScan();

private:
    MockScanner* m_scanner;
    QTemporaryDir* m_tempDir;
    QString m_testFilePath;
};

void ScannerInterfaceTest::initTestCase()
{
    m_tempDir = new QTemporaryDir;
    QVERIFY(m_tempDir->isValid());
    
    // Create a test file
    m_testFilePath = m_tempDir->path() + "/test_file.txt";
    QFile testFile(m_testFilePath);
    QVERIFY(testFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&testFile);
    out << "This is a test file for scanner testing.";
    testFile.close();
}

void ScannerInterfaceTest::cleanupTestCase()
{
    delete m_tempDir;
}

void ScannerInterfaceTest::init()
{
    m_scanner = new MockScanner();
}

void ScannerInterfaceTest::cleanup()
{
    delete m_scanner;
}

void ScannerInterfaceTest::testSelectFile()
{
    bool result = m_scanner->selectFile();
    QVERIFY(result);
    QVERIFY(!m_scanner->getFile().isEmpty());
}

void ScannerInterfaceTest::testScanFileValid()
{
    bool result = m_scanner->scanFile(m_testFilePath);
    QVERIFY(result);
    QCOMPARE(m_scanner->getFile(), m_testFilePath);
    QVERIFY(!m_scanner->getResults().isEmpty());
    QVERIFY(m_scanner->getResults().contains("CLEAN"));
}

void ScannerInterfaceTest::testScanFileInvalid()
{
    bool result = m_scanner->scanFile("");
    QVERIFY(!result);
    QVERIFY(!m_scanner->getLastError().isEmpty());
    QVERIFY(m_scanner->getLastError().contains("Invalid file path"));
}

void ScannerInterfaceTest::testScanFileMalicious()
{
    QString maliciousFile = m_tempDir->path() + "/malicious_file.txt";
    bool result = m_scanner->scanFile(maliciousFile);
    QVERIFY(result);
    QVERIFY(m_scanner->getResults().contains("THREAT DETECTED"));
}

void ScannerInterfaceTest::testScanFileError()
{
    QString errorFile = m_tempDir->path() + "/error_file.txt";
    bool result = m_scanner->scanFile(errorFile);
    QVERIFY(!result);
    QVERIFY(!m_scanner->getLastError().isEmpty());
    QVERIFY(m_scanner->getLastError().contains("Scan failed"));
}

void ScannerInterfaceTest::testGetSelectedFile()
{
    m_scanner->setFile(m_testFilePath);
    QFileInfo fileInfo = m_scanner->getSelectedFile();
    QCOMPARE(fileInfo.absoluteFilePath(), m_testFilePath);
}

void ScannerInterfaceTest::testGetResults()
{
    m_scanner->scanFile(m_testFilePath);
    QString results = m_scanner->getResults();
    QVERIFY(!results.isEmpty());
    QVERIFY(results.contains("CLEAN") || results.contains("THREAT"));
}

void ScannerInterfaceTest::testIsScanning()
{
    QVERIFY(!m_scanner->isScanning());
    
    m_scanner->setScanning(true);
    QVERIFY(m_scanner->isScanning());
    
    m_scanner->setScanning(false);
    QVERIFY(!m_scanner->isScanning());
}

void ScannerInterfaceTest::testCancelScan()
{
    m_scanner->setScanning(true);
    QVERIFY(m_scanner->isScanning());
    
    bool cancelled = m_scanner->cancelScan();
    QVERIFY(cancelled);
    QVERIFY(!m_scanner->isScanning());
    QVERIFY(m_scanner->getResults().contains("cancelled"));
}

void ScannerInterfaceTest::testCancelScanWhenNotScanning()
{
    QVERIFY(!m_scanner->isScanning());
    bool cancelled = m_scanner->cancelScan();
    QVERIFY(!cancelled);
}

void ScannerInterfaceTest::testGetLastError()
{
    // Initially no error
    QVERIFY(m_scanner->getLastError().isEmpty());
    
    // Cause an error
    m_scanner->scanFile("");
    QVERIFY(!m_scanner->getLastError().isEmpty());
}

void ScannerInterfaceTest::testSetAndGetFile()
{
    QString testPath = "/test/path/file.txt";
    m_scanner->setFile(testPath);
    QCOMPARE(m_scanner->getFile(), testPath);
}

void ScannerInterfaceTest::testScanWhileAlreadyScanning()
{
    m_scanner->setScanning(true);
    bool result = m_scanner->scanFile(m_testFilePath);
    QVERIFY(!result);
    QVERIFY(m_scanner->getLastError().contains("already running"));
}

void ScannerInterfaceTest::testEmptyFilePath()
{
    m_scanner->setFile("");
    QFileInfo fileInfo = m_scanner->getSelectedFile();
    QVERIFY(fileInfo.filePath().isEmpty());
}

void ScannerInterfaceTest::testNonExistentFile()
{
    QString nonExistentFile = "/non/existent/file.txt";
    m_scanner->setFile(nonExistentFile);
    QFileInfo fileInfo = m_scanner->getSelectedFile();
    QVERIFY(!fileInfo.exists());
}

void ScannerInterfaceTest::testResultsBeforeScan()
{
    QString results = m_scanner->getResults();
    QVERIFY(results.isEmpty());
}

QTEST_MAIN(ScannerInterfaceTest)
#include "ScannerInterfaceTest.moc"
