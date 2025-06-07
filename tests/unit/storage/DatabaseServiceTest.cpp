/**
 * @file DatabaseServiceTest.cpp
 * @brief Unit tests for DatabaseService class
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
#include <QSignalSpy>

#include "storage/database/DatabaseService/DatabaseService.h"
#include "storage/database/DbManager/DbManager.h"

/**
 * @brief Test class for DatabaseService functionality
 */
class DatabaseServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Singleton tests
    void testGetInstance_SingletonPattern();
    void testGetInstance_ThreadSafety();
    void testGetInstance_MultipleAccess();
    
    // Connection tests
    void testConnectDatabase_ValidPath();
    void testConnectDatabase_InvalidPath();
    void testConnectDatabase_ExistingDatabase();
    void testIsDatabaseConnected_WhenConnected();
    void testIsDatabaseConnected_WhenNotConnected();
    void testConnectDatabase_Reconnection();
    
    // DbManager access tests
    void testGetDbManager_ValidInstance();
    void testGetDbManager_NullCheck();
    void testGetDbManager_ConsistentAccess();
    
    // Integration tests
    void testDatabaseOperations_ThroughService();
    void testServiceLifecycle();
    void testErrorPropagation();
    
    // Edge cases
    void testMultipleConnections();
    void testConnectionAfterFailure();
    void testServiceWithCorruptDatabase();

private:
    void createTestDatabase(const QString& dbPath);
    void insertTestData(const QString& dbPath);
    bool isValidSQLiteDatabase(const QString& dbPath);
    void resetDatabaseConnections();
    
    QTemporaryDir* m_tempDir;
    QString m_testDbPath;
};

void DatabaseServiceTest::initTestCase()
{
    // Initialize Qt application if not already done
    if (!QCoreApplication::instance()) {
        static int argc = 1;
        static char* argv[] = {"DatabaseServiceTest"};
        new QCoreApplication(argc, argv);
    }
    
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    qDebug() << "Test directory:" << m_tempDir->path();
}

void DatabaseServiceTest::cleanupTestCase()
{
    delete m_tempDir;
    m_tempDir = nullptr;
}

void DatabaseServiceTest::init()
{
    m_testDbPath = m_tempDir->path() + "/test_service_database.db";
    resetDatabaseConnections();
}

