/**
 * @file SettingsWidget.cpp
 * @brief Implementation of SettingsWidget class
 */

#include "SettingsWidget.h"
#include "../../../core/config/SettingsManager.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QStandardPaths>

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
    , m_tabWidget(nullptr)
    , m_languageCombo(nullptr)
    , m_themeCombo(nullptr)
    , m_virusTotalApiKeyEdit(nullptr)
    , m_autoScanCheckBox(nullptr)
    , m_scanTimeoutSpinBox(nullptr)
    , m_quarantinePathEdit(nullptr)
    , m_browseQuarantineButton(nullptr)
    , m_networkMonitoringCheckBox(nullptr)
    , m_debugModeCheckBox(nullptr)
    , m_maxThreadsSpinBox(nullptr)
    , m_databasePathEdit(nullptr)
    , m_browseDatabaseButton(nullptr)
    , m_applyButton(nullptr)
    , m_resetButton(nullptr)
{
    setupUI();
    loadSettings();
}

SettingsWidget::~SettingsWidget()
{
}

void SettingsWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);
    
    setupGeneralTab();
    setupScanningTab();
    setupNetworkTab();
    setupAdvancedTab();
    setupButtonBox(mainLayout);
    
    setLayout(mainLayout);
}

void SettingsWidget::setupGeneralTab()
{
    QWidget *generalTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(generalTab);
    
    // Language and Theme group
    QGroupBox *uiGroup = new QGroupBox(tr("Kullanıcı Arayüzü"));
    QFormLayout *uiLayout = new QFormLayout(uiGroup);
    
    // Language selection
    m_languageCombo = new QComboBox();
    m_languageCombo->addItem(tr("Türkçe"), "tr_TR");
    m_languageCombo->addItem(tr("English"), "en_US");
    uiLayout->addRow(tr("Dil:"), m_languageCombo);
    
    // Theme selection
    m_themeCombo = new QComboBox();
    m_themeCombo->addItem(tr("Koyu Tema"), "dark");
    m_themeCombo->addItem(tr("Açık Tema"), "light");
    m_themeCombo->addItem(tr("Sistem"), "system");
    uiLayout->addRow(tr("Tema:"), m_themeCombo);
    
    layout->addWidget(uiGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(generalTab, tr("Genel"));
}

void SettingsWidget::setupScanningTab()
{
    QWidget *scanningTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(scanningTab);
    
    // VirusTotal API group
    QGroupBox *vtGroup = new QGroupBox(tr("VirusTotal API"));
    QFormLayout *vtLayout = new QFormLayout(vtGroup);
    
    m_virusTotalApiKeyEdit = new QLineEdit();
    m_virusTotalApiKeyEdit->setEchoMode(QLineEdit::Password);
    m_virusTotalApiKeyEdit->setPlaceholderText(tr("VirusTotal API anahtarınızı girin"));
    vtLayout->addRow(tr("API Anahtarı:"), m_virusTotalApiKeyEdit);
    
    layout->addWidget(vtGroup);
    
    // Scanning preferences group
    QGroupBox *scanGroup = new QGroupBox(tr("Tarama Ayarları"));
    QFormLayout *scanLayout = new QFormLayout(scanGroup);
    
    m_autoScanCheckBox = new QCheckBox(tr("Dosyalar açıldığında otomatik tara"));
    scanLayout->addRow(m_autoScanCheckBox);
    
    m_scanTimeoutSpinBox = new QSpinBox();
    m_scanTimeoutSpinBox->setRange(5, 300);
    m_scanTimeoutSpinBox->setSuffix(tr(" saniye"));
    scanLayout->addRow(tr("Tarama Zaman Aşımı:"), m_scanTimeoutSpinBox);
    
    // Quarantine path
    QHBoxLayout *quarantineLayout = new QHBoxLayout();
    m_quarantinePathEdit = new QLineEdit();
    m_quarantinePathEdit->setPlaceholderText(tr("Karantina klasörü yolu"));
    m_browseQuarantineButton = new QPushButton(tr("Gözat..."));
    connect(m_browseQuarantineButton, &QPushButton::clicked, this, &SettingsWidget::onBrowseQuarantineClicked);
    
    quarantineLayout->addWidget(m_quarantinePathEdit);
    quarantineLayout->addWidget(m_browseQuarantineButton);
    scanLayout->addRow(tr("Karantina Klasörü:"), quarantineLayout);
    
    layout->addWidget(scanGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(scanningTab, tr("Tarama"));
}

void SettingsWidget::setupNetworkTab()
{
    QWidget *networkTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(networkTab);
    
    // Network group
    QGroupBox *networkGroup = new QGroupBox(tr("Ağ İzleme"));
    QVBoxLayout *networkLayout = new QVBoxLayout(networkGroup);
    
    m_networkMonitoringCheckBox = new QCheckBox(tr("Ağ trafiğini izle"));
    m_networkMonitoringCheckBox->setToolTip(tr("Şüpheli ağ etkinliğini otomatik olarak izler"));
    networkLayout->addWidget(m_networkMonitoringCheckBox);
    
    layout->addWidget(networkGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(networkTab, tr("Ağ"));
}

void SettingsWidget::setupAdvancedTab()
{
    QWidget *advancedTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(advancedTab);
    
    // Debug group
    QGroupBox *debugGroup = new QGroupBox(tr("Hata Ayıklama"));
    QVBoxLayout *debugLayout = new QVBoxLayout(debugGroup);
    
    m_debugModeCheckBox = new QCheckBox(tr("Hata ayıklama modunu etkinleştir"));
    m_debugModeCheckBox->setToolTip(tr("Gelişmiş günlükleme ve hata ayıklama bilgilerini etkinleştirir"));
    debugLayout->addWidget(m_debugModeCheckBox);
    
    layout->addWidget(debugGroup);
    
    // Performance group
    QGroupBox *perfGroup = new QGroupBox(tr("Performans"));
    QFormLayout *perfLayout = new QFormLayout(perfGroup);
    
    m_maxThreadsSpinBox = new QSpinBox();
    m_maxThreadsSpinBox->setRange(1, 16);
    m_maxThreadsSpinBox->setToolTip(tr("Eşzamanlı tarama için maksimum iş parçacığı sayısı"));
    perfLayout->addRow(tr("Maksimum İş Parçacığı:"), m_maxThreadsSpinBox);
    
    layout->addWidget(perfGroup);
    
    // Database group
    QGroupBox *dbGroup = new QGroupBox(tr("Veritabanı"));
    QFormLayout *dbLayout = new QFormLayout(dbGroup);
    
    QHBoxLayout *dbPathLayout = new QHBoxLayout();
    m_databasePathEdit = new QLineEdit();
    m_databasePathEdit->setPlaceholderText(tr("Veritabanı dosya yolu"));
    m_browseDatabaseButton = new QPushButton(tr("Gözat..."));
    connect(m_browseDatabaseButton, &QPushButton::clicked, this, &SettingsWidget::onBrowseDatabaseClicked);
    
    dbPathLayout->addWidget(m_databasePathEdit);
    dbPathLayout->addWidget(m_browseDatabaseButton);
    dbLayout->addRow(tr("Veritabanı Yolu:"), dbPathLayout);
    
    layout->addWidget(dbGroup);
    
    // Documentation group
    QGroupBox *docsGroup = new QGroupBox(tr("Dokümantasyon"));
    QVBoxLayout *docsLayout = new QVBoxLayout(docsGroup);
    
    QLabel *docsInfo = new QLabel(tr("Proje dokümantasyonu ve API referansları:"));
    docsInfo->setWordWrap(true);
    docsLayout->addWidget(docsInfo);
    
    QHBoxLayout *docsButtonLayout = new QHBoxLayout();
    
    QPushButton *openDocsButton = new QPushButton(tr("📚 Dokümantasyonu Aç"));
    openDocsButton->setToolTip(tr("Proje dokümantasyonunu browser'da aç"));
    connect(openDocsButton, &QPushButton::clicked, this, &SettingsWidget::onOpenDocumentationClicked);
    
    QPushButton *generateDocsButton = new QPushButton(tr("🔄 Dokümantasyon Oluştur"));
    generateDocsButton->setToolTip(tr("Güncel dokümantasyonu yeniden oluştur"));
    connect(generateDocsButton, &QPushButton::clicked, this, &SettingsWidget::onGenerateDocumentationClicked);
    
    docsButtonLayout->addWidget(openDocsButton);
    docsButtonLayout->addWidget(generateDocsButton);
    docsLayout->addLayout(docsButtonLayout);
    
    layout->addWidget(docsGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(advancedTab, tr("Gelişmiş"));
}

void SettingsWidget::setupButtonBox(QVBoxLayout *mainLayout)
{
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_resetButton = new QPushButton(tr("Varsayılanlara Dön"));
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsWidget::onResetToDefaultsClicked);
    
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addStretch();
    
    m_applyButton = new QPushButton(tr("Uygula"));
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsWidget::onApplyClicked);
    
    buttonLayout->addWidget(m_applyButton);
    
    mainLayout->addLayout(buttonLayout);
}

void SettingsWidget::loadSettings()
{
    SettingsManager& settings = SettingsManager::getInstance();
    
    // General settings
    QString language = settings.getLanguage();
    int langIndex = m_languageCombo->findData(language);
    if (langIndex >= 0) {
        m_languageCombo->setCurrentIndex(langIndex);
    }
    
    QString theme = settings.getTheme();
    int themeIndex = m_themeCombo->findData(theme);
    if (themeIndex >= 0) {
        m_themeCombo->setCurrentIndex(themeIndex);
    }
    
    // Scanning settings
    m_virusTotalApiKeyEdit->setText(settings.getVirusTotalApiKey());
    m_autoScanCheckBox->setChecked(settings.getAutoScanEnabled());
    m_scanTimeoutSpinBox->setValue(settings.getScanTimeout());
    m_quarantinePathEdit->setText(settings.getQuarantinePath());
    
    // Network settings
    m_networkMonitoringCheckBox->setChecked(settings.getNetworkMonitoringEnabled());
    
    // Advanced settings
    m_debugModeCheckBox->setChecked(settings.getDebugModeEnabled());
    m_maxThreadsSpinBox->setValue(settings.getMaxScanThreads());
    m_databasePathEdit->setText(settings.getDatabasePath());
}

void SettingsWidget::saveSettings()
{
    SettingsManager& settings = SettingsManager::getInstance();
    
    // General settings
    settings.setLanguage(m_languageCombo->currentData().toString());
    settings.setTheme(m_themeCombo->currentData().toString());
    
    // Scanning settings
    settings.setVirusTotalApiKey(m_virusTotalApiKeyEdit->text());
    settings.setAutoScanEnabled(m_autoScanCheckBox->isChecked());
    settings.setScanTimeout(m_scanTimeoutSpinBox->value());
    settings.setQuarantinePath(m_quarantinePathEdit->text());
    
    // Network settings
    settings.setNetworkMonitoringEnabled(m_networkMonitoringCheckBox->isChecked());
    
    // Advanced settings
    settings.setDebugModeEnabled(m_debugModeCheckBox->isChecked());
    settings.setMaxScanThreads(m_maxThreadsSpinBox->value());
    settings.setDatabasePath(m_databasePathEdit->text());
    
    settings.sync();
    
    // Emit signal to notify that settings have changed
    emit settingsChanged();
}

void SettingsWidget::onBrowseQuarantineClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Karantina Klasörünü Seçin"), 
                                                   m_quarantinePathEdit->text());
    if (!dir.isEmpty()) {
        m_quarantinePathEdit->setText(dir);
    }
}

