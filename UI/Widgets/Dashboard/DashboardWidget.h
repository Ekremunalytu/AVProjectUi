#ifndef  DASHBOARDWIDGET_H
#define  DASHBOARDWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class DashboardWidget; }
QT_END_NAMESPACE
class DashboardWidget : public QWidget {
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget *parent = nullptr);
    ~DashboardWidget();

private:
    Ui::DashboardWidget *ui;
};

#endif // DASHBOARDWIDGET_H