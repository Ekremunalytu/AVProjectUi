\
#ifndef NETWORKMONITOR_H
#define NETWORKMONITOR_H

#include <QObject>
#include <QString>
#include <QTimer> // For simulating network logs

class NetworkMonitor : public QObject
{
    Q_OBJECT
public:
    explicit NetworkMonitor(QObject *parent = nullptr);
    void startMonitoring(); // To start generating/capturing logs
    void stopMonitoring();  // To stop generating/capturing logs
    bool isMonitoring() const; // To check if monitoring is active

signals:
    void newLogMessage(const QString& message);

private slots:
    void generateSimulatedLog(); // Slot to periodically generate a log

private:
    QTimer *m_simulationTimer;
    int m_logCounter;
};

#endif // NETWORKMONITOR_H
