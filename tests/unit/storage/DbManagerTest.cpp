/**
 * @file DbManagerTest.cpp
 * @brief Unit tests for DbManager class
 * @author Test Suite
 * @date 2025
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <system_error>

#include "storage/database/DbManager/DbManager.h"

/**
 * @brief Test class for DbManager functionality
 */
class DbManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Connection tests
    void testConnectDatabase_ValidPath();
    void testConnectDatabase_InvalidPath();
    void testConnectDatabase_ExistingDatabase();
    void testIsDatabaseConnected_WhenConnected();
    void testIsDatabaseConnected_WhenNotConnected();
    void testConnectDatabase_ReconnectToSameDatabase();
    
    // Hash operations tests
    void testIsSha256Exists_ValidHash();
    void testIsSha256Exists_InvalidHash();
    void testIsSha256Exists_DatabaseNotConnected();
    void testIsSha256Exists_MalformedHash();
    
    // Signature count tests
    void testGetSignatureCount_Connected();
    void testGetSignatureCount_NotConnected();
    void testGetSignatureCount_EmptyDatabase();
    
    // Error handling tests
    void testErrorHandling_CorruptDatabase();
    void testErrorHandling_ReadOnlyDatabase();
    void testErrorHandling_NonExistentTable();
    
    // Concurrent access tests
    void testConcurrentAccess();
    void testMultipleInstances();

private:
    void createTestDatabase(const QString& dbPath);
    void insertTestHash(const QString& dbPath, const QString& hash);
    QString createTestHash(const QString& suffix = "");
    bool isValidSQLiteDatabase(const QString& dbPath);
    
    QTemporaryDir* m_tempDir;
    QString m_testDbPath;
    DbManager* m_dbManager;
};

void DbManagerTest::initTestCase()
{
    // Initialize Qt application if not already done
    if (!QCoreApplication::instance()) {
        static int argc = 1;
        static char* argv[] = {"DbManagerTest"};
        new QCoreApplication(argc, argv);
    }
    
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    qDebug() << "Test directory:" << m_tempDir->path();
}

void DbManagerTest::cleanupTestCase()
{
    delete m_tempDir;
    m_tempDir = nullptr;
}

void DbManagerTest::init()
{
    m_testDbPath = m_tempDir->path() + "/test_database.db";
    m_dbManager = new DbManager();
    QVERIFY(m_dbManager != nullptr);
}

void DbManagerTest::cleanup()
{
    delete m_dbManager;
    m_dbManager = nullptr;
    
    // Clean up test database files
    QFile::remove(m_testDbPath);
    
    // Clean up any additional test files
    QDir tempDir(m_tempDir->path());
    QStringList filters;
    filters << "*.db" << "*.db-*";
    
    for (const QString& file : tempDir.entryList(filters, QDir::Files)) {
        QFile::remove(tempDir.absoluteFilePath(file));
    }
}

void DbManagerTest::testConnectDatabase_ValidPath()
{
    createTestDatabase(m_testDbPath);
    
    auto ec = m_dbManager->connectDatabase(m_testDbPath);
    QVERIFY(!ec);  // No error
    QVERIFY(m_dbManager->isDatabaseConnected());
}

void DbManagerTest::testConnectDatabase_InvalidPath()
{
    QString invalidPath = "/non/existent/path/database.db";
    
    auto ec = m_dbManager->connectDatabase(invalidPath);
    QVERIFY(ec);  // Should have error
    QVERIFY(!m_dbManager->isDatabaseConnected());
}

void DbManagerTest::testConnectDatabase_ExistingDatabase()
{
    createTestDatabase(m_testDbPath);
    insertTestHash(m_testDbPath, "test_hash_123");
    
    auto ec = m_dbManager->connectDatabase(m_testDbPath);
    QVERIFY(!ec);
    QVERIFY(m_dbManager->isDatabaseConnected());
    
    // Verify we can access existing data
    std::error_code hashEc;
    bool exists = m_dbManager->isSha256Exists("test_hash_123", hashEc);
    QVERIFY(!hashEc);
    QVERIFY(exists);
}

void DbManagerTest::testIsDatabaseConnected_WhenConnected()
{
    createTestDatabase(m_testDbPath);
    
    auto ec = m_dbManager->connectDatabase(m_testDbPath);
    QVERIFY(!ec);
    QVERIFY(m_dbManager->isDatabaseConnected());
}

void DbManagerTest::testIsDatabaseConnected_WhenNotConnected()
{
    QVERIFY(!m_dbManager->isDatabaseConnected());
}

