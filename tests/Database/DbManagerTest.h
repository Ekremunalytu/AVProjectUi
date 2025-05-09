#ifndef DBMANAGERTEST_H
#define DBMANAGERTEST_H

#include <QTest>
#include <QObject>
#include "Database/DbManager/DbManager.h"

class DbManagerTest : public QObject
{
    Q_OBJECT

private slots:
    // Test case başlangıcında çalışacak fonksiyon
    void initTestCase();

    // Test case bitiminde çalışacak fonksiyon
    void cleanupTestCase();

    // Her test fonksiyonu öncesi çalışacak fonksiyon
    void init();

    // Her test fonksiyonu sonrası çalışacak fonksiyon
    void cleanup();

    // Test fonksiyonları
    void testConnection();
    void testCreateTable();
    void testInsert();
    void testSelect();
    void testUpdate();
    void testDelete();
    void testTransaction();
    void testSha256Exists();
    
    // Parametre bazlı test
    void testConnectionWithDifferentPaths_data();
    void testConnectionWithDifferentPaths();

private:
    DbManager *dbManager;
};

#endif // DBMANAGERTEST_H 