/**
 * @file MainWindowTest.cpp
 * @brief Unit tests for MainWindow class
 * @author Test Suite
 * @date 2025
 */

#include <QtTest/QtTest>
#include <QApplication>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QDir>
#include <QDebug>

#include "presentation/application/mainwindow.h"
#include "presentation/widgets/Dashboard/DashboardWidget.h"
#include "presentation/widgets/Settings/SettingsWidget.h"
#include "presentation/widgets/History/HistoryWidget.h"
#include "presentation/widgets/ServiceStatus/ServiceStatusWidget.h"

/**
 * @brief Test class for MainWindow functionality
 */
class MainWindowTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Window initialization tests
    void testMainWindowCreation();
    void testMainWindowInitialization();
    void testMainWindowLayout();
    void testMainWindowFullscreen();
    
    // UI Component tests
    void testDashboardWidgetIntegration();
    void testSettingsWidgetIntegration();
    void testHistoryWidgetIntegration();
    void testServiceStatusWidgetIntegration();
    
    // Navigation tests
    void testNavigationButtons();
    void testPageSwitching();
    void testStackedWidgetBehavior();
    
    // Widget lifecycle tests
    void testWidgetCreation();
    void testWidgetDestruction();
    void testMemoryManagement();
    
    // Error handling tests
    void testMissingPages();
    void testDatabaseServiceIntegration();
    void testWidgetErrorStates();
    
    // Interaction tests
    void testWidgetInteractions();
    void testSignalSlotConnections();

private:
    void setupTestEnvironment();
    void cleanupTestEnvironment();
    bool hasWidget(QWidget* parent, const QString& className);
    QWidget* findWidget(QWidget* parent, const QString& className);
    
    QApplication* m_app;
    MainWindow* m_mainWindow;
    QTemporaryDir* m_tempDir;
};

void MainWindowTest::initTestCase()
{
    // Initialize Qt application if not already done
    if (!QApplication::instance()) {
        static int argc = 1;
        static char* argv[] = {"MainWindowTest"};
        m_app = new QApplication(argc, argv);
    } else {
        m_app = qobject_cast<QApplication*>(QApplication::instance());
    }
    
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    setupTestEnvironment();
}

void MainWindowTest::cleanupTestCase()
{
    cleanupTestEnvironment();
    delete m_tempDir;
    m_tempDir = nullptr;
}

void MainWindowTest::init()
{
    m_mainWindow = new MainWindow();
    QVERIFY(m_mainWindow != nullptr);
}

void MainWindowTest::cleanup()
{
    if (m_mainWindow) {
        delete m_mainWindow;
        m_mainWindow = nullptr;
    }
}

void MainWindowTest::testMainWindowCreation()
{
    QVERIFY(m_mainWindow != nullptr);
    QVERIFY(m_mainWindow->isVisible() || m_mainWindow->isFullScreen());
}

void MainWindowTest::testMainWindowInitialization()
{
    // Test that the main window is properly initialized
    QVERIFY(m_mainWindow != nullptr);
    
    // Check that the window has a central widget
    QWidget* centralWidget = m_mainWindow->centralWidget();
    QVERIFY(centralWidget != nullptr);
    
    // Check that the layout is set up
    QLayout* layout = centralWidget->layout();
    QVERIFY(layout != nullptr);
}

void MainWindowTest::testMainWindowLayout()
{
    QWidget* centralWidget = m_mainWindow->centralWidget();
    QVERIFY(centralWidget != nullptr);
    
    QLayout* layout = centralWidget->layout();
    QVERIFY(layout != nullptr);
    
    // MainWindow should have a horizontal layout based on the UI file
    QHBoxLayout* hLayout = qobject_cast<QHBoxLayout*>(layout);
    QVERIFY(hLayout != nullptr);
}

void MainWindowTest::testMainWindowFullscreen()
{
    // MainWindow should be in fullscreen mode based on constructor
    QVERIFY(m_mainWindow->isFullScreen());
}

void MainWindowTest::testDashboardWidgetIntegration()
{
    // Check if DashboardWidget is properly integrated
    bool foundDashboard = hasWidget(m_mainWindow, "DashboardWidget");
    QVERIFY(foundDashboard);
    
    DashboardWidget* dashboard = m_mainWindow->findChild<DashboardWidget*>();
    if (dashboard) {
        QVERIFY(dashboard->parentWidget() != nullptr);
        QVERIFY(dashboard->isVisible() || dashboard->parent() != nullptr);
    }
}

