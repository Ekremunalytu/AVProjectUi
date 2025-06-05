//
// Created by Ekrem Ünal on 11.05.2025.
//

#include "HistoryWidget.h"
#include "ui_HistoryWidget.h"
#include "../../../storage/database/Models/ScanHistoryModel.h"
#include "../../../storage/database/DbManager/DbManager.h"
#include "../../../storage/database/DatabaseService/DatabaseService.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QFileInfo>
#include <QDebug>

// Using declarations for enum types
using ScanType = ScanType;
using ScanStatus = HistoryScanStatus;
using RiskLevel = RiskLevel;
using ActionStatus = ActionStatus;

HistoryWidget::HistoryWidget(DbManager* dbManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HistoryWidget),
    m_model(nullptr),
    m_filterModel(nullptr),
    m_dbManager(dbManager)
{
    ui->setupUi(this);

    // Get database manager if not provided
    if (!m_dbManager) {
        DatabaseService& dbService = DatabaseService::getInstance();
        m_dbManager = dbService.getDbManager();
    }

    // Initialize models
    if (m_dbManager) {
        m_model = new ScanHistoryModel(m_dbManager, this);
        m_filterModel = new ScanHistoryFilterModel(this);
        m_filterModel->setSourceModel(m_model);
    } else {
        qWarning() << "No database manager available for HistoryWidget";
    }

    setupTableView();
    setupFilters();

    // Setup splitter for better layout
    // ...existing code...
    ui->mainLayout->removeWidget(ui->historyTableView);
    ui->mainLayout->removeWidget(ui->detailsAndActionsGroupBox);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(ui->historyTableView);
    splitter->addWidget(ui->detailsAndActionsGroupBox);
    ui->mainLayout->insertWidget(1, splitter);

    // Connect signals and slots
    // ...existing code...
    // Connect signals and slots
    connect(ui->SearchLineEdit, &QLineEdit::textChanged, this, &HistoryWidget::onSearchTextChanged);
    connect(ui->scanTypeFilterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistoryWidget::onScanTypeFilterChanged);
    connect(ui->startDateEdit, &QDateEdit::dateChanged, this, &HistoryWidget::onDateFilterChanged);
    connect(ui->endDateEdit, &QDateEdit::dateChanged, this, &HistoryWidget::onDateFilterChanged);

    connect(ui->historyTableView, &QTableView::clicked, this, &HistoryWidget::onHistoryTableViewClicked);
    connect(ui->historyTableView, &QTableView::customContextMenuRequested, this, &HistoryWidget::showContextMenu);

    connect(ui->viewDetailsButton, &QPushButton::clicked, this, &HistoryWidget::onViewDetailsClicked);
    connect(ui->deleteEventButton, &QPushButton::clicked, this, &HistoryWidget::onDeleteEventClicked);
    connect(ui->exportButton, &QPushButton::clicked, this, &HistoryWidget::onExportClicked);
    connect(ui->clearAllHistoryButton, &QPushButton::clicked, this, &HistoryWidget::onClearAllHistoryClicked);

    // Set default date range (last 30 days)
    ui->startDateEdit->setDate(QDate::currentDate().addDays(-30));
    ui->endDateEdit->setDate(QDate::currentDate());
}

HistoryWidget::~HistoryWidget()
{
    delete ui;
}

void HistoryWidget::setupTableView()
{
    if (!m_filterModel) {
        qWarning() << "Filter model not available";
        return;
    }

    ui->historyTableView->setModel(m_filterModel);
    ui->historyTableView->setAlternatingRowColors(true);
    ui->historyTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->historyTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->historyTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->historyTableView->setSortingEnabled(true);

    // Configure headers
    QHeaderView* header = ui->historyTableView->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(ScanHistoryModel::ColumnDateTime, 150);
    header->resizeSection(ScanHistoryModel::ColumnFileName, 200);
    header->resizeSection(ScanHistoryModel::ColumnScanType, 100);
    header->resizeSection(ScanHistoryModel::ColumnStatus, 100);
    header->resizeSection(ScanHistoryModel::ColumnRiskLevel, 100);
    header->resizeSection(ScanHistoryModel::ColumnActionStatus, 100);
    header->resizeSection(ScanHistoryModel::ColumnFileSize, 100);

    // Sort by date descending by default
    ui->historyTableView->sortByColumn(ScanHistoryModel::ColumnDateTime, Qt::DescendingOrder);
}

