#ifndef NETWORKMONITOR_H
#define NETWORKMONITOR_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QProcess>

class NetworkMonitor : public QObject
{
    Q_OBJECT
public:
    explicit NetworkMonitor(QObject *parent = nullptr);
    ~NetworkMonitor();
    void startMonitoring(); // To start monitoring network traffic
    void stopMonitoring();  // To stop monitoring network traffic
    bool isMonitoring() const; // To check if monitoring is active

signals:
    void newLogMessage(const QString& message);
    void monitoringStateChanged(bool isMonitoring, const QString& statusMessage); // Yeni sinyal

private slots:
    void processNetworkActivity();
    void readNetworkData();
    void handleNetworkError(QNetworkReply::NetworkError error);
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void readProcessOutput();

private:
    bool captureNetworkInterfaces();
    void startNetworkProcess();
    
    QTimer *m_updateTimer;
    QNetworkAccessManager *m_networkManager;
    QList<QNetworkInterface> m_interfaces;
    QProcess *m_networkProcess;
    bool m_isMonitoring;
    QMap<QString, QVariantMap> m_lastStats; // Store last interface statistics
};

#endif // NETWORKMONITOR_H
