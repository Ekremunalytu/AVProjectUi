/********************************************************************************
** Form generated from reading UI file 'dashboardwidget.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DASHBOARDWIDGET_H
#define UI_DASHBOARDWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DashboardWidget
{
public:
    QVBoxLayout *mainVerticalLayout;
    QTabWidget *dashboardTabWidget;
    QWidget *basicScanTab;
    QVBoxLayout *basicScanTabLayout;
    QFrame *basicScanControlsFrame;
    QHBoxLayout *basicScanControlsLayout;
    QPushButton *selectFileButton_dashboard;
    QPushButton *basicScanButton_dashboard;
    QPushButton *scanDirectoryButton_dashboard;
    QSpacerItem *basicScanControlsSpacer;
    QLabel *directoryScanStatusLabel_dashboard;
    QProgressBar *directoryScanProgressBar_dashboard;
    QFrame *basicScanResultsFrame;
    QVBoxLayout *basicScanResultsFrameLayout;
    QTextEdit *basicScanResultsTextEdit;
    QWidget *advancedScanTab;
    QVBoxLayout *advancedScanTabLayout;
    QFrame *advancedScanResultsFrame;
    QVBoxLayout *advancedScanResultsFrameLayout;
    QTableWidget *advancedScanResultsTableWidget;
    QPushButton *advancedScanButton_dashboard;
    QWidget *cdrTab;
    QVBoxLayout *cdrTabLayout;
    QFrame *cdrResultsFrame;
    QVBoxLayout *cdrResultsFrameLayout;
    QTextEdit *cdrResultsTextEdit;
    QPushButton *cdrScanButton_dashboard;
    QWidget *sandboxTab;
    QVBoxLayout *sandboxTabLayout;
    QFrame *sandboxResultsFrame;
    QVBoxLayout *sandboxResultsFrameLayout;
    QTextEdit *sandboxResultsTextEdit;
    QPushButton *sandboxScanButton_dashboard;
    QWidget *networkTab;
    QVBoxLayout *networkTabLayout;
    QFrame *networkResultsFrame;
    QVBoxLayout *networkResultsFrameLayout;
    QTextEdit *networkCommunicationTextEdit;
    QPushButton *networkMonitorButton;
    QHBoxLayout *buttonsLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *configButton;
    QPushButton *refreshButton;

    void setupUi(QWidget *DashboardWidget)
    {
        if (DashboardWidget->objectName().isEmpty())
            DashboardWidget->setObjectName("DashboardWidget");
        DashboardWidget->resize(750, 550);
        mainVerticalLayout = new QVBoxLayout(DashboardWidget);
        mainVerticalLayout->setObjectName("mainVerticalLayout");
        dashboardTabWidget = new QTabWidget(DashboardWidget);
        dashboardTabWidget->setObjectName("dashboardTabWidget");
        dashboardTabWidget->setTabShape(QTabWidget::Rounded);
        dashboardTabWidget->setDocumentMode(true);
        dashboardTabWidget->setTabsClosable(false);
        dashboardTabWidget->setMovable(false);
        basicScanTab = new QWidget();
        basicScanTab->setObjectName("basicScanTab");
        basicScanTabLayout = new QVBoxLayout(basicScanTab);
        basicScanTabLayout->setObjectName("basicScanTabLayout");
        basicScanControlsFrame = new QFrame(basicScanTab);
        basicScanControlsFrame->setObjectName("basicScanControlsFrame");
        basicScanControlsFrame->setFrameShape(QFrame::StyledPanel);
        basicScanControlsFrame->setFrameShadow(QFrame::Raised);
        basicScanControlsLayout = new QHBoxLayout(basicScanControlsFrame);
        basicScanControlsLayout->setObjectName("basicScanControlsLayout");
        selectFileButton_dashboard = new QPushButton(basicScanControlsFrame);
        selectFileButton_dashboard->setObjectName("selectFileButton_dashboard");
        selectFileButton_dashboard->setMinimumSize(QSize(150, 40));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/openFile.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        selectFileButton_dashboard->setIcon(icon);

        basicScanControlsLayout->addWidget(selectFileButton_dashboard);

        basicScanButton_dashboard = new QPushButton(basicScanControlsFrame);
        basicScanButton_dashboard->setObjectName("basicScanButton_dashboard");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(basicScanButton_dashboard->sizePolicy().hasHeightForWidth());
        basicScanButton_dashboard->setSizePolicy(sizePolicy);
        basicScanButton_dashboard->setMinimumSize(QSize(150, 40));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/basicScan.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        basicScanButton_dashboard->setIcon(icon1);

        basicScanControlsLayout->addWidget(basicScanButton_dashboard);

        scanDirectoryButton_dashboard = new QPushButton(basicScanControlsFrame);
        scanDirectoryButton_dashboard->setObjectName("scanDirectoryButton_dashboard");
        scanDirectoryButton_dashboard->setMinimumSize(QSize(150, 40));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/scanFolder.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        scanDirectoryButton_dashboard->setIcon(icon2);

        basicScanControlsLayout->addWidget(scanDirectoryButton_dashboard);

        basicScanControlsSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        basicScanControlsLayout->addItem(basicScanControlsSpacer);


        basicScanTabLayout->addWidget(basicScanControlsFrame);

        directoryScanStatusLabel_dashboard = new QLabel(basicScanTab);
        directoryScanStatusLabel_dashboard->setObjectName("directoryScanStatusLabel_dashboard");
        directoryScanStatusLabel_dashboard->setAlignment(Qt::AlignCenter);

        basicScanTabLayout->addWidget(directoryScanStatusLabel_dashboard);

        directoryScanProgressBar_dashboard = new QProgressBar(basicScanTab);
        directoryScanProgressBar_dashboard->setObjectName("directoryScanProgressBar_dashboard");
        directoryScanProgressBar_dashboard->setValue(0);
        directoryScanProgressBar_dashboard->setTextVisible(true);

        basicScanTabLayout->addWidget(directoryScanProgressBar_dashboard);

        basicScanResultsFrame = new QFrame(basicScanTab);
        basicScanResultsFrame->setObjectName("basicScanResultsFrame");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(1);
        sizePolicy1.setHeightForWidth(basicScanResultsFrame->sizePolicy().hasHeightForWidth());
        basicScanResultsFrame->setSizePolicy(sizePolicy1);
        basicScanResultsFrame->setFrameShape(QFrame::StyledPanel);
        basicScanResultsFrame->setFrameShadow(QFrame::Sunken);
        basicScanResultsFrameLayout = new QVBoxLayout(basicScanResultsFrame);
        basicScanResultsFrameLayout->setObjectName("basicScanResultsFrameLayout");
        basicScanResultsTextEdit = new QTextEdit(basicScanResultsFrame);
        basicScanResultsTextEdit->setObjectName("basicScanResultsTextEdit");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(basicScanResultsTextEdit->sizePolicy().hasHeightForWidth());
        basicScanResultsTextEdit->setSizePolicy(sizePolicy2);
        basicScanResultsTextEdit->setReadOnly(true);

        basicScanResultsFrameLayout->addWidget(basicScanResultsTextEdit);


        basicScanTabLayout->addWidget(basicScanResultsFrame);

        dashboardTabWidget->addTab(basicScanTab, QString());
        advancedScanTab = new QWidget();
        advancedScanTab->setObjectName("advancedScanTab");
        advancedScanTabLayout = new QVBoxLayout(advancedScanTab);
        advancedScanTabLayout->setObjectName("advancedScanTabLayout");
        advancedScanResultsFrame = new QFrame(advancedScanTab);
        advancedScanResultsFrame->setObjectName("advancedScanResultsFrame");
        sizePolicy1.setHeightForWidth(advancedScanResultsFrame->sizePolicy().hasHeightForWidth());
        advancedScanResultsFrame->setSizePolicy(sizePolicy1);
        advancedScanResultsFrame->setFrameShape(QFrame::StyledPanel);
        advancedScanResultsFrame->setFrameShadow(QFrame::Sunken);
        advancedScanResultsFrameLayout = new QVBoxLayout(advancedScanResultsFrame);
        advancedScanResultsFrameLayout->setObjectName("advancedScanResultsFrameLayout");
        advancedScanResultsTableWidget = new QTableWidget(advancedScanResultsFrame);
        advancedScanResultsTableWidget->setObjectName("advancedScanResultsTableWidget");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(advancedScanResultsTableWidget->sizePolicy().hasHeightForWidth());
        advancedScanResultsTableWidget->setSizePolicy(sizePolicy3);
        advancedScanResultsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        advancedScanResultsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        advancedScanResultsTableWidget->setAlternatingRowColors(true);
        advancedScanResultsTableWidget->horizontalHeader()->setStretchLastSection(true);
        advancedScanResultsTableWidget->verticalHeader()->setVisible(false);

        advancedScanResultsFrameLayout->addWidget(advancedScanResultsTableWidget);


        advancedScanTabLayout->addWidget(advancedScanResultsFrame);

        advancedScanButton_dashboard = new QPushButton(advancedScanTab);
        advancedScanButton_dashboard->setObjectName("advancedScanButton_dashboard");
        sizePolicy.setHeightForWidth(advancedScanButton_dashboard->sizePolicy().hasHeightForWidth());
        advancedScanButton_dashboard->setSizePolicy(sizePolicy);
        advancedScanButton_dashboard->setMinimumSize(QSize(150, 40));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/advancedScan.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        advancedScanButton_dashboard->setIcon(icon3);

        advancedScanTabLayout->addWidget(advancedScanButton_dashboard);

        dashboardTabWidget->addTab(advancedScanTab, QString());
        cdrTab = new QWidget();
        cdrTab->setObjectName("cdrTab");
        cdrTabLayout = new QVBoxLayout(cdrTab);
        cdrTabLayout->setObjectName("cdrTabLayout");
        cdrResultsFrame = new QFrame(cdrTab);
        cdrResultsFrame->setObjectName("cdrResultsFrame");
        sizePolicy1.setHeightForWidth(cdrResultsFrame->sizePolicy().hasHeightForWidth());
        cdrResultsFrame->setSizePolicy(sizePolicy1);
        cdrResultsFrame->setFrameShape(QFrame::StyledPanel);
        cdrResultsFrame->setFrameShadow(QFrame::Sunken);
        cdrResultsFrameLayout = new QVBoxLayout(cdrResultsFrame);
        cdrResultsFrameLayout->setObjectName("cdrResultsFrameLayout");
        cdrResultsTextEdit = new QTextEdit(cdrResultsFrame);
        cdrResultsTextEdit->setObjectName("cdrResultsTextEdit");
        sizePolicy3.setHeightForWidth(cdrResultsTextEdit->sizePolicy().hasHeightForWidth());
        cdrResultsTextEdit->setSizePolicy(sizePolicy3);
        cdrResultsTextEdit->setReadOnly(true);

        cdrResultsFrameLayout->addWidget(cdrResultsTextEdit);


        cdrTabLayout->addWidget(cdrResultsFrame);

        cdrScanButton_dashboard = new QPushButton(cdrTab);
        cdrScanButton_dashboard->setObjectName("cdrScanButton_dashboard");
        sizePolicy.setHeightForWidth(cdrScanButton_dashboard->sizePolicy().hasHeightForWidth());
        cdrScanButton_dashboard->setSizePolicy(sizePolicy);
        cdrScanButton_dashboard->setMinimumSize(QSize(150, 40));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/images/cdrScan.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        cdrScanButton_dashboard->setIcon(icon4);

        cdrTabLayout->addWidget(cdrScanButton_dashboard);

        dashboardTabWidget->addTab(cdrTab, QString());
        sandboxTab = new QWidget();
        sandboxTab->setObjectName("sandboxTab");
        sandboxTabLayout = new QVBoxLayout(sandboxTab);
        sandboxTabLayout->setObjectName("sandboxTabLayout");
        sandboxResultsFrame = new QFrame(sandboxTab);
        sandboxResultsFrame->setObjectName("sandboxResultsFrame");
        sizePolicy1.setHeightForWidth(sandboxResultsFrame->sizePolicy().hasHeightForWidth());
        sandboxResultsFrame->setSizePolicy(sizePolicy1);
        sandboxResultsFrame->setFrameShape(QFrame::StyledPanel);
        sandboxResultsFrame->setFrameShadow(QFrame::Sunken);
        sandboxResultsFrameLayout = new QVBoxLayout(sandboxResultsFrame);
        sandboxResultsFrameLayout->setObjectName("sandboxResultsFrameLayout");
        sandboxResultsTextEdit = new QTextEdit(sandboxResultsFrame);
        sandboxResultsTextEdit->setObjectName("sandboxResultsTextEdit");
        sizePolicy3.setHeightForWidth(sandboxResultsTextEdit->sizePolicy().hasHeightForWidth());
        sandboxResultsTextEdit->setSizePolicy(sizePolicy3);
        sandboxResultsTextEdit->setReadOnly(true);

        sandboxResultsFrameLayout->addWidget(sandboxResultsTextEdit);


        sandboxTabLayout->addWidget(sandboxResultsFrame);

        sandboxScanButton_dashboard = new QPushButton(sandboxTab);
        sandboxScanButton_dashboard->setObjectName("sandboxScanButton_dashboard");
        sizePolicy.setHeightForWidth(sandboxScanButton_dashboard->sizePolicy().hasHeightForWidth());
        sandboxScanButton_dashboard->setSizePolicy(sizePolicy);
        sandboxScanButton_dashboard->setMinimumSize(QSize(150, 40));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/images/sandboxScan.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        sandboxScanButton_dashboard->setIcon(icon5);

        sandboxTabLayout->addWidget(sandboxScanButton_dashboard);

        dashboardTabWidget->addTab(sandboxTab, QString());
        networkTab = new QWidget();
        networkTab->setObjectName("networkTab");
        networkTabLayout = new QVBoxLayout(networkTab);
        networkTabLayout->setObjectName("networkTabLayout");
        networkResultsFrame = new QFrame(networkTab);
        networkResultsFrame->setObjectName("networkResultsFrame");
        sizePolicy1.setHeightForWidth(networkResultsFrame->sizePolicy().hasHeightForWidth());
        networkResultsFrame->setSizePolicy(sizePolicy1);
        networkResultsFrame->setFrameShape(QFrame::StyledPanel);
        networkResultsFrame->setFrameShadow(QFrame::Sunken);
        networkResultsFrameLayout = new QVBoxLayout(networkResultsFrame);
        networkResultsFrameLayout->setObjectName("networkResultsFrameLayout");
        networkCommunicationTextEdit = new QTextEdit(networkResultsFrame);
        networkCommunicationTextEdit->setObjectName("networkCommunicationTextEdit");
        sizePolicy3.setHeightForWidth(networkCommunicationTextEdit->sizePolicy().hasHeightForWidth());
        networkCommunicationTextEdit->setSizePolicy(sizePolicy3);
        networkCommunicationTextEdit->setReadOnly(true);

        networkResultsFrameLayout->addWidget(networkCommunicationTextEdit);


        networkTabLayout->addWidget(networkResultsFrame);

        networkMonitorButton = new QPushButton(networkTab);
        networkMonitorButton->setObjectName("networkMonitorButton");
        sizePolicy.setHeightForWidth(networkMonitorButton->sizePolicy().hasHeightForWidth());
        networkMonitorButton->setSizePolicy(sizePolicy);
        networkMonitorButton->setMinimumSize(QSize(150, 40));

        networkTabLayout->addWidget(networkMonitorButton);

        dashboardTabWidget->addTab(networkTab, QString());

        mainVerticalLayout->addWidget(dashboardTabWidget);

        buttonsLayout = new QHBoxLayout();
        buttonsLayout->setObjectName("buttonsLayout");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        buttonsLayout->addItem(horizontalSpacer);

        configButton = new QPushButton(DashboardWidget);
        configButton->setObjectName("configButton");
        sizePolicy.setHeightForWidth(configButton->sizePolicy().hasHeightForWidth());
        configButton->setSizePolicy(sizePolicy);
        configButton->setMinimumSize(QSize(120, 40));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/images/settings.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        configButton->setIcon(icon6);

        buttonsLayout->addWidget(configButton);

        refreshButton = new QPushButton(DashboardWidget);
        refreshButton->setObjectName("refreshButton");
        sizePolicy.setHeightForWidth(refreshButton->sizePolicy().hasHeightForWidth());
        refreshButton->setSizePolicy(sizePolicy);
        refreshButton->setMinimumSize(QSize(120, 40));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/images/refresh.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        refreshButton->setIcon(icon7);

        buttonsLayout->addWidget(refreshButton);


        mainVerticalLayout->addLayout(buttonsLayout);


        retranslateUi(DashboardWidget);

        dashboardTabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(DashboardWidget);
    } // setupUi

    void retranslateUi(QWidget *DashboardWidget)
    {
        DashboardWidget->setWindowTitle(QCoreApplication::translate("DashboardWidget", "Form", nullptr));
        dashboardTabWidget->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QTabWidget::pane { \n"
"  border: 1px solid #4A4A4A;\n"
"  border-radius: 4px;\n"
"  background-color: rgba(0, 0, 0, 0.02);\n"
"  margin-top: -1px;\n"
"}\n"
"QTabWidget::tab-bar {\n"
"  left: 5px;\n"
"}\n"
"QTabBar::tab {\n"
"  background-color: rgba(0, 0, 0, 0.05);\n"
"  border: 1px solid #3A3A3A;\n"
"  border-bottom: 0px;\n"
"  border-top-left-radius: 4px;\n"
"  border-top-right-radius: 4px;\n"
"  padding: 8px 16px;\n"
"  margin-right: 2px;\n"
"  color: #CCCCCC;\n"
"}\n"
"QTabBar::tab:selected {\n"
"  background-color: rgba(30, 30, 30, 0.1);\n"
"  color: #FFFFFF;\n"
"}\n"
"QTabBar::tab:hover:!selected {\n"
"  background-color: rgba(60, 60, 60, 0.2);\n"
"}\n"
"      ", nullptr));
        selectFileButton_dashboard->setText(QCoreApplication::translate("DashboardWidget", "Select File", nullptr));
        basicScanButton_dashboard->setText(QCoreApplication::translate("DashboardWidget", "Run Basic Scan", nullptr));
        scanDirectoryButton_dashboard->setText(QCoreApplication::translate("DashboardWidget", "Scan Directory", nullptr));
        directoryScanStatusLabel_dashboard->setText(QString());
        basicScanResultsFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid #4A4A4A;\n"
"    background-color: rgba(0, 0, 0, 0.05);\n"
"    border-radius: 4px;\n"
"}\n"
"          ", nullptr));
        basicScanResultsTextEdit->setPlaceholderText(QCoreApplication::translate("DashboardWidget", "Basic scan results will be shown here...", nullptr));
        dashboardTabWidget->setTabText(dashboardTabWidget->indexOf(basicScanTab), QCoreApplication::translate("DashboardWidget", "Basic Scan", nullptr));
        advancedScanResultsFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid #4A4A4A;\n"
"    background-color: rgba(0, 0, 0, 0.05);\n"
"    border-radius: 4px;\n"
"}\n"
"          ", nullptr));
        advancedScanButton_dashboard->setText(QCoreApplication::translate("DashboardWidget", "Run Advanced Scan", nullptr));
        dashboardTabWidget->setTabText(dashboardTabWidget->indexOf(advancedScanTab), QCoreApplication::translate("DashboardWidget", "Advanced Scan", nullptr));
        cdrResultsFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid #4A4A4A;\n"
"    background-color: rgba(0, 0, 0, 0.05);\n"
"    border-radius: 4px;\n"
"}\n"
"          ", nullptr));
        cdrResultsTextEdit->setPlaceholderText(QCoreApplication::translate("DashboardWidget", "CDR (Content Disarm & Reconstruction) results will be shown here...", nullptr));
        cdrScanButton_dashboard->setText(QCoreApplication::translate("DashboardWidget", "Run CDR Scan", nullptr));
        dashboardTabWidget->setTabText(dashboardTabWidget->indexOf(cdrTab), QCoreApplication::translate("DashboardWidget", "CDR", nullptr));
        sandboxResultsFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid #4A4A4A;\n"
"    background-color: rgba(0, 0, 0, 0.05);\n"
"    border-radius: 4px;\n"
"}\n"
"          ", nullptr));
        sandboxResultsTextEdit->setPlaceholderText(QCoreApplication::translate("DashboardWidget", "Sandbox analysis results will be shown here...", nullptr));
        sandboxScanButton_dashboard->setText(QCoreApplication::translate("DashboardWidget", "Run Sandbox Analysis", nullptr));
        dashboardTabWidget->setTabText(dashboardTabWidget->indexOf(sandboxTab), QCoreApplication::translate("DashboardWidget", "Sandbox", nullptr));
        networkResultsFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid #4A4A4A;\n"
"    background-color: rgba(0, 0, 0, 0.05);\n"
"    border-radius: 4px;\n"
"}\n"
"          ", nullptr));
        networkCommunicationTextEdit->setPlaceholderText(QCoreApplication::translate("DashboardWidget", "Network activity will be shown here...", nullptr));
        networkMonitorButton->setText(QCoreApplication::translate("DashboardWidget", "Start Network Monitor", nullptr));
        dashboardTabWidget->setTabText(dashboardTabWidget->indexOf(networkTab), QCoreApplication::translate("DashboardWidget", "Network", nullptr));
        configButton->setText(QCoreApplication::translate("DashboardWidget", "Configuration", nullptr));
        refreshButton->setText(QCoreApplication::translate("DashboardWidget", "Refresh", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DashboardWidget: public Ui_DashboardWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DASHBOARDWIDGET_H