void HistoryWidget::setupFilters()
{
    if (!ui->scanTypeFilterComboBox) return;

    // Setup scan type filter
    ui->scanTypeFilterComboBox->addItem(tr("All Scan Types"), static_cast<int>(ScanType::Unknown));
    ui->scanTypeFilterComboBox->addItem(tr("Basic Scan"), static_cast<int>(ScanType::Basic));
    ui->scanTypeFilterComboBox->addItem(tr("VirusTotal"), static_cast<int>(ScanType::VirusTotal));
    ui->scanTypeFilterComboBox->addItem(tr("Docker Scan"), static_cast<int>(ScanType::Docker));
}

void HistoryWidget::updateDetailsDisplay()
{
    QModelIndexList selection = ui->historyTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        ui->detailsTextBrowser->clear();
        return;
    }

    QModelIndex selectedIndex = selection.first();
    ScanHistoryRecord record = selectedIndex.data(Qt::UserRole).value<ScanHistoryRecord>();
    
    QString details = formatScanDetails(record);
    ui->detailsTextBrowser->setHtml(details);
}

void HistoryWidget::showContextMenu(const QPoint& position)
{
    QModelIndex index = ui->historyTableView->indexAt(position);
    if (!index.isValid()) return;

    QMenu contextMenu(this);
    contextMenu.addAction(tr("View Details"), this, &HistoryWidget::onViewDetailsClicked);
    contextMenu.addAction(tr("Copy File Path"), [this, index]() {
        ScanHistoryRecord record = index.data(Qt::UserRole).value<ScanHistoryRecord>();
        QApplication::clipboard()->setText(record.filePath);
    });
    contextMenu.addSeparator();
    contextMenu.addAction(tr("Delete Entry"), this, &HistoryWidget::onDeleteEventClicked);

    contextMenu.exec(ui->historyTableView->mapToGlobal(position));
}

QString HistoryWidget::formatScanDetails(const ScanHistoryRecord& record) const
{
    QString html = QStringLiteral("<h3>") + tr("Scan Details") + QStringLiteral("</h3>");
    html += QStringLiteral("<table border='0' cellpadding='5'>");
    html += QStringLiteral("<tr><td><b>") + tr("File Name:") + QStringLiteral("</b></td><td>") + record.fileName + QStringLiteral("</td></tr>");
    html += QStringLiteral("<tr><td><b>") + tr("File Path:") + QStringLiteral("</b></td><td>") + record.filePath + QStringLiteral("</td></tr>");
    html += QStringLiteral("<tr><td><b>") + tr("File Size:") + QStringLiteral("</b></td><td>") + QLocale().formattedDataSize(record.fileSize) + QStringLiteral("</td></tr>");
    html += QStringLiteral("<tr><td><b>") + tr("Scan Date:") + QStringLiteral("</b></td><td>") + record.scanDateTime.toString(QStringLiteral("dddd, MMMM d, yyyy h:mm:ss AP")) + QStringLiteral("</td></tr>");
    html += QStringLiteral("<tr><td><b>") + tr("Scan Type:") + QStringLiteral("</b></td><td>") + m_model->scanTypeToString(record.scanType) + QStringLiteral("</td></tr>");
    html += QStringLiteral("<tr><td><b>") + tr("Status:") + QStringLiteral("</b></td><td>") + m_model->scanStatusToString(record.scanStatus) + QStringLiteral("</td></tr>");
    html += QStringLiteral("<tr><td><b>") + tr("Risk Level:") + QStringLiteral("</b></td><td>") + m_model->riskLevelToString(record.riskLevel) + QStringLiteral("</td></tr>");
    html += QStringLiteral("<tr><td><b>") + tr("Action Taken:") + QStringLiteral("</b></td><td>") + m_model->actionStatusToString(record.actionStatus) + QStringLiteral("</td></tr>");
    if (!record.threatName.isEmpty()) {
        html += QStringLiteral("<tr><td><b>") + tr("Threat Name:") + QStringLiteral("</b></td><td>") + record.threatName + QStringLiteral("</td></tr>");
    }
    if (!record.fileHash.isEmpty()) {
        html += QStringLiteral("<tr><td><b>") + tr("File Hash:") + QStringLiteral("</b></td><td style='font-family: monospace; font-size: 10px;'>") + record.fileHash + QStringLiteral("</td></tr>");
    }
    html += QStringLiteral("</table>");

    if (!record.scanResults.isEmpty()) {
        html += QStringLiteral("<h4>") + tr("Scan Results") + QStringLiteral("</h4>");
        html += QStringLiteral("<pre>") + record.scanResults + QStringLiteral("</pre>");
    }

    return html;
}

