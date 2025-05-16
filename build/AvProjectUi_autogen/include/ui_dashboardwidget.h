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
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
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
    QFrame *basicScanToolbarFrame;
    QHBoxLayout *basicScanToolbarLayout;
    QLabel *basicScanLabel;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *basicScanButton;
    QFrame *basicScanResultsFrame;
    QVBoxLayout *basicScanResultsFrameLayout;
    QTextEdit *basicScanResultsTextEdit;
    QWidget *advancedScanTab;
    QVBoxLayout *advancedScanTabLayout;
    QFrame *advancedScanToolbarFrame;
    QHBoxLayout *advancedScanToolbarLayout;
    QLabel *advancedScanLabel;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *advancedScanButton;
    QFrame *advancedScanResultsFrame;
    QVBoxLayout *advancedScanResultsFrameLayout;
    QTableWidget *advancedScanResultsTableWidget;
    QWidget *cdrTab;
    QVBoxLayout *cdrTabLayout;
    QFrame *cdrToolbarFrame;
    QHBoxLayout *cdrToolbarLayout;
    QLabel *cdrScanLabel;
    QSpacerItem *horizontalSpacer_4;
    QComboBox *cdrFileTypeComboBox;
    QPushButton *cdrScanButton;
    QFrame *cdrResultsFrame;
    QVBoxLayout *cdrResultsFrameLayout;
    QTextEdit *cdrResultsTextEdit;
    QWidget *sandboxTab;
    QVBoxLayout *sandboxTabLayout;
    QFrame *sandboxToolbarFrame;
    QHBoxLayout *sandboxToolbarLayout;
    QLabel *sandboxLabel;
    QSpacerItem *horizontalSpacer_5;
    QComboBox *sandboxTimeoutComboBox;
    QPushButton *sandboxScanButton;
    QFrame *sandboxResultsFrame;
    QVBoxLayout *sandboxResultsFrameLayout;
    QTextEdit *sandboxResultsTextEdit;
    QWidget *networkTab;
    QVBoxLayout *networkTabLayout;
    QFrame *networkToolbarFrame;
    QHBoxLayout *networkToolbarLayout;
    QLabel *networkLabel;
    QSpacerItem *horizontalSpacer_6;
    QComboBox *networkFilterComboBox;
    QPushButton *networkMonitorButton;
    QFrame *networkResultsFrame;
    QVBoxLayout *networkResultsFrameLayout;
    QTextEdit *networkCommunicationTextEdit;
    QFrame *statusBarFrame;
    QHBoxLayout *statusBarLayout;
    QLabel *statusLabel;
    QLabel *lastScanLabel;
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
        basicScanToolbarFrame = new QFrame(basicScanTab);
        basicScanToolbarFrame->setObjectName("basicScanToolbarFrame");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(basicScanToolbarFrame->sizePolicy().hasHeightForWidth());
        basicScanToolbarFrame->setSizePolicy(sizePolicy);
        basicScanToolbarFrame->setMinimumSize(QSize(0, 70));
        basicScanToolbarFrame->setMaximumSize(QSize(16777215, 70));
        basicScanToolbarFrame->setFrameShape(QFrame::StyledPanel);
        basicScanToolbarFrame->setFrameShadow(QFrame::Raised);
        basicScanToolbarLayout = new QHBoxLayout(basicScanToolbarFrame);
        basicScanToolbarLayout->setSpacing(10);
        basicScanToolbarLayout->setObjectName("basicScanToolbarLayout");
        basicScanToolbarLayout->setContentsMargins(12, 8, 12, 8);
        basicScanLabel = new QLabel(basicScanToolbarFrame);
        basicScanLabel->setObjectName("basicScanLabel");
        QFont font;
        font.setPointSize(12);
        font.setBold(true);
        basicScanLabel->setFont(font);

        basicScanToolbarLayout->addWidget(basicScanLabel);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        basicScanToolbarLayout->addItem(horizontalSpacer_2);

        basicScanButton = new QPushButton(basicScanToolbarFrame);
        basicScanButton->setObjectName("basicScanButton");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(basicScanButton->sizePolicy().hasHeightForWidth());
        basicScanButton->setSizePolicy(sizePolicy1);
        basicScanButton->setMinimumSize(QSize(160, 40));
        basicScanButton->setMaximumSize(QSize(180, 40));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/basicScan.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        basicScanButton->setIcon(icon);

        basicScanToolbarLayout->addWidget(basicScanButton);


        basicScanTabLayout->addWidget(basicScanToolbarFrame);

        basicScanResultsFrame = new QFrame(basicScanTab);
        basicScanResultsFrame->setObjectName("basicScanResultsFrame");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(1);
        sizePolicy2.setHeightForWidth(basicScanResultsFrame->sizePolicy().hasHeightForWidth());
        basicScanResultsFrame->setSizePolicy(sizePolicy2);
        basicScanResultsFrame->setFrameShape(QFrame::StyledPanel);
        basicScanResultsFrame->setFrameShadow(QFrame::Sunken);
        basicScanResultsFrameLayout = new QVBoxLayout(basicScanResultsFrame);
        basicScanResultsFrameLayout->setSpacing(6);
        basicScanResultsFrameLayout->setObjectName("basicScanResultsFrameLayout");
        basicScanResultsFrameLayout->setContentsMargins(8, 8, 8, 8);
        basicScanResultsTextEdit = new QTextEdit(basicScanResultsFrame);
        basicScanResultsTextEdit->setObjectName("basicScanResultsTextEdit");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(basicScanResultsTextEdit->sizePolicy().hasHeightForWidth());
        basicScanResultsTextEdit->setSizePolicy(sizePolicy3);
        basicScanResultsTextEdit->setStyleSheet(QString::fromUtf8("\n"
"QTextEdit {\n"
"    font-family: \"Consolas\", \"Monaco\", \"Menlo\", monospace;\n"
"    font-size: 12px;\n"
"    line-height: 1.4;\n"
"    padding: 8px;\n"
"}\n"
"             "));
        basicScanResultsTextEdit->setReadOnly(true);

        basicScanResultsFrameLayout->addWidget(basicScanResultsTextEdit);


        basicScanTabLayout->addWidget(basicScanResultsFrame);

        dashboardTabWidget->addTab(basicScanTab, QString());
        advancedScanTab = new QWidget();
        advancedScanTab->setObjectName("advancedScanTab");
        advancedScanTabLayout = new QVBoxLayout(advancedScanTab);
        advancedScanTabLayout->setObjectName("advancedScanTabLayout");
        advancedScanToolbarFrame = new QFrame(advancedScanTab);
        advancedScanToolbarFrame->setObjectName("advancedScanToolbarFrame");
        sizePolicy.setHeightForWidth(advancedScanToolbarFrame->sizePolicy().hasHeightForWidth());
        advancedScanToolbarFrame->setSizePolicy(sizePolicy);
        advancedScanToolbarFrame->setMinimumSize(QSize(0, 70));
        advancedScanToolbarFrame->setMaximumSize(QSize(16777215, 70));
        advancedScanToolbarFrame->setFrameShape(QFrame::StyledPanel);
        advancedScanToolbarFrame->setFrameShadow(QFrame::Raised);
        advancedScanToolbarLayout = new QHBoxLayout(advancedScanToolbarFrame);
        advancedScanToolbarLayout->setSpacing(10);
        advancedScanToolbarLayout->setObjectName("advancedScanToolbarLayout");
        advancedScanToolbarLayout->setContentsMargins(12, 8, 12, 8);
        advancedScanLabel = new QLabel(advancedScanToolbarFrame);
        advancedScanLabel->setObjectName("advancedScanLabel");
        advancedScanLabel->setFont(font);

        advancedScanToolbarLayout->addWidget(advancedScanLabel);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        advancedScanToolbarLayout->addItem(horizontalSpacer_3);

        advancedScanButton = new QPushButton(advancedScanToolbarFrame);
        advancedScanButton->setObjectName("advancedScanButton");
        sizePolicy1.setHeightForWidth(advancedScanButton->sizePolicy().hasHeightForWidth());
        advancedScanButton->setSizePolicy(sizePolicy1);
        advancedScanButton->setMinimumSize(QSize(170, 40));
        advancedScanButton->setMaximumSize(QSize(190, 40));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/advancedScan.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        advancedScanButton->setIcon(icon1);

        advancedScanToolbarLayout->addWidget(advancedScanButton);


        advancedScanTabLayout->addWidget(advancedScanToolbarFrame);

        advancedScanResultsFrame = new QFrame(advancedScanTab);
        advancedScanResultsFrame->setObjectName("advancedScanResultsFrame");
        sizePolicy2.setHeightForWidth(advancedScanResultsFrame->sizePolicy().hasHeightForWidth());
        advancedScanResultsFrame->setSizePolicy(sizePolicy2);
        advancedScanResultsFrame->setFrameShape(QFrame::StyledPanel);
        advancedScanResultsFrame->setFrameShadow(QFrame::Sunken);
        advancedScanResultsFrameLayout = new QVBoxLayout(advancedScanResultsFrame);
        advancedScanResultsFrameLayout->setSpacing(6);
        advancedScanResultsFrameLayout->setObjectName("advancedScanResultsFrameLayout");
        advancedScanResultsFrameLayout->setContentsMargins(8, 8, 8, 8);
        advancedScanResultsTableWidget = new QTableWidget(advancedScanResultsFrame);
        advancedScanResultsTableWidget->setObjectName("advancedScanResultsTableWidget");
        QSizePolicy sizePolicy4(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(advancedScanResultsTableWidget->sizePolicy().hasHeightForWidth());
        advancedScanResultsTableWidget->setSizePolicy(sizePolicy4);
        advancedScanResultsTableWidget->setStyleSheet(QString::fromUtf8("\n"
"QTableWidget {\n"
"    font-family: \"Segoe UI\", \"Helvetica Neue\", sans-serif;\n"
"    font-size: 12px;\n"
"    gridline-color: rgba(70, 70, 70, 0.2);\n"
"    alternate-background-color: rgba(45, 45, 45, 0.1);\n"
"}\n"
"QHeaderView::section {\n"
"    font-weight: bold;\n"
"    padding: 5px;\n"
"}\n"
"             "));
        advancedScanResultsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        advancedScanResultsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        advancedScanResultsTableWidget->setAlternatingRowColors(true);
        advancedScanResultsTableWidget->horizontalHeader()->setStretchLastSection(true);
        advancedScanResultsTableWidget->verticalHeader()->setVisible(false);

        advancedScanResultsFrameLayout->addWidget(advancedScanResultsTableWidget);


        advancedScanTabLayout->addWidget(advancedScanResultsFrame);

        dashboardTabWidget->addTab(advancedScanTab, QString());
        cdrTab = new QWidget();
        cdrTab->setObjectName("cdrTab");
        cdrTabLayout = new QVBoxLayout(cdrTab);
        cdrTabLayout->setObjectName("cdrTabLayout");
        cdrToolbarFrame = new QFrame(cdrTab);
        cdrToolbarFrame->setObjectName("cdrToolbarFrame");
        sizePolicy.setHeightForWidth(cdrToolbarFrame->sizePolicy().hasHeightForWidth());
        cdrToolbarFrame->setSizePolicy(sizePolicy);
        cdrToolbarFrame->setMinimumSize(QSize(0, 70));
        cdrToolbarFrame->setMaximumSize(QSize(16777215, 70));
        cdrToolbarFrame->setFrameShape(QFrame::StyledPanel);
        cdrToolbarFrame->setFrameShadow(QFrame::Raised);
        cdrToolbarLayout = new QHBoxLayout(cdrToolbarFrame);
        cdrToolbarLayout->setSpacing(10);
        cdrToolbarLayout->setObjectName("cdrToolbarLayout");
        cdrToolbarLayout->setContentsMargins(12, 8, 12, 8);
        cdrScanLabel = new QLabel(cdrToolbarFrame);
        cdrScanLabel->setObjectName("cdrScanLabel");
        cdrScanLabel->setFont(font);

        cdrToolbarLayout->addWidget(cdrScanLabel);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        cdrToolbarLayout->addItem(horizontalSpacer_4);

        cdrFileTypeComboBox = new QComboBox(cdrToolbarFrame);
        cdrFileTypeComboBox->addItem(QString());
        cdrFileTypeComboBox->addItem(QString());
        cdrFileTypeComboBox->addItem(QString());
        cdrFileTypeComboBox->addItem(QString());
        cdrFileTypeComboBox->setObjectName("cdrFileTypeComboBox");
        cdrFileTypeComboBox->setMinimumSize(QSize(140, 30));
        cdrFileTypeComboBox->setMaximumSize(QSize(180, 30));

        cdrToolbarLayout->addWidget(cdrFileTypeComboBox);

        cdrScanButton = new QPushButton(cdrToolbarFrame);
        cdrScanButton->setObjectName("cdrScanButton");
        sizePolicy1.setHeightForWidth(cdrScanButton->sizePolicy().hasHeightForWidth());
        cdrScanButton->setSizePolicy(sizePolicy1);
        cdrScanButton->setMinimumSize(QSize(150, 40));
        cdrScanButton->setMaximumSize(QSize(170, 40));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/cdrScan.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        cdrScanButton->setIcon(icon2);

        cdrToolbarLayout->addWidget(cdrScanButton);


        cdrTabLayout->addWidget(cdrToolbarFrame);

        cdrResultsFrame = new QFrame(cdrTab);
        cdrResultsFrame->setObjectName("cdrResultsFrame");
        sizePolicy2.setHeightForWidth(cdrResultsFrame->sizePolicy().hasHeightForWidth());
        cdrResultsFrame->setSizePolicy(sizePolicy2);
        cdrResultsFrame->setFrameShape(QFrame::StyledPanel);
        cdrResultsFrame->setFrameShadow(QFrame::Sunken);
        cdrResultsFrameLayout = new QVBoxLayout(cdrResultsFrame);
        cdrResultsFrameLayout->setSpacing(6);
        cdrResultsFrameLayout->setObjectName("cdrResultsFrameLayout");
        cdrResultsFrameLayout->setContentsMargins(8, 8, 8, 8);
        cdrResultsTextEdit = new QTextEdit(cdrResultsFrame);
        cdrResultsTextEdit->setObjectName("cdrResultsTextEdit");
        sizePolicy4.setHeightForWidth(cdrResultsTextEdit->sizePolicy().hasHeightForWidth());
        cdrResultsTextEdit->setSizePolicy(sizePolicy4);
        cdrResultsTextEdit->setStyleSheet(QString::fromUtf8("\n"
"QTextEdit {\n"
"    font-family: \"Consolas\", \"Monaco\", \"Menlo\", monospace;\n"
"    font-size: 12px;\n"
"    line-height: 1.4;\n"
"    padding: 8px;\n"
"}\n"
"             "));
        cdrResultsTextEdit->setReadOnly(true);

        cdrResultsFrameLayout->addWidget(cdrResultsTextEdit);


        cdrTabLayout->addWidget(cdrResultsFrame);

        dashboardTabWidget->addTab(cdrTab, QString());
        sandboxTab = new QWidget();
        sandboxTab->setObjectName("sandboxTab");
        sandboxTabLayout = new QVBoxLayout(sandboxTab);
        sandboxTabLayout->setObjectName("sandboxTabLayout");
        sandboxToolbarFrame = new QFrame(sandboxTab);
        sandboxToolbarFrame->setObjectName("sandboxToolbarFrame");
        sizePolicy.setHeightForWidth(sandboxToolbarFrame->sizePolicy().hasHeightForWidth());
        sandboxToolbarFrame->setSizePolicy(sizePolicy);
        sandboxToolbarFrame->setMinimumSize(QSize(0, 70));
        sandboxToolbarFrame->setMaximumSize(QSize(16777215, 70));
        sandboxToolbarFrame->setFrameShape(QFrame::StyledPanel);
        sandboxToolbarFrame->setFrameShadow(QFrame::Raised);
        sandboxToolbarLayout = new QHBoxLayout(sandboxToolbarFrame);
        sandboxToolbarLayout->setSpacing(10);
        sandboxToolbarLayout->setObjectName("sandboxToolbarLayout");
        sandboxToolbarLayout->setContentsMargins(12, 8, 12, 8);
        sandboxLabel = new QLabel(sandboxToolbarFrame);
        sandboxLabel->setObjectName("sandboxLabel");
        sandboxLabel->setFont(font);

        sandboxToolbarLayout->addWidget(sandboxLabel);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        sandboxToolbarLayout->addItem(horizontalSpacer_5);

        sandboxTimeoutComboBox = new QComboBox(sandboxToolbarFrame);
        sandboxTimeoutComboBox->addItem(QString());
        sandboxTimeoutComboBox->addItem(QString());
        sandboxTimeoutComboBox->addItem(QString());
        sandboxTimeoutComboBox->setObjectName("sandboxTimeoutComboBox");
        sandboxTimeoutComboBox->setMinimumSize(QSize(140, 30));
        sandboxTimeoutComboBox->setMaximumSize(QSize(180, 30));

        sandboxToolbarLayout->addWidget(sandboxTimeoutComboBox);

        sandboxScanButton = new QPushButton(sandboxToolbarFrame);
        sandboxScanButton->setObjectName("sandboxScanButton");
        sizePolicy1.setHeightForWidth(sandboxScanButton->sizePolicy().hasHeightForWidth());
        sandboxScanButton->setSizePolicy(sizePolicy1);
        sandboxScanButton->setMinimumSize(QSize(180, 40));
        sandboxScanButton->setMaximumSize(QSize(200, 40));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/sandboxScan.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        sandboxScanButton->setIcon(icon3);

        sandboxToolbarLayout->addWidget(sandboxScanButton);


        sandboxTabLayout->addWidget(sandboxToolbarFrame);

        sandboxResultsFrame = new QFrame(sandboxTab);
        sandboxResultsFrame->setObjectName("sandboxResultsFrame");
        sizePolicy2.setHeightForWidth(sandboxResultsFrame->sizePolicy().hasHeightForWidth());
        sandboxResultsFrame->setSizePolicy(sizePolicy2);
        sandboxResultsFrame->setFrameShape(QFrame::StyledPanel);
        sandboxResultsFrame->setFrameShadow(QFrame::Sunken);
        sandboxResultsFrameLayout = new QVBoxLayout(sandboxResultsFrame);
        sandboxResultsFrameLayout->setSpacing(6);
        sandboxResultsFrameLayout->setObjectName("sandboxResultsFrameLayout");
        sandboxResultsFrameLayout->setContentsMargins(8, 8, 8, 8);
        sandboxResultsTextEdit = new QTextEdit(sandboxResultsFrame);
        sandboxResultsTextEdit->setObjectName("sandboxResultsTextEdit");
        sizePolicy4.setHeightForWidth(sandboxResultsTextEdit->sizePolicy().hasHeightForWidth());
        sandboxResultsTextEdit->setSizePolicy(sizePolicy4);
        sandboxResultsTextEdit->setStyleSheet(QString::fromUtf8("\n"
"QTextEdit {\n"
"    font-family: \"Consolas\", \"Monaco\", \"Menlo\", monospace;\n"
"    font-size: 12px;\n"
"    line-height: 1.4;\n"
"    padding: 8px;\n"
"}\n"
"             "));
        sandboxResultsTextEdit->setReadOnly(true);

        sandboxResultsFrameLayout->addWidget(sandboxResultsTextEdit);


        sandboxTabLayout->addWidget(sandboxResultsFrame);

        dashboardTabWidget->addTab(sandboxTab, QString());
        networkTab = new QWidget();
        networkTab->setObjectName("networkTab");
        networkTabLayout = new QVBoxLayout(networkTab);
        networkTabLayout->setObjectName("networkTabLayout");
        networkToolbarFrame = new QFrame(networkTab);
        networkToolbarFrame->setObjectName("networkToolbarFrame");
        sizePolicy.setHeightForWidth(networkToolbarFrame->sizePolicy().hasHeightForWidth());
        networkToolbarFrame->setSizePolicy(sizePolicy);
        networkToolbarFrame->setMinimumSize(QSize(0, 70));
        networkToolbarFrame->setMaximumSize(QSize(16777215, 70));
        networkToolbarFrame->setFrameShape(QFrame::StyledPanel);
        networkToolbarFrame->setFrameShadow(QFrame::Raised);
        networkToolbarLayout = new QHBoxLayout(networkToolbarFrame);
        networkToolbarLayout->setSpacing(10);
        networkToolbarLayout->setObjectName("networkToolbarLayout");
        networkToolbarLayout->setContentsMargins(12, 8, 12, 8);
        networkLabel = new QLabel(networkToolbarFrame);
        networkLabel->setObjectName("networkLabel");
        networkLabel->setFont(font);

        networkToolbarLayout->addWidget(networkLabel);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        networkToolbarLayout->addItem(horizontalSpacer_6);

        networkFilterComboBox = new QComboBox(networkToolbarFrame);
        networkFilterComboBox->addItem(QString());
        networkFilterComboBox->addItem(QString());
        networkFilterComboBox->addItem(QString());
        networkFilterComboBox->addItem(QString());
        networkFilterComboBox->setObjectName("networkFilterComboBox");
        networkFilterComboBox->setMinimumSize(QSize(140, 30));
        networkFilterComboBox->setMaximumSize(QSize(180, 30));

        networkToolbarLayout->addWidget(networkFilterComboBox);

        networkMonitorButton = new QPushButton(networkToolbarFrame);
        networkMonitorButton->setObjectName("networkMonitorButton");
        sizePolicy1.setHeightForWidth(networkMonitorButton->sizePolicy().hasHeightForWidth());
        networkMonitorButton->setSizePolicy(sizePolicy1);
        networkMonitorButton->setMinimumSize(QSize(180, 40));
        networkMonitorButton->setMaximumSize(QSize(200, 40));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/images/network.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        networkMonitorButton->setIcon(icon4);

        networkToolbarLayout->addWidget(networkMonitorButton);


        networkTabLayout->addWidget(networkToolbarFrame);

        networkResultsFrame = new QFrame(networkTab);
        networkResultsFrame->setObjectName("networkResultsFrame");
        sizePolicy2.setHeightForWidth(networkResultsFrame->sizePolicy().hasHeightForWidth());
        networkResultsFrame->setSizePolicy(sizePolicy2);
        networkResultsFrame->setFrameShape(QFrame::StyledPanel);
        networkResultsFrame->setFrameShadow(QFrame::Sunken);
        networkResultsFrameLayout = new QVBoxLayout(networkResultsFrame);
        networkResultsFrameLayout->setSpacing(6);
        networkResultsFrameLayout->setObjectName("networkResultsFrameLayout");
        networkResultsFrameLayout->setContentsMargins(8, 8, 8, 8);
        networkCommunicationTextEdit = new QTextEdit(networkResultsFrame);
        networkCommunicationTextEdit->setObjectName("networkCommunicationTextEdit");
        sizePolicy4.setHeightForWidth(networkCommunicationTextEdit->sizePolicy().hasHeightForWidth());
        networkCommunicationTextEdit->setSizePolicy(sizePolicy4);
        networkCommunicationTextEdit->setStyleSheet(QString::fromUtf8("\n"
"QTextEdit {\n"
"    font-family: \"Consolas\", \"Monaco\", \"Menlo\", monospace;\n"
"    font-size: 12px;\n"
"    line-height: 1.4;\n"
"    padding: 8px;\n"
"}\n"
"             "));
        networkCommunicationTextEdit->setReadOnly(true);

        networkResultsFrameLayout->addWidget(networkCommunicationTextEdit);


        networkTabLayout->addWidget(networkResultsFrame);

        dashboardTabWidget->addTab(networkTab, QString());

        mainVerticalLayout->addWidget(dashboardTabWidget);

        statusBarFrame = new QFrame(DashboardWidget);
        statusBarFrame->setObjectName("statusBarFrame");
        QSizePolicy sizePolicy5(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(statusBarFrame->sizePolicy().hasHeightForWidth());
        statusBarFrame->setSizePolicy(sizePolicy5);
        statusBarFrame->setMinimumSize(QSize(0, 60));
        statusBarFrame->setMaximumSize(QSize(16777215, 60));
        statusBarFrame->setFrameShape(QFrame::StyledPanel);
        statusBarFrame->setFrameShadow(QFrame::Raised);
        statusBarLayout = new QHBoxLayout(statusBarFrame);
        statusBarLayout->setSpacing(12);
        statusBarLayout->setObjectName("statusBarLayout");
        statusBarLayout->setContentsMargins(12, 6, 12, 6);
        statusLabel = new QLabel(statusBarFrame);
        statusLabel->setObjectName("statusLabel");

        statusBarLayout->addWidget(statusLabel);

        lastScanLabel = new QLabel(statusBarFrame);
        lastScanLabel->setObjectName("lastScanLabel");
        lastScanLabel->setMargin(0);
        lastScanLabel->setIndent(20);

        statusBarLayout->addWidget(lastScanLabel);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        statusBarLayout->addItem(horizontalSpacer);

        configButton = new QPushButton(statusBarFrame);
        configButton->setObjectName("configButton");
        sizePolicy1.setHeightForWidth(configButton->sizePolicy().hasHeightForWidth());
        configButton->setSizePolicy(sizePolicy1);
        configButton->setMinimumSize(QSize(120, 40));
        configButton->setMaximumSize(QSize(140, 40));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/images/settings.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        configButton->setIcon(icon5);

        statusBarLayout->addWidget(configButton);

        refreshButton = new QPushButton(statusBarFrame);
        refreshButton->setObjectName("refreshButton");
        sizePolicy1.setHeightForWidth(refreshButton->sizePolicy().hasHeightForWidth());
        refreshButton->setSizePolicy(sizePolicy1);
        refreshButton->setMinimumSize(QSize(120, 40));
        refreshButton->setMaximumSize(QSize(140, 40));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/images/refresh.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        refreshButton->setIcon(icon6);

        statusBarLayout->addWidget(refreshButton);


        mainVerticalLayout->addWidget(statusBarFrame);


        retranslateUi(DashboardWidget);

        dashboardTabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(DashboardWidget);
    } // setupUi

    void retranslateUi(QWidget *DashboardWidget)
    {
        DashboardWidget->setWindowTitle(QCoreApplication::translate("DashboardWidget", "Form", nullptr));
        dashboardTabWidget->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QTabWidget::pane { \n"
"  border: 1px solid rgba(74, 74, 74, 0.6);\n"
"  border-radius: 6px;\n"
"  background-color: rgba(0, 0, 0, 0.01);\n"
"  margin-top: -1px;\n"
"}\n"
"QTabWidget::tab-bar {\n"
"  left: 5px;\n"
"}\n"
"QTabBar::tab {\n"
"  background-color: rgba(0, 0, 0, 0.08);\n"
"  border: 1px solid rgba(58, 58, 58, 0.7);\n"
"  border-bottom: 0px;\n"
"  border-top-left-radius: 5px;\n"
"  border-top-right-radius: 5px;\n"
"  padding: 10px 18px;\n"
"  margin-right: 3px;\n"
"  color: #DDDDDD;\n"
"  font-weight: bold;\n"
"}\n"
"QTabBar::tab:selected {\n"
"  background-color: rgba(42, 130, 218, 0.15);\n"
"  color: #FFFFFF;\n"
"}\n"
"QTabBar::tab:hover:!selected {\n"
"  background-color: rgba(60, 60, 60, 0.3);\n"
"}\n"
"      ", nullptr));
        basicScanToolbarFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid rgba(74, 74, 74, 0.5);\n"
"    background-color: rgba(20, 20, 20, 0.15);\n"
"    border-radius: 6px;\n"
"}\n"
"QPushButton {\n"
"    background-color: #2a82da;\n"
"    color: white;\n"
"    border-radius: 5px;\n"
"    padding: 6px 14px;\n"
"    border: none;\n"
"    font-weight: bold;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #3a92ea;\n"
"}\n"
"QPushButton:pressed {\n"
"    background-color: #1a72ca;\n"
"}\n"
"QLabel {\n"
"    color: #e0e0e0;\n"
"}\n"
"          ", nullptr));
        basicScanLabel->setText(QCoreApplication::translate("DashboardWidget", "Basic Malware Scan", nullptr));
        basicScanButton->setText(QCoreApplication::translate("DashboardWidget", "Run Basic Scan", nullptr));
        basicScanResultsFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid rgba(74, 74, 74, 0.4);\n"
