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
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
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
        horizontalLayout_2->setContentsMargins(16, -1, -1, -1);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        navDashbardButton = new QPushButton(centralwidget);
        navDashbardButton->setObjectName("navDashbardButton");

        verticalLayout->addWidget(navDashbardButton);

        navHistoryButton = new QPushButton(centralwidget);
        navHistoryButton->setObjectName("navHistoryButton");

        verticalLayout->addWidget(navHistoryButton);

        navServiceStatusButton = new QPushButton(centralwidget);
        navServiceStatusButton->setObjectName("navServiceStatusButton");

        verticalLayout->addWidget(navServiceStatusButton);

        navSettingsButton = new QPushButton(centralwidget);
        navSettingsButton->setObjectName("navSettingsButton");

        verticalLayout->addWidget(navSettingsButton);


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

        horizontalLayout_2->setStretch(0, 1);
        horizontalLayout_2->setStretch(1, 6);
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
        navDashbardButton->setText(QCoreApplication::translate("MainWindow", "Dashboard", nullptr));
        navHistoryButton->setText(QCoreApplication::translate("MainWindow", "History", nullptr));
        navServiceStatusButton->setText(QCoreApplication::translate("MainWindow", "Service Status", nullptr));
        navSettingsButton->setText(QCoreApplication::translate("MainWindow", "Settings", nullptr));
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