void HistoryWidget::addScanRecord(const QString& fileName, const QString& filePath, 
                                 const QString& fileHash, const QString& threatName,
                                 bool isMalicious, int scanType)
{
    if (!m_model) {
        qWarning() << "Scan history model not available";
        return;
    }

    ScanHistoryRecord record;
    record.fileName = fileName;
    record.filePath = filePath;
    record.fileHash = fileHash;
    record.scanDateTime = QDateTime::currentDateTime();
    record.scanType = static_cast<ScanType>(scanType);
    record.scanStatus = ScanStatus::Completed;
    record.riskLevel = isMalicious ? RiskLevel::High : RiskLevel::None;
    record.actionStatus = ActionStatus::None;
    record.threatName = threatName;
    record.scanResults = isMalicious ? tr("Threat detected") : tr("No threats found");
    
    QFileInfo fileInfo(filePath);
    record.fileSize = fileInfo.size();

    m_model->addScanRecord(record);
}

// Slot implementations
void HistoryWidget::onViewDetailsClicked()
{
    QModelIndexList selection = ui->historyTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, tr("No Selection"), tr("Please select a scan entry to view details."));
        return;
    }

    updateDetailsDisplay();
}

void HistoryWidget::onDeleteEventClicked() // Renamed from onDeleteClicked
{
    QModelIndexList selection = ui->historyTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, tr("No Selection"), tr("Please select a scan entry to delete."));
        return;
    }

    int ret = QMessageBox::question(this, tr("Delete Entry"), 
                                   tr("Are you sure you want to delete the selected scan entry?"),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes && m_model) {
        QModelIndex selectedIndex = m_filterModel->mapToSource(selection.first());
        m_model->removeRow(selectedIndex.row());
    }
}

void HistoryWidget::onExportClicked()
{
    if (!m_model) {
        QMessageBox::warning(this, tr("Export Error"), tr("No data model available for export."));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Scan History"), 
                                                   QStringLiteral("scan_history.csv"), tr("CSV Files (*.csv)"));
    if (!fileName.isEmpty()) {
        if (m_model->exportToCSV(fileName)) {
            QMessageBox::information(this, tr("Export Complete"), 
                                   tr("Scan history exported successfully to: %1").arg(fileName));
        } else {
            QMessageBox::critical(this, tr("Export Error"), 
                                tr("Failed to export scan history to CSV file."));
        }
    }
}

void HistoryWidget::onSearchTextChanged(const QString &text)
{
    if (m_filterModel) {
        m_filterModel->setSearchText(text);
    }
}