"    background-color: rgba(0, 0, 0, 0.02);\n"
"    border-radius: 6px;\n"
"}\n"
"QTextEdit {\n"
"    background-color: rgba(0, 0, 0, 0.1);\n"
"    border: none;\n"
"    color: #e0e0e0;\n"
"    selection-background-color: #2a82da;\n"
"    selection-color: white;\n"
"}\n"
"          ", nullptr));
        basicScanResultsTextEdit->setPlaceholderText(QCoreApplication::translate("DashboardWidget", "Basic scan results will be shown here...", nullptr));
        dashboardTabWidget->setTabText(dashboardTabWidget->indexOf(basicScanTab), QCoreApplication::translate("DashboardWidget", "Basic Scan", nullptr));
        advancedScanToolbarFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid rgba(74, 74, 74, 0.5);\n"
"    background-color: rgba(20, 20, 20, 0.15);\n"
"    border-radius: 6px;\n"
"}\n"
"QPushButton {\n"
"    background-color: #2a82da;\n"
"    color: white;\n"
"    border-radius: 5px;\n"
"    padding: 6px 14px;\n"
"    border: none;\n"
"    font-weight: bold;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #3a92ea;\n"
"}\n"
"QPushButton:pressed {\n"
"    background-color: #1a72ca;\n"
"}\n"
"QLabel {\n"
"    color: #e0e0e0;\n"
"}\n"
"          ", nullptr));
        advancedScanLabel->setText(QCoreApplication::translate("DashboardWidget", "Advanced Threat Analysis", nullptr));
        advancedScanButton->setText(QCoreApplication::translate("DashboardWidget", "Run Advanced Scan", nullptr));
        advancedScanResultsFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid rgba(74, 74, 74, 0.4);\n"
