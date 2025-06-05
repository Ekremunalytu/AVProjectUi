/**
 * @file ScanHistoryModel.cpp
 * @brief Implementation of the ScanHistoryModel and ScanHistoryFilterModel classes
 * @author Ekrem Ünal  
 * @date 30.05.2025
 */

#include "ScanHistoryModel.h"
#include "../DbManager/DbManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QIcon>
#include <QColor>
#include <QFileInfo>
#include <QTextStream>
#include <QApplication>
#include <QBrush>
#include <QStyle>

using namespace Qt::StringLiterals;

ScanHistoryModel::ScanHistoryModel(DbManager* dbManager, QObject* parent)
    : QAbstractTableModel(parent)
    , m_dbManager(dbManager)
{
    if (m_dbManager) {
        createTableIfNotExists();
        refreshData();
    }
}

int ScanHistoryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_records.size();
}

int ScanHistoryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant ScanHistoryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_records.size())
        return QVariant();

    const ScanHistoryRecord& record = m_records.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case ColumnDateTime:
            return record.scanDateTime.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
        case ColumnFileName:
            return record.fileName;
        case ColumnFilePath:
            return record.filePath;
        case ColumnScanType:
            return scanTypeToString(record.scanType);
        case ColumnStatus:
            return scanStatusToString(record.scanStatus);
        case ColumnRiskLevel:
            return riskLevelToString(record.riskLevel);
        case ColumnActionStatus:
            return actionStatusToString(record.actionStatus);
        case ColumnThreatName:
            return record.threatName.isEmpty() ? tr("No threat detected") : record.threatName;
        case ColumnFileSize:
            return QLocale().formattedDataSize(record.fileSize);
        }
        break;

    case Qt::DecorationRole:
        if (index.column() == ColumnRiskLevel) {
            return getRiskLevelIcon(record.riskLevel);
        }
        break;

    case Qt::BackgroundRole:
        if (index.column() == ColumnRiskLevel) {
            return QBrush(getRiskLevelColor(record.riskLevel));
        }
        break;

    case Qt::ToolTipRole:
        switch (index.column()) {
        case ColumnDateTime:
            return tr("Scan performed on: %1").arg(record.scanDateTime.toString(QStringLiteral("dddd, MMMM d, yyyy h:mm:ss AP")));
        case ColumnFileName:
            return tr("File: %1\nPath: %2\nSize: %3")
                   .arg(record.fileName)
                   .arg(record.filePath)
                   .arg(QLocale().formattedDataSize(record.fileSize));
        case ColumnScanType:
            return tr("Scan method used: %1").arg(scanTypeToString(record.scanType));
        case ColumnStatus:
            return tr("Scan status: %1").arg(scanStatusToString(record.scanStatus));
        case ColumnRiskLevel:
            return tr("Threat level: %1").arg(riskLevelToString(record.riskLevel));
        case ColumnActionStatus:
            return tr("Action taken: %1").arg(actionStatusToString(record.actionStatus));
        case ColumnThreatName:
            return record.threatName.isEmpty() ? tr("No threat detected") : tr("Threat: %1").arg(record.threatName);
        }
        break;

    case Qt::UserRole: // Return the raw record data
        return QVariant::fromValue(record);
    }

    return QVariant();
}

QVariant ScanHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
    case ColumnDateTime:
        return tr("Date & Time");
    case ColumnFileName:
        return tr("File Name");
    case ColumnFilePath:
        return tr("File Path");
    case ColumnScanType:
        return tr("Scan Type");
    case ColumnStatus:
        return tr("Status");
    case ColumnRiskLevel:
        return tr("Risk Level");
    case ColumnActionStatus:
        return tr("Action");
    case ColumnThreatName:
        return tr("Threat Name");
    case ColumnFileSize:
        return tr("File Size");
    default:
        return QVariant();
    }
}