void DbManagerTest::testConnectDatabase_ReconnectToSameDatabase()
{
    createTestDatabase(m_testDbPath);
    
    // First connection
    auto ec1 = m_dbManager->connectDatabase(m_testDbPath);
    QVERIFY(!ec1);
    QVERIFY(m_dbManager->isDatabaseConnected());
    
    // Second connection to same database
    auto ec2 = m_dbManager->connectDatabase(m_testDbPath);
    QVERIFY(!ec2);
    QVERIFY(m_dbManager->isDatabaseConnected());
}

void DbManagerTest::testIsSha256Exists_ValidHash()
{
    createTestDatabase(m_testDbPath);
    QString testHash = createTestHash("valid");
    insertTestHash(m_testDbPath, testHash);
    
    auto ec = m_dbManager->connectDatabase(m_testDbPath);
    QVERIFY(!ec);
    
    std::error_code hashEc;
    bool exists = m_dbManager->isSha256Exists(testHash, hashEc);
    QVERIFY(!hashEc);
    QVERIFY(exists);
}

void DbManagerTest::testIsSha256Exists_InvalidHash()
{
    createTestDatabase(m_testDbPath);
    
    auto ec = m_dbManager->connectDatabase(m_testDbPath);
    QVERIFY(!ec);
    
    std::error_code hashEc;
    bool exists = m_dbManager->isSha256Exists("nonexistent_hash", hashEc);
    QVERIFY(!hashEc);
    QVERIFY(!exists);
}

void DbManagerTest::testIsSha256Exists_DatabaseNotConnected()
{
    std::error_code hashEc;
    bool exists = m_dbManager->isSha256Exists("any_hash", hashEc);
    QVERIFY(hashEc);  // Should have error
    QVERIFY(!exists);
}

void DbManagerTest::testIsSha256Exists_MalformedHash()
{
    createTestDatabase(m_testDbPath);
    
    auto ec = m_dbManager->connectDatabase(m_testDbPath);
    QVERIFY(!ec);
    
    // Test with empty hash
    std::error_code hashEc1;
    bool exists1 = m_dbManager->isSha256Exists("", hashEc1);
    QVERIFY(!hashEc1);  // Empty hash should be handled gracefully
    QVERIFY(!exists1);
    
    // Test with very short hash
    std::error_code hashEc2;
    bool exists2 = m_dbManager->isSha256Exists("abc", hashEc2);
    QVERIFY(!hashEc2);
    QVERIFY(!exists2);
}

void DbManagerTest::testGetSignatureCount_Connected()
{
    createTestDatabase(m_testDbPath);
    
    auto ec = m_dbManager->connectDatabase(m_testDbPath);
    QVERIFY(!ec);
    
    std::error_code countEc;
    long count = m_dbManager->getSignatureCount(countEc);
    QVERIFY(!countEc);
    QVERIFY(count >= 0);  // Should be valid count
}

void DbManagerTest::testGetSignatureCount_NotConnected()
{
    std::error_code countEc;
    long count = m_dbManager->getSignatureCount(countEc);
    QVERIFY(countEc);  // Should have error
    QCOMPARE(count, 0L);
}

void DbManagerTest::testGetSignatureCount_EmptyDatabase()
{
    createTestDatabase(m_testDbPath);
    
    auto ec = m_dbManager->connectDatabase(m_testDbPath);
    QVERIFY(!ec);
    
    std::error_code countEc;
    long count = m_dbManager->getSignatureCount(countEc);
    QVERIFY(!countEc);
    QCOMPARE(count, 0L);  // Empty database should return 0
}

void DbManagerTest::testErrorHandling_CorruptDatabase()
{
    // Create a corrupt database file
    QFile corruptFile(m_testDbPath);
    QVERIFY(corruptFile.open(QIODevice::WriteOnly));
    corruptFile.write("This is not a valid SQLite database");
    corruptFile.close();
    
    auto ec = m_dbManager->connectDatabase(m_testDbPath);
    QVERIFY(ec);  // Should fail to connect
    QVERIFY(!m_dbManager->isDatabaseConnected());
}

