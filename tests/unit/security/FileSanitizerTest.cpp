/**
 * @file FileSanitizerTest.cpp
 * @brief Unit tests for FileSanitizer base class and derived classes
 * @author Test Suite
 * @version 1.0
 * @date 2024
 */

#include <QTest>
#include <QObject>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QDir>
#include <QDebug>
#include <memory>
#include <fstream>

// Test target
#include "security/cdr/FileSanitizer.h"
#include "security/cdr/CdrTypes.h"

// Mock FileSanitizer implementation for testing
class MockFileSanitizer : public CDR::FileSanitizer
{
public:
    MockFileSanitizer() = default;
    virtual ~MockFileSanitizer() = default;
    
    CDR::SanitizationResult sanitizeFile(const std::string& inputPath,
                                       const std::string& outputPath,
                                       const CDR::CdrConfiguration& config) override {
        CDR::SanitizationResult result;
        result.inputPath = inputPath;
        result.outputPath = outputPath;
        result.fileType = CDR::FileType::TEXT_DOCUMENT;
        
        // Simple mock implementation
        if (inputPath.empty() || outputPath.empty()) {
            result.success = false;
            result.errorMessage = "Invalid paths provided";
            return result;
        }
        
        // Check if input file exists
        std::ifstream inputFile(inputPath);
        if (!inputFile.is_open()) {
            result.success = false;
            result.errorMessage = "Input file not found";
            return result;
        }
        
        // Read content
        std::string content((std::istreambuf_iterator<char>(inputFile)),
                           std::istreambuf_iterator<char>());
        inputFile.close();
        
        // Simple "sanitization" - remove script tags
        size_t scriptPos = content.find("<script>");
        while (scriptPos != std::string::npos) {
            size_t endPos = content.find("</script>", scriptPos);
            if (endPos != std::string::npos) {
                content.erase(scriptPos, endPos - scriptPos + 9);
                result.threatsRemoved++;
            } else {
                break;
            }
            scriptPos = content.find("<script>", scriptPos);
        }
        
        // Write sanitized content
        std::ofstream outputFile(outputPath);
        if (!outputFile.is_open()) {
            result.success = false;
            result.errorMessage = "Cannot create output file";
            return result;
        }
        
        outputFile << content;
        outputFile.close();
        
        result.success = true;
        result.sanitizedSize = content.size();
        return result;
    }
    
    bool canSanitize(CDR::FileType fileType) const override {
        return fileType == CDR::FileType::TEXT_DOCUMENT ||
               fileType == CDR::FileType::HTML_DOCUMENT;
    }
    
    std::string getSanitizerName() const override {
        return "MockFileSanitizer";
    }
    
    std::vector<CDR::FileType> getSupportedTypes() const override {
        return {CDR::FileType::TEXT_DOCUMENT, CDR::FileType::HTML_DOCUMENT};
    }
};

// Mock specialized sanitizers
class MockTextSanitizer : public CDR::FileSanitizer
{
public:
    CDR::SanitizationResult sanitizeFile(const std::string& inputPath,
                                       const std::string& outputPath,
                                       const CDR::CdrConfiguration& config) override {
        CDR::SanitizationResult result;
        result.inputPath = inputPath;
        result.outputPath = outputPath;
        result.fileType = CDR::FileType::TEXT_DOCUMENT;
        result.success = true;
        result.sanitizedSize = 100; // Mock size
        return result;
    }
    
    bool canSanitize(CDR::FileType fileType) const override {
        return fileType == CDR::FileType::TEXT_DOCUMENT;
    }
    
    std::string getSanitizerName() const override {
        return "MockTextSanitizer";
    }
    
    std::vector<CDR::FileType> getSupportedTypes() const override {
        return {CDR::FileType::TEXT_DOCUMENT};
    }
};

class MockPdfSanitizer : public CDR::FileSanitizer
{
public:
    CDR::SanitizationResult sanitizeFile(const std::string& inputPath,
                                       const std::string& outputPath,
                                       const CDR::CdrConfiguration& config) override {
        CDR::SanitizationResult result;
        result.inputPath = inputPath;
        result.outputPath = outputPath;
        result.fileType = CDR::FileType::PDF_DOCUMENT;
        result.success = true;
        result.sanitizedSize = 500; // Mock size
        return result;
    }
    
    bool canSanitize(CDR::FileType fileType) const override {
        return fileType == CDR::FileType::PDF_DOCUMENT;
    }
    
    std::string getSanitizerName() const override {
        return "MockPdfSanitizer";
    }
    
    std::vector<CDR::FileType> getSupportedTypes() const override {
        return {CDR::FileType::PDF_DOCUMENT};
    }
};

class FileSanitizerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Base class functionality tests
    void testMockSanitizerBasicFunctionality();
    void testCanSanitize();
    void testGetSanitizerName();
    void testGetSupportedTypes();

    // Sanitization tests
    void testSanitizeFile_ValidInput();
    void testSanitizeFile_InvalidInput();
    void testSanitizeFile_EmptyPaths();
    void testSanitizeFile_NonExistentFile();

    // Configuration impact tests
    void testSanitizationWithDifferentConfigs();
    void testSecurityLevelImpact();
    void testPreserveOriginalSetting();

    // Content sanitization tests
    void testThreatRemoval();
    void testMetadataRemoval();
    void testScriptRemoval();

    // Polymorphism tests
    void testPolymorphicBehavior();
    void testSpecializedSanitizers();

    // Error handling tests
    void testInvalidFileTypes();
    void testSanitizationFailure();
    void testOutputFileCreationFailure();

    // Performance tests
    void testLargeFileSanitization();
    void testEmptyFileSanitization();

private:
    // Helper methods
    QString createTestFile(const QString& content, const QString& extension = ".txt");
    QString createMaliciousTestFile();
    CDR::CdrConfiguration createTestConfig();
    void verifyValidSanitizationResult(const CDR::SanitizationResult& result, bool shouldSucceed = true);

    QTemporaryDir m_tempDir;
    std::unique_ptr<MockFileSanitizer> m_mockSanitizer;
};

void FileSanitizerTest::initTestCase()
{
    qDebug() << "FileSanitizerTest::initTestCase()";
    QVERIFY(m_tempDir.isValid());
}

void FileSanitizerTest::cleanupTestCase()
{
    qDebug() << "FileSanitizerTest::cleanupTestCase()";
}

void FileSanitizerTest::init()
{
    m_mockSanitizer = std::make_unique<MockFileSanitizer>();
    QVERIFY(m_mockSanitizer != nullptr);
}

void FileSanitizerTest::cleanup()
{
    m_mockSanitizer.reset();
}

void FileSanitizerTest::testMockSanitizerBasicFunctionality()
{
    // Test that our mock sanitizer works correctly
    QVERIFY(m_mockSanitizer != nullptr);
    QCOMPARE(m_mockSanitizer->getSanitizerName(), std::string("MockFileSanitizer"));
    
    auto supportedTypes = m_mockSanitizer->getSupportedTypes();
    QVERIFY(supportedTypes.size() == 2);
    QVERIFY(std::find(supportedTypes.begin(), supportedTypes.end(), CDR::FileType::TEXT_DOCUMENT) != supportedTypes.end());
    QVERIFY(std::find(supportedTypes.begin(), supportedTypes.end(), CDR::FileType::HTML_DOCUMENT) != supportedTypes.end());
}

void FileSanitizerTest::testCanSanitize()
{
    // Test supported types
    QVERIFY(m_mockSanitizer->canSanitize(CDR::FileType::TEXT_DOCUMENT));
    QVERIFY(m_mockSanitizer->canSanitize(CDR::FileType::HTML_DOCUMENT));
    
    // Test unsupported types
    QVERIFY(!m_mockSanitizer->canSanitize(CDR::FileType::PDF_DOCUMENT));
    QVERIFY(!m_mockSanitizer->canSanitize(CDR::FileType::OFFICE_DOCUMENT));
    QVERIFY(!m_mockSanitizer->canSanitize(CDR::FileType::ARCHIVE_FILE));
    QVERIFY(!m_mockSanitizer->canSanitize(CDR::FileType::IMAGE_FILE));
    QVERIFY(!m_mockSanitizer->canSanitize(CDR::FileType::UNKNOWN));
}

void FileSanitizerTest::testGetSanitizerName()
{
    QCOMPARE(m_mockSanitizer->getSanitizerName(), std::string("MockFileSanitizer"));
    
    // Test with specialized sanitizers
    MockTextSanitizer textSanitizer;
    QCOMPARE(textSanitizer.getSanitizerName(), std::string("MockTextSanitizer"));
    
    MockPdfSanitizer pdfSanitizer;
    QCOMPARE(pdfSanitizer.getSanitizerName(), std::string("MockPdfSanitizer"));
}

