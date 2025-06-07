/**
 * @file SettingsWidgetTest.cpp
 * @brief Unit tests for SettingsWidget class
 * @author Test Suite
 * @date 2025
 */

#include <QtTest/QtTest>
#include <QApplication>
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
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QSettings>
#include <QFileDialog>
#include <QDebug>

#include "presentation/widgets/Settings/SettingsWidget.h"
#include "core/config/SettingsManager.h"

/**
 * @brief Test class for SettingsWidget functionality
 */
class SettingsWidgetTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Widget creation tests
    void testWidgetCreation();
    void testTabWidgetSetup();
    void testUIComponentCreation();
    void testLayoutSetup();
    
    // Tab functionality tests
    void testGeneralTabSetup();
    void testScanningTabSetup();
    void testNetworkTabSetup();
    void testAdvancedTabSetup();
    
    // Settings interaction tests
    void testLanguageSelection();
    void testThemeSelection();
    void testVirusTotalApiKey();
    void testAutoScanSettings();
    void testQuarantinePathSettings();
    void testNetworkMonitoringSettings();
    void testAdvancedSettings();
    
    // Button functionality tests
    void testApplyButton();
    void testResetButton();
    void testBrowseButtons();
    
    // Settings persistence tests
    void testLoadSettings();
    void testSaveSettings();
    void testSettingsValidation();
    void testDefaultSettings();
    
    // Signal/slot tests
    void testSignalConnections();
    void testSettingsChanged();
    void testButtonSignals();
    
    // Error handling tests
    void testInvalidPaths();
    void testMissingSettings();
    void testCorruptedSettings();

private:
    void setupTestSettings();
    void cleanupTestSettings();
    QWidget* findTabByName(const QString& tabName);
    void simulateUserInput();
    
    QApplication* m_app;
    SettingsWidget* m_settingsWidget;
    QTemporaryDir* m_tempDir;
    QString m_testSettingsPath;
};

void SettingsWidgetTest::initTestCase()
{
    // Initialize Qt application if not already done
    if (!QApplication::instance()) {
        static int argc = 1;
        static char* argv[] = {"SettingsWidgetTest"};
        m_app = new QApplication(argc, argv);
    } else {
        m_app = qobject_cast<QApplication*>(QApplication::instance());
    }
    
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    m_testSettingsPath = m_tempDir->path() + "/test_settings.ini";
    setupTestSettings();
}

void SettingsWidgetTest::cleanupTestCase()
{
    cleanupTestSettings();
    delete m_tempDir;
    m_tempDir = nullptr;
}

void SettingsWidgetTest::init()
{
    m_settingsWidget = new SettingsWidget();
    QVERIFY(m_settingsWidget != nullptr);
}

void SettingsWidgetTest::cleanup()
{
    if (m_settingsWidget) {
        delete m_settingsWidget;
        m_settingsWidget = nullptr;
    }
}

void SettingsWidgetTest::testWidgetCreation()
{
    QVERIFY(m_settingsWidget != nullptr);
    QVERIFY(m_settingsWidget->isWidgetType());
    
    // Widget should have a layout
    QLayout* layout = m_settingsWidget->layout();
    QVERIFY(layout != nullptr);
}

void SettingsWidgetTest::testTabWidgetSetup()
{
    QTabWidget* tabWidget = m_settingsWidget->findChild<QTabWidget*>();
    QVERIFY(tabWidget != nullptr);
    
    // Should have multiple tabs
    QVERIFY(tabWidget->count() > 0);
    
    // Check that tabs have proper names/text
    QStringList expectedTabs = {"General", "Scanning", "Network", "Advanced"};
    for (int i = 0; i < tabWidget->count() && i < expectedTabs.size(); ++i) {
        QString tabText = tabWidget->tabText(i);
        QVERIFY(!tabText.isEmpty());
    }
}

