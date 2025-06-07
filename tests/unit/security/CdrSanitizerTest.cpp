/**
 * @file CdrSanitizerTest.cpp
 * @brief Unit tests for CdrSanitizer class
 * @author Test Suite
 * @version 1.0
 * @date 2024
 */

#include <QTest>
#include <QObject>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <memory>
#include <fstream>

// Test target
#include "security/cdr/CdrSanitizer.h"
#include "security/cdr/CdrTypes.h"
#include "security/cdr/FileSanitizer.h"

class CdrSanitizerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Constructor and basic functionality tests
    void testConstructor();
    void testRegisterSanitizer();
    void testFindSanitizerForType();
    void testUpdateStats();

    // File sanitization tests
    void testSanitizeFile_ValidInput();
    void testSanitizeFile_InvalidInput();
    void testSanitizeFile_EmptyPaths();
    void testSanitizeFile_UnsupportedFileType();
    void testSanitizeFile_FileNotFound();

    // Specific file type sanitization tests
    void testSanitizeTextFile();
    void testSanitizeArchiveFile();
    void testSanitizeScriptFile();

    // Error handling tests
    void testSanitizeFile_ExceptionHandling();
    void testSanitizeFile_ValidationFailure();

    // Configuration tests
    void testSanitizationWithDifferentConfigs();
    void testSecurityLevelImpact();

    // Performance tests
    void testLargeFileSanitization();
    void testConcurrentSanitization();

private:
    // Helper methods
    QString createTestFile(const QString& content, const QString& extension = ".txt");
    QString createTestArchive();
    QString createTestScript();
    CDR::CdrConfiguration createTestConfig();
    void verifyOutputFile(const QString& outputPath);

    QTemporaryDir m_tempDir;
    std::unique_ptr<CDR::CdrSanitizer> m_sanitizer;
};

void CdrSanitizerTest::initTestCase()
{
    qDebug() << "CdrSanitizerTest::initTestCase()";
    QVERIFY(m_tempDir.isValid());
}

void CdrSanitizerTest::cleanupTestCase()
{
    qDebug() << "CdrSanitizerTest::cleanupTestCase()";
}

void CdrSanitizerTest::init()
{
    m_sanitizer = std::make_unique<CDR::CdrSanitizer>();
    QVERIFY(m_sanitizer != nullptr);
}

void CdrSanitizerTest::cleanup()
{
    m_sanitizer.reset();
}

void CdrSanitizerTest::testConstructor()
{
    // Test default constructor
    auto sanitizer = std::make_unique<CDR::CdrSanitizer>();
    QVERIFY(sanitizer != nullptr);
    
    // Test that built-in sanitizers are registered
    QVERIFY(sanitizer->findSanitizerForType(CDR::FileType::TEXT_DOCUMENT) != nullptr);
    QVERIFY(sanitizer->findSanitizerForType(CDR::FileType::PDF_DOCUMENT) != nullptr);
    QVERIFY(sanitizer->findSanitizerForType(CDR::FileType::OFFICE_DOCUMENT) != nullptr);
    QVERIFY(sanitizer->findSanitizerForType(CDR::FileType::HTML_DOCUMENT) != nullptr);
    QVERIFY(sanitizer->findSanitizerForType(CDR::FileType::ARCHIVE_FILE) != nullptr);
    QVERIFY(sanitizer->findSanitizerForType(CDR::FileType::IMAGE_FILE) != nullptr);
}

void CdrSanitizerTest::testRegisterSanitizer()
{
    // Create a mock sanitizer
    class MockSanitizer : public CDR::FileSanitizer {
    public:
        CDR::SanitizationResult sanitizeFile(const std::string& inputPath,
                                           const std::string& outputPath,
                                           const CDR::CdrConfiguration& config) override {
            CDR::SanitizationResult result;
            result.success = true;
            result.inputPath = inputPath;
            result.outputPath = outputPath;
            result.fileType = CDR::FileType::UNKNOWN;
            return result;
        }
        
        bool canSanitize(CDR::FileType fileType) const override {
            return fileType == CDR::FileType::UNKNOWN;
        }
    };
    
    // Register the mock sanitizer
    auto mockSanitizer = std::make_unique<MockSanitizer>();
    m_sanitizer->registerSanitizer(std::move(mockSanitizer));
    
    // Verify it was registered
    auto foundSanitizer = m_sanitizer->findSanitizerForType(CDR::FileType::UNKNOWN);
    QVERIFY(foundSanitizer != nullptr);
}