void FileSanitizerTest::testGetSupportedTypes()
{
    auto supportedTypes = m_mockSanitizer->getSupportedTypes();
    QVERIFY(!supportedTypes.empty());
    QVERIFY(supportedTypes.size() == 2);
    
    // Test specialized sanitizers
    MockTextSanitizer textSanitizer;
    auto textTypes = textSanitizer.getSupportedTypes();
    QVERIFY(textTypes.size() == 1);
    QCOMPARE(textTypes[0], CDR::FileType::TEXT_DOCUMENT);
    
    MockPdfSanitizer pdfSanitizer;
    auto pdfTypes = pdfSanitizer.getSupportedTypes();
    QVERIFY(pdfTypes.size() == 1);
    QCOMPARE(pdfTypes[0], CDR::FileType::PDF_DOCUMENT);
}

void FileSanitizerTest::testSanitizeFile_ValidInput()
{
    QString inputFile = createTestFile("Clean test content without threats", ".txt");
    QString outputFile = m_tempDir.filePath("sanitized_output.txt");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_mockSanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config
    );
    
    verifyValidSanitizationResult(result, true);
    QCOMPARE(QString::fromStdString(result.inputPath), inputFile);
    QCOMPARE(QString::fromStdString(result.outputPath), outputFile);
    QCOMPARE(result.fileType, CDR::FileType::TEXT_DOCUMENT);
    QVERIFY(QFile::exists(outputFile));
}

void FileSanitizerTest::testSanitizeFile_InvalidInput()
{
    QString nonExistentFile = m_tempDir.filePath("nonexistent.txt");
    QString outputFile = m_tempDir.filePath("output.txt");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_mockSanitizer->sanitizeFile(
        nonExistentFile.toStdString(),
        outputFile.toStdString(),
        config
    );
    
    verifyValidSanitizationResult(result, false);
    QVERIFY(!result.success);
    QVERIFY(!result.errorMessage.empty());
}

void FileSanitizerTest::testSanitizeFile_EmptyPaths()
{
    CDR::CdrConfiguration config = createTestConfig();
    
    // Test empty input path
    CDR::SanitizationResult result1 = m_mockSanitizer->sanitizeFile(
        "",
        m_tempDir.filePath("output.txt").toStdString(),
        config
    );
    verifyValidSanitizationResult(result1, false);
    QVERIFY(!result1.success);
    
    // Test empty output path
    QString inputFile = createTestFile("Test content", ".txt");
    CDR::SanitizationResult result2 = m_mockSanitizer->sanitizeFile(
        inputFile.toStdString(),
        "",
        config
    );
    verifyValidSanitizationResult(result2, false);
    QVERIFY(!result2.success);
}

void FileSanitizerTest::testSanitizeFile_NonExistentFile()
{
    QString nonExistentFile = "/absolutely/nonexistent/path/file.txt";
    QString outputFile = m_tempDir.filePath("output.txt");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_mockSanitizer->sanitizeFile(
        nonExistentFile,
        outputFile.toStdString(),
        config
    );
    
    verifyValidSanitizationResult(result, false);
    QVERIFY(!result.success);
    QVERIFY(result.errorMessage.find("not found") != std::string::npos);
}

void FileSanitizerTest::testSanitizationWithDifferentConfigs()
{
    QString inputFile = createTestFile("Test content for config testing", ".txt");
    QString outputFile1 = m_tempDir.filePath("output1.txt");
    QString outputFile2 = m_tempDir.filePath("output2.txt");
    
    // Low security config
    CDR::CdrConfiguration lowConfig = createTestConfig();
    lowConfig.securityLevel = CDR::CdrConfiguration::SecurityLevel::LOW;
    
    // High security config
    CDR::CdrConfiguration highConfig = createTestConfig();
    highConfig.securityLevel = CDR::CdrConfiguration::SecurityLevel::HIGH;
    
    CDR::SanitizationResult result1 = m_mockSanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile1.toStdString(),
        lowConfig
    );
    
    CDR::SanitizationResult result2 = m_mockSanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile2.toStdString(),
        highConfig
    );
    
    verifyValidSanitizationResult(result1, true);
    verifyValidSanitizationResult(result2, true);
    QVERIFY(QFile::exists(outputFile1));
    QVERIFY(QFile::exists(outputFile2));
}

void FileSanitizerTest::testSecurityLevelImpact()
{
    QString maliciousFile = createMaliciousTestFile();
    QString outputFile = m_tempDir.filePath("secure_output.html");
    
    CDR::CdrConfiguration config = createTestConfig();
    config.securityLevel = CDR::CdrConfiguration::SecurityLevel::HIGH;
    
    CDR::SanitizationResult result = m_mockSanitizer->sanitizeFile(
        maliciousFile.toStdString(),
        outputFile.toStdString(),
        config
    );
    
    verifyValidSanitizationResult(result, true);
    
    // Should have removed threats
    QVERIFY(result.threatsRemoved > 0);
    QVERIFY(QFile::exists(outputFile));
    
    // Verify content was sanitized
    QFile sanitizedFile(outputFile);
    if (sanitizedFile.open(QIODevice::ReadOnly)) {
        QString content = sanitizedFile.readAll();
        QVERIFY(!content.contains("<script>"));
        sanitizedFile.close();
    }
}

