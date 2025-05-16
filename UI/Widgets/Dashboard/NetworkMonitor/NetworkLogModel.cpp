#include "NetworkLogModel.h"
#include <QRegularExpression>
#include <QStringLiteral>

using namespace Qt::StringLiterals;

NetworkLogModel::NetworkLogModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_isFiltered(false)
    , m_activeConnections(0)
    , m_currentFilterProtocol(QString())
    , m_currentFilterIpAddress(QString())
    , m_currentFilterPort(0)
{
}

int NetworkLogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_isFiltered ? m_filteredLogEntries.size() : m_allLogEntries.size();
}

int NetworkLogModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 7; // Timestamp, Protocol, Source IP:Port, Destination IP:Port, Status, Message
}

QVariant NetworkLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const auto &entries = m_isFiltered ? m_filteredLogEntries : m_allLogEntries;
    
    if (index.row() >= entries.size() || index.row() < 0)
        return QVariant();

    const auto &entry = entries.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: // Timestamp
                return entry.timestamp.toString(u"hh:mm:ss.zzz"_s);
            case 1: // Protocol
                return entry.protocol;
            case 2: // Source IP
                return entry.sourceIp;
            case 3: // Source Port
                return entry.sourcePort > 0 ? QString::number(entry.sourcePort) : QString();
            case 4: // Destination IP
                return entry.destinationIp;
            case 5: // Destination Port
                return entry.destinationPort > 0 ? QString::number(entry.destinationPort) : QString();
            case 6: // Status
                return entry.status;
        }
    } else if (role == Qt::ForegroundRole) {
        return entry.color;
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == 3 || index.column() == 5) // Ports
            return Qt::AlignRight;
        return Qt::AlignLeft;
    } else if (role == Qt::ToolTipRole) {
        return entry.message;
    }

    return QVariant();
}

QVariant NetworkLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return u"Zaman"_s;
            case 1: return u"Protokol"_s;
            case 2: return u"Kaynak IP"_s;
            case 3: return u"K. Port"_s;
            case 4: return u"Hedef IP"_s;
            case 5: return u"H. Port"_s;
            case 6: return u"Durum"_s;
            default: return QVariant();
        }
    }
    
    return QVariant();
}

void NetworkLogModel::addLogEntry(const QString &logMessage)
{
    NetworkLogEntry entry = parseLogMessage(logMessage);
    
    beginInsertRows(QModelIndex(), m_allLogEntries.size(), m_allLogEntries.size());
    m_allLogEntries.append(entry);
    endInsertRows();
    
    // If connection is established, increment active connections
    if (entry.status.contains(u"ESTABLISHED"_s, Qt::CaseInsensitive)) {
        m_activeConnections++;
    }
    
    // Apply filter if needed
    if (m_isFiltered) {
        // Mevcut filtreleri kullanarak yeniden uygula
        applyFilter(m_currentFilterProtocol, m_currentFilterIpAddress, m_currentFilterPort);
    }
}

void NetworkLogModel::clearLogs()
{
    beginResetModel();
    m_allLogEntries.clear();
    m_filteredLogEntries.clear();
    m_activeConnections = 0;
    endResetModel();
}

int NetworkLogModel::logCount() const
{
    return m_allLogEntries.size();
}

int NetworkLogModel::activeConnectionCount() const
{
    return m_activeConnections;
}