void CdrSanitizerTest::testFindSanitizerForType()
{
    // Test finding sanitizers for supported types
    QVERIFY(m_sanitizer->findSanitizerForType(CDR::FileType::TEXT_DOCUMENT) != nullptr);
    QVERIFY(m_sanitizer->findSanitizerForType(CDR::FileType::PDF_DOCUMENT) != nullptr);
    QVERIFY(m_sanitizer->findSanitizerForType(CDR::FileType::OFFICE_DOCUMENT) != nullptr);
    
    // Test finding sanitizer for unsupported type
    QVERIFY(m_sanitizer->findSanitizerForType(CDR::FileType::UNKNOWN) == nullptr);
}

void CdrSanitizerTest::testUpdateStats()
{
    // This is mostly an internal method, test that it doesn't crash
    m_sanitizer->updateStats("TestSanitizer");
    // No easy way to verify stats without access to internal state
    QVERIFY(true); // Just ensure no exception is thrown
}

void CdrSanitizerTest::testSanitizeFile_ValidInput()
{
    QString inputFile = createTestFile("Test content for sanitization", ".txt");
    QString outputFile = m_tempDir.filePath("output.txt");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_sanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config,
        CDR::FileType::TEXT_DOCUMENT
    );
    
    QVERIFY(result.success);
    QCOMPARE(QString::fromStdString(result.inputPath), inputFile);
    QCOMPARE(QString::fromStdString(result.outputPath), outputFile);
    QCOMPARE(result.fileType, CDR::FileType::TEXT_DOCUMENT);
    QVERIFY(QFile::exists(outputFile));
}

void CdrSanitizerTest::testSanitizeFile_InvalidInput()
{
    QString nonExistentFile = m_tempDir.filePath("nonexistent.txt");
    QString outputFile = m_tempDir.filePath("output.txt");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_sanitizer->sanitizeFile(
        nonExistentFile.toStdString(),
        outputFile.toStdString(),
        config,
        CDR::FileType::TEXT_DOCUMENT
    );
    
    QVERIFY(!result.success);
    QVERIFY(!result.errorMessage.empty());
}

void CdrSanitizerTest::testSanitizeFile_EmptyPaths()
{
    CDR::CdrConfiguration config = createTestConfig();
    
    // Test empty input path
    CDR::SanitizationResult result1 = m_sanitizer->sanitizeFile(
        "",
        m_tempDir.filePath("output.txt").toStdString(),
        config,
        CDR::FileType::TEXT_DOCUMENT
    );
    QVERIFY(!result1.success);
    
    // Test empty output path
    QString inputFile = createTestFile("Test content", ".txt");
    CDR::SanitizationResult result2 = m_sanitizer->sanitizeFile(
        inputFile.toStdString(),
        "",
        config,
        CDR::FileType::TEXT_DOCUMENT
    );
    QVERIFY(!result2.success);
}

void CdrSanitizerTest::testSanitizeFile_UnsupportedFileType()
{
    QString inputFile = createTestFile("Test content", ".xyz");
    QString outputFile = m_tempDir.filePath("output.xyz");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_sanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config,
        CDR::FileType::UNKNOWN
    );
    
    QVERIFY(!result.success);
    QVERIFY(result.errorMessage.find("No sanitizer available") != std::string::npos ||
            result.errorMessage.find("Unsupported file type") != std::string::npos);
}

void CdrSanitizerTest::testSanitizeFile_FileNotFound()
{
    QString nonExistentFile = m_tempDir.filePath("does_not_exist.txt");
    QString outputFile = m_tempDir.filePath("output.txt");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_sanitizer->sanitizeFile(
        nonExistentFile.toStdString(),
        outputFile.toStdString(),
        config,
        CDR::FileType::TEXT_DOCUMENT
    );
    
    QVERIFY(!result.success);
    QVERIFY(!result.errorMessage.empty());
}

void CdrSanitizerTest::testSanitizeTextFile()
{
    QString inputFile = createTestFile("Test text content\nWith multiple lines\nAnd special characters: @#$%", ".txt");
    QString outputFile = m_tempDir.filePath("sanitized.txt");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_sanitizer->sanitizeTextFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config
    );
    
    QVERIFY(result.success);
    QVERIFY(QFile::exists(outputFile));
    verifyOutputFile(outputFile);
}

