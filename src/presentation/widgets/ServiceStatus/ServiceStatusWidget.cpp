#include "ServiceStatusWidget.h" // Corrected header include
#include "ui_ServiceStatusWidget.h" // Corrected UI header include

#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>

ServiceStatusWidget::ServiceStatusWidget(QWidget *parent) : // Corrected constructor definition
    QWidget(parent),
    ui(new Ui::ServiceStatusWidget) // Corrected UI class instantiation
{
    ui->setupUi(this);
    initialUISetup();
}

ServiceStatusWidget::~ServiceStatusWidget() // Corrected destructor definition
{
    delete ui;
}

void ServiceStatusWidget::initialUISetup()
{
    updateLastRefreshTime();
    on_checkBoxDetailedView_toggled(ui->checkBoxDetailedView->isChecked());
    updateOverallStatus();
    updateCoreProtectionStatus();
    updateTechnicalComponentsStatus();
    updateOtherAdditionsStatus();
}

void ServiceStatusWidget::on_buttonRefresh_clicked()
{
    qDebug() << "ServiceStatusWidget: Refresh button clicked.";
    updateOverallStatus();
    updateCoreProtectionStatus();
    updateTechnicalComponentsStatus();
    updateOtherAdditionsStatus();
    updateLastRefreshTime();
}

void ServiceStatusWidget::on_checkBoxDetailedView_toggled(bool checked)
{
    qDebug() << "ServiceStatusWidget: Detailed view toggled:" << checked;
    ui->groupBoxTechnicalComponents->setVisible(checked);
    if (checked) {
        populateDynamicApiKeys();
        populateDynamicContainers();
        populateDynamicImages();
    } else {
        // Optionally clear dynamic content
    }
}

void ServiceStatusWidget::updateLastRefreshTime()
{
    ui->labelLastRefreshTime->setText(QString::fromUtf8("Last Refresh: %1").arg(QDateTime::currentDateTime().toString(QString::fromUtf8("dd.MM.yyyy hh:mm:ss"))));
}

void ServiceStatusWidget::updateOverallStatus()
{
    qDebug() << "ServiceStatusWidget: Updating overall status section...";
    // Placeholder: ui->labelOverallStatusValue->setText("All systems operational.");
}

void ServiceStatusWidget::updateCoreProtectionStatus()
{
    qDebug() << "ServiceStatusWidget: Updating core protection status section...";
    // Placeholder: ui->labelRealTimeStatus->setText("Active");
}

void ServiceStatusWidget::updateTechnicalComponentsStatus()
{
    qDebug() << "ServiceStatusWidget: Updating technical components section...";
    if (ui->checkBoxDetailedView->isChecked()) {
        populateDynamicApiKeys();
        populateDynamicContainers();
        populateDynamicImages();
    }
}

void ServiceStatusWidget::updateOtherAdditionsStatus()
{
    qDebug() << "ServiceStatusWidget: Updating other additions status section...";
}

void ServiceStatusWidget::populateDynamicApiKeys()
{
    qDebug() << "ServiceStatusWidget: Populating dynamic API keys...";
    // Placeholder: Clear and populate ui->dynamicApiKeysLayout
    // ui->frameApiExample->setVisible(false); // Hide example if real data is populated
}

void ServiceStatusWidget::populateDynamicContainers()
{
    qDebug() << "ServiceStatusWidget: Populating dynamic container statuses...";
    // Placeholder: Clear and populate ui->dynamicContainersLayout
    // ui->frameContainerExample->setVisible(false); // Hide example
}

void ServiceStatusWidget::populateDynamicImages()
{
    qDebug() << "ServiceStatusWidget: Populating dynamic image statuses...";
    // Placeholder: Clear and populate ui->dynamicImagesLayout
    // ui->frameImageExample->setVisible(false); // Hide example
}