void FileSanitizerTest::testPreserveOriginalSetting()
{
    QString inputFile = createTestFile("Original content to preserve", ".txt");
    QString outputFile = m_tempDir.filePath("preserved_output.txt");
    
    CDR::CdrConfiguration config = createTestConfig();
    config.preserveOriginal = true;
    
    CDR::SanitizationResult result = m_mockSanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config
    );
    
    verifyValidSanitizationResult(result, true);
    
    // Original file should still exist
    QVERIFY(QFile::exists(inputFile));
    QVERIFY(QFile::exists(outputFile));
}

void FileSanitizerTest::testThreatRemoval()
{
    QString maliciousContent = R"(
        <html>
        <body>
            Normal content
            <script>alert('threat1');</script>
            More normal content
            <script>eval('threat2');</script>
            End content
        </body>
        </html>
    )";
    
    QString inputFile = createTestFile(maliciousContent, ".html");
    QString outputFile = m_tempDir.filePath("clean_output.html");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_mockSanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config
    );
    
    verifyValidSanitizationResult(result, true);
    QVERIFY(result.threatsRemoved >= 2); // Should remove both script tags
}

void FileSanitizerTest::testMetadataRemoval()
{
    QString inputFile = createTestFile("Content with metadata", ".txt");
    QString outputFile = m_tempDir.filePath("metadata_removed.txt");
    
    CDR::CdrConfiguration config = createTestConfig();
    config.removeMetadata = true;
    
    CDR::SanitizationResult result = m_mockSanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config
    );
    
    verifyValidSanitizationResult(result, true);
    // Mock implementation doesn't actually remove metadata,
    // but should process the configuration
}

void FileSanitizerTest::testScriptRemoval()
{
    QString scriptContent = R"(
        function maliciousFunction() {
            document.write("<script>alert('xss')</script>");
        }
        <script>
            eval("dangerous code");
        </script>
    )";
    
    QString inputFile = createTestFile(scriptContent, ".html");
    QString outputFile = m_tempDir.filePath("script_removed.html");
    CDR::CdrConfiguration config = createTestConfig();
    config.blockAllScripts = true;
    
    CDR::SanitizationResult result = m_mockSanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config
    );
    
    verifyValidSanitizationResult(result, true);
    QVERIFY(result.threatsRemoved >= 1);
}

void FileSanitizerTest::testPolymorphicBehavior()
{
    // Test polymorphic behavior using base class pointers
    std::vector<std::unique_ptr<CDR::FileSanitizer>> sanitizers;
    sanitizers.push_back(std::make_unique<MockTextSanitizer>());
    sanitizers.push_back(std::make_unique<MockPdfSanitizer>());
    
    for (auto& sanitizer : sanitizers) {
        // Test virtual function calls
        std::string name = sanitizer->getSanitizerName();
        QVERIFY(!name.empty());
        
        auto types = sanitizer->getSupportedTypes();
        QVERIFY(!types.empty());
        
        // Test type checking
        bool canSanitizeText = sanitizer->canSanitize(CDR::FileType::TEXT_DOCUMENT);
        bool canSanitizePdf = sanitizer->canSanitize(CDR::FileType::PDF_DOCUMENT);
        
        // At least one should be true for each sanitizer
        QVERIFY(canSanitizeText || canSanitizePdf);
    }
}

void FileSanitizerTest::testSpecializedSanitizers()
{
    MockTextSanitizer textSanitizer;
    MockPdfSanitizer pdfSanitizer;
    
    // Test text sanitizer
    QVERIFY(textSanitizer.canSanitize(CDR::FileType::TEXT_DOCUMENT));
    QVERIFY(!textSanitizer.canSanitize(CDR::FileType::PDF_DOCUMENT));
    
    // Test PDF sanitizer
    QVERIFY(!pdfSanitizer.canSanitize(CDR::FileType::TEXT_DOCUMENT));
    QVERIFY(pdfSanitizer.canSanitize(CDR::FileType::PDF_DOCUMENT));
    
    // Test sanitization
    QString textFile = createTestFile("Text content", ".txt");
    QString outputFile = m_tempDir.filePath("text_output.txt");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = textSanitizer.sanitizeFile(
        textFile.toStdString(),
        outputFile.toStdString(),
        config
    );
    
    QVERIFY(result.success);
    QCOMPARE(result.fileType, CDR::FileType::TEXT_DOCUMENT);
}

