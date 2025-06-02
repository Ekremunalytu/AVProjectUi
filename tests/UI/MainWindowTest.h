#ifndef MAINWINDOWTEST_H
#define MAINWINDOWTEST_H

#include <QTest>
#include <QObject>
#include "UI/Mainwindow/mainwindow.h"
#include "Database/DbManager/DbManager.h"

class MainWindowTest : public QObject
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
    void testInitialization();
    void testNavigationButtons();
    void testDashboardWidgetIntegration();

private:
    MainWindow *mainWindow;
    DbManager *testDbManager;
};

#endif // MAINWINDOWTEST_H 