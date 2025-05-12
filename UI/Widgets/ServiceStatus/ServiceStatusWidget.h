#ifndef SERVICESTATUSWIDGET_H
#define SERVICESTATUSWIDGET_H

#include <QWidget>
#include <QDateTime>

namespace Ui {
class ServiceStatusWidget; // Corrected class name
}

class ServiceStatusWidget : public QWidget // Corrected class name
{
    Q_OBJECT

public:
    explicit ServiceStatusWidget(QWidget *parent = nullptr); // Corrected constructor name
    ~ServiceStatusWidget(); // Corrected destructor name

private slots:
    void on_buttonRefresh_clicked();
    void on_checkBoxDetailedView_toggled(bool checked);

private:
    Ui::ServiceStatusWidget *ui; // Corrected UI class type

    void initialUISetup();
    void updateOverallStatus();
    void updateCoreProtectionStatus();
    void updateTechnicalComponentsStatus();
    void updateOtherAdditionsStatus();
    void populateDynamicApiKeys();
    void populateDynamicContainers();
    void populateDynamicImages();
    void updateLastRefreshTime();
};

#endif // SERVICESTATUSWIDGET_H