void ScanHistoryModel::refreshData()
{
    if (!m_dbManager) {
        qWarning() << "No database manager available";
        return;
    }

    beginResetModel();
    m_records.clear();

    QSqlQuery query(m_dbManager->getDatabase());
    query.prepare(u"SELECT id, file_name, file_path, file_hash, scan_datetime, scan_type, scan_status, "
                  u"risk_level, action_status, threat_name, scan_results, file_size "
                  u"FROM scan_history ORDER BY scan_datetime DESC"_s);

    if (!query.exec()) {
        qWarning() << "Failed to fetch scan history:" << query.lastError().text();
        endResetModel();
        return;
    }

    while (query.next()) {
        ScanHistoryRecord record;
        record.id = query.value(0).toInt();
        record.fileName = query.value(1).toString();
        record.filePath = query.value(2).toString();
        record.fileHash = query.value(3).toString();
        record.scanDateTime = query.value(4).toDateTime();
        record.scanType = static_cast<ScanType>(query.value(5).toInt());
        record.scanStatus = static_cast<HistoryScanStatus>(query.value(6).toInt());
        record.riskLevel = static_cast<RiskLevel>(query.value(7).toInt());
        record.actionStatus = static_cast<ActionStatus>(query.value(8).toInt());
        record.threatName = query.value(9).toString();
        record.scanResults = query.value(10).toString();
        record.fileSize = query.value(11).toLongLong();

        m_records.append(record);
    }

    endResetModel();
    emit dataRefreshed();
}

bool ScanHistoryModel::addScanRecord(const ScanHistoryRecord& record)
{
    if (!m_dbManager) {
        qWarning() << "No database manager available";
        return false;
    }

    QSqlQuery query(m_dbManager->getDatabase());
    query.prepare(u"INSERT INTO scan_history (file_name, file_path, file_hash, scan_datetime, scan_type, "
                  u"scan_status, risk_level, action_status, threat_name, scan_results, file_size) "
                  u"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"_s);

    query.addBindValue(record.fileName);
    query.addBindValue(record.filePath);
    query.addBindValue(record.fileHash);
    query.addBindValue(record.scanDateTime);
    query.addBindValue(static_cast<int>(record.scanType));
    query.addBindValue(static_cast<int>(record.scanStatus));
    query.addBindValue(static_cast<int>(record.riskLevel));
    query.addBindValue(static_cast<int>(record.actionStatus));
    query.addBindValue(record.threatName);
    query.addBindValue(record.scanResults);
    query.addBindValue(record.fileSize);

    if (!query.exec()) {
        qWarning() << "Failed to insert scan record:" << query.lastError().text();
        return false;
    }

    // Add to our local cache and update the model
    ScanHistoryRecord newRecord = record;
    newRecord.id = query.lastInsertId().toInt();
    
    beginInsertRows(QModelIndex(), 0, 0);
    m_records.prepend(newRecord);
    endInsertRows();

    emit recordAdded(newRecord);
    return true;
}

bool ScanHistoryModel::removeScanRecord(int recordId)
{
    if (!m_dbManager) {
        qWarning() << "No database manager available";
        return false;
    }

    // Find the record in our cache
    int row = -1;
    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records.at(i).id == recordId) {
            row = i;
            break;
        }
    }

    if (row == -1) {
        qWarning() << "Record with ID" << recordId << "not found";
        return false;
    }

    QSqlQuery query(m_dbManager->getDatabase());
    query.prepare(u"DELETE FROM scan_history WHERE id = ?"_s);
    query.addBindValue(recordId);

    if (!query.exec()) {
        qWarning() << "Failed to delete scan record:" << query.lastError().text();
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_records.removeAt(row);
    endRemoveRows();

    emit recordRemoved(recordId);
    return true;
}

bool ScanHistoryModel::updateActionStatus(int recordId, ActionStatus newStatus)
{
    if (!m_dbManager) {
        qWarning() << "No database manager available";
        return false;
    }

    QSqlQuery query(m_dbManager->getDatabase());
    query.prepare(u"UPDATE scan_history SET action_status = ? WHERE id = ?"_s);
    query.addBindValue(static_cast<int>(newStatus));
    query.addBindValue(recordId);

    if (!query.exec()) {
        qWarning() << "Failed to update action status:" << query.lastError().text();
        return false;
    }

    // Update our local cache
    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records[i].id == recordId) {
            m_records[i].actionStatus = newStatus;
            QModelIndex index = createIndex(i, ColumnActionStatus);
            emit dataChanged(index, index, {Qt::DisplayRole});
            break;
        }
    }

    return true;
}

void ScanHistoryModel::clearAllHistory()
{
    if (!m_dbManager) {
        qWarning() << "No database manager available";
        return;
    }

    QSqlQuery query(m_dbManager->getDatabase());
    if (!query.exec(u"DELETE FROM scan_history"_s)) {
        qWarning() << "Failed to clear scan history:" << query.lastError().text();
        return;
    }

    beginResetModel();
    m_records.clear();
    endResetModel();
}