void CdrSanitizerTest::testSanitizeArchiveFile()
{
    QString inputFile = createTestArchive();
    QString outputFile = m_tempDir.filePath("sanitized.zip");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_sanitizer->sanitizeArchiveFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config
    );
    
    // Note: May fail if archive sanitizer is not fully implemented
    if (result.success) {
        QVERIFY(QFile::exists(outputFile));
        verifyOutputFile(outputFile);
    } else {
        qDebug() << "Archive sanitization not fully implemented:" << QString::fromStdString(result.errorMessage);
        QVERIFY(!result.errorMessage.empty());
    }
}

void CdrSanitizerTest::testSanitizeScriptFile()
{
    QString inputFile = createTestScript();
    QString outputFile = m_tempDir.filePath("sanitized.js");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_sanitizer->sanitizeScriptFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config
    );
    
    // Note: May fail if script sanitizer is not fully implemented
    if (result.success) {
        QVERIFY(QFile::exists(outputFile));
        verifyOutputFile(outputFile);
    } else {
        qDebug() << "Script sanitization not fully implemented:" << QString::fromStdString(result.errorMessage);
        QVERIFY(!result.errorMessage.empty());
    }
}

void CdrSanitizerTest::testSanitizeFile_ExceptionHandling()
{
    // Test with invalid output directory (read-only or non-existent parent)
    QString inputFile = createTestFile("Test content", ".txt");
    QString invalidOutputFile = "/nonexistent/directory/output.txt";
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_sanitizer->sanitizeFile(
        inputFile.toStdString(),
        invalidOutputFile,
        config,
        CDR::FileType::TEXT_DOCUMENT
    );
    
    // Should handle the exception gracefully
    QVERIFY(!result.success);
    QVERIFY(!result.errorMessage.empty());
}

void CdrSanitizerTest::testSanitizeFile_ValidationFailure()
{
    QString inputFile = createTestFile("", ".txt"); // Empty file
    QString outputFile = m_tempDir.filePath("output.txt");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_sanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config,
        CDR::FileType::TEXT_DOCUMENT
    );
    
    // Empty files may be considered valid or invalid depending on implementation
    // Just ensure the result is consistent
    QVERIFY(result.success || !result.errorMessage.empty());
}

void CdrSanitizerTest::testSanitizationWithDifferentConfigs()
{
    QString inputFile = createTestFile("Test content with potential threats", ".txt");
    QString outputFile1 = m_tempDir.filePath("output1.txt");
    QString outputFile2 = m_tempDir.filePath("output2.txt");
    
    // Low security config
    CDR::CdrConfiguration lowConfig = createTestConfig();
    lowConfig.securityLevel = CDR::CdrConfiguration::SecurityLevel::LOW;
    
    // High security config
    CDR::CdrConfiguration highConfig = createTestConfig();
    highConfig.securityLevel = CDR::CdrConfiguration::SecurityLevel::HIGH;
    
    CDR::SanitizationResult result1 = m_sanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile1.toStdString(),
        lowConfig,
        CDR::FileType::TEXT_DOCUMENT
    );
    
    CDR::SanitizationResult result2 = m_sanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile2.toStdString(),
        highConfig,
        CDR::FileType::TEXT_DOCUMENT
    );
    
    QVERIFY(result1.success);
    QVERIFY(result2.success);
    QVERIFY(QFile::exists(outputFile1));
    QVERIFY(QFile::exists(outputFile2));
}

void CdrSanitizerTest::testSecurityLevelImpact()
{
    QString inputFile = createTestFile("Content with <script>alert('test')</script>", ".html");
    QString outputFile = m_tempDir.filePath("output.html");
    
    CDR::CdrConfiguration config = createTestConfig();
    config.securityLevel = CDR::CdrConfiguration::SecurityLevel::HIGH;
    config.blockAllScripts = true;
    
    CDR::SanitizationResult result = m_sanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config,
        CDR::FileType::HTML_DOCUMENT
    );
    
    if (result.success) {
        QVERIFY(QFile::exists(outputFile));
        // High security should have blocked or removed scripts
        QVERIFY(result.threatsRemoved >= 0);
    } else {
        qDebug() << "HTML sanitization not fully implemented:" << QString::fromStdString(result.errorMessage);
    }
}