"    background-color: rgba(0, 0, 0, 0.02);\n"
"    border-radius: 6px;\n"
"}\n"
"QTableWidget {\n"
"    background-color: rgba(0, 0, 0, 0.1);\n"
"    border: none;\n"
"    color: #e0e0e0;\n"
"    gridline-color: rgba(80, 80, 80, 0.2);\n"
"    selection-background-color: rgba(42, 130, 218, 0.4);\n"
"    selection-color: white;\n"
"}\n"
"QHeaderView::section {\n"
"    background-color: rgba(30, 30, 30, 0.2);\n"
"    color: #e0e0e0;\n"
"    padding: 4px;\n"
"    border: none;\n"
"    border-right: 1px solid rgba(80, 80, 80, 0.3);\n"
"    border-bottom: 1px solid rgba(80, 80, 80, 0.3);\n"
"}\n"
"          ", nullptr));
        dashboardTabWidget->setTabText(dashboardTabWidget->indexOf(advancedScanTab), QCoreApplication::translate("DashboardWidget", "Advanced Scan", nullptr));
        cdrToolbarFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid rgba(74, 74, 74, 0.5);\n"
"    background-color: rgba(20, 20, 20, 0.15);\n"
"    border-radius: 6px;\n"
"}\n"
"QPushButton {\n"
"    background-color: #2a82da;\n"
"    color: white;\n"
"    border-radius: 5px;\n"
"    padding: 6px 14px;\n"
"    border: none;\n"
"    font-weight: bold;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #3a92ea;\n"
"}\n"
"QPushButton:pressed {\n"
"    background-color: #1a72ca;\n"
"}\n"
"QLabel {\n"
"    color: #e0e0e0;\n"
"}\n"
"QComboBox {\n"
"    background-color: rgba(40, 40, 40, 0.5);\n"
"    border: 1px solid rgba(80, 80, 80, 0.6);\n"
"    border-radius: 4px;\n"
"    color: #e0e0e0;\n"
"    padding: 4px 8px;\n"
"    selection-background-color: #2a82da;\n"
"}\n"
"QComboBox::drop-down {\n"
"    border: none;\n"
"}\n"
"QComboBox QAbstractItemView {\n"
"    background-color: #2c2c2c;\n"
"    color: #e0e0e0;\n"
"    selection-background-color: #2a82da;\n"
"}\n"
"          ", nullptr));
        cdrScanLabel->setText(QCoreApplication::translate("DashboardWidget", "Content Disarm & Reconstruction", nullptr));
        cdrFileTypeComboBox->setItemText(0, QCoreApplication::translate("DashboardWidget", "PDF Files", nullptr));
        cdrFileTypeComboBox->setItemText(1, QCoreApplication::translate("DashboardWidget", "Office Documents", nullptr));
        cdrFileTypeComboBox->setItemText(2, QCoreApplication::translate("DashboardWidget", "Image Files", nullptr));
        cdrFileTypeComboBox->setItemText(3, QCoreApplication::translate("DashboardWidget", "All Files", nullptr));

        cdrScanButton->setText(QCoreApplication::translate("DashboardWidget", "Run CDR Scan", nullptr));
        cdrResultsFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid rgba(74, 74, 74, 0.4);\n"