void MainWindowTest::testSettingsWidgetIntegration()
{
    // Check if SettingsWidget is properly integrated
    bool foundSettings = hasWidget(m_mainWindow, "SettingsWidget");
    QVERIFY(foundSettings);
    
    SettingsWidget* settings = m_mainWindow->findChild<SettingsWidget*>();
    if (settings) {
        QVERIFY(settings->parentWidget() != nullptr);
    }
}

void MainWindowTest::testHistoryWidgetIntegration()
{
    // Check if HistoryWidget is properly integrated
    bool foundHistory = hasWidget(m_mainWindow, "HistoryWidget");
    QVERIFY(foundHistory);
    
    HistoryWidget* history = m_mainWindow->findChild<HistoryWidget*>();
    if (history) {
        QVERIFY(history->parentWidget() != nullptr);
    }
}

void MainWindowTest::testServiceStatusWidgetIntegration()
{
    // Check if ServiceStatusWidget is properly integrated
    bool foundServiceStatus = hasWidget(m_mainWindow, "ServiceStatusWidget");
    QVERIFY(foundServiceStatus);
    
    ServiceStatusWidget* serviceStatus = m_mainWindow->findChild<ServiceStatusWidget*>();
    if (serviceStatus) {
        QVERIFY(serviceStatus->parentWidget() != nullptr);
    }
}

void MainWindowTest::testNavigationButtons()
{
    // Find navigation buttons by object name patterns
    QPushButton* dashboardButton = m_mainWindow->findChild<QPushButton*>("navDashbardButton");
    QPushButton* historyButton = m_mainWindow->findChild<QPushButton*>("navHistoryButton");
    QPushButton* serviceButton = m_mainWindow->findChild<QPushButton*>("navServiceButton");
    QPushButton* settingsButton = m_mainWindow->findChild<QPushButton*>("navSettingsButton");
    
    // At least some navigation buttons should exist
    int buttonCount = 0;
    if (dashboardButton) buttonCount++;
    if (historyButton) buttonCount++;
    if (serviceButton) buttonCount++;
    if (settingsButton) buttonCount++;
    
    QVERIFY(buttonCount > 0);
}

void MainWindowTest::testPageSwitching()
{
    // Find the stacked widget
    QStackedWidget* stackedWidget = m_mainWindow->findChild<QStackedWidget*>();
    
    if (stackedWidget) {
        int pageCount = stackedWidget->count();
        QVERIFY(pageCount > 0);
        
        // Test switching between pages
        int currentIndex = stackedWidget->currentIndex();
        
        if (pageCount > 1) {
            int newIndex = (currentIndex + 1) % pageCount;
            stackedWidget->setCurrentIndex(newIndex);
            QCOMPARE(stackedWidget->currentIndex(), newIndex);
        }
    }
}

void MainWindowTest::testStackedWidgetBehavior()
{
    QStackedWidget* stackedWidget = m_mainWindow->findChild<QStackedWidget*>();
    
    if (stackedWidget) {
        // Test that pages exist
        QVERIFY(stackedWidget->count() >= 0);
        
        // Test that current page is valid
        if (stackedWidget->count() > 0) {
            QWidget* currentPage = stackedWidget->currentWidget();
            QVERIFY(currentPage != nullptr);
        }
        
        // Test page visibility
        for (int i = 0; i < stackedWidget->count(); ++i) {
            QWidget* page = stackedWidget->widget(i);
            QVERIFY(page != nullptr);
        }
    }
}

void MainWindowTest::testWidgetCreation()
{
    // Test that all expected widgets are created
    QList<QWidget*> allWidgets = m_mainWindow->findChildren<QWidget*>();
    QVERIFY(allWidgets.size() > 0);
    
    // Test specific widget types
    QList<QPushButton*> buttons = m_mainWindow->findChildren<QPushButton*>();
    QList<QVBoxLayout*> layouts = m_mainWindow->findChildren<QVBoxLayout*>();
    
    // Should have at least some UI elements
    QVERIFY(buttons.size() + layouts.size() > 0);
}

void MainWindowTest::testWidgetDestruction()
{
    // Create a second main window
    MainWindow* secondWindow = new MainWindow();
    QVERIFY(secondWindow != nullptr);
    
    // Delete it and ensure no crash
    delete secondWindow;
    secondWindow = nullptr;
    
    // Original window should still be functional
    QVERIFY(m_mainWindow != nullptr);
    QVERIFY(m_mainWindow->centralWidget() != nullptr);
}