void HistoryWidget::onScanTypeFilterChanged(int index)
{
    if (m_filterModel && ui->scanTypeFilterComboBox) {
        QVariant data = ui->scanTypeFilterComboBox->itemData(index);
        if (data.isValid()) {
            ScanType scanType = static_cast<ScanType>(data.toInt());
            m_filterModel->setScanTypeFilter(scanType);
        }
    }
}

void HistoryWidget::onDateFilterChanged()
{
    if (m_filterModel) {
        QDate startDate = ui->startDateEdit->date();
        QDate endDate = ui->endDateEdit->date();
        m_filterModel->setDateRange(startDate, endDate);
    }
}

void HistoryWidget::onStatusFilterChanged(int index)
{
    if (m_filterModel && ui->statusFilterComboBox) {
        QVariant data = ui->statusFilterComboBox->itemData(index);
        if (data.isValid()) {
            ScanStatus status = static_cast<ScanStatus>(data.toInt());
            m_filterModel->setStatusFilter(status);
        }
    }
}

void HistoryWidget::onRiskLevelFilterChanged(int index)
{
    if (m_filterModel && ui->riskLevelFilterComboBox) {
        QVariant data = ui->riskLevelFilterComboBox->itemData(index);
        if (data.isValid()) {
            RiskLevel riskLevel = static_cast<RiskLevel>(data.toInt());
            m_filterModel->setRiskLevelFilter(riskLevel);
        }
    }
}

void HistoryWidget::onActionStatusFilterChanged(int index)
{
    if (m_filterModel && ui->actionStatusFilterComboBox) {
        QVariant data = ui->actionStatusFilterComboBox->itemData(index);
        if (data.isValid()) {
            ActionStatus actionStatus = static_cast<ActionStatus>(data.toInt());
            m_filterModel->setActionStatusFilter(actionStatus);
        }
    }
}

void HistoryWidget::onHistoryTableViewClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        updateDetailsDisplay();
    } else {
        ui->detailsTextBrowser->clear();
    }
}

// Implement action button slots
void HistoryWidget::onGoToQuarantineClicked()
{
    // TODO: Navigate to quarantine section - this would typically emit a signal
    // to the main window to switch to the quarantine tab.
    // Example: emit navigateToQuarantineRequested();
    QMessageBox::information(this, tr("Quarantine"), tr("Navigating to quarantine section... (Implementation Pending: Emit signal to main window)"));
}

void HistoryWidget::onRestoreFileClicked()
{
    QModelIndexList selection = ui->historyTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, tr("No Selection"), tr("Please select a quarantined file to restore."));
        return;
    }

    QModelIndex selectedIndex = m_filterModel->mapToSource(selection.first());
    ScanHistoryRecord record = selectedIndex.data(Qt::UserRole).value<ScanHistoryRecord>();
    
    int ret = QMessageBox::question(this, tr("Restore File"), 
                                   tr("Are you sure you want to restore the file:\n%1").arg(record.fileName),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // TODO: Implement actual restore logic.
        // This should involve:
        // 1. Identifying the file's original location and quarantine path.
        // 2. Copying/moving the file from quarantine to its original location.
        // 3. Updating the database record (e.g., actionStatus to Restored).
        // 4. Potentially removing the entry from a quarantine-specific list.
        // Example: m_quarantineManager->restoreFile(record.filePath, record.originalPath);
        //          m_model->updateActionStatus(record.id, ActionStatus::Restored);
        QMessageBox::information(this, tr("Restore"), tr("File restore functionality would be implemented here. See TODO comments for details."));
    }
}

