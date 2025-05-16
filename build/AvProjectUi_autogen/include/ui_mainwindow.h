/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout;
    QPushButton *navDashbardButton;
    QPushButton *navHistoryButton;
    QPushButton *navServiceStatusButton;
    QPushButton *navSettingsButton;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QStackedWidget *contentStackedWidget;
    QWidget *dashboardPage;
    QLabel *dashboardLabel;
    QWidget *serviceStatusPage;
    QLabel *seviceStatusLabel;
    QWidget *settingsPage;
    QLabel *settingsLabel;
    QWidget *historyPage;
    QLabel *historyLabel;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout_2 = new QHBoxLayout(centralwidget);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setContentsMargins(10, 10, 10, 10);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(12);
        verticalLayout->setObjectName("verticalLayout");
        navDashbardButton = new QPushButton(centralwidget);
        navDashbardButton->setObjectName("navDashbardButton");
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/dashboardLogo.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        navDashbardButton->setIcon(icon);
        navDashbardButton->setIconSize(QSize(52, 52));
        navDashbardButton->setFlat(true);
        navDashbardButton->setCheckable(false);
        navDashbardButton->setChecked(false);

        verticalLayout->addWidget(navDashbardButton);

        navHistoryButton = new QPushButton(centralwidget);
        navHistoryButton->setObjectName("navHistoryButton");
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/history.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        navHistoryButton->setIcon(icon1);
        navHistoryButton->setIconSize(QSize(52, 52));
        navHistoryButton->setFlat(true);
        navHistoryButton->setCheckable(false);

        verticalLayout->addWidget(navHistoryButton);

        navServiceStatusButton = new QPushButton(centralwidget);
        navServiceStatusButton->setObjectName("navServiceStatusButton");
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/ServiceStatus.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        navServiceStatusButton->setIcon(icon2);
        navServiceStatusButton->setIconSize(QSize(52, 52));
        navServiceStatusButton->setFlat(true);
        navServiceStatusButton->setCheckable(false);

        verticalLayout->addWidget(navServiceStatusButton);

        navSettingsButton = new QPushButton(centralwidget);
        navSettingsButton->setObjectName("navSettingsButton");
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/Settings.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        navSettingsButton->setIcon(icon3);
        navSettingsButton->setIconSize(QSize(52, 52));
        navSettingsButton->setFlat(true);
        navSettingsButton->setCheckable(false);

        verticalLayout->addWidget(navSettingsButton);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        horizontalLayout_2->addLayout(verticalLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        contentStackedWidget = new QStackedWidget(centralwidget);
        contentStackedWidget->setObjectName("contentStackedWidget");
        dashboardPage = new QWidget();
        dashboardPage->setObjectName("dashboardPage");
        dashboardLabel = new QLabel(dashboardPage);
        dashboardLabel->setObjectName("dashboardLabel");
        dashboardLabel->setGeometry(QRect(190, 40, 58, 16));
        contentStackedWidget->addWidget(dashboardPage);
        serviceStatusPage = new QWidget();
        serviceStatusPage->setObjectName("serviceStatusPage");
        seviceStatusLabel = new QLabel(serviceStatusPage);
        seviceStatusLabel->setObjectName("seviceStatusLabel");
        seviceStatusLabel->setGeometry(QRect(90, 60, 58, 16));
        contentStackedWidget->addWidget(serviceStatusPage);
        settingsPage = new QWidget();
        settingsPage->setObjectName("settingsPage");
        settingsLabel = new QLabel(settingsPage);
        settingsLabel->setObjectName("settingsLabel");
        settingsLabel->setGeometry(QRect(120, 40, 58, 16));
        contentStackedWidget->addWidget(settingsPage);
        historyPage = new QWidget();
        historyPage->setObjectName("historyPage");
        historyLabel = new QLabel(historyPage);
        historyLabel->setObjectName("historyLabel");
        historyLabel->setGeometry(QRect(220, 70, 58, 16));
        contentStackedWidget->addWidget(historyPage);

        horizontalLayout->addWidget(contentStackedWidget);


        horizontalLayout_2->addLayout(horizontalLayout);

        horizontalLayout_2->setStretch(0, 2);
        horizontalLayout_2->setStretch(1, 4);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 38));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        contentStackedWidget->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        navDashbardButton->setText(QString());
        navDashbardButton->setStyleSheet(QCoreApplication::translate("MainWindow", "\n"
"QPushButton {\n"
"    min-width: 60px;\n"
"    max-width: 60px;\n"
"    min-height: 60px;\n"
"    max-height: 60px;\n"
"    padding: 4px;\n"
"    border: none;\n"
"    background-color: transparent;\n"
"    border-radius: 8px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: rgba(255, 255, 255, 0.08);\n"
"}\n"
"QPushButton:pressed, QPushButton:checked {\n"
"    background-color: rgba(74, 144, 226, 0.2);\n"
"    border: 2px solid #4A90E2;\n"
"    padding: 2px; /* Adjust padding to account for border */\n"
"}\n"
"QPushButton:focus {\n"
"    outline: none;\n"
"}\n"
"         ", nullptr));
        navHistoryButton->setText(QString());
        navHistoryButton->setStyleSheet(QCoreApplication::translate("MainWindow", "\n"
"QPushButton {\n"
"    min-width: 60px;\n"
"    max-width: 60px;\n"
"    min-height: 60px;\n"
"    max-height: 60px;\n"
"    padding: 4px;\n"
"    border: none;\n"
"    background-color: transparent;\n"
"    border-radius: 8px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: rgba(255, 255, 255, 0.08);\n"
"}\n"
"QPushButton:pressed, QPushButton:checked {\n"
"    background-color: rgba(74, 144, 226, 0.2);\n"
"    border: 2px solid #4A90E2;\n"
"    padding: 2px; /* Adjust padding to account for border */\n"
"}\n"
"QPushButton:focus {\n"
"    outline: none;\n"
"}\n"
"         ", nullptr));
        navServiceStatusButton->setText(QString());
        navServiceStatusButton->setStyleSheet(QCoreApplication::translate("MainWindow", "\n"
"QPushButton {\n"
"    min-width: 60px;\n"
"    max-width: 60px;\n"
"    min-height: 60px;\n"
"    max-height: 60px;\n"
"    padding: 4px;\n"
"    border: none;\n"
"    background-color: transparent;\n"
"    border-radius: 8px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: rgba(255, 255, 255, 0.08);\n"
"}\n"
"QPushButton:pressed, QPushButton:checked {\n"
"    background-color: rgba(74, 144, 226, 0.2);\n"
"    border: 2px solid #4A90E2;\n"
"    padding: 2px; /* Adjust padding to account for border */\n"
"}\n"
"QPushButton:focus {\n"
"    outline: none;\n"
"}\n"
"         ", nullptr));
        navSettingsButton->setText(QString());
        navSettingsButton->setStyleSheet(QCoreApplication::translate("MainWindow", "\n"
"QPushButton {\n"
"    min-width: 60px;\n"
"    max-width: 60px;\n"
"    min-height: 60px;\n"
"    max-height: 60px;\n"
"    padding: 4px;\n"
"    border: none;\n"
"    background-color: transparent;\n"
"    border-radius: 8px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: rgba(255, 255, 255, 0.08);\n"
"}\n"
"QPushButton:pressed, QPushButton:checked {\n"
"    background-color: rgba(74, 144, 226, 0.2);\n"
"    border: 2px solid #4A90E2;\n"
"    padding: 2px; /* Adjust padding to account for border */\n"
"}\n"
"QPushButton:focus {\n"
"    outline: none;\n"
"}\n"
"         ", nullptr));
        dashboardLabel->setText(QCoreApplication::translate("MainWindow", "Dashboard Sayfa \304\260\303\247eri\304\237i", nullptr));
        seviceStatusLabel->setText(QCoreApplication::translate("MainWindow", "Service status bilgileri", nullptr));
        settingsLabel->setText(QCoreApplication::translate("MainWindow", "Ayarlar \304\260\303\247eri\304\237i", nullptr));
        historyLabel->setText(QCoreApplication::translate("MainWindow", "History i\303\247eri\304\237i", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