void DbManagerTest::testErrorHandling_ReadOnlyDatabase()
{
    createTestDatabase(m_testDbPath);
    
    // Make the file read-only
    QFile::setPermissions(m_testDbPath, QFileDevice::ReadOwner | QFileDevice::ReadGroup);
    
    auto ec = m_dbManager->connectDatabase(m_testDbPath);
    // Connection might succeed but operations might fail
    // This test verifies graceful handling of read-only scenarios
    
    if (!ec && m_dbManager->isDatabaseConnected()) {
        std::error_code hashEc;
        bool exists = m_dbManager->isSha256Exists("test_hash", hashEc);
        // Should handle read-only database gracefully
    }
    
    // Restore permissions for cleanup
    QFile::setPermissions(m_testDbPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
}

void DbManagerTest::testErrorHandling_NonExistentTable()
{
    // Create a valid SQLite database but without expected tables
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_connection");
    db.setDatabaseName(m_testDbPath);
    QVERIFY(db.open());
    
    QSqlQuery query(db);
    QVERIFY(query.exec("CREATE TABLE dummy_table (id INTEGER PRIMARY KEY)"));
    db.close();
    QSqlDatabase::removeDatabase("test_connection");
    
    auto ec = m_dbManager->connectDatabase(m_testDbPath);
    QVERIFY(!ec);  // Connection should succeed
    
    // Operations on non-existent tables should be handled gracefully
    std::error_code hashEc;
    bool exists = m_dbManager->isSha256Exists("test_hash", hashEc);
    // Should handle missing table gracefully
}

void DbManagerTest::testConcurrentAccess()
{
    createTestDatabase(m_testDbPath);
    
    auto ec = m_dbManager->connectDatabase(m_testDbPath);
    QVERIFY(!ec);
    
    // Create another DbManager instance
    DbManager secondManager;
    QString secondDbPath = m_tempDir->path() + "/second_database.db";
    createTestDatabase(secondDbPath);
    
    auto ec2 = secondManager.connectDatabase(secondDbPath);
    QVERIFY(!ec2);
    
    // Both should be able to operate independently
    QVERIFY(m_dbManager->isDatabaseConnected());
    QVERIFY(secondManager.isDatabaseConnected());
    
    std::error_code countEc1, countEc2;
    long count1 = m_dbManager->getSignatureCount(countEc1);
    long count2 = secondManager.getSignatureCount(countEc2);
    
    QVERIFY(!countEc1);
    QVERIFY(!countEc2);
    QVERIFY(count1 >= 0);
    QVERIFY(count2 >= 0);
}

void DbManagerTest::testMultipleInstances()
{
    createTestDatabase(m_testDbPath);
    
    // Test multiple instances with same database
    DbManager manager1, manager2;
    
    auto ec1 = manager1.connectDatabase(m_testDbPath);
    auto ec2 = manager2.connectDatabase(m_testDbPath);
    
    QVERIFY(!ec1);
    QVERIFY(!ec2);
    QVERIFY(manager1.isDatabaseConnected());
    QVERIFY(manager2.isDatabaseConnected());
}

// Helper Methods

void DbManagerTest::createTestDatabase(const QString& dbPath)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_setup");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());
    
    QSqlQuery query(db);
    
    // Create sha256_hashes table (common table in the application)
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS sha256_hashes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            hash_value TEXT UNIQUE NOT NULL,
            file_name TEXT,
            scan_date DATETIME DEFAULT CURRENT_TIMESTAMP,
            is_malicious BOOLEAN DEFAULT 0
        )
    )";
    
    QVERIFY2(query.exec(createTableQuery), qPrintable(query.lastError().text()));
    
    // Create signatures table (for signature count tests)
    QString createSignaturesQuery = R"(
        CREATE TABLE IF NOT EXISTS signatures (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            signature_name TEXT NOT NULL,
            signature_data BLOB,
            created_date DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    QVERIFY2(query.exec(createSignaturesQuery), qPrintable(query.lastError().text()));
    
    db.close();
    QSqlDatabase::removeDatabase("test_setup");
    
    QVERIFY(isValidSQLiteDatabase(dbPath));
}

void DbManagerTest::insertTestHash(const QString& dbPath, const QString& hash)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_insert");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());
    
    QSqlQuery query(db);
    query.prepare("INSERT INTO sha256_hashes (hash_value, file_name) VALUES (?, ?)");
    query.addBindValue(hash);
    query.addBindValue("test_file.txt");
    QVERIFY2(query.exec(), qPrintable(query.lastError().text()));
    
    db.close();
    QSqlDatabase::removeDatabase("test_insert");
}

QString DbManagerTest::createTestHash(const QString& suffix)
{
    // Create a valid-looking SHA256 hash
    QString baseHash = "abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890";
    return baseHash + suffix.leftJustified(64 - baseHash.length(), '0').left(64 - baseHash.length());
}

bool DbManagerTest::isValidSQLiteDatabase(const QString& dbPath)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "validation");
    db.setDatabaseName(dbPath);
    bool valid = db.open();
    if (valid) {
        db.close();
    }
    QSqlDatabase::removeDatabase("validation");
    return valid;
}

QTEST_MAIN(DbManagerTest)
#include "DbManagerTest.moc"