void SettingsWidget::onBrowseDatabaseClicked()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Veritabanı Dosyasını Seçin"),
                                               m_databasePathEdit->text(),
                                               tr("Veritabanı Dosyaları (*.db *.sqlite)"));
    if (!file.isEmpty()) {
        m_databasePathEdit->setText(file);
    }
}

void SettingsWidget::onResetToDefaultsClicked()
{
    int ret = QMessageBox::question(this, tr("Varsayılan Ayarlar"), 
                                   tr("Tüm ayarları varsayılan değerlere döndürmek istediğinizden emin misiniz?"),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        resetToDefaults();
    }
}

void SettingsWidget::onApplyClicked()
{
    saveSettings();
    QMessageBox::information(this, tr("Ayarlar"), tr("Ayarlar başarıyla kaydedildi."));
}

void SettingsWidget::resetToDefaults()
{
    SettingsManager& settings = SettingsManager::getInstance();
    settings.resetToDefaults();
    loadSettings();
}

void SettingsWidget::onOpenDocumentationClicked()
{
    // Find project root and check if documentation exists
    QString projectRoot;
    QString currentPath = QCoreApplication::applicationDirPath();
    
    // Check multiple possible locations for the project root
    QStringList possiblePaths = {
        currentPath,                                    // Application directory
        QDir::currentPath(),                           // Current working directory  
        currentPath + "/../..",                        // Two levels up from app dir
        currentPath + "/../../..",                     // Three levels up from app dir
        "/Volumes/Crucial/AVProjectUi"                 // Fallback to known path
    };
    
    QFileInfo docFile;
    for (const QString& path : possiblePaths) {
        QDir dir(path);
        QString docPath = dir.absolutePath() + "/docs/html/index.html";
        QFileInfo testFile(docPath);
        
        if (testFile.exists()) {
            docFile = testFile;
            projectRoot = dir.absolutePath();
            break;
        }
        
        // Also check if this looks like the project root (has characteristic files)
        if (dir.exists("tools/scripts/docs.sh") && dir.exists("CMakeLists.txt")) {
            projectRoot = dir.absolutePath();
        }
    }
    
    if (!docFile.exists()) {
        int ret = QMessageBox::question(this, tr("Dokümantasyon"), 
                                       tr("Dokümantasyon dosyası bulunamadı. Şimdi oluşturmak ister misiniz?"),
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            onGenerateDocumentationClicked();
        }
        return;
    }
    
    // Open documentation in default browser
    QString url = "file://" + docFile.absoluteFilePath();
    if (QDesktopServices::openUrl(QUrl(url))) {
        QMessageBox::information(this, tr("Dokümantasyon"), 
                               tr("Dokümantasyon browser'da açıldı."));
    } else {
        QMessageBox::warning(this, tr("Hata"), 
                           tr("Dokümantasyon açılamadı. Lütfen manuel olarak şu yolu açın:\n") + 
                           docFile.absoluteFilePath());
    }
}