"    background-color: rgba(0, 0, 0, 0.02);\n"
"    border-radius: 6px;\n"
"}\n"
"QTextEdit {\n"
"    background-color: rgba(0, 0, 0, 0.1);\n"
"    border: none;\n"
"    color: #e0e0e0;\n"
"    selection-background-color: #2a82da;\n"
"    selection-color: white;\n"
"}\n"
"          ", nullptr));
        cdrResultsTextEdit->setPlaceholderText(QCoreApplication::translate("DashboardWidget", "CDR (Content Disarm & Reconstruction) results will be shown here...", nullptr));
        dashboardTabWidget->setTabText(dashboardTabWidget->indexOf(cdrTab), QCoreApplication::translate("DashboardWidget", "CDR", nullptr));
        sandboxToolbarFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid rgba(74, 74, 74, 0.5);\n"
"    background-color: rgba(20, 20, 20, 0.15);\n"
"    border-radius: 6px;\n"
"}\n"
"QPushButton {\n"
"    background-color: #2a82da;\n"
"    color: white;\n"
"    border-radius: 5px;\n"
"    padding: 6px 14px;\n"
"    border: none;\n"
"    font-weight: bold;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #3a92ea;\n"
"}\n"
"QPushButton:pressed {\n"
"    background-color: #1a72ca;\n"
"}\n"
"QLabel {\n"
"    color: #e0e0e0;\n"
"}\n"
"QComboBox {\n"
"    background-color: rgba(40, 40, 40, 0.5);\n"
"    border: 1px solid rgba(80, 80, 80, 0.6);\n"
"    border-radius: 4px;\n"
"    color: #e0e0e0;\n"
"    padding: 4px 8px;\n"
"    selection-background-color: #2a82da;\n"
"}\n"
"QComboBox::drop-down {\n"
"    border: none;\n"
"}\n"
"QComboBox QAbstractItemView {\n"
"    background-color: #2c2c2c;\n"
"    color: #e0e0e0;\n"
"    selection-background-color: #2a82da;\n"
"}\n"
"          ", nullptr));
        sandboxLabel->setText(QCoreApplication::translate("DashboardWidget", "Sandbox Environment Analysis", nullptr));
        sandboxTimeoutComboBox->setItemText(0, QCoreApplication::translate("DashboardWidget", "60 seconds", nullptr));
        sandboxTimeoutComboBox->setItemText(1, QCoreApplication::translate("DashboardWidget", "120 seconds", nullptr));
        sandboxTimeoutComboBox->setItemText(2, QCoreApplication::translate("DashboardWidget", "300 seconds", nullptr));

        sandboxScanButton->setText(QCoreApplication::translate("DashboardWidget", "Run Sandbox Analysis", nullptr));
        sandboxResultsFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid rgba(74, 74, 74, 0.4);\n"