void NetworkLogModel::applyFilter(const QString &protocol, const QString &ipAddress, int port)
{
    beginResetModel();
    
    // Filtre kriterlerini kaydet
    m_currentFilterProtocol = protocol;
    m_currentFilterIpAddress = ipAddress;
    m_currentFilterPort = port;
    
    m_filteredLogEntries.clear();
    
    if (protocol.isEmpty() && ipAddress.isEmpty() && port <= 0) {
        m_isFiltered = false;
    } else {
        m_isFiltered = true;
        
        for (const auto &entry : m_allLogEntries) {
            bool match = true;
            
            if (!protocol.isEmpty() && protocol != u"Tüm Protokoller"_s) {
                match = match && entry.protocol.contains(protocol, Qt::CaseInsensitive);
            }
            
            if (!ipAddress.isEmpty()) {
                match = match && (entry.sourceIp.contains(ipAddress) || entry.destinationIp.contains(ipAddress));
            }
            
            if (port > 0) {
                match = match && (entry.sourcePort == port || entry.destinationPort == port);
            }
            
            if (match) {
                m_filteredLogEntries.append(entry);
            }
        }
    }
    
    endResetModel();
}

void NetworkLogModel::resetFilter()
{
    beginResetModel();
    m_isFiltered = false;
    m_filteredLogEntries.clear();
    endResetModel();
}

NetworkLogEntry NetworkLogModel::parseLogMessage(const QString &logMessage)
{
    NetworkLogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.message = logMessage;
    entry.color = determineLogColor(logMessage);
    
    // Extract protocol
    entry.protocol = extractProtocol(logMessage);
    
    // Extract IP addresses
    QStringList ipAddresses = extractIpAddresses(logMessage);
    if (ipAddresses.size() > 0) {
        entry.sourceIp = ipAddresses.first();
    }
    if (ipAddresses.size() > 1) {
        entry.destinationIp = ipAddresses.at(1);
    }
    
    // Extract ports
    QList<int> ports = extractPorts(logMessage);
    if (ports.size() > 0) {
        entry.sourcePort = ports.first();
    }
    if (ports.size() > 1) {
        entry.destinationPort = ports.at(1);
    }
    
    // Extract status
    entry.status = extractStatus(logMessage);
    
    return entry;
}

QColor NetworkLogModel::determineLogColor(const QString &logMessage)
{
    if (logMessage.contains(u"ERROR"_s, Qt::CaseInsensitive) || 
        logMessage.contains(u"FAILED"_s, Qt::CaseInsensitive) ||
        logMessage.contains(u"killed"_s, Qt::CaseInsensitive)) {
        return QColor(u"#FF5252"_s); // Red for errors
    } else if (logMessage.contains(u"WARNING"_s, Qt::CaseInsensitive) ||
              logMessage.contains(u"polling"_s, Qt::CaseInsensitive) ||
              logMessage.contains(u"attempt"_s, Qt::CaseInsensitive)) {
        return QColor(u"#FFA726"_s); // Orange for warnings
    } else if (logMessage.contains(u"INFO"_s, Qt::CaseInsensitive) ||
              logMessage.contains(u"Interface"_s, Qt::CaseInsensitive)) {
        return QColor(u"#2196F3"_s); // Blue for info
    } else if (logMessage.contains(u"CONNECTION"_s, Qt::CaseInsensitive) || 
              logMessage.contains(u"CONNECTED"_s, Qt::CaseInsensitive) ||
              logMessage.contains(u"ESTABLISHED"_s, Qt::CaseInsensitive) ||
              logMessage.contains(u"established"_s, Qt::CaseInsensitive)) {
        return QColor(u"#4CAF50"_s); // Green for connections
    } else if (logMessage.contains(u"IP:"_s) ||
              logMessage.contains(u"MAC:"_s)) {
        return QColor(u"#9C27B0"_s); // Purple for network addresses
    } else if (logMessage.contains(u"TCP"_s) ||
              logMessage.contains(u"UDP"_s) ||
              logMessage.contains(u"LISTEN"_s)) {
        return QColor(u"#00BCD4"_s); // Cyan for network protocols
    }
    
    return QColor(u"#E0E0E0"_s); // Default light gray
}

