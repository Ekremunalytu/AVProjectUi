#include "NetworkMonitor.h"
#include <QDateTime>
#include <QStringLiteral>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QProcess>
#include <QStringList>
#include <Qt>  // For Qt::SkipEmptyParts

NetworkMonitor::NetworkMonitor(QObject *parent)
    : QObject(parent), m_isMonitoring(false), m_networkManager(nullptr)
{
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(2000); // Update every 2 seconds
    connect(m_updateTimer, &QTimer::timeout, this, &NetworkMonitor::processNetworkActivity);
    
    m_networkProcess = new QProcess(this);
    connect(m_networkProcess, &QProcess::readyReadStandardOutput, this, &NetworkMonitor::readProcessOutput);
    connect(m_networkProcess, &QProcess::finished, this, &NetworkMonitor::processFinished);

    m_networkManager = new QNetworkAccessManager(this);
}

NetworkMonitor::~NetworkMonitor()
{
    stopMonitoring();
    if (m_networkProcess && m_networkProcess->state() != QProcess::NotRunning) {
        m_networkProcess->kill();
    }
}

void NetworkMonitor::startMonitoring()
{
    if (!m_isMonitoring) {
        m_isMonitoring = true;
        QString startMsg = QStringLiteral("Network monitoring started...");
        emit newLogMessage(startMsg);
        emit monitoringStateChanged(true, startMsg); // Yeni sinyal burada yayınlanıyor
        
        // Start the network process for real-time monitoring
        startNetworkProcess();
        
        // Also start the timer for periodic updates
        m_updateTimer->start();
    }
}

void NetworkMonitor::stopMonitoring()
{
    if (m_isMonitoring) {
        // İlk önce flag'i değiştir - processFinished'ın yeniden başlatmaması için
        m_isMonitoring = false;
        QString stopMsg = QStringLiteral("Network monitoring stopping - timer stopped...");
        emit newLogMessage(stopMsg);
        // Timer'ı durdur
        m_updateTimer->stop();
        // emit newLogMessage(QStringLiteral("Network monitoring stopping - timer stopped...")); // Tekrar yayınlamaya gerek yok
        
        // Process'i sonlandır - radıkal bir yaklaşım kullanıyoruz
        if (m_networkProcess) {
            // Önce terminat etmeyi dene
            if (m_networkProcess->state() != QProcess::NotRunning) {
                emit newLogMessage(QStringLiteral("Attempting to terminate network process..."));
                m_networkProcess->terminate();
                
                if (!m_networkProcess->waitForFinished(1000)) {
                    emit newLogMessage(QStringLiteral("Force killing network process..."));
                    m_networkProcess->kill();
                    m_networkProcess->waitForFinished(1000);
                }
            }
            
            // Process'i tamamen yenile
            m_networkProcess->disconnect(); // Bağlantıları kopar
            delete m_networkProcess;
            m_networkProcess = new QProcess(this);
            connect(m_networkProcess, &QProcess::readyReadStandardOutput, this, &NetworkMonitor::readProcessOutput);
            connect(m_networkProcess, &QProcess::finished, this, &NetworkMonitor::processFinished);
            
            emit newLogMessage(QStringLiteral("Network process reset."));
        }
        
        QString stoppedCompletelyMsg = QStringLiteral("Network monitoring stopped completely.");
        emit newLogMessage(stoppedCompletelyMsg);
        emit monitoringStateChanged(false, stoppedCompletelyMsg); // Yeni sinyal burada yayınlanıyor
    }
}

bool NetworkMonitor::isMonitoring() const
{
    return m_isMonitoring;
}

void NetworkMonitor::processNetworkActivity()
{
    // Get all network interfaces
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    
    for (const QNetworkInterface &iface : interfaces) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) && 
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            
            QString interfaceName = iface.humanReadableName();
            QString interfaceHardwareAddr = iface.hardwareAddress();
            
            // Log interface information
            if (!interfaceHardwareAddr.isEmpty()) {
                emit newLogMessage(QStringLiteral("Interface: %1, MAC: %2, Status: Up")
                    .arg(interfaceName)
                    .arg(interfaceHardwareAddr));
                
                // Log IP addresses
                const QList<QNetworkAddressEntry> addresses = iface.addressEntries();
                for (const QNetworkAddressEntry &entry : addresses) {
                    QHostAddress addr = entry.ip();
                    if (!addr.isNull()) {
                        QString ipString = addr.toString();
                        QString netmaskString = entry.netmask().toString();
                        
                        emit newLogMessage(QStringLiteral("   IP: %1, Netmask: %2")
                            .arg(ipString)
                            .arg(netmaskString));
                    }
                }
            }
        }
    }
}