bool ScanHistoryModel::clearAllRecords()
{
    if (!m_dbManager) {
        qWarning() << "No database manager available";
        return false;
    }

    QSqlQuery query(m_dbManager->getDatabase());
    if (!query.exec(u"DELETE FROM scan_history"_s)) {
        qWarning() << "Failed to clear scan history:" << query.lastError().text();
        return false;
    }

    beginResetModel();
    m_records.clear();
    endResetModel();
    
    return true;
}

ScanHistoryRecord ScanHistoryModel::getRecord(int row) const
{
    if (row >= 0 && row < m_records.size()) {
        return m_records.at(row);
    }
    return ScanHistoryRecord{};
}

QList<ScanHistoryRecord> ScanHistoryModel::getAllRecords() const
{
    return m_records;
}

bool ScanHistoryModel::exportToCSV(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    
    // Write header
    stream << "Date & Time,File Name,File Path,Scan Type,Status,Risk Level,Action,Threat Name,File Size\n";
    
    // Write data
    for (const auto& record : m_records) {
        stream << record.scanDateTime.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")) << ","
               << "\"" << record.fileName << "\","
               << "\"" << record.filePath << "\","
               << scanTypeToString(record.scanType) << ","
               << scanStatusToString(record.scanStatus) << ","
               << riskLevelToString(record.riskLevel) << ","
               << actionStatusToString(record.actionStatus) << ","
               << "\"" << (record.threatName.isEmpty() ? QStringLiteral("No threat detected") : record.threatName) << "\","
               << record.fileSize << "\n";
    }

    return true;
}

bool ScanHistoryModel::createTableIfNotExists()
{
    if (!m_dbManager) {
        qWarning() << "No database manager available";
        return false;
    }

    QSqlQuery query(m_dbManager->getDatabase());
    const QString createTableSQL = QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS scan_history (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            file_name TEXT NOT NULL,
            file_path TEXT NOT NULL,
            file_hash TEXT,
            scan_datetime DATETIME NOT NULL,
            scan_type INTEGER NOT NULL,
            scan_status INTEGER NOT NULL,
            risk_level INTEGER NOT NULL,
            action_status INTEGER NOT NULL,
            threat_name TEXT,
            scan_results TEXT,
            file_size INTEGER NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )");

    if (!query.exec(createTableSQL)) {
        qWarning() << "Failed to create scan_history table:" << query.lastError().text();
        return false;
    }

    return true;
}

QString ScanHistoryModel::scanTypeToString(ScanType type) const
{
    switch (type) {
    case ScanType::Basic:
        return tr("Basic Scan");
    case ScanType::VirusTotal:
        return tr("VirusTotal");
    case ScanType::Docker:
        return tr("Docker Scan");
    case ScanType::Unknown:
    default:
        return tr("Unknown");
    }
}

QString ScanHistoryModel::scanStatusToString(HistoryScanStatus status) const
{
    switch (status) {
    case HistoryScanStatus::Completed:
        return tr("Completed");
    case HistoryScanStatus::Failed:
        return tr("Failed");
    case HistoryScanStatus::Cancelled:
        return tr("Cancelled");
    case HistoryScanStatus::InProgress:
        return tr("In Progress");
    default:
        return tr("Unknown");
    }
}

QString ScanHistoryModel::riskLevelToString(RiskLevel level) const
{
    switch (level) {
    case RiskLevel::None:
        return tr("None");
    case RiskLevel::Low:
        return tr("Low");
    case RiskLevel::Medium:
        return tr("Medium");
    case RiskLevel::High:
        return tr("High");
    case RiskLevel::Critical:
        return tr("Critical");
    default:
        return tr("Unknown");
    }
}

QString ScanHistoryModel::actionStatusToString(ActionStatus status) const
{
    switch (status) {
    case ActionStatus::None:
        return tr("None");
    case ActionStatus::Quarantined:
        return tr("Quarantined");
    case ActionStatus::Deleted:
        return tr("Deleted");
    case ActionStatus::Allowed:
        return tr("Allowed");
    case ActionStatus::Restored:
        return tr("Restored");
    default:
        return tr("Unknown");
    }
}

