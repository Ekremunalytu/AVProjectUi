//
// Created by Ekrem Ünal on 11.05.2025.
//

#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QWidget>

// Forward declaration for the UI class generated from HistoryWidget.ui
namespace Ui {
class HistoryWidget;
}

class HistoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryWidget(QWidget *parent = nullptr);
    ~HistoryWidget();

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
    // setupUi() fonksiyonuna artık gerek yok, ui dosyası halledecek.
    // UI Elemanları ve Layoutlar ui dosyası üzerinden yönetilecek.
    Ui::HistoryWidget *ui; // UI sınıfı için işaretçi
};

#endif //HISTORYWIDGET_H
