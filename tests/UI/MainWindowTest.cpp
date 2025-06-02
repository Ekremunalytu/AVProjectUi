#include "MainWindowTest.h"
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "UI/Widgets/Dashboard/DashboardWidget.h"

// Test case başlangıcında çalışacak
void MainWindowTest::initTestCase()
{
    qDebug() << "MainWindowTest başlatılıyor...";
}

// Test case bitiminde çalışacak
void MainWindowTest::cleanupTestCase()
{
    qDebug() << "MainWindowTest temizleniyor...";
}

// Her test başlangıcında çalışacak
void MainWindowTest::init()
{
    // Create test database manager instance
    testDbManager = new DbManager();
    
    // Create MainWindow with test database manager
    mainWindow = new MainWindow(testDbManager);
}

// Her test bitiminde çalışacak
void MainWindowTest::cleanup()
{
    delete mainWindow;
    mainWindow = nullptr;
    delete testDbManager;
    testDbManager = nullptr;
}

// MainWindow'un doğru şekilde başlatılıp başlatılmadığını test et
void MainWindowTest::testInitialization()
{
    QVERIFY(mainWindow != nullptr);
    
    // contentStackedWidget'ın varlığını kontrol et
    QStackedWidget *contentWidget = mainWindow->findChild<QStackedWidget*>("contentStackedWidget");
    QVERIFY(contentWidget != nullptr);
    
    // Başlangıçta dashboard sayfasının (0. index) görüntülendiğini kontrol et
    QCOMPARE(contentWidget->currentIndex(), 0);
}

// Navigasyon butonlarının doğru çalışıp çalışmadığını test et
void MainWindowTest::testNavigationButtons()
{
    // Gerekli widget'ları bul
    QStackedWidget *contentWidget = mainWindow->findChild<QStackedWidget*>("contentStackedWidget");
    QVERIFY(contentWidget != nullptr);
    
    // Navigasyon butonlarını bul
    QPushButton *navDashbardButton = mainWindow->findChild<QPushButton*>("navDashbardButton");
    QPushButton *navHistoryButton = mainWindow->findChild<QPushButton*>("navHistoryButton");
    QPushButton *navServiceStatusButton = mainWindow->findChild<QPushButton*>("navServiceStatusButton");
    QPushButton *navSettingsButton = mainWindow->findChild<QPushButton*>("navSettingsButton");
    
    QVERIFY(navDashbardButton != nullptr);
    QVERIFY(navHistoryButton != nullptr);
    QVERIFY(navServiceStatusButton != nullptr);
    QVERIFY(navSettingsButton != nullptr);
    
    // History butonuna tıkla
    QTest::mouseClick(navHistoryButton, Qt::LeftButton);
    QCOMPARE(contentWidget->currentIndex(), 3); // History sayfası
    
    // Service status butonuna tıkla
    QTest::mouseClick(navServiceStatusButton, Qt::LeftButton);
    QCOMPARE(contentWidget->currentIndex(), 1); // Service status sayfası
    
    // Settings butonuna tıkla
    QTest::mouseClick(navSettingsButton, Qt::LeftButton);
    QCOMPARE(contentWidget->currentIndex(), 2); // Settings sayfası
    
    // Dashboard butonuna tıkla
    QTest::mouseClick(navDashbardButton, Qt::LeftButton);
    QCOMPARE(contentWidget->currentIndex(), 0); // Dashboard sayfası
}

// Dashboard widget'ının MainWindow'a doğru entegre edilip edilmediğini test et
void MainWindowTest::testDashboardWidgetIntegration()
{
    // dashboardPage widget'ını bul
    QWidget *dashboardPage = mainWindow->findChild<QWidget*>("dashboardPage");
    QVERIFY(dashboardPage != nullptr);
    
    // dashboardPage'in bir layout'u olmalı
    QVERIFY(dashboardPage->layout() != nullptr);
    
    // Layout içinde bir DashboardWidget olmalı
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(dashboardPage->layout());
    QVERIFY(layout != nullptr);
    QVERIFY(layout->count() > 0);
    
    // İlk widget bir DashboardWidget olmalı
    QWidget *widget = layout->itemAt(0)->widget();
    QVERIFY(widget != nullptr);
    DashboardWidget *dashboardWidget = qobject_cast<DashboardWidget*>(widget);
    QVERIFY(dashboardWidget != nullptr);
}

// Test sınıfını Qt test framework'üne kaydet
QTEST_MAIN(MainWindowTest) 