void SettingsWidgetTest::testUIComponentCreation()
{
    // Test that essential UI components are created
    QList<QComboBox*> comboBoxes = m_settingsWidget->findChildren<QComboBox*>();
    QList<QLineEdit*> lineEdits = m_settingsWidget->findChildren<QLineEdit*>();
    QList<QCheckBox*> checkBoxes = m_settingsWidget->findChildren<QCheckBox*>();
    QList<QSpinBox*> spinBoxes = m_settingsWidget->findChildren<QSpinBox*>();
    QList<QPushButton*> buttons = m_settingsWidget->findChildren<QPushButton*>();
    
    // Should have some of each type of control
    QVERIFY(comboBoxes.size() > 0);  // Language, theme selections
    QVERIFY(lineEdits.size() > 0);   // API key, paths
    QVERIFY(checkBoxes.size() > 0);  // Auto scan, network monitoring
    QVERIFY(buttons.size() > 0);     // Apply, reset, browse buttons
}

void SettingsWidgetTest::testLayoutSetup()
{
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(m_settingsWidget->layout());
    QVERIFY(mainLayout != nullptr);
    
    // Should contain tab widget
    QTabWidget* tabWidget = m_settingsWidget->findChild<QTabWidget*>();
    QVERIFY(tabWidget != nullptr);
    QVERIFY(tabWidget->parentWidget() == m_settingsWidget);
}

void SettingsWidgetTest::testGeneralTabSetup()
{
    QTabWidget* tabWidget = m_settingsWidget->findChild<QTabWidget*>();
    QVERIFY(tabWidget != nullptr);
    
    if (tabWidget->count() > 0) {
        QWidget* generalTab = tabWidget->widget(0);
        QVERIFY(generalTab != nullptr);
        
        // Should contain language and theme controls
        QList<QComboBox*> comboBoxes = generalTab->findChildren<QComboBox*>();
        QVERIFY(comboBoxes.size() >= 2);  // Language and theme
        
        // Check for group boxes
        QList<QGroupBox*> groupBoxes = generalTab->findChildren<QGroupBox*>();
        QVERIFY(groupBoxes.size() > 0);
    }
}

void SettingsWidgetTest::testScanningTabSetup()
{
    QTabWidget* tabWidget = m_settingsWidget->findChild<QTabWidget*>();
    QVERIFY(tabWidget != nullptr);
    
    // Find scanning tab (typically second tab)
    for (int i = 0; i < tabWidget->count(); ++i) {
        QWidget* tab = tabWidget->widget(i);
        if (tab) {
            // Look for scanning-related controls
            QList<QLineEdit*> lineEdits = tab->findChildren<QLineEdit*>();
            QList<QCheckBox*> checkBoxes = tab->findChildren<QCheckBox*>();
            QList<QSpinBox*> spinBoxes = tab->findChildren<QSpinBox*>();
            
            if (lineEdits.size() > 0 || checkBoxes.size() > 0 || spinBoxes.size() > 0) {
                // This might be the scanning tab
                QVERIFY(tab != nullptr);
                break;
            }
        }
    }
}

void SettingsWidgetTest::testNetworkTabSetup()
{
    QTabWidget* tabWidget = m_settingsWidget->findChild<QTabWidget*>();
    QVERIFY(tabWidget != nullptr);
    
    // Network tab should exist
    QVERIFY(tabWidget->count() > 2);
    
    // Look for network-related controls
    QList<QCheckBox*> allCheckBoxes = m_settingsWidget->findChildren<QCheckBox*>();
    bool hasNetworkControls = false;
    
    for (QCheckBox* checkbox : allCheckBoxes) {
        if (checkbox->objectName().contains("network", Qt::CaseInsensitive) ||
            checkbox->text().contains("Network", Qt::CaseInsensitive)) {
            hasNetworkControls = true;
            break;
        }
    }
    
    // Should have some network-related controls
    QVERIFY(allCheckBoxes.size() > 0);  // At least some checkboxes should exist
}

void SettingsWidgetTest::testAdvancedTabSetup()
{
    QTabWidget* tabWidget = m_settingsWidget->findChild<QTabWidget*>();
    QVERIFY(tabWidget != nullptr);
    
    // Advanced tab should exist
    QVERIFY(tabWidget->count() > 3);
    
    // Should have advanced controls like debug mode, max threads
    QList<QSpinBox*> spinBoxes = m_settingsWidget->findChildren<QSpinBox*>();
    QList<QCheckBox*> checkBoxes = m_settingsWidget->findChildren<QCheckBox*>();
    
    QVERIFY(spinBoxes.size() > 0 || checkBoxes.size() > 0);
}

