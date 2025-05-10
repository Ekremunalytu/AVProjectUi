#ifndef DASHBOARDTEST_H
#define DASHBOARDTEST_H

#include <QTest>
#include <QObject>
#include "UI/Widgets/Dashboard/DashboardWidget.h"
#include "Database/DbManager/DbManager.h"

/**
 * @brief Test class for the DashboardWidget functionality
 * 
 * This class contains unit tests for the dashboard widget's UI 
 * elements and scan functionality.
 */
class DashboardTest : public QObject
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
    void testInitialization();
    void testBasicScanButton();
    void testAdvancedScanButton();
    void testCdrScanButton();
    void testSandboxScanButton();
    void testWidgetVisibility();

private:
    DashboardWidget* dashboard = nullptr;
    DbManager* testDbManager = nullptr;
};

#endif // DASHBOARDTEST_H 