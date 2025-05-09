#include "DashboardTest.h"
#include <QSignalSpy>
#include <QPushButton>
#include <QTextEdit>

// Function to run at the beginning of test case
void DashboardTest::initTestCase()
{
    qDebug() << "Starting DashboardTest...";
}

// Function to run at the end of test case
void DashboardTest::cleanupTestCase()
{
    qDebug() << "Cleaning up DashboardTest...";
}

// Function to run before each test
void DashboardTest::init()
{
    // Create test database manager instance
    testDbManager = new DbManager();
    
    // Create dashboard widget with test database manager
    dashboard = new DashboardWidget(nullptr);
}

// Function to run after each test
void DashboardTest::cleanup()
{
    delete dashboard;
    delete testDbManager;
}

// Test if widget initializes correctly
void DashboardTest::testInitialization()
{
    // Test whether the widget is initialized correctly
    QVERIFY(dashboard != nullptr);
    QVERIFY(dashboard->isVisible() == false); // Should be invisible initially
}

// Test Basic Scan button functionality
void DashboardTest::testBasicScanButton()
{
    // Find the Basic Scan button
    QPushButton* basicScanButton = dashboard->findChild<QPushButton*>("basicScanButton_dashboard");
    QVERIFY(basicScanButton != nullptr);
    
    // Find the results text area
    QTextEdit* resultText = dashboard->findChild<QTextEdit*>("scanResultsTextEdit_dashboard");
    QVERIFY(resultText != nullptr);
    
    // Clear any existing text
    resultText->clear();
    
    // Simulate button click
    QTest::mouseClick(basicScanButton, Qt::LeftButton);
    
    // Expected result: Check if it contains "Starting Basic Scan..."
    QVERIFY(resultText->toPlainText().contains("Starting Basic Scan..."));
}

// Test Advanced Scan button functionality
void DashboardTest::testAdvancedScanButton()
{
    // Find the Advanced Scan button
    QPushButton* advancedScanButton = dashboard->findChild<QPushButton*>("advancedScanButton_dashboard");
    QVERIFY(advancedScanButton != nullptr);
    
    // Find the results text area
    QTextEdit* resultText = dashboard->findChild<QTextEdit*>("scanResultsTextEdit_dashboard");
    QVERIFY(resultText != nullptr);
    
    // Clear any existing text
    resultText->clear();
    
    // Simulate button click
    QTest::mouseClick(advancedScanButton, Qt::LeftButton);
    
    // Expected result: Check if it contains "Starting Advanced Scan..."
    QVERIFY(resultText->toPlainText().contains("Starting Advanced Scan..."));
}

// Test CDR Scan button functionality
void DashboardTest::testCdrScanButton()
{
    // Find the CDR Scan button
    QPushButton* cdrScanButton = dashboard->findChild<QPushButton*>("cdrScanButton_dashboard");
    QVERIFY(cdrScanButton != nullptr);
    
    // Find the results text area
    QTextEdit* resultText = dashboard->findChild<QTextEdit*>("scanResultsTextEdit_dashboard");
    QVERIFY(resultText != nullptr);
    
    // Clear any existing text
    resultText->clear();
    
    // Simulate button click
    QTest::mouseClick(cdrScanButton, Qt::LeftButton);
    
    // Expected result: Check if it contains "Starting CDR Scan..."
    QVERIFY(resultText->toPlainText().contains("Starting CDR Scan..."));
}

// Test Sandbox Scan button functionality
void DashboardTest::testSandboxScanButton()
{
    // Find the Sandbox Scan button
    QPushButton* sandboxScanButton = dashboard->findChild<QPushButton*>("sandboxScanButton_dashboard");
    QVERIFY(sandboxScanButton != nullptr);
    
    // Find the results text area
    QTextEdit* resultText = dashboard->findChild<QTextEdit*>("scanResultsTextEdit_dashboard");
    QVERIFY(resultText != nullptr);
    
    // Clear any existing text
    resultText->clear();
    
    // Simulate button click
    QTest::mouseClick(sandboxScanButton, Qt::LeftButton);
    
    // Expected result: Check if it contains "Starting Sandbox Scan..."
    QVERIFY(resultText->toPlainText().contains("Starting Sandbox Scan..."));
}

// Widget görünürlüğünü test et
void DashboardTest::testWidgetVisibility()
{
    dashboard->show();
    QVERIFY(dashboard->isVisible() == true);
    
    dashboard->hide();
    QVERIFY(dashboard->isVisible() == false);
}

// Register test class with Qt test framework
QTEST_MAIN(DashboardTest) 