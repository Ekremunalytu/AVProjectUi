#ifndef SCANHISTORYMODEL_H
#define SCANHISTORYMODEL_H

#include <QAbstractTableModel>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <memory>
#include <QIcon>
#include <QColor>

class DbManager;

/**
 * @brief Enum representing different scan types
 */
enum class ScanType {
    Basic = 0,
    VirusTotal,
    Docker,
    Unknown
};

/**
 * @brief Enum representing scan statuses in the history database
 */
enum class HistoryScanStatus {
    Completed = 0,
    Failed,
    Cancelled,
    InProgress
};

/**
 * @brief Enum representing threat risk levels
 */
enum class RiskLevel {
    None = 0,
    Low,
    Medium,
    High,
    Critical
};

/**
 * @brief Enum representing action statuses taken on threats
 */
enum class ActionStatus {
    None = 0,
    Quarantined,
    Deleted,
    Allowed,
    Restored
};

/**
 * @brief Structure representing a scan history record
 */
struct ScanHistoryRecord {
    int id;
    QString fileName;
    QString filePath;
    QString fileHash;
    QDateTime scanDateTime;
    ScanType scanType;
    HistoryScanStatus scanStatus;
    RiskLevel riskLevel;
    ActionStatus actionStatus;
    QString threatName;
    QString scanResults;
    qint64 fileSize;
};

/**
 * @brief Model for displaying scan history data in a table view
 */
class ScanHistoryModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColumnDateTime = 0,
        ColumnFileName,
        ColumnFilePath,
        ColumnScanType,
        ColumnStatus,
        ColumnRiskLevel,
        ColumnActionStatus,
        ColumnThreatName,
        ColumnFileSize,
        ColumnCount
    };

    explicit ScanHistoryModel(DbManager* dbManager, QObject* parent = nullptr);
    
    // QAbstractTableModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    // Data management
    void refreshData();
    bool addScanRecord(const ScanHistoryRecord& record);
    bool removeScanRecord(int recordId);
    bool updateActionStatus(int recordId, ActionStatus newStatus);
    void clearAllHistory();
    
    // Data access
    ScanHistoryRecord getRecord(int row) const;
    QList<ScanHistoryRecord> getAllRecords() const;
    
    // String conversion methods
    QString scanTypeToString(ScanType type) const;
    QString scanStatusToString(HistoryScanStatus status) const;
    QString riskLevelToString(RiskLevel level) const;
    QString actionStatusToString(ActionStatus status) const;
    
    // Additional methods
    bool clearAllRecords();
    
    // Export functionality
    bool exportToCSV(const QString& filePath) const;

signals:
    void dataRefreshed();
    void recordAdded(const ScanHistoryRecord& record);
    void recordRemoved(int recordId);

private:
    bool createTableIfNotExists();
    QIcon getRiskLevelIcon(RiskLevel level) const;
    QColor getRiskLevelColor(RiskLevel level) const;
    
    DbManager* m_dbManager;
    QList<ScanHistoryRecord> m_records;
};

/**
 * @brief Proxy model for filtering and sorting scan history
 */
class ScanHistoryFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ScanHistoryFilterModel(QObject* parent = nullptr);
    
    // Filter methods
    void setDateRange(const QDate& startDate, const QDate& endDate);
    void setScanTypeFilter(ScanType type);
    void setStatusFilter(HistoryScanStatus status);
    void setRiskLevelFilter(RiskLevel level);
    void setActionStatusFilter(ActionStatus status);
    void setSearchText(const QString& text);
    void clearAllFilters();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QDate m_startDate;
    QDate m_endDate;
    ScanType m_scanTypeFilter;
    HistoryScanStatus m_statusFilter;
    RiskLevel m_riskLevelFilter;
    ActionStatus m_actionStatusFilter;
    QString m_searchText;
    bool m_dateFilterEnabled;
    bool m_scanTypeFilterEnabled;
    bool m_statusFilterEnabled;
    bool m_riskLevelFilterEnabled;
    bool m_actionStatusFilterEnabled;
};

#endif // SCANHISTORYMODEL_H
