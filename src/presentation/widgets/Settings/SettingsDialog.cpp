/**
 * @file SettingsDialog.cpp
 * @brief Implementation of SettingsDialog class
 */

#include "SettingsDialog.h"
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
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QApplication>
#include <QStandardPaths>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
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
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_applyButton(nullptr)
    , m_resetButton(nullptr)
{
    setWindowTitle(tr("Ayarlar"));
    setWindowIcon(QIcon(":/images/settings.png"));
    setModal(true);
    resize(500, 400);
    
    setupUI();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);
    
    setupGeneralTab();
    setupScanningTab();
    setupNetworkTab();
    setupAdvancedTab();
    setupButtonBox();
    
    mainLayout->addLayout(new QHBoxLayout()); // Spacer
}

void SettingsDialog::setupGeneralTab()
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

void SettingsDialog::setupScanningTab()
{
    QWidget *scanningTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(scanningTab);
    
    // VirusTotal API group
    QGroupBox *vtGroup = new QGroupBox(tr("VirusTotal API"));
    QFormLayout *vtLayout = new QFormLayout(vtGroup);
    
    m_virusTotalApiKeyEdit = new QLineEdit();
    m_virusTotalApiKeyEdit->setEchoMode(QLineEdit::Password);
    m_virusTotalApiKeyEdit->setPlaceholderText(tr("Enter your VirusTotal API key"));
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
    m_quarantinePathEdit->setPlaceholderText(tr("Quarantine folder path"));
    m_browseQuarantineButton = new QPushButton(tr("Gözat..."));
    connect(m_browseQuarantineButton, &QPushButton::clicked, this, &SettingsDialog::onBrowseQuarantineClicked);
    
    quarantineLayout->addWidget(m_quarantinePathEdit);
    quarantineLayout->addWidget(m_browseQuarantineButton);
    scanLayout->addRow(tr("Karantina Klasörü:"), quarantineLayout);
    
    layout->addWidget(scanGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(scanningTab, tr("Tarama"));
}

void SettingsDialog::setupNetworkTab()
{
    QWidget *networkTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(networkTab);
    
    QGroupBox *networkGroup = new QGroupBox(tr("Ağ İzleme"));
    QVBoxLayout *networkLayout = new QVBoxLayout(networkGroup);
    
    m_networkMonitoringCheckBox = new QCheckBox(tr("Ağ trafiğini izle"));
    m_networkMonitoringCheckBox->setToolTip(tr("Şüpheli ağ etkinliğini otomatik olarak izler"));
    networkLayout->addWidget(m_networkMonitoringCheckBox);
    
    layout->addWidget(networkGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(networkTab, tr("Ağ"));
}

void SettingsDialog::setupAdvancedTab()
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
    m_databasePathEdit->setPlaceholderText(tr("Database file path"));
    m_browseDatabaseButton = new QPushButton(tr("Gözat..."));
    connect(m_browseDatabaseButton, &QPushButton::clicked, this, &SettingsDialog::onBrowseDatabaseClicked);
    
    dbPathLayout->addWidget(m_databasePathEdit);
    dbPathLayout->addWidget(m_browseDatabaseButton);
    dbLayout->addRow(tr("Veritabanı Yolu:"), dbPathLayout);
    
    layout->addWidget(dbGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(advancedTab, tr("Gelişmiş"));
}

void SettingsDialog::setupButtonBox()
{
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_resetButton = new QPushButton(tr("Varsayılanlara Dön"));
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsDialog::onResetToDefaultsClicked);
    
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton(tr("Tamam"));
    m_cancelButton = new QPushButton(tr("İptal"));
    m_applyButton = new QPushButton(tr("Uygula"));
    
    connect(m_okButton, &QPushButton::clicked, this, &SettingsDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::reject);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
    
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_applyButton);
    
    layout()->addItem(buttonLayout);
}

void SettingsDialog::loadSettings()
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

void SettingsDialog::saveSettings()
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

void SettingsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}

void SettingsDialog::reject()
{
    QDialog::reject();
}

void SettingsDialog::onBrowseQuarantineClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Karantina Klasörünü Seçin"), 
                                                   m_quarantinePathEdit->text());
    if (!dir.isEmpty()) {
        m_quarantinePathEdit->setText(dir);
    }
}

void SettingsDialog::onBrowseDatabaseClicked()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Veritabanı Dosyasını Seçin"),
                                               m_databasePathEdit->text(),
                                               tr("Veritabanı Dosyaları (*.db *.sqlite)"));
    if (!file.isEmpty()) {
        m_databasePathEdit->setText(file);
    }
}

void SettingsDialog::onResetToDefaultsClicked()
{
    int ret = QMessageBox::question(this, tr("Varsayılan Ayarlar"), 
                                   tr("Tüm ayarları varsayılan değerlere döndürmek istediğinizden emin misiniz?"),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        resetToDefaults();
    }
}

void SettingsDialog::onApplyClicked()
{
    saveSettings();
    // Note: Don't close dialog when apply is clicked, just save and emit signal
}

void SettingsDialog::resetToDefaults()
{
    SettingsManager& settings = SettingsManager::getInstance();
    settings.resetToDefaults();
    loadSettings();
}