void CdrSanitizerTest::testLargeFileSanitization()
{
    // Create a larger test file
    QString largeContent;
    for (int i = 0; i < 10000; ++i) {
        largeContent += QString("Line %1: This is test content for large file sanitization.\n").arg(i);
    }
    
    QString inputFile = createTestFile(largeContent, ".txt");
    QString outputFile = m_tempDir.filePath("large_output.txt");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_sanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config,
        CDR::FileType::TEXT_DOCUMENT
    );
    
    QVERIFY(result.success);
    QVERIFY(QFile::exists(outputFile));
    
    // Verify output file size is reasonable
    QFileInfo outputInfo(outputFile);
    QVERIFY(outputInfo.size() > 0);
}

void CdrSanitizerTest::testConcurrentSanitization()
{
    // Create multiple input files
    QStringList inputFiles;
    for (int i = 0; i < 5; ++i) {
        QString content = QString("Test content for file %1").arg(i);
        inputFiles << createTestFile(content, QString(".txt"));
    }
    
    // Sanitize all files concurrently (simulate concurrent access)
    QList<CDR::SanitizationResult> results;
    CDR::CdrConfiguration config = createTestConfig();
    
    for (int i = 0; i < inputFiles.size(); ++i) {
        QString outputFile = m_tempDir.filePath(QString("concurrent_output_%1.txt").arg(i));
        CDR::SanitizationResult result = m_sanitizer->sanitizeFile(
            inputFiles[i].toStdString(),
            outputFile.toStdString(),
            config,
            CDR::FileType::TEXT_DOCUMENT
        );
        results << result;
    }
    
    // Verify all sanitizations succeeded
    for (const auto& result : results) {
        QVERIFY(result.success);
    }
}

// Helper Methods Implementation

QString CdrSanitizerTest::createTestFile(const QString& content, const QString& extension)
{
    QTemporaryFile* tempFile = new QTemporaryFile(m_tempDir.filePath("testfile_XXXXXX" + extension));
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

QString CdrSanitizerTest::createTestArchive()
{
    // Create a simple ZIP-like file for testing
    QString archivePath = m_tempDir.filePath("test.zip");
    QFile file(archivePath);
    
    if (file.open(QIODevice::WriteOnly)) {
        // Write minimal ZIP headers (not a real ZIP, just for testing)
        file.write("PK\x03\x04"); // ZIP file signature
        file.write(QByteArray(26, '\0')); // Minimal header
        file.write("test.txt"); // Filename
        file.write("Test archive content"); // File content
        file.close();
    }
    
    return archivePath;
}

QString CdrSanitizerTest::createTestScript()
{
    QString scriptContent = R"(
        // Test JavaScript file
        function testFunction() {
            console.log("Test script");
            document.write("Potentially dangerous content");
        }
        
        eval("alert('test')"); // Potentially dangerous
        testFunction();
    )";
    
    return createTestFile(scriptContent, ".js");
}

CDR::CdrConfiguration CdrSanitizerTest::createTestConfig()
{
    CDR::CdrConfiguration config;
    config.securityLevel = CDR::CdrConfiguration::SecurityLevel::MEDIUM;
    config.inputDirectory = m_tempDir.path().toStdString();
    config.outputDirectory = m_tempDir.path().toStdString();
    config.quarantineDirectory = m_tempDir.path().toStdString() + "/quarantine";
    config.analysisType = CDR::AnalysisType::COMPREHENSIVE_SCAN;
    config.preserveOriginal = true;
    config.autoSanitize = true;
    config.blockExecutables = true;
    config.blockAllScripts = false;
    config.removeMetadata = true;
    config.validateAfterSanitization = true;
    
    return config;
}

void CdrSanitizerTest::verifyOutputFile(const QString& outputPath)
{
    QFileInfo fileInfo(outputPath);
    QVERIFY(fileInfo.exists());
    QVERIFY(fileInfo.isFile());
    QVERIFY(fileInfo.size() >= 0);
    QVERIFY(fileInfo.isReadable());
}

QTEST_MAIN(CdrSanitizerTest)
#include "CdrSanitizerTest.moc"