"    background-color: rgba(0, 0, 0, 0.02);\n"
"    border-radius: 6px;\n"
"}\n"
"QTextEdit {\n"
"    background-color: rgba(0, 0, 0, 0.1);\n"
"    border: none;\n"
"    color: #e0e0e0;\n"
"    selection-background-color: #2a82da;\n"
"    selection-color: white;\n"
"}\n"
"          ", nullptr));
        sandboxResultsTextEdit->setPlaceholderText(QCoreApplication::translate("DashboardWidget", "Sandbox analysis results will be shown here...", nullptr));
        dashboardTabWidget->setTabText(dashboardTabWidget->indexOf(sandboxTab), QCoreApplication::translate("DashboardWidget", "Sandbox", nullptr));
        networkToolbarFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid rgba(74, 74, 74, 0.5);\n"
"    background-color: rgba(20, 20, 20, 0.15);\n"
"    border-radius: 6px;\n"
"}\n"
"QPushButton {\n"
"    background-color: #2a82da;\n"
"    color: white;\n"
"    border-radius: 5px;\n"
"    padding: 6px 14px;\n"
"    border: none;\n"
"    font-weight: bold;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #3a92ea;\n"
"}\n"
"QPushButton:pressed {\n"
"    background-color: #1a72ca;\n"
"}\n"
"QPushButton[monitoring=\"true\"] {\n"
"    background-color: #e74c3c;\n"
"}\n"
"QPushButton[monitoring=\"true\"]:hover {\n"
"    background-color: #f75c4c;\n"
"}\n"
"QLabel {\n"
"    color: #e0e0e0;\n"
"}\n"
"QComboBox {\n"
"    background-color: rgba(40, 40, 40, 0.5);\n"
"    border: 1px solid rgba(80, 80, 80, 0.6);\n"
"    border-radius: 4px;\n"
"    color: #e0e0e0;\n"
"    padding: 4px 8px;\n"
"    selection-background-color: #2a82da;\n"
"}\n"
"QComboBox::drop-down {\n"
"    border: none;\n"
"}\n"
"QComboBox QAbstractItemView {\n"
"    background-c"
                        "olor: #2c2c2c;\n"