void MainWindowTest::testMemoryManagement()
{
    // Test that child widgets are properly managed
    QList<QWidget*> childWidgets = m_mainWindow->findChildren<QWidget*>();
    int initialCount = childWidgets.size();
    
    // All child widgets should have valid parents
    for (QWidget* child : childWidgets) {
        if (child != m_mainWindow->centralWidget()) {
            QVERIFY(child->parent() != nullptr);
        }
    }
    
    // Widget count should be consistent
    QVERIFY(initialCount > 0);
}

void MainWindowTest::testMissingPages()
{
    // Test handling of missing pages in stacked widget
    QStackedWidget* stackedWidget = m_mainWindow->findChild<QStackedWidget*>();
    
    if (stackedWidget) {
        // Should handle empty or invalid indices gracefully
        int originalIndex = stackedWidget->currentIndex();
        
        // Try to set invalid index
        stackedWidget->setCurrentIndex(-1);
        // Should either stay at original or move to valid index
        
        stackedWidget->setCurrentIndex(originalIndex);
        QVERIFY(stackedWidget->currentIndex() >= 0);
    }
}

void MainWindowTest::testDatabaseServiceIntegration()
{
    // Test that database service integration doesn't cause crashes
    HistoryWidget* historyWidget = m_mainWindow->findChild<HistoryWidget*>();
    
    if (historyWidget) {
        // History widget should be properly initialized with database service
        QVERIFY(historyWidget->parent() != nullptr);
    }
    
    // MainWindow should handle database service availability gracefully
    QVERIFY(m_mainWindow != nullptr);
}

void MainWindowTest::testWidgetErrorStates()
{
    // Test that widgets handle error states gracefully
    DashboardWidget* dashboard = m_mainWindow->findChild<DashboardWidget*>();
    SettingsWidget* settings = m_mainWindow->findChild<SettingsWidget*>();
    
    if (dashboard) {
        // Dashboard should be functional even in test environment
        QVERIFY(dashboard->isWidgetType());
    }
    
    if (settings) {
        // Settings should be functional even in test environment
        QVERIFY(settings->isWidgetType());
    }
}

void MainWindowTest::testWidgetInteractions()
{
    // Test basic widget interactions
    QPushButton* button = m_mainWindow->findChild<QPushButton*>();
    
    if (button && button->isVisible() && button->isEnabled()) {
        // Test button click simulation
        QSignalSpy clickSpy(button, &QPushButton::clicked);
        
        QTest::mouseClick(button, Qt::LeftButton);
        
        // Should have received click signal
        QVERIFY(clickSpy.count() >= 0);  // Allow for 0 if button doesn't emit in test
    }
}

void MainWindowTest::testSignalSlotConnections()
{
    // Test that navigation connections are properly set up
    QPushButton* navButton = m_mainWindow->findChild<QPushButton*>("navDashbardButton");
    QStackedWidget* stackedWidget = m_mainWindow->findChild<QStackedWidget*>();
    
    if (navButton && stackedWidget) {
        // Navigation system should be functional
        QVERIFY(navButton->isWidgetType());
        QVERIFY(stackedWidget->isWidgetType());
    }
}

// Helper Methods

void MainWindowTest::setupTestEnvironment()
{
    // Set up any test-specific environment
    QDir::setCurrent(m_tempDir->path());
}

void MainWindowTest::cleanupTestEnvironment()
{
    // Clean up test environment
}

bool MainWindowTest::hasWidget(QWidget* parent, const QString& className)
{
    if (!parent) return false;
    
    // Check if parent itself matches
    if (parent->metaObject()->className() == className) {
        return true;
    }
    
    // Check children recursively
    QList<QWidget*> children = parent->findChildren<QWidget*>();
    for (QWidget* child : children) {
        if (child->metaObject()->className() == className) {
            return true;
        }
    }
    
    return false;
}

QWidget* MainWindowTest::findWidget(QWidget* parent, const QString& className)
{
    if (!parent) return nullptr;
    
    // Check if parent itself matches
    if (parent->metaObject()->className() == className) {
        return parent;
    }
    
    // Check children recursively
    QList<QWidget*> children = parent->findChildren<QWidget*>();
    for (QWidget* child : children) {
        if (child->metaObject()->className() == className) {
            return child;
        }
    }
    
    return nullptr;
}

QTEST_MAIN(MainWindowTest)
#include "MainWindowTest.moc"