void NetworkMonitor::startNetworkProcess()
{
    // Eğer izleme aktif değilse, süreci başlatma
    if (!m_isMonitoring) {
        emit newLogMessage(QStringLiteral("Not starting network process - monitoring is inactive."));
        return;
    }
    
    // Log ekle
    emit newLogMessage(QStringLiteral("Starting network process..."));
    
#ifdef Q_OS_WIN
    // For Windows, netstat can be used
    m_networkProcess->start(QStringLiteral("netstat"), QStringList() << QStringLiteral("-a") << QStringLiteral("-b") << QStringLiteral("-o") << QStringLiteral("-n"));
#elif defined(Q_OS_MAC)
    // For macOS, use lsof with a shorter timeout instead of continuous running
    // -i network connections, -nP numeric ports, grep filter
    m_networkProcess->start(QStringLiteral("sh"), QStringList() << QStringLiteral("-c") << QStringLiteral("lsof -i -nP | grep -E 'ESTABLISHED|LISTEN|TCP|UDP'"));
#else
    // For Linux/Unix systems, use ss or netstat
    m_networkProcess->start(QStringLiteral("ss"), QStringList() << QStringLiteral("-tunap"));
#endif
}

void NetworkMonitor::readProcessOutput()
{
    QByteArray output = m_networkProcess->readAllStandardOutput();
    if (!output.isEmpty()) {
        QString outputStr = QString::fromUtf8(output);
        QStringList lines = outputStr.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        
        for (const QString &line : lines) {
            if (line.contains(QStringLiteral("ESTABLISHED")) || 
                line.contains(QStringLiteral("LISTEN")) || 
                line.contains(QStringLiteral("TCP")) || 
                line.contains(QStringLiteral("UDP"))) {
                
                emit newLogMessage(line.trimmed());
            }
        }
    }
}

void NetworkMonitor::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // Proses sonlandığında izleme durumunu çift kontrol et
    if (m_isMonitoring) {
        // Ek log ekleyelim - hata ayıklama için faydalı olabilir
        QString restartMsg = QStringLiteral("Network process finished. Restarting after delay...");
        emit newLogMessage(restartMsg);
        // emit monitoringStateChanged(m_isMonitoring, restartMsg); // Süreç yeniden başlarken durum değişmedi
        
        // Sadece izleme gerçekten aktifse yeniden başlat
        // 3 saniye sonra bir kez daha kontrol et - bu sürede stopMonitoring çağrılmış olabilir
        QTimer::singleShot(1500, this, [this]() {
            if (m_isMonitoring) {
                emit newLogMessage(QStringLiteral("Restarting network monitoring process..."));
                startNetworkProcess();
            } else {
                QString remainsStoppedMsg = QStringLiteral("Network monitoring remains stopped.");
                emit newLogMessage(remainsStoppedMsg);
                // emit monitoringStateChanged(false, remainsStoppedMsg); // Zaten false, tekrar yayınlamaya gerek yok
            }
        });
        
        if (exitCode != 0) {
            QString exitCodeMsg = QStringLiteral("Network process exited with code %1").arg(exitCode);
            emit newLogMessage(exitCodeMsg);
            // emit monitoringStateChanged(m_isMonitoring, exitCodeMsg); // Durum değişmedi
        }
    } else {
        QString stoppedProcessFinishedMsg = QStringLiteral("Network monitoring stopped - process finished.");
        emit newLogMessage(stoppedProcessFinishedMsg);
        // emit monitoringStateChanged(false, stoppedProcessFinishedMsg); // Zaten false
    }
}

void NetworkMonitor::readNetworkData()
{
    // This method is intended to read data from network requests if you use QNetworkAccessManager
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    // Read the response data
    QByteArray responseData = reply->readAll();
    if (!responseData.isEmpty()) {
        emit newLogMessage(QStringLiteral("Received network data: %1 bytes").arg(responseData.size()));
    }

    reply->deleteLater();
}

void NetworkMonitor::handleNetworkError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        QString errorMessage = reply->errorString();
        emit newLogMessage(QStringLiteral("Network error occurred: %1").arg(errorMessage));
        reply->deleteLater();
    } else {
        emit newLogMessage(QStringLiteral("Unknown network error code: %1").arg(static_cast<int>(error)));
    }
}

bool NetworkMonitor::captureNetworkInterfaces()
{
    m_interfaces = QNetworkInterface::allInterfaces();
    return !m_interfaces.isEmpty();
}