"    color: #e0e0e0;\n"
"    selection-background-color: #2a82da;\n"
"}\n"
"          ", nullptr));
        networkLabel->setText(QCoreApplication::translate("DashboardWidget", "Network Activity Monitoring", nullptr));
        networkFilterComboBox->setItemText(0, QCoreApplication::translate("DashboardWidget", "All Traffic", nullptr));
        networkFilterComboBox->setItemText(1, QCoreApplication::translate("DashboardWidget", "HTTP/HTTPS Only", nullptr));
        networkFilterComboBox->setItemText(2, QCoreApplication::translate("DashboardWidget", "DNS Only", nullptr));
        networkFilterComboBox->setItemText(3, QCoreApplication::translate("DashboardWidget", "Malicious Only", nullptr));

        networkMonitorButton->setText(QCoreApplication::translate("DashboardWidget", "Start Network Monitor", nullptr));
        networkResultsFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid rgba(74, 74, 74, 0.4);\n"
"    background-color: rgba(0, 0, 0, 0.02);\n"
"    border-radius: 6px;\n"
"}\n"
"QTextEdit {\n"
"    background-color: rgba(0, 0, 0, 0.1);\n"
"    border: none;\n"
"    color: #e0e0e0;\n"
"    selection-background-color: #2a82da;\n"
"    selection-color: white;\n"
"}\n"
"          ", nullptr));
        networkCommunicationTextEdit->setPlaceholderText(QCoreApplication::translate("DashboardWidget", "Network activity will be shown here...", nullptr));
        dashboardTabWidget->setTabText(dashboardTabWidget->indexOf(networkTab), QCoreApplication::translate("DashboardWidget", "Network", nullptr));
        statusBarFrame->setStyleSheet(QCoreApplication::translate("DashboardWidget", "\n"
"QFrame {\n"
"    border: 1px solid rgba(74, 74, 74, 0.5);\n"
"    background-color: rgba(25, 25, 25, 0.1);\n"
"    border-radius: 5px;\n"
"}\n"
"QPushButton {\n"
"    background-color: #2a82da;\n"
"    color: white;\n"
"    border-radius: 4px;\n"
"    padding: 6px 14px;\n"
"    border: none;\n"
"    font-weight: bold;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #3a92ea;\n"
"}\n"
"QPushButton:pressed {\n"
"    background-color: #1a72ca;\n"
"}\n"
"QLabel {\n"
"    color: #e0e0e0;\n"
"    font-weight: bold;\n"
"}\n"
"      ", nullptr));
        statusLabel->setText(QCoreApplication::translate("DashboardWidget", "Status: Ready", nullptr));
        lastScanLabel->setText(QCoreApplication::translate("DashboardWidget", "Last Scan: None", nullptr));
        configButton->setText(QCoreApplication::translate("DashboardWidget", "Configuration", nullptr));
        refreshButton->setText(QCoreApplication::translate("DashboardWidget", "Refresh", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DashboardWidget: public Ui_DashboardWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DASHBOARDWIDGET_H
