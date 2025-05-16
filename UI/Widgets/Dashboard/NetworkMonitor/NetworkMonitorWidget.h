#ifndef NETWORKMONITORWIDGET_H
#define NETWORKMONITORWIDGET_H

#include <QWidget>
#include <QTimer>
#include "Network/Monitor/NetworkMonitor.h"
#include "NetworkLogModel.h"

namespace Ui {
class NetworkMonitorWidget;
}

/**
 * @brief The NetworkMonitorWidget class provides a UI for monitoring network activity
 * 
 * This widget displays real-time network logs and allows starting/stopping the network
 * monitoring functionality. It integrates with the NetworkMonitor class to receive and display
 * network events.
 */
class NetworkMonitorWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a NetworkMonitorWidget.
     * @param parent The parent widget.
     */
    explicit NetworkMonitorWidget(QWidget *parent = nullptr);
    
    /**
     * @brief Destroys the NetworkMonitorWidget and frees resources.
     */
    ~NetworkMonitorWidget();

public slots:
    /**
     * @brief Adds a new log message to the network log display
     * @param logMessage The message to be displayed
     */
    void appendNetworkLog(const QString& logMessage);

    /**
     * @brief Clears all network logs from the display
     */
    void clearLogs();

public slots:
    /**
     * @brief Handles the start monitoring button click
     */
    void onStartMonitoringClicked();
    
    /**
     * @brief Handles the stop monitoring button click
     */
    void onStopMonitoringClicked();
    
    /**
     * @brief Handles the clear logs button click
     */
    void onClearLogsClicked();
    
    /**
     * @brief Updates the connection status indicator
     */
    void updateStatusIndicator();
    
    /**
     * @brief Handles filter apply button click
     */
    void onApplyFilterClicked();
    
    /**
     * @brief Handles filter reset button click
     */
    void onResetFilterClicked();
    
    /**
     * @brief Handles export logs button click
     */
    void onExportLogsClicked();
    
    /**
     * @brief Updates active connections count display
     */
    void updateActiveConnections();

private slots: // private slots olarak değiştirildi
    void handleMonitoringStateChanged(bool isMonitoring, const QString& statusMessage); // Yeni slot

private:
    Ui::NetworkMonitorWidget *ui;
    NetworkMonitor *m_networkMonitor;
    NetworkLogModel *m_logModel;
    QTimer *m_statusUpdateTimer;
    QTimer *m_connectionUpdateTimer;
    int m_logCount;
    bool m_isActive;

    /**
     * @brief Sets up the UI connections
     */
    void setupConnections();
    
    /**
     * @brief Sets up the table view
     */
    void setupTableView();
};

#endif // NETWORKMONITORWIDGET_H