void HistoryWidget::onPermanentlyDeleteFileClicked()
{
    QModelIndexList selection = ui->historyTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, tr("No Selection"), tr("Please select a file to delete permanently."));
        return;
    }

    QModelIndex selectedIndex = m_filterModel->mapToSource(selection.first());
    ScanHistoryRecord record = selectedIndex.data(Qt::UserRole).value<ScanHistoryRecord>();
    
    int ret = QMessageBox::critical(this, tr("Permanent Delete"), 
                                   tr("Are you sure you want to PERMANENTLY delete the file:\n%1\n\nThis action cannot be undone!").arg(record.fileName),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // TODO: Implement actual permanent deletion logic.
        // This should involve:
        // 1. Securely deleting the file from the disk (if applicable, e.g., from quarantine).
        // 2. Removing the scan history record from the database.
        // Example: m_fileSystemManager->secureDelete(record.filePath);
        //          m_model->removeRow(selectedIndex.row()); // Already does part of this if file is only in history
        QMessageBox::information(this, tr("Delete"), tr("Permanent file deletion functionality would be implemented here. See TODO comments for details."));
    }
}

void HistoryWidget::onAddToExclusionsClicked()
{
    QModelIndexList selection = ui->historyTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, tr("No Selection"), tr("Please select a file to add to exclusions."));
        return;
    }

    QModelIndex selectedIndex = m_filterModel->mapToSource(selection.first());
    ScanHistoryRecord record = selectedIndex.data(Qt::UserRole).value<ScanHistoryRecord>();
    
    int ret = QMessageBox::question(this, tr("Add to Exclusions"), 
                                   tr("Add the following file to scan exclusions?\n%1").arg(record.filePath),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // TODO: Implement exclusions logic.
        // This should involve:
        // 1. Getting the file path or hash from the 'record'.
        // 2. Adding this identifier to an exclusions list (e.g., managed by AppConfig, DbManager, or a dedicated ExclusionsManager).
        // 3. Ensuring future scans skip this file/path.
        // Example: m_exclusionManager->addExclusion(record.filePath);
        QMessageBox::information(this, tr("Exclusions"), tr("File added to exclusions list. (Implementation Pending: See TODO comments)"));
    }
}

void HistoryWidget::onSubmitSampleClicked()
{
    QModelIndexList selection = ui->historyTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, tr("No Selection"), tr("Please select a detected threat to submit as a sample."));
        return;
    }

    QModelIndex selectedIndex = m_filterModel->mapToSource(selection.first());
    ScanHistoryRecord record = selectedIndex.data(Qt::UserRole).value<ScanHistoryRecord>();
    
    if (record.riskLevel == RiskLevel::None) {
        QMessageBox::information(this, tr("Submit Sample"), tr("Only detected threats can be submitted as samples."));
        return;
    }
    
    int ret = QMessageBox::question(this, tr("Submit Sample"), 
                                   tr("Submit the following file as a sample for analysis?\n%1").arg(record.fileName),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // TODO: Implement sample submission logic
        QMessageBox::information(this, tr("Submit Sample"), tr("Sample submission functionality would be implemented here."));
    }
}

void HistoryWidget::onGenerateReportClicked()
{
    if (!m_model) {
        QMessageBox::warning(this, tr("Generate Report"), tr("No data available for report generation."));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Generate Scan Report"), 
                                                   QStringLiteral("scan_report.html"), tr("HTML Files (*.html)"));
    if (!fileName.isEmpty()) {
        // TODO: Implement report generation
        QMessageBox::information(this, tr("Generate Report"), tr("Report generation functionality would be implemented here."));
    }
}

void HistoryWidget::onClearAllHistoryClicked()
{
    if (!m_model) {
        QMessageBox::warning(this, tr("Clear History"), tr("No data model available."));
        return;
    }

    int ret = QMessageBox::question(this, tr("Clear All History"), 
                                   tr("Are you sure you want to delete ALL scan history entries?\n\nThis action cannot be undone."),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_model->clearAllRecords()) {
            ui->detailsTextBrowser->clear();
            QMessageBox::information(this, tr("Clear Complete"), tr("All scan history entries have been deleted."));
        } else {
            QMessageBox::critical(this, tr("Clear Error"), tr("Failed to clear scan history."));
        }
    }
}