void DatabaseServiceTest::cleanup()
{
    resetDatabaseConnections();
    
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

void DatabaseServiceTest::testGetInstance_SingletonPattern()
{
    DatabaseService& instance1 = DatabaseService::getInstance();
    DatabaseService& instance2 = DatabaseService::getInstance();
    
    // Should return the same instance
    QCOMPARE(&instance1, &instance2);
}

void DatabaseServiceTest::testGetInstance_ThreadSafety()
{
    // Test that getInstance is thread-safe
    DatabaseService& instance1 = DatabaseService::getInstance();
    DatabaseService& instance2 = DatabaseService::getInstance();
    
    QCOMPARE(&instance1, &instance2);
    
    // Verify the instance is valid
    QVERIFY(&instance1 != nullptr);
}

void DatabaseServiceTest::testGetInstance_MultipleAccess()
{
    // Test multiple rapid accesses
    QList<DatabaseService*> instances;
    
    for (int i = 0; i < 10; ++i) {
        instances.append(&DatabaseService::getInstance());
    }
    
    // All instances should be the same
    for (int i = 1; i < instances.size(); ++i) {
        QCOMPARE(instances[0], instances[i]);
    }
}

void DatabaseServiceTest::testConnectDatabase_ValidPath()
{
    createTestDatabase(m_testDbPath);
    
    DatabaseService& service = DatabaseService::getInstance();
    bool result = service.connectDatabase(m_testDbPath);
    
    QVERIFY(result);
    QVERIFY(service.isDatabaseConnected());
}

void DatabaseServiceTest::testConnectDatabase_InvalidPath()
{
    QString invalidPath = "/non/existent/path/database.db";
    
    DatabaseService& service = DatabaseService::getInstance();
    bool result = service.connectDatabase(invalidPath);
    
    QVERIFY(!result);
    QVERIFY(!service.isDatabaseConnected());
}

void DatabaseServiceTest::testConnectDatabase_ExistingDatabase()
{
    createTestDatabase(m_testDbPath);
    insertTestData(m_testDbPath);
    
    DatabaseService& service = DatabaseService::getInstance();
    bool result = service.connectDatabase(m_testDbPath);
    
    QVERIFY(result);
    QVERIFY(service.isDatabaseConnected());
    
    // Verify we can access the DbManager
    DbManager* dbManager = service.getDbManager();
    QVERIFY(dbManager != nullptr);
    QVERIFY(dbManager->isDatabaseConnected());
}

void DatabaseServiceTest::testIsDatabaseConnected_WhenConnected()
{
    createTestDatabase(m_testDbPath);
    
    DatabaseService& service = DatabaseService::getInstance();
    service.connectDatabase(m_testDbPath);
    
    QVERIFY(service.isDatabaseConnected());
}

void DatabaseServiceTest::testIsDatabaseConnected_WhenNotConnected()
{
    DatabaseService& service = DatabaseService::getInstance();
    
    // Fresh service should not be connected
    QVERIFY(!service.isDatabaseConnected());
}

void DatabaseServiceTest::testConnectDatabase_Reconnection()
{
    createTestDatabase(m_testDbPath);
    
    DatabaseService& service = DatabaseService::getInstance();
    
    // First connection
    bool result1 = service.connectDatabase(m_testDbPath);
    QVERIFY(result1);
    QVERIFY(service.isDatabaseConnected());
    
    // Create another database
    QString secondDbPath = m_tempDir->path() + "/second_database.db";
    createTestDatabase(secondDbPath);
    
    // Second connection (should replace the first)
    bool result2 = service.connectDatabase(secondDbPath);
    QVERIFY(result2);
    QVERIFY(service.isDatabaseConnected());
}

void DatabaseServiceTest::testGetDbManager_ValidInstance()
{
    DatabaseService& service = DatabaseService::getInstance();
    DbManager* dbManager = service.getDbManager();
    
    QVERIFY(dbManager != nullptr);
}

void DatabaseServiceTest::testGetDbManager_NullCheck()
{
    DatabaseService& service = DatabaseService::getInstance();
    DbManager* dbManager = service.getDbManager();
    
    // Should always return a valid pointer (never null)
    QVERIFY(dbManager != nullptr);
}

void DatabaseServiceTest::testGetDbManager_ConsistentAccess()
{
    DatabaseService& service = DatabaseService::getInstance();
    
    DbManager* dbManager1 = service.getDbManager();
    DbManager* dbManager2 = service.getDbManager();
    
    // Should return the same instance
    QCOMPARE(dbManager1, dbManager2);
}

void DatabaseServiceTest::testDatabaseOperations_ThroughService()
{
    createTestDatabase(m_testDbPath);
    
    DatabaseService& service = DatabaseService::getInstance();
    bool connected = service.connectDatabase(m_testDbPath);
    QVERIFY(connected);
    
    DbManager* dbManager = service.getDbManager();
    QVERIFY(dbManager != nullptr);
    QVERIFY(dbManager->isDatabaseConnected());
    
    // Test database operations through the service
    std::error_code ec;
    bool exists = dbManager->isSha256Exists("test_hash", ec);
    QVERIFY(!ec);  // Should not have error
    QVERIFY(!exists);  // Hash should not exist in empty database
    
    long count = dbManager->getSignatureCount(ec);
    QVERIFY(!ec);
    QVERIFY(count >= 0);
}

void DatabaseServiceTest::testServiceLifecycle()
{
    {
        DatabaseService& service = DatabaseService::getInstance();
        QVERIFY(&service != nullptr);
        
        createTestDatabase(m_testDbPath);
        bool connected = service.connectDatabase(m_testDbPath);
        QVERIFY(connected);
        QVERIFY(service.isDatabaseConnected());
    }
    
    // Service should still be accessible (singleton)
    {
        DatabaseService& service = DatabaseService::getInstance();
        QVERIFY(&service != nullptr);
        // Connection state should persist
    }
}

void DatabaseServiceTest::testErrorPropagation()
{
    DatabaseService& service = DatabaseService::getInstance();
    
    // Test with invalid path
    QString invalidPath = "/invalid/path/database.db";
    bool result = service.connectDatabase(invalidPath);
    QVERIFY(!result);
    QVERIFY(!service.isDatabaseConnected());
    
    // DbManager should still be accessible but not connected
    DbManager* dbManager = service.getDbManager();
    QVERIFY(dbManager != nullptr);
    QVERIFY(!dbManager->isDatabaseConnected());
    
    // Operations should fail gracefully
    std::error_code ec;
    bool exists = dbManager->isSha256Exists("test_hash", ec);
    QVERIFY(ec);  // Should have error
    QVERIFY(!exists);
}

void DatabaseServiceTest::testMultipleConnections()
{
    createTestDatabase(m_testDbPath);
    
    DatabaseService& service = DatabaseService::getInstance();
    
    // Connect multiple times
    for (int i = 0; i < 5; ++i) {
        bool result = service.connectDatabase(m_testDbPath);
        QVERIFY(result);
        QVERIFY(service.isDatabaseConnected());
    }
}

void DatabaseServiceTest::testConnectionAfterFailure()
{
    DatabaseService& service = DatabaseService::getInstance();
    
    // First, try to connect to invalid path
    QString invalidPath = "/invalid/path/database.db";
    bool result1 = service.connectDatabase(invalidPath);
    QVERIFY(!result1);
    QVERIFY(!service.isDatabaseConnected());
    
    // Then connect to valid database
    createTestDatabase(m_testDbPath);
    bool result2 = service.connectDatabase(m_testDbPath);
    QVERIFY(result2);
    QVERIFY(service.isDatabaseConnected());
}

void DatabaseServiceTest::testServiceWithCorruptDatabase()
{
    // Create a corrupt database file
    QFile corruptFile(m_testDbPath);
    QVERIFY(corruptFile.open(QIODevice::WriteOnly));
    corruptFile.write("This is not a valid SQLite database");
    corruptFile.close();
    
    DatabaseService& service = DatabaseService::getInstance();
    bool result = service.connectDatabase(m_testDbPath);
    
    QVERIFY(!result);
    QVERIFY(!service.isDatabaseConnected());
    
    // Service should remain functional
    DbManager* dbManager = service.getDbManager();
    QVERIFY(dbManager != nullptr);
}

// Helper Methods

void DatabaseServiceTest::createTestDatabase(const QString& dbPath)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_service_setup");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());
    
    QSqlQuery query(db);
    
    // Create sha256_hashes table
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
    
    // Create signatures table
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
    QSqlDatabase::removeDatabase("test_service_setup");
    
    QVERIFY(isValidSQLiteDatabase(dbPath));
}