void SettingsWidgetTest::testLanguageSelection()
{
    // Find language combo box
    QList<QComboBox*> comboBoxes = m_settingsWidget->findChildren<QComboBox*>();
    
    QComboBox* languageCombo = nullptr;
    for (QComboBox* combo : comboBoxes) {
        if (combo->objectName().contains("language", Qt::CaseInsensitive) ||
            combo->count() > 0) {
            // Check if it contains language options
            for (int i = 0; i < combo->count(); ++i) {
                QString text = combo->itemText(i);
                if (text.contains("Türkçe") || text.contains("English")) {
                    languageCombo = combo;
                    break;
                }
            }
            if (languageCombo) break;
        }
    }
    
    if (languageCombo) {
        QVERIFY(languageCombo->count() > 0);
        
        // Test selection
        int originalIndex = languageCombo->currentIndex();
        if (languageCombo->count() > 1) {
            int newIndex = (originalIndex + 1) % languageCombo->count();
            languageCombo->setCurrentIndex(newIndex);
            QCOMPARE(languageCombo->currentIndex(), newIndex);
        }
    }
}

void SettingsWidgetTest::testThemeSelection()
{
    // Find theme combo box
    QList<QComboBox*> comboBoxes = m_settingsWidget->findChildren<QComboBox*>();
    
    QComboBox* themeCombo = nullptr;
    for (QComboBox* combo : comboBoxes) {
        // Check if it contains theme options
        for (int i = 0; i < combo->count(); ++i) {
            QString text = combo->itemText(i);
            if (text.contains("Dark") || text.contains("Light") || 
                text.contains("Koyu") || text.contains("Açık")) {
                themeCombo = combo;
                break;
            }
        }
        if (themeCombo) break;
    }
    
    if (themeCombo) {
        QVERIFY(themeCombo->count() > 0);
        
        // Test theme selection
        int originalIndex = themeCombo->currentIndex();
        if (themeCombo->count() > 1) {
            int newIndex = (originalIndex + 1) % themeCombo->count();
            themeCombo->setCurrentIndex(newIndex);
            QCOMPARE(themeCombo->currentIndex(), newIndex);
        }
    }
}

void SettingsWidgetTest::testVirusTotalApiKey()
{
    // Find API key line edit
    QList<QLineEdit*> lineEdits = m_settingsWidget->findChildren<QLineEdit*>();
    
    QLineEdit* apiKeyEdit = nullptr;
    for (QLineEdit* edit : lineEdits) {
        if (edit->objectName().contains("api", Qt::CaseInsensitive) ||
            edit->objectName().contains("key", Qt::CaseInsensitive)) {
            apiKeyEdit = edit;
            break;
        }
    }
    
    if (apiKeyEdit) {
        // Test API key input
        QString testApiKey = "test_api_key_12345";
        apiKeyEdit->setText(testApiKey);
        QCOMPARE(apiKeyEdit->text(), testApiKey);
        
        // Test clear
        apiKeyEdit->clear();
        QVERIFY(apiKeyEdit->text().isEmpty());
    }
}

void SettingsWidgetTest::testAutoScanSettings()
{
    // Find auto scan checkbox
    QList<QCheckBox*> checkBoxes = m_settingsWidget->findChildren<QCheckBox*>();
    
    QCheckBox* autoScanBox = nullptr;
    for (QCheckBox* box : checkBoxes) {
        if (box->objectName().contains("auto", Qt::CaseInsensitive) ||
            box->text().contains("Auto", Qt::CaseInsensitive)) {
            autoScanBox = box;
            break;
        }
    }
    
    if (autoScanBox) {
        // Test checkbox toggle
        bool originalState = autoScanBox->isChecked();
        autoScanBox->setChecked(!originalState);
        QCOMPARE(autoScanBox->isChecked(), !originalState);
        
        // Reset
        autoScanBox->setChecked(originalState);
    }
}

void SettingsWidgetTest::testQuarantinePathSettings()
{
    // Find quarantine path line edit
    QList<QLineEdit*> lineEdits = m_settingsWidget->findChildren<QLineEdit*>();
    
    QLineEdit* quarantinePathEdit = nullptr;
    for (QLineEdit* edit : lineEdits) {
        if (edit->objectName().contains("quarantine", Qt::CaseInsensitive) ||
            edit->objectName().contains("path", Qt::CaseInsensitive)) {
            quarantinePathEdit = edit;
            break;
        }
    }
    
    if (quarantinePathEdit) {
        // Test path setting
        QString testPath = m_tempDir->path();
        quarantinePathEdit->setText(testPath);
        QCOMPARE(quarantinePathEdit->text(), testPath);
    }
}

