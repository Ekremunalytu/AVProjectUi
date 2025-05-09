#ifndef DASHBOARDTEST_H
#define DASHBOARDTEST_H

#include <QTest>
#include <QObject>
#include "UI/Widgets/Dashboard/DashboardWidget.h"

class DashboardTest : public QObject
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
    void testBasicScanButton();
    void testAdvancedScanButton();
    void testCdrScanButton();
    void testSandboxScanButton();
    void testWidgetVisibility();

private:
    DashboardWidget *dashboard;
};

#endif // DASHBOARDTEST_H 