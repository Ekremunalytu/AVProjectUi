//
// Created by Ekrem Ünal on 11.05.2025.
//

#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QWidget>
#include <QModelIndex>

// Forward declarations
namespace Ui {
class HistoryWidget;
}

class ScanHistoryModel;
class ScanHistoryFilterModel;
class DbManager;

class HistoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryWidget(DbManager* dbManager = nullptr, QWidget *parent = nullptr);
    ~HistoryWidget();

    // Public methods for external components to add scan records
    void addScanRecord(const QString& fileName, const QString& filePath, 
                      const QString& fileHash, const QString& threatName,
                      bool isMalicious, int scanType = 0);

private slots:
    // Slotlar kullanıcı etkileşimlerini işlemek için
    void onViewDetailsClicked();
    void onDeleteEventClicked(); // Renamed from onDeleteClicked
    void onExportClicked();
    void onSearchTextChanged(const QString &text);
    void onScanTypeFilterChanged(int index);
    void onDateFilterChanged(); // Hem başlangıç hem de bitiş tarihi için
    void onStatusFilterChanged(int index);
    void onRiskLevelFilterChanged(int index); // New slot
    void onActionStatusFilterChanged(int index); // New slot
    void onHistoryTableViewClicked(const QModelIndex &index); // New slot for table view click

    // New slots for action buttons
    void onGoToQuarantineClicked();
    void onRestoreFileClicked();
    void onPermanentlyDeleteFileClicked();
    void onAddToExclusionsClicked();
    void onSubmitSampleClicked();
    void onGenerateReportClicked();
    void onClearAllHistoryClicked();

private:
    void setupTableView();
    void setupFilters();
    void updateDetailsDisplay();
    void showContextMenu(const QPoint& position);
    QString formatScanDetails(const struct ScanHistoryRecord& record) const;
    
    Ui::HistoryWidget *ui;
    ScanHistoryModel* m_model;
    ScanHistoryFilterModel* m_filterModel;
    DbManager* m_dbManager;
};

#endif //HISTORYWIDGET_H