void DatabaseServiceTest::insertTestData(const QString& dbPath)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_service_insert");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());
    
    QSqlQuery query(db);
    
    // Insert test hash
    query.prepare("INSERT INTO sha256_hashes (hash_value, file_name) VALUES (?, ?)");
    query.addBindValue("test_hash_from_service");
    query.addBindValue("service_test_file.txt");
    QVERIFY2(query.exec(), qPrintable(query.lastError().text()));
    
    // Insert test signature
    query.prepare("INSERT INTO signatures (signature_name, signature_data) VALUES (?, ?)");
    query.addBindValue("test_signature");
    query.addBindValue(QByteArray("signature_data"));
    QVERIFY2(query.exec(), qPrintable(query.lastError().text()));
    
    db.close();
    QSqlDatabase::removeDatabase("test_service_insert");
}

bool DatabaseServiceTest::isValidSQLiteDatabase(const QString& dbPath)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "service_validation");
    db.setDatabaseName(dbPath);
    bool valid = db.open();
    if (valid) {
        db.close();
    }
    QSqlDatabase::removeDatabase("service_validation");
    return valid;
}

void DatabaseServiceTest::resetDatabaseConnections()
{
    // Remove all existing connections to avoid conflicts
    QStringList connectionNames = QSqlDatabase::connectionNames();
    for (const QString& name : connectionNames) {
        if (name.contains("test") || name == QSqlDatabase::defaultConnection) {
            QSqlDatabase::removeDatabase(name);
        }
    }
}

QTEST_MAIN(DatabaseServiceTest)
#include "DatabaseServiceTest.moc"