QIcon ScanHistoryModel::getRiskLevelIcon(RiskLevel level) const
{
    switch (level) {
    case RiskLevel::None:
        return QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
    case RiskLevel::Low:
        return QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
    case RiskLevel::Medium:
        return QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
    case RiskLevel::High:
    case RiskLevel::Critical:
        return QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
    default:
        return QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion);
    }
}

QColor ScanHistoryModel::getRiskLevelColor(RiskLevel level) const
{
    switch (level) {
    case RiskLevel::None:
        return QColor(200, 255, 200); // Light green
    case RiskLevel::Low:
        return QColor(255, 255, 200); // Light yellow
    case RiskLevel::Medium:
        return QColor(255, 230, 200); // Light orange
    case RiskLevel::High:
        return QColor(255, 200, 200); // Light red
    case RiskLevel::Critical:
        return QColor(255, 150, 150); // Dark red
    default:
        return QColor(240, 240, 240); // Light gray
    }
}

// ScanHistoryFilterModel implementation

ScanHistoryFilterModel::ScanHistoryFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
    , m_scanTypeFilter(ScanType::Unknown)
    , m_statusFilter(HistoryScanStatus::Completed)
    , m_riskLevelFilter(RiskLevel::None)
    , m_actionStatusFilter(ActionStatus::None)
    , m_dateFilterEnabled(false)
    , m_scanTypeFilterEnabled(false)
    , m_statusFilterEnabled(false)
    , m_riskLevelFilterEnabled(false)
    , m_actionStatusFilterEnabled(false)
{
    setDynamicSortFilter(true);
}

void ScanHistoryFilterModel::setDateRange(const QDate& startDate, const QDate& endDate)
{
    m_startDate = startDate;
    m_endDate = endDate;
    m_dateFilterEnabled = startDate.isValid() && endDate.isValid();
    beginResetModel();
    endResetModel();
}

void ScanHistoryFilterModel::setScanTypeFilter(ScanType type)
{
    m_scanTypeFilter = type;
    m_scanTypeFilterEnabled = (type != ScanType::Unknown);
    beginResetModel();
    endResetModel();
}

void ScanHistoryFilterModel::setStatusFilter(HistoryScanStatus status)
{
    m_statusFilter = status;
    m_statusFilterEnabled = true;
    beginResetModel();
    endResetModel();
}

void ScanHistoryFilterModel::setRiskLevelFilter(RiskLevel level)
{
    m_riskLevelFilter = level;
    m_riskLevelFilterEnabled = true;
    beginResetModel();
    endResetModel();
}

void ScanHistoryFilterModel::setActionStatusFilter(ActionStatus status)
{
    m_actionStatusFilter = status;
    m_actionStatusFilterEnabled = true;
    beginResetModel();
    endResetModel();
}

void ScanHistoryFilterModel::setSearchText(const QString& text)
{
    m_searchText = text.trimmed();
    beginResetModel();
    endResetModel();
}

void ScanHistoryFilterModel::clearAllFilters()
{
    m_dateFilterEnabled = false;
    m_scanTypeFilterEnabled = false;
    m_statusFilterEnabled = false;
    m_riskLevelFilterEnabled = false;
    m_actionStatusFilterEnabled = false;
    m_searchText.clear();
    beginResetModel();
    endResetModel();
}

bool ScanHistoryFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (!index.isValid())
        return false;

    ScanHistoryRecord record = index.data(Qt::UserRole).value<ScanHistoryRecord>();

    // Date filter
    if (m_dateFilterEnabled) {
        QDate scanDate = record.scanDateTime.date();
        if (scanDate < m_startDate || scanDate > m_endDate)
            return false;
    }

    // Scan type filter
    if (m_scanTypeFilterEnabled && record.scanType != m_scanTypeFilter)
        return false;

    // Status filter
    if (m_statusFilterEnabled && record.scanStatus != m_statusFilter)
        return false;

    // Risk level filter
    if (m_riskLevelFilterEnabled && record.riskLevel != m_riskLevelFilter)
        return false;

    // Action status filter
    if (m_actionStatusFilterEnabled && record.actionStatus != m_actionStatusFilter)
        return false;

    // Search text filter
    if (!m_searchText.isEmpty()) {
        QString searchLower = m_searchText.toLower();
        if (!record.fileName.toLower().contains(searchLower) &&
            !record.filePath.toLower().contains(searchLower) &&
            !record.threatName.toLower().contains(searchLower)) {
            return false;
        }
    }

    return true;
}