void SettingsWidget::onGenerateDocumentationClicked()
{
    QMessageBox::information(this, tr("Dokümantasyon"), 
                           tr("Dokümantasyon oluşturuluyor... Bu işlem birkaç saniye sürebilir."));
    
    // Find project root by looking for characteristic files
    QString projectRoot;
    QString currentPath = QCoreApplication::applicationDirPath();
    
    // Check multiple possible locations for the project root
    QStringList possiblePaths = {
        currentPath,                                    // Application directory
        QDir::currentPath(),                           // Current working directory  
        currentPath + "/../..",                        // Two levels up from app dir
        currentPath + "/../../..",                     // Three levels up from app dir
        "/Volumes/Crucial/AVProjectUi"                 // Fallback to known path
    };
    
    for (const QString& path : possiblePaths) {
        QDir dir(path);
        if (dir.exists("tools/scripts/docs.sh") && dir.exists("CMakeLists.txt")) {
            projectRoot = dir.absolutePath();
            break;
        }
    }
    
    if (projectRoot.isEmpty()) {
        QMessageBox::warning(this, tr("Hata"), 
                           tr("Proje kök dizini bulunamadı. Dokümantasyon scripti çalıştırılamıyor."));
        return;
    }
    
    // Run documentation generation script
    QString scriptPath = projectRoot + "/tools/scripts/docs.sh";
    QProcess process;
    process.setWorkingDirectory(projectRoot);
    
    QStringList args;
    args << "generate";
    
    process.start("bash", QStringList() << scriptPath << args);
    process.waitForFinished(30000); // 30 second timeout
    
    QString stdOut = process.readAllStandardOutput();
    QString stdErr = process.readAllStandardError();
    
    if (process.exitCode() == 0) {
        QMessageBox::information(this, tr("Dokümantasyon"), 
                               tr("Dokümantasyon başarıyla oluşturuldu!"));
        
        // Optionally open the generated documentation
        int ret = QMessageBox::question(this, tr("Dokümantasyon"), 
                                       tr("Oluşturulan dokümantasyonu şimdi açmak ister misiniz?"),
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            onOpenDocumentationClicked();
        }
    } else {
        QString errorMsg = tr("Dokümantasyon oluşturulurken hata oluştu:\n\n");
        if (!stdErr.isEmpty()) {
            errorMsg += tr("Hata mesajı:\n") + stdErr + "\n\n";
        }
        if (!stdOut.isEmpty()) {
            errorMsg += tr("Çıktı:\n") + stdOut;
        }
        errorMsg += tr("\n\nProje kökü: ") + projectRoot;
        errorMsg += tr("\nScript yolu: ") + scriptPath;
        
        QMessageBox::warning(this, tr("Hata"), errorMsg);
    }
}