/**
 * @file SettingsWidget.h
 * @brief Settings widget for the main window settings page
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 */

#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
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

QT_BEGIN_NAMESPACE
class QTabWidget;
class QVBoxLayout;
class QHBoxLayout;
class QFormLayout;
class QGroupBox;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QComboBox;
class QPushButton;
class QLabel;
QT_END_NAMESPACE

/**
 * @brief Settings widget for configuring application preferences in the main window
 */
class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

public slots:
    void saveSettings();
    void loadSettings();

signals:
    void settingsChanged();

private slots:
    void onBrowseQuarantineClicked();
    void onBrowseDatabaseClicked();
    void onResetToDefaultsClicked();
    void onApplyClicked();

private:
    void setupUI();
    void setupGeneralTab();
    void setupScanningTab();
    void setupNetworkTab();
    void setupAdvancedTab();
    void setupButtonBox(QVBoxLayout *mainLayout);
    
    void resetToDefaults();

    // UI Components
    QTabWidget *m_tabWidget;
    
    // General tab
    QComboBox *m_languageCombo;
    QComboBox *m_themeCombo;
    
    // Scanning tab
    QLineEdit *m_virusTotalApiKeyEdit;
    QCheckBox *m_autoScanCheckBox;
    QSpinBox *m_scanTimeoutSpinBox;
    QLineEdit *m_quarantinePathEdit;
    QPushButton *m_browseQuarantineButton;
    
    // Network tab
    QCheckBox *m_networkMonitoringCheckBox;
    
    // Advanced tab
    QCheckBox *m_debugModeCheckBox;
    QSpinBox *m_maxThreadsSpinBox;
    QLineEdit *m_databasePathEdit;
    QPushButton *m_browseDatabaseButton;
    
    // Buttons
    QPushButton *m_applyButton;
    QPushButton *m_resetButton;
};

#endif // SETTINGSWIDGET_H