void FileSanitizerTest::testInvalidFileTypes()
{
    // Try to sanitize unsupported file types
    QString inputFile = createTestFile("Test content", ".xyz");
    QString outputFile = m_tempDir.filePath("output.xyz");
    CDR::CdrConfiguration config = createTestConfig();
    
    // Mock sanitizer should only handle TEXT and HTML
    QVERIFY(!m_mockSanitizer->canSanitize(CDR::FileType::UNKNOWN));
    QVERIFY(!m_mockSanitizer->canSanitize(CDR::FileType::ARCHIVE_FILE));
    QVERIFY(!m_mockSanitizer->canSanitize(CDR::FileType::IMAGE_FILE));
}

void FileSanitizerTest::testSanitizationFailure()
{
    QString inputFile = createTestFile("Test content", ".txt");
    QString invalidOutputPath = "/invalid/path/that/cannot/be/created/output.txt";
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_mockSanitizer->sanitizeFile(
        inputFile.toStdString(),
        invalidOutputPath,
        config
    );
    
    verifyValidSanitizationResult(result, false);
    QVERIFY(!result.success);
    QVERIFY(result.errorMessage.find("Cannot create") != std::string::npos);
}

void FileSanitizerTest::testOutputFileCreationFailure()
{
    QString inputFile = createTestFile("Test content", ".txt");
    
    // Try to write to a directory instead of a file
    QString invalidOutput = m_tempDir.path(); // Directory path, not file
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_mockSanitizer->sanitizeFile(
        inputFile.toStdString(),
        invalidOutput.toStdString(),
        config
    );
    
    // Should fail because we're trying to write to a directory
    verifyValidSanitizationResult(result, false);
}

void FileSanitizerTest::testLargeFileSanitization()
{
    // Create a large test file
    QString largeContent;
    for (int i = 0; i < 10000; ++i) {
        largeContent += QString("Line %1: Large file content for sanitization testing.\n").arg(i);
    }
    
    QString inputFile = createTestFile(largeContent, ".txt");
    QString outputFile = m_tempDir.filePath("large_sanitized.txt");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_mockSanitizer->sanitizeFile(
        inputFile.toStdString(),
        outputFile.toStdString(),
        config
    );
    
    verifyValidSanitizationResult(result, true);
    QVERIFY(result.sanitizedSize > 0);
    QVERIFY(QFile::exists(outputFile));
}

void FileSanitizerTest::testEmptyFileSanitization()
{
    QString emptyFile = createTestFile("", ".txt");
    QString outputFile = m_tempDir.filePath("empty_sanitized.txt");
    CDR::CdrConfiguration config = createTestConfig();
    
    CDR::SanitizationResult result = m_mockSanitizer->sanitizeFile(
        emptyFile.toStdString(),
        outputFile.toStdString(),
        config
    );
    
    verifyValidSanitizationResult(result, true);
    QVERIFY(QFile::exists(outputFile));
}

// Helper Methods Implementation

QString FileSanitizerTest::createTestFile(const QString& content, const QString& extension)
{
    QTemporaryFile* tempFile = new QTemporaryFile(m_tempDir.filePath("sanitizertest_XXXXXX" + extension));
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

QString FileSanitizerTest::createMaliciousTestFile()
{
    QString maliciousContent = R"(
        <html>
        <head><title>Malicious Test File</title></head>
        <body>
            <p>Normal content</p>
            <script>alert('malicious script 1');</script>
            <p>More content</p>
            <script>eval('dangerous code');</script>
            <p>End content</p>
        </body>
        </html>
    )";
    
    return createTestFile(maliciousContent, ".html");
}

CDR::CdrConfiguration FileSanitizerTest::createTestConfig()
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

void FileSanitizerTest::verifyValidSanitizationResult(const CDR::SanitizationResult& result, bool shouldSucceed)
{
    // Basic structure verification
    QVERIFY(!result.inputPath.empty());
    QVERIFY(!result.outputPath.empty());
    
    if (shouldSucceed) {
        QVERIFY(result.success);
        QVERIFY(result.errorMessage.empty());
        QVERIFY(result.sanitizedSize >= 0);
        QVERIFY(result.threatsRemoved >= 0);
    } else {
        QVERIFY(!result.success);
        QVERIFY(!result.errorMessage.empty());
    }
    
    // File type should be valid
    QVERIFY(result.fileType != CDR::FileType::UNKNOWN || !shouldSucceed);
}

QTEST_MAIN(FileSanitizerTest)
#include "FileSanitizerTest.moc"
