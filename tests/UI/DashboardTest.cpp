#include "DashboardTest.h"
#include <QSignalSpy>
#include <QPushButton>
#include <QTextEdit>

// Test case başlangıcında çalışacak
void DashboardTest::initTestCase()
{
    qDebug() << "DashboardTest başlatılıyor...";
}

// Test case bitiminde çalışacak
void DashboardTest::cleanupTestCase()
{
    qDebug() << "DashboardTest temizleniyor...";
}

// Her test başlangıcında çalışacak
void DashboardTest::init()
{
    dashboard = new DashboardWidget();
}

// Her test bitiminde çalışacak
void DashboardTest::cleanup()
{
    delete dashboard;
    dashboard = nullptr;
}

// Widget'ın doğru şekilde başlatılıp başlatılmadığını test et
void DashboardTest::testInitialization()
{
    QVERIFY(dashboard != nullptr);
    QVERIFY(dashboard->isVisible() == false); // Başlangıçta görünmez olmalı
}

// Basic scan butonunun çalışıp çalışmadığını test et
void DashboardTest::testBasicScanButton()
{
    // Buton referansını al
    QPushButton *basicScanButton = dashboard->findChild<QPushButton*>("basicScanButton_dashboard");
    QVERIFY(basicScanButton != nullptr);
    
    // Sonuç alanının referansını al
    QTextEdit *resultText = dashboard->findChild<QTextEdit*>("scanResultsTextEdit_dashboard");
    QVERIFY(resultText != nullptr);
    
    // Buton tıklanması öncesi içeriğin boş olduğunu kontrol et
    QVERIFY(resultText->toPlainText().isEmpty());
    
    // Butona tıkla
    QTest::mouseClick(basicScanButton, Qt::LeftButton);
    
    // Beklenen sonuç: "Basic Scan başlatılıyor..." içeriyor mu kontrol et
    QVERIFY(resultText->toPlainText().contains("Basic Scan başlatılıyor..."));
}

// Advanced scan butonunun çalışıp çalışmadığını test et
void DashboardTest::testAdvancedScanButton()
{
    // Buton referansını al
    QPushButton *advancedScanButton = dashboard->findChild<QPushButton*>("advancedScanButton_dashboard");
    QVERIFY(advancedScanButton != nullptr);
    
    // Sonuç alanının referansını al
    QTextEdit *resultText = dashboard->findChild<QTextEdit*>("scanResultsTextEdit_dashboard");
    QVERIFY(resultText != nullptr);
    
    // Buton tıklanması öncesi içeriğin boş olduğunu kontrol et
    QVERIFY(resultText->toPlainText().isEmpty());
    
    // Butona tıkla
    QTest::mouseClick(advancedScanButton, Qt::LeftButton);
    
    // Beklenen sonuç: "Advanced Scan başlatılıyor..." içeriyor mu kontrol et
    QVERIFY(resultText->toPlainText().contains("Advanced Scan başlatılıyor..."));
}

// CDR scan butonunun çalışıp çalışmadığını test et
void DashboardTest::testCdrScanButton()
{
    // Buton referansını al
    QPushButton *cdrScanButton = dashboard->findChild<QPushButton*>("cdrScanButton_dashboard");
    QVERIFY(cdrScanButton != nullptr);
    
    // Sonuç alanının referansını al
    QTextEdit *resultText = dashboard->findChild<QTextEdit*>("scanResultsTextEdit_dashboard");
    QVERIFY(resultText != nullptr);
    
    // Buton tıklanması öncesi içeriğin boş olduğunu kontrol et
    QVERIFY(resultText->toPlainText().isEmpty());
    
    // Butona tıkla
    QTest::mouseClick(cdrScanButton, Qt::LeftButton);
    
    // Beklenen sonuç: "CDR Scan başlatılıyor..." içeriyor mu kontrol et
    QVERIFY(resultText->toPlainText().contains("CDR Scan başlatılıyor..."));
}

// Sandbox scan butonunun çalışıp çalışmadığını test et
void DashboardTest::testSandboxScanButton()
{
    // Buton referansını al
    QPushButton *sandboxScanButton = dashboard->findChild<QPushButton*>("sandboxScanButton_dashboard");
    QVERIFY(sandboxScanButton != nullptr);
    
    // Sonuç alanının referansını al
    QTextEdit *resultText = dashboard->findChild<QTextEdit*>("scanResultsTextEdit_dashboard");
    QVERIFY(resultText != nullptr);
    
    // Buton tıklanması öncesi içeriğin boş olduğunu kontrol et
    QVERIFY(resultText->toPlainText().isEmpty());
    
    // Butona tıkla
    QTest::mouseClick(sandboxScanButton, Qt::LeftButton);
    
    // Beklenen sonuç: "Sandbox Scan başlatılıyor..." içeriyor mu kontrol et
    QVERIFY(resultText->toPlainText().contains("Sandbox Scan başlatılıyor..."));
}

// Widget görünürlüğünü test et
void DashboardTest::testWidgetVisibility()
{
    dashboard->show();
    QVERIFY(dashboard->isVisible() == true);
    
    dashboard->hide();
    QVERIFY(dashboard->isVisible() == false);
}

// Test sınıfını Qt test framework'üne kaydet
QTEST_MAIN(DashboardTest) 