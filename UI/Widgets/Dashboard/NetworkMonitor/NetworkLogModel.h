#ifndef NETWORKLOGMODEL_H
#define NETWORKLOGMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QDateTime>
#include <QColor>

/**
 * @brief The NetworkLogEntry struct represents a single network log entry with detailed information
 */
struct NetworkLogEntry {
    QDateTime timestamp;
    QString protocol;
    QString sourceIp;
    QString destinationIp;
    int sourcePort;
    int destinationPort;
    QString status;
    QString message;
    QColor color;
};

/**
 * @brief The NetworkLogModel class provides a data model for displaying network logs in a QTableView
 */
class NetworkLogModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a NetworkLogModel.
     * @param parent The parent object.
     */
    explicit NetworkLogModel(QObject *parent = nullptr);

    /**
     * @brief Returns the number of rows in the model
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns the number of columns in the model
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns the data at the specified index
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Returns the header data for the specified section
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * @brief Adds a new network log entry to the model
     * @param logMessage Raw log message string to parse
     */
    void addLogEntry(const QString &logMessage);

    /**
     * @brief Clears all log entries from the model
     */
    void clearLogs();

    /**
     * @brief Returns the count of log entries
     */
    int logCount() const;

    /**
     * @brief Returns the count of active connections
     */
    int activeConnectionCount() const;

    /**
     * @brief Applies filtering to the log entries
     * @param protocol Protocol filter (empty means all)
     * @param ipAddress IP address filter (empty means all)
     * @param port Port filter (0 means all)
     */
    void applyFilter(const QString &protocol, const QString &ipAddress, int port);

    /**
     * @brief Resets any applied filters
     */
    void resetFilter();

private:
    /**
     * @brief Parses a raw log message into structured data
     */
    NetworkLogEntry parseLogMessage(const QString &logMessage);

    /**
     * @brief Determines the text color based on the log message content
     */
    QColor determineLogColor(const QString &logMessage);

    /**
     * @brief Extracts protocol from log message
     */
    QString extractProtocol(const QString &logMessage);

    /**
     * @brief Extracts IP addresses from log message
     */
    QStringList extractIpAddresses(const QString &logMessage);

    /**
     * @brief Extracts ports from log message
     */
    QList<int> extractPorts(const QString &logMessage);

    /**
     * @brief Extracts connection status from log message
     */
    QString extractStatus(const QString &logMessage);

private:
    QVector<NetworkLogEntry> m_allLogEntries;
    QVector<NetworkLogEntry> m_filteredLogEntries;
    bool m_isFiltered;
    int m_activeConnections;
    
    // Filtre kriterleri için saklamak üzere değişkenler
    QString m_currentFilterProtocol;
    QString m_currentFilterIpAddress;
    int m_currentFilterPort;
};

#endif // NETWORKLOGMODEL_H
