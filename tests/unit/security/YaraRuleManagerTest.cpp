#include <QtTest>
#include <QTemporaryFile>
#include <QTextStream>
#include "../../../src/security/scanning/yara/YaraRuleManager.h"

class YaraRuleManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testInitialization();
    void testRuleLoading();
    void testFileScanningClean();
    void testFileScanningMalicious();
    void testMemoryScanning();
    void testErrorHandling();
    void testStatistics();
    void testPerformance();

private:
    std::unique_ptr<YaraRuleManager> m_yaraManager;
    QString m_testRulesDir;
    
    void createTestRule(const QString& filePath, const QString& ruleContent);
    QTemporaryFile* createTestFile(const QString& content);
};

void YaraRuleManagerTest::initTestCase()
{
    m_yaraManager = std::make_unique<YaraRuleManager>();
    
    // Create temporary rules directory
    QTemporaryDir tempDir;
    tempDir.setAutoRemove(false);
    m_testRulesDir = tempDir.path();
    
    // Create test rules
    createTestRule(m_testRulesDir + "/test_rule.yar", R"(
        rule TestRule {
            meta:
                description = "Test rule for unit testing"
            strings:
                $test = "MALICIOUS_PATTERN"
            condition:
                $test
        }
    )");
}

void YaraRuleManagerTest::cleanupTestCase()
{
    m_yaraManager.reset();
    // Clean up test directory
    QDir(m_testRulesDir).removeRecursively();
}

void YaraRuleManagerTest::testInitialization()
{
    QVERIFY(m_yaraManager != nullptr);
    
    auto result = m_yaraManager->initialize();
    QVERIFY(!result); // Should be success (no error)
    
    // Test double initialization
    auto result2 = m_yaraManager->initialize();
    QVERIFY(result2); // Should fail (already initialized)
}

void YaraRuleManagerTest::testRuleLoading()
{
    auto result = m_yaraManager->loadRules(m_testRulesDir.toStdString());
    QVERIFY(!result); // Should be success
    
    auto ruleNames = m_yaraManager->getLoadedRuleNames();
    QVERIFY(!ruleNames.empty());
    QVERIFY(std::find(ruleNames.begin(), ruleNames.end(), "TestRule") != ruleNames.end());
}

void YaraRuleManagerTest::testFileScanningClean()
{
    auto cleanFile = createTestFile("This is a clean test file");
    QVERIFY(cleanFile->open());
    
    std::vector<std::string> matches;
    auto result = m_yaraManager->scanFile(cleanFile->fileName().toStdString(), matches);
    
    QVERIFY(!result); // Should be success
    QVERIFY(matches.empty()); // Should have no matches
    
    delete cleanFile;
}

void YaraRuleManagerTest::testFileScanningMalicious()
{
    auto maliciousFile = createTestFile("This file contains MALICIOUS_PATTERN");
    QVERIFY(maliciousFile->open());
    
    std::vector<std::string> matches;
    auto result = m_yaraManager->scanFile(maliciousFile->fileName().toStdString(), matches);
    
    QVERIFY(!result); // Should be success
    QVERIFY(!matches.empty()); // Should have matches
    QVERIFY(std::find(matches.begin(), matches.end(), "TestRule") != matches.end());
    
    delete maliciousFile;
}

void YaraRuleManagerTest::testMemoryScanning()
{
    std::string testData = "This memory contains MALICIOUS_PATTERN";
    std::vector<std::string> matches;
    
    auto result = m_yaraManager->scanMemory(
        reinterpret_cast<const uint8_t*>(testData.data()),
        testData.size(),
        matches
    );
    
    QVERIFY(!result); // Should be success
    QVERIFY(!matches.empty()); // Should have matches
}

void YaraRuleManagerTest::testErrorHandling()
{
    // Test scanning non-existent file
    std::vector<std::string> matches;
    auto result = m_yaraManager->scanFile("/non/existent/file", matches);
    QVERIFY(result); // Should fail
}

void YaraRuleManagerTest::testStatistics()
{
    auto stats = m_yaraManager->getStatistics();
    QVERIFY(stats.totalRulesLoaded > 0);
    QVERIFY(stats.filesScanned > 0);
}

void YaraRuleManagerTest::testPerformance()
{
    auto start = std::chrono::high_resolution_clock::now();
    
    // Create a larger test file
    auto testFile = createTestFile(QString("A").repeated(1024 * 1024)); // 1MB file
    std::vector<std::string> matches;
    auto result = m_yaraManager->scanFile(testFile->fileName().toStdString(), matches);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    QVERIFY(!result); // Should be success
    QVERIFY(duration.count() < 5000); // Should complete within 5 seconds
    
    qDebug() << "Scan time for 1MB file:" << duration.count() << "ms";
    delete testFile;
}

void YaraRuleManagerTest::createTestRule(const QString& filePath, const QString& ruleContent)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << ruleContent;
    }
}

QTemporaryFile* YaraRuleManagerTest::createTestFile(const QString& content)
{
    auto file = new QTemporaryFile();
    if (file->open()) {
        QTextStream out(file);
        out << content;
        file->close();
    }
    return file;
}

QTEST_MAIN(YaraRuleManagerTest)
#include "YaraRuleManagerTest.moc"
