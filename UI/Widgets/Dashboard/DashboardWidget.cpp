#include "DashboardWidget.h"
#include "ui_dashboardwidget.h"

DashboardWidget::DashboardWidget(QWidget *parent):
    QWidget(parent),
    ui(new Ui::DashboardWidget)
{
    ui->setupUi(this);

}
DashboardWidget::~DashboardWidget() {
    delete ui;
}
