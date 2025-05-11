#include "NetworkMonitor.h"
#include <QDateTime>
#include <QRandomGenerator>
#include <QStringLiteral>

NetworkMonitor::NetworkMonitor(QObject *parent)
    : QObject(parent), m_logCounter(0)
{
    m_simulationTimer = new QTimer(this);
    connect(m_simulationTimer, &QTimer::timeout, this, &NetworkMonitor::generateSimulatedLog);
}

void NetworkMonitor::startMonitoring()
{
    m_simulationTimer->start(2000 + (QRandomGenerator::global()->bounded(3000)));
    emit newLogMessage(QStringLiteral("Network monitoring started..."));
}

void NetworkMonitor::stopMonitoring()
{
    m_simulationTimer->stop();
    emit newLogMessage(QStringLiteral("Network monitoring stopped."));
}

void NetworkMonitor::generateSimulatedLog()
{
    m_logCounter++;
    QString log = QStringLiteral("%1: Simulated network event #%2: Connection to 192.168.1.%3 established.")
                      .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")))
                      .arg(m_logCounter)
                      .arg(QRandomGenerator::global()->bounded(255));
    emit newLogMessage(log);

    m_simulationTimer->start(2000 + (QRandomGenerator::global()->bounded(3000)));
}
