#ifndef DBMANAGERTEST_H
#define DBMANAGERTEST_H

#include <QTest>
#include <QObject>
#include "Database/DbManager/DbManager.h"

/**
 * @brief Test class for the DbManager functionality
 * 
 * This class contains unit tests for various database operations
 * provided by the DbManager class.
 */
class DbManagerTest : public QObject
{
    Q_OBJECT

private slots:
    // Function to run at the beginning of test case
    void initTestCase();
    
    // Function to run at the end of test case
    void cleanupTestCase();
    
    // Function to run before each test
    void init();
    
    // Function to run after each test
    void cleanup();
    
    // Test methods
    void testConnection();
    void testConnectionWithDifferentPaths_data();
    void testConnectionWithDifferentPaths();
    void testCreateTable();
    void testInsert();
    void testSelect();
    void testUpdate();
    void testDelete();
    void testTransaction();
    void testSha256Exists();

private:
    DbManager* dbManager = nullptr;
};

#endif // DBMANAGERTEST_H 