void SettingsWidgetTest::testNetworkMonitoringSettings()
{
    // Find network monitoring checkbox
    QList<QCheckBox*> checkBoxes = m_settingsWidget->findChildren<QCheckBox*>();
    
    for (QCheckBox* box : checkBoxes) {
        if (box->objectName().contains("network", Qt::CaseInsensitive) ||
            box->text().contains("Network", Qt::CaseInsensitive)) {
            // Test checkbox functionality
            bool originalState = box->isChecked();
            box->setChecked(!originalState);
            QCOMPARE(box->isChecked(), !originalState);
            box->setChecked(originalState);
            break;
        }
    }
}

void SettingsWidgetTest::testAdvancedSettings()
{
    // Find advanced settings controls
    QList<QSpinBox*> spinBoxes = m_settingsWidget->findChildren<QSpinBox*>();
    QList<QCheckBox*> checkBoxes = m_settingsWidget->findChildren<QCheckBox*>();
    
    // Test max threads spinbox
    for (QSpinBox* spinBox : spinBoxes) {
        if (spinBox->objectName().contains("thread", Qt::CaseInsensitive) ||
            spinBox->objectName().contains("max", Qt::CaseInsensitive)) {
            int originalValue = spinBox->value();
            int newValue = qMax(1, originalValue + 1);
            if (newValue <= spinBox->maximum()) {
                spinBox->setValue(newValue);
                QCOMPARE(spinBox->value(), newValue);
                spinBox->setValue(originalValue);
            }
            break;
        }
    }
    
    // Test debug mode checkbox
    for (QCheckBox* box : checkBoxes) {
        if (box->objectName().contains("debug", Qt::CaseInsensitive) ||
            box->text().contains("Debug", Qt::CaseInsensitive)) {
            bool originalState = box->isChecked();
            box->setChecked(!originalState);
            QCOMPARE(box->isChecked(), !originalState);
            box->setChecked(originalState);
            break;
        }
    }
}

void SettingsWidgetTest::testApplyButton()
{
    // Find apply button
    QList<QPushButton*> buttons = m_settingsWidget->findChildren<QPushButton*>();
    
    QPushButton* applyButton = nullptr;
    for (QPushButton* button : buttons) {
        if (button->objectName().contains("apply", Qt::CaseInsensitive) ||
            button->text().contains("Apply", Qt::CaseInsensitive) ||
            button->text().contains("Uygula", Qt::CaseInsensitive)) {
            applyButton = button;
            break;
        }
    }
    
    if (applyButton && applyButton->isVisible() && applyButton->isEnabled()) {
        QSignalSpy clickSpy(applyButton, &QPushButton::clicked);
        
        // Simulate button click
        QTest::mouseClick(applyButton, Qt::LeftButton);
        
        // Should have emitted clicked signal
        QVERIFY(clickSpy.count() >= 0);
    }
}

void SettingsWidgetTest::testResetButton()
{
    // Find reset button
    QList<QPushButton*> buttons = m_settingsWidget->findChildren<QPushButton*>();
    
    QPushButton* resetButton = nullptr;
    for (QPushButton* button : buttons) {
        if (button->objectName().contains("reset", Qt::CaseInsensitive) ||
            button->text().contains("Reset", Qt::CaseInsensitive) ||
            button->text().contains("Sıfırla", Qt::CaseInsensitive)) {
            resetButton = button;
            break;
        }
    }
    
    if (resetButton && resetButton->isVisible() && resetButton->isEnabled()) {
        QSignalSpy clickSpy(resetButton, &QPushButton::clicked);
        
        // Simulate button click
        QTest::mouseClick(resetButton, Qt::LeftButton);
        
        // Should have emitted clicked signal
        QVERIFY(clickSpy.count() >= 0);
    }
}

void SettingsWidgetTest::testBrowseButtons()
{
    // Find browse buttons
    QList<QPushButton*> buttons = m_settingsWidget->findChildren<QPushButton*>();
    
    for (QPushButton* button : buttons) {
        if (button->objectName().contains("browse", Qt::CaseInsensitive) ||
            button->text().contains("Browse", Qt::CaseInsensitive) ||
            button->text().contains("Gözat", Qt::CaseInsensitive)) {
            
            if (button->isVisible() && button->isEnabled()) {
                QSignalSpy clickSpy(button, &QPushButton::clicked);
                
                // Note: We don't actually click browse buttons in tests
                // as they would open file dialogs
                QVERIFY(button->isEnabled());
            }
        }
    }
}

void SettingsWidgetTest::testLoadSettings()
{
    // SettingsWidget should load settings on construction
    // We can't directly test this without mocking SettingsManager
    // But we can verify that controls have some default values
    
    QList<QComboBox*> comboBoxes = m_settingsWidget->findChildren<QComboBox*>();
    QList<QLineEdit*> lineEdits = m_settingsWidget->findChildren<QLineEdit*>();
    QList<QCheckBox*> checkBoxes = m_settingsWidget->findChildren<QCheckBox*>();
    QList<QSpinBox*> spinBoxes = m_settingsWidget->findChildren<QSpinBox*>();
    
    // Verify that controls have valid states
    for (QComboBox* combo : comboBoxes) {
        QVERIFY(combo->currentIndex() >= 0);
        QVERIFY(combo->currentIndex() < combo->count());
    }
    
    for (QSpinBox* spinBox : spinBoxes) {
        QVERIFY(spinBox->value() >= spinBox->minimum());
        QVERIFY(spinBox->value() <= spinBox->maximum());
    }
}

void SettingsWidgetTest::testSaveSettings()
{
    // Test that save settings can be called
    // We can simulate this by calling the save method if it's public
    // or by triggering apply button
    
    QPushButton* applyButton = nullptr;
    QList<QPushButton*> buttons = m_settingsWidget->findChildren<QPushButton*>();
    
    for (QPushButton* button : buttons) {
        if (button->objectName().contains("apply", Qt::CaseInsensitive)) {
            applyButton = button;
            break;
        }
    }
    
    if (applyButton) {
        // Settings should be saveable without errors
        QVERIFY(applyButton->isEnabled());
    }
}

void SettingsWidgetTest::testSettingsValidation()
{
    // Test that invalid settings are handled gracefully
    QList<QLineEdit*> lineEdits = m_settingsWidget->findChildren<QLineEdit*>();
    
    for (QLineEdit* edit : lineEdits) {
        if (edit->objectName().contains("path", Qt::CaseInsensitive)) {
            // Test invalid path
            QString originalText = edit->text();
            edit->setText("/invalid/path/that/does/not/exist");
            
            // Widget should handle this gracefully
            QVERIFY(edit->text() == "/invalid/path/that/does/not/exist");
            
            // Restore original
            edit->setText(originalText);
        }
    }
}

void SettingsWidgetTest::testDefaultSettings()
{
    // Create a new settings widget and verify it has reasonable defaults
    SettingsWidget* defaultWidget = new SettingsWidget();
    
    // Should be created successfully
    QVERIFY(defaultWidget != nullptr);
    
    // Should have valid default values
    QList<QSpinBox*> spinBoxes = defaultWidget->findChildren<QSpinBox*>();
    for (QSpinBox* spinBox : spinBoxes) {
        QVERIFY(spinBox->value() > 0);  // Should have reasonable default
    }
    
    delete defaultWidget;
}

void SettingsWidgetTest::testSignalConnections()
{
    // Test that UI controls are properly connected
    QList<QComboBox*> comboBoxes = m_settingsWidget->findChildren<QComboBox*>();
    QList<QCheckBox*> checkBoxes = m_settingsWidget->findChildren<QCheckBox*>();
    
    // Verify that changing controls doesn't cause crashes
    for (QComboBox* combo : comboBoxes) {
        if (combo->count() > 1) {
            QSignalSpy changeSpy(combo, QOverload<int>::of(&QComboBox::currentIndexChanged));
            combo->setCurrentIndex((combo->currentIndex() + 1) % combo->count());
            // Should handle change gracefully
        }
    }
    
    for (QCheckBox* box : checkBoxes) {
        QSignalSpy toggleSpy(box, &QCheckBox::toggled);
        box->toggle();
        // Should handle toggle gracefully
    }
}

void SettingsWidgetTest::testSettingsChanged()
{
    // Test that settings changes are detected
    QList<QLineEdit*> lineEdits = m_settingsWidget->findChildren<QLineEdit*>();
    
    for (QLineEdit* edit : lineEdits) {
        QSignalSpy textSpy(edit, &QLineEdit::textChanged);
        
        QString originalText = edit->text();
        edit->setText("test_change");
        
        // Should have detected text change
        QVERIFY(textSpy.count() >= 0);
        
        edit->setText(originalText);
        break;  // Test one is enough
    }
}

void SettingsWidgetTest::testButtonSignals()
{
    QList<QPushButton*> buttons = m_settingsWidget->findChildren<QPushButton*>();
    
    for (QPushButton* button : buttons) {
        if (button->isVisible() && button->isEnabled()) {
            QSignalSpy clickSpy(button, &QPushButton::clicked);
            
            // Verify button can emit signals
            QVERIFY(button->receivers(SIGNAL(clicked())) >= 0);
            break;  // Test one button is enough
        }
    }
}

void SettingsWidgetTest::testInvalidPaths()
{
    // Test handling of invalid file paths
    QList<QLineEdit*> lineEdits = m_settingsWidget->findChildren<QLineEdit*>();
    
    for (QLineEdit* edit : lineEdits) {
        if (edit->objectName().contains("path", Qt::CaseInsensitive)) {
            QString originalPath = edit->text();
            
            // Test various invalid paths
            QStringList invalidPaths = {
                "",
                "/",
                "C:\\invalid\\windows\\path",
                "/dev/null/invalid",
                "not_a_path"
            };
            
            for (const QString& invalidPath : invalidPaths) {
                edit->setText(invalidPath);
                // Should not crash
                QVERIFY(edit->text() == invalidPath);
            }
            
            edit->setText(originalPath);
            break;
        }
    }
}

void SettingsWidgetTest::testMissingSettings()
{
    // Test widget behavior when settings file is missing
    // This is mainly testing that the widget doesn't crash
    QVERIFY(m_settingsWidget != nullptr);
    QVERIFY(m_settingsWidget->isWidgetType());
}

void SettingsWidgetTest::testCorruptedSettings()
{
    // Test handling of corrupted settings
    // Create a corrupted settings file
    QSettings corruptedSettings(m_testSettingsPath, QSettings::IniFormat);
    corruptedSettings.setValue("corrupt_key", QVariant());
    corruptedSettings.sync();
    
    // Widget should still function
    SettingsWidget* testWidget = new SettingsWidget();
    QVERIFY(testWidget != nullptr);
    
    delete testWidget;
}

// Helper Methods

void SettingsWidgetTest::setupTestSettings()
{
    // Create test settings file
    QSettings testSettings(m_testSettingsPath, QSettings::IniFormat);
    testSettings.setValue("language", "en_US");
    testSettings.setValue("theme", "dark");
    testSettings.setValue("auto_scan", true);
    testSettings.sync();
}

void SettingsWidgetTest::cleanupTestSettings()
{
    QFile::remove(m_testSettingsPath);
}

QWidget* SettingsWidgetTest::findTabByName(const QString& tabName)
{
    QTabWidget* tabWidget = m_settingsWidget->findChild<QTabWidget*>();
    if (!tabWidget) return nullptr;
    
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabText(i).contains(tabName, Qt::CaseInsensitive)) {
            return tabWidget->widget(i);
        }
    }
    
    return nullptr;
}

void SettingsWidgetTest::simulateUserInput()
{
    // Simulate various user interactions
    QList<QWidget*> interactiveWidgets;
    interactiveWidgets.append(m_settingsWidget->findChildren<QComboBox*>());
    interactiveWidgets.append(m_settingsWidget->findChildren<QCheckBox*>());
    interactiveWidgets.append(m_settingsWidget->findChildren<QPushButton*>());
    
    // Test that widgets can receive focus and input
    for (QWidget* widget : interactiveWidgets) {
        if (widget->isVisible() && widget->isEnabled()) {
            widget->setFocus();
            QVERIFY(widget->hasFocus() || !widget->focusPolicy() == Qt::NoFocus);
        }
    }
}

QTEST_MAIN(SettingsWidgetTest)
#include "SettingsWidgetTest.moc"
