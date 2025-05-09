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
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DashboardWidget
{
public:
    QHBoxLayout *mainHorizontalLayout;
    QWidget *leftPanelWidget;
    QVBoxLayout *leftPanelVerticalLayout;
    QTextEdit *scanResultsTextEdit_dashboard;
    QWidget *rightPanelWidget;
    QVBoxLayout *rightPanelVerticalLayout;
    QPushButton *basicScanButton_dashboard;
    QPushButton *advancedScanButton_dashboard;
    QPushButton *cdrScanButton_dashboard;
    QPushButton *sandboxScanButton_dashboard;
    QSpacerItem *verticalSpacer_buttons;
    QFrame *line;
    QHBoxLayout *totalScansHorizontalLayout;
    QLabel *totalScansLabel_dashboard;
    QLabel *totalScansValue_dashboard;
    QSpacerItem *verticalSpacer_bottom;

    void setupUi(QWidget *DashboardWidget)
    {
        if (DashboardWidget->objectName().isEmpty())
            DashboardWidget->setObjectName("DashboardWidget");
        DashboardWidget->resize(750, 550);
        mainHorizontalLayout = new QHBoxLayout(DashboardWidget);
        mainHorizontalLayout->setObjectName("mainHorizontalLayout");
        leftPanelWidget = new QWidget(DashboardWidget);
        leftPanelWidget->setObjectName("leftPanelWidget");
        leftPanelVerticalLayout = new QVBoxLayout(leftPanelWidget);
        leftPanelVerticalLayout->setObjectName("leftPanelVerticalLayout");
        scanResultsTextEdit_dashboard = new QTextEdit(leftPanelWidget);
        scanResultsTextEdit_dashboard->setObjectName("scanResultsTextEdit_dashboard");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(scanResultsTextEdit_dashboard->sizePolicy().hasHeightForWidth());
        scanResultsTextEdit_dashboard->setSizePolicy(sizePolicy);
        scanResultsTextEdit_dashboard->setReadOnly(true);

        leftPanelVerticalLayout->addWidget(scanResultsTextEdit_dashboard);

        leftPanelVerticalLayout->setStretch(0, 1);

        mainHorizontalLayout->addWidget(leftPanelWidget);

        rightPanelWidget = new QWidget(DashboardWidget);
        rightPanelWidget->setObjectName("rightPanelWidget");
        rightPanelVerticalLayout = new QVBoxLayout(rightPanelWidget);
        rightPanelVerticalLayout->setObjectName("rightPanelVerticalLayout");
        basicScanButton_dashboard = new QPushButton(rightPanelWidget);
        basicScanButton_dashboard->setObjectName("basicScanButton_dashboard");

        rightPanelVerticalLayout->addWidget(basicScanButton_dashboard);

        advancedScanButton_dashboard = new QPushButton(rightPanelWidget);
        advancedScanButton_dashboard->setObjectName("advancedScanButton_dashboard");

        rightPanelVerticalLayout->addWidget(advancedScanButton_dashboard);

        cdrScanButton_dashboard = new QPushButton(rightPanelWidget);
        cdrScanButton_dashboard->setObjectName("cdrScanButton_dashboard");

        rightPanelVerticalLayout->addWidget(cdrScanButton_dashboard);

        sandboxScanButton_dashboard = new QPushButton(rightPanelWidget);
        sandboxScanButton_dashboard->setObjectName("sandboxScanButton_dashboard");

        rightPanelVerticalLayout->addWidget(sandboxScanButton_dashboard);

        verticalSpacer_buttons = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        rightPanelVerticalLayout->addItem(verticalSpacer_buttons);

        line = new QFrame(rightPanelWidget);
        line->setObjectName("line");
        line->setFrameShape(QFrame::Shape::HLine);
        line->setFrameShadow(QFrame::Shadow::Sunken);

        rightPanelVerticalLayout->addWidget(line);

        totalScansHorizontalLayout = new QHBoxLayout();
        totalScansHorizontalLayout->setObjectName("totalScansHorizontalLayout");
        totalScansLabel_dashboard = new QLabel(rightPanelWidget);
        totalScansLabel_dashboard->setObjectName("totalScansLabel_dashboard");

        totalScansHorizontalLayout->addWidget(totalScansLabel_dashboard);

        totalScansValue_dashboard = new QLabel(rightPanelWidget);
        totalScansValue_dashboard->setObjectName("totalScansValue_dashboard");
        QFont font;
        font.setBold(true);
        totalScansValue_dashboard->setFont(font);
        totalScansValue_dashboard->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        totalScansHorizontalLayout->addWidget(totalScansValue_dashboard);


        rightPanelVerticalLayout->addLayout(totalScansHorizontalLayout);

        verticalSpacer_bottom = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        rightPanelVerticalLayout->addItem(verticalSpacer_bottom);


        mainHorizontalLayout->addWidget(rightPanelWidget);

        mainHorizontalLayout->setStretch(0, 3);
        mainHorizontalLayout->setStretch(1, 1);

        retranslateUi(DashboardWidget);

        QMetaObject::connectSlotsByName(DashboardWidget);
    } // setupUi

    void retranslateUi(QWidget *DashboardWidget)
    {
        DashboardWidget->setWindowTitle(QCoreApplication::translate("DashboardWidget", "Form", nullptr));
        scanResultsTextEdit_dashboard->setPlaceholderText(QCoreApplication::translate("DashboardWidget", "Tarama sonu\303\247lar\304\261 burada g\303\266r\303\274necek...", nullptr));
        basicScanButton_dashboard->setText(QCoreApplication::translate("DashboardWidget", "BASIC SCAN", nullptr));
        advancedScanButton_dashboard->setText(QCoreApplication::translate("DashboardWidget", "ADVANCED SCAN", nullptr));
        cdrScanButton_dashboard->setText(QCoreApplication::translate("DashboardWidget", "CDR SCAN", nullptr));
        sandboxScanButton_dashboard->setText(QCoreApplication::translate("DashboardWidget", "SANDBOX SCAN", nullptr));
        totalScansLabel_dashboard->setText(QCoreApplication::translate("DashboardWidget", "Toplam Taramalar:", nullptr));
        totalScansValue_dashboard->setText(QCoreApplication::translate("DashboardWidget", "0", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DashboardWidget: public Ui_DashboardWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DASHBOARDWIDGET_H