QString NetworkLogModel::extractProtocol(const QString &logMessage)
{
    if (logMessage.contains(u"TCP"_s, Qt::CaseInsensitive)) {
        return u"TCP"_s;
    } else if (logMessage.contains(u"UDP"_s, Qt::CaseInsensitive)) {
        return u"UDP"_s;
    } else if (logMessage.contains(u"ICMP"_s, Qt::CaseInsensitive)) {
        return u"ICMP"_s;
    } else if (logMessage.contains(u"HTTP"_s, Qt::CaseInsensitive) && !logMessage.contains(u"HTTPS"_s, Qt::CaseInsensitive)) {
        return u"HTTP"_s;
    } else if (logMessage.contains(u"HTTPS"_s, Qt::CaseInsensitive)) {
        return u"HTTPS"_s;
    } else if (logMessage.contains(u"DNS"_s, Qt::CaseInsensitive)) {
        return u"DNS"_s;
    }
    
    return u"Diğer"_s;
}

QStringList NetworkLogModel::extractIpAddresses(const QString &logMessage)
{
    QStringList result;
    
    // Regular expression for IPv4 addresses
    QRegularExpression ipv4Regex(u"\\b(?:\\d{1,3}\\.){3}\\d{1,3}\\b"_s);
    QRegularExpressionMatchIterator ipv4Matches = ipv4Regex.globalMatch(logMessage);
    
    while (ipv4Matches.hasNext()) {
        QRegularExpressionMatch match = ipv4Matches.next();
        result.append(match.captured(0));
    }
    
    return result;
}

QList<int> NetworkLogModel::extractPorts(const QString &logMessage)
{
    QList<int> result;
    
    // Look for port indicators
    QRegularExpression portRegex(u"\\b(?:port|PORT|Port|:)\\s*(\\d+)\\b"_s);
    QRegularExpressionMatchIterator portMatches = portRegex.globalMatch(logMessage);
    
    while (portMatches.hasNext()) {
        QRegularExpressionMatch match = portMatches.next();
        bool ok;
        int port = match.captured(1).toInt(&ok);
        if (ok && port > 0 && port <= 65535) {
            result.append(port);
        }
    }
    
    // Also match typical port patterns like 192.168.1.1:8080
    QRegularExpression ipPortRegex(u"\\b(?:\\d{1,3}\\.){3}\\d{1,3}:(\\d+)\\b"_s);
    QRegularExpressionMatchIterator ipPortMatches = ipPortRegex.globalMatch(logMessage);
    
    while (ipPortMatches.hasNext()) {
        QRegularExpressionMatch match = ipPortMatches.next();
        bool ok;
        int port = match.captured(1).toInt(&ok);
        if (ok && port > 0 && port <= 65535 && !result.contains(port)) {
            result.append(port);
        }
    }
    
    return result;
}

QString NetworkLogModel::extractStatus(const QString &logMessage)
{
    if (logMessage.contains(u"ESTABLISHED"_s, Qt::CaseInsensitive)) {
        return u"ESTABLISHED"_s;
    } else if (logMessage.contains(u"LISTEN"_s, Qt::CaseInsensitive)) {
        return u"LISTEN"_s;
    } else if (logMessage.contains(u"SYN_SENT"_s, Qt::CaseInsensitive)) {
        return u"SYN_SENT"_s;
    } else if (logMessage.contains(u"TIME_WAIT"_s, Qt::CaseInsensitive)) {
        return u"TIME_WAIT"_s;
    } else if (logMessage.contains(u"CLOSE_WAIT"_s, Qt::CaseInsensitive)) {
        return u"CLOSE_WAIT"_s;
    } else if (logMessage.contains(u"FIN_WAIT"_s, Qt::CaseInsensitive)) {
        return u"FIN_WAIT"_s;
    } else if (logMessage.contains(u"CLOSED"_s, Qt::CaseInsensitive)) {
        return u"CLOSED"_s;
    } else if (logMessage.contains(u"ERROR"_s, Qt::CaseInsensitive)) {
        return u"ERROR"_s;
    }
    
    return u""_s;
}
