//
// Created by Ekrem Ünal on 11.05.2025.
//

#include "HistoryWidget.h"
#include "ui_HistoryWidget.h" // .ui dosyasından oluşturulan başlık dosyasını dahil et
#include <QSplitter>
#include <QVBoxLayout> // mainLayout'a erişim için gerekebilir, ancak ui->mainLayout zaten var.

// Gerekli olabilecek diğer Qt başlıkları (QDate vb. ui dosyasında tanımlıysa buraya gerek kalmayabilir)
// #include <QDate> // ui->startDateEdit vb. için QDate gerekebilir, ancak genellikle ui_*.h halleder.

HistoryWidget::HistoryWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HistoryWidget) // ui nesnesini oluştur
{
    ui->setupUi(this); // .ui dosyasındaki arayüzü bu widget'a yükle

    // Mevcut historyTableView ve detailsAndActionsGroupBox'ı ana düzenden çıkarın
    // Bu widget'lar ui->mainLayout'un doğrudan çocukları olmalı.
    // Eğer başka bir alt düzen içindeyseler, o düzeni bulup oradan çıkarmak gerekir.
    // .ui dosyasını incelediğimde, bunlar doğrudan ui->mainLayout'un item'ları.
    ui->mainLayout->removeWidget(ui->historyTableView);
    ui->mainLayout->removeWidget(ui->detailsAndActionsGroupBox);

    // Dikey bir QSplitter oluşturun
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(ui->historyTableView);
    splitter->addWidget(ui->detailsAndActionsGroupBox);

    // Splitter'daki bölümlerin başlangıç boyutlarını ayarlayabilirsiniz (isteğe bağlı)
    // Örneğin, tabloya daha fazla, detaylara daha az yer vermek için:
    // QList<int> sizes;
    // sizes << 400 << 200; // İlk widget (tablo) için 400px, ikinci (detaylar) için 200px
    // splitter->setSizes(sizes);

    // Splitter'ı ana düzene (mainLayout) ekleyin.
    // .ui dosyasında historyTableView ve detailsAndActionsGroupBox'ın bulunduğu yere ekliyoruz.
    // filterControlsLayout'tan sonra ve actionButtonsLayout'tan önce olmalı.
    // ui->mainLayout bir QVBoxLayout olduğu için insertWidget ile doğru indekse ekleyebiliriz.
    // filterControlsLayout 0. indekste, historyTableView 1. indekste, detailsAndActionsGroupBox 2. indekste idi.
    // Dolayısıyla splitter'ı 1. indekse ekleyebiliriz.
    ui->mainLayout->insertWidget(1, splitter);

    // Örnek: Tarih alanlarına varsayılan değerleri atama (isteğe bağlı, .ui dosyasında da yapılabilir)
    // ui->startDateEdit->setDate(QDate::currentDate().addMonths(-1));
    // ui->endDateEdit->setDate(QDate::currentDate());

    // Sinyalleri ve slotları bağlayın
    // Not: Slot isimleri .h dosyasındaki ile aynı olmalı
    // ve .ui dosyasındaki widget isimleri (objectName) doğru olmalı.
    connect(ui->SearchLineEdit, &QLineEdit::textChanged, this, &HistoryWidget::onSearchTextChanged); // Corrected: searchLineEdit -> SearchLineEdit
    connect(ui->scanTypeFilterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistoryWidget::onScanTypeFilterChanged);
    connect(ui->startDateEdit, &QDateEdit::dateChanged, this, &HistoryWidget::onDateFilterChanged);
    connect(ui->endDateEdit, &QDateEdit::dateChanged, this, &HistoryWidget::onDateFilterChanged);

    connect(ui->historyTableView, &QTableView::clicked, this, &HistoryWidget::onHistoryTableViewClicked);

    connect(ui->viewDetailsButton, &QPushButton::clicked, this, &HistoryWidget::onViewDetailsClicked);
    connect(ui->deleteEventButton, &QPushButton::clicked, this, &HistoryWidget::onDeleteEventClicked); // Renamed from deleteButton
    connect(ui->exportButton, &QPushButton::clicked, this, &HistoryWidget::onExportClicked);

    // Connect new action buttons
    // TODO: Verify the object names of these buttons in your .ui file. They are commented out as they were not found.
    // connect(ui->goToQuarantineButton, &QPushButton::clicked, this, &HistoryWidget::onGoToQuarantineClicked);
    // connect(ui->restoreFileButton, &QPushButton::clicked, this, &HistoryWidget::onRestoreFileClicked);
    // connect(ui->permanentlyDeleteFileButton, &QPushButton::clicked, this, &HistoryWidget::onPermanentlyDeleteFileClicked);
    // connect(ui->addToExclusionsButton, &QPushButton::clicked, this, &HistoryWidget::onAddToExclusionsClicked);
    // connect(ui->submitSampleButton, &QPushButton::clicked, this, &HistoryWidget::onSubmitSampleClicked);
    // connect(ui->generateReportButton, &QPushButton::clicked, this, &HistoryWidget::onGenerateReportClicked);
    connect(ui->clearAllHistoryButton, &QPushButton::clicked, this, &HistoryWidget::onClearAllHistoryClicked);
}

HistoryWidget::~HistoryWidget()
{
    delete ui; // ui nesnesini sil
}

// Slot implementasyonları (şimdilik boş veya temel)
void HistoryWidget::onViewDetailsClicked()
{
    // TODO: Seçili öğenin detaylarını gösterme mantığını implemente edin
    // Örneğin:
    // QModelIndexList selection = ui->historyTableView->selectionModel()->selectedRows();
    // if (!selection.isEmpty()) {
    //     // Seçili satırdan veri al ve detayları göster
    // }
}

void HistoryWidget::onDeleteEventClicked() // Renamed from onDeleteClicked
{
    // TODO: Seçili olayı silme mantığını implemente edin
    // QModelIndexList selection = ui->historyTableView->selectionModel()->selectedRows();
    // if (!selection.isEmpty()) {
    //     // Seçili olayı modelden ve veritabanından sil
    // }
}

void HistoryWidget::onExportClicked()
{
    // TODO: Seçili öğeyi/tüm listeyi dışa aktarma mantığını implemente edin
}

void HistoryWidget::onSearchTextChanged(const QString &text)
{
    // TODO: Metne göre filtreleme mantığını implemente edin
    // Genellikle QSortFilterProxyModel kullanılır.
    // Örnek: proxyModel->setFilterFixedString(text);
    Q_UNUSED(text); // Eğer text parametresi şimdilik kullanılmıyorsa uyarıyı engelle
}

void HistoryWidget::onScanTypeFilterChanged(int index)
{
    // TODO: Tarama türüne göre filtreleme mantığını implemente edin
    // Örnek: QString selectedType = ui->scanTypeFilterComboBox->itemText(index);
    // proxyModel->setFilterKeyColumn(COLUMN_FOR_SCAN_TYPE); // Sütun numarasını ayarlayın
    // proxyModel->setFilterRegularExpression(QRegularExpression(selectedType, QRegularExpression::CaseInsensitiveOption));
    Q_UNUSED(index); // Eğer index parametresi şimdilik kullanılmıyorsa uyarıyı engelle
}

void HistoryWidget::onDateFilterChanged()
{
    // TODO: Tarih aralığına göre filtreleme mantığını implemente edin
    // Örnek:
    // QDate startDate = ui->startDateEdit->date();
    // QDate endDate = ui->endDateEdit->date();
    // proxyModel->setFilterDateRange(startDate, endDate); // Örnek bir proxy model fonksiyonu
}

void HistoryWidget::onStatusFilterChanged(int index)
{
    // TODO: Duruma göre filtreleme mantığını implemente edin
    // Örnek: QString selectedStatus = ui->statusFilterComboBox->itemText(index);
    // proxyModel->setFilterKeyColumn(COLUMN_FOR_STATUS); // Sütun numarasını ayarlayın
    // proxyModel->setFilterRegularExpression(QRegularExpression(selectedStatus, QRegularExpression::CaseInsensitiveOption));
    Q_UNUSED(index); // Eğer index parametresi şimdilik kullanılmıyorsa uyarıyı engelle
}

void HistoryWidget::onRiskLevelFilterChanged(int index)
{
    // TODO: Risk seviyesine göre filtreleme mantığını implemente edin
    // Örnek: QString selectedRiskLevel = ui->riskLevelFilterComboBox->itemText(index);
    // proxyModel->setFilterKeyColumn(COLUMN_FOR_RISK_LEVEL); // Sütun numarasını ayarlayın
    // proxyModel->setFilterRegularExpression(QRegularExpression(selectedRiskLevel, QRegularExpression::CaseInsensitiveOption));
    Q_UNUSED(index);
}

void HistoryWidget::onActionStatusFilterChanged(int index)
{
    // TODO: Eylem durumuna göre filtreleme mantığını implemente edin
    // Örnek: QString selectedActionStatus = ui->actionStatusFilterComboBox->itemText(index);
    // proxyModel->setFilterKeyColumn(COLUMN_FOR_ACTION_STATUS); // Sütun numarasını ayarlayın
    // proxyModel->setFilterRegularExpression(QRegularExpression(selectedActionStatus, QRegularExpression::CaseInsensitiveOption));
    Q_UNUSED(index);
}

void HistoryWidget::onHistoryTableViewClicked(const QModelIndex &index)
{
    // TODO: Seçili öğenin detaylarını detailsTextBrowser'da göster
    // if (index.isValid()) {
    //     QString detailText = "Seçilen Olay Detayları:\n";
    //     // Modelinizden ilgili satırın verilerini alın
    //     // Örneğin, her sütun için:
    //     // detailText += ui->historyTableView->model()->headerData(0, Qt::Horizontal).toString() + ": " + index.sibling(index.row(), 0).data().toString() + "\n";
    //     // detailText += ui->historyTableView->model()->headerData(1, Qt::Horizontal).toString() + ": " + index.sibling(index.row(), 1).data().toString() + "\n";
    //     // ... ve diğer sütunlar
    //     ui->detailsTextBrowser->setText(detailText);
    // } else {
    //     ui->detailsTextBrowser->clear();
    // }
    Q_UNUSED(index);
}

// Implement new action button slots
void HistoryWidget::onGoToQuarantineClicked()
{
    // TODO: Karantina bölümüne gitme mantığını implemente edin
}

void HistoryWidget::onRestoreFileClicked()
{
    // TODO: Seçili dosyayı karantinadan geri yükleme mantığını implemente edin
}

void HistoryWidget::onPermanentlyDeleteFileClicked()
{
    // TODO: Seçili dosyayı kalıcı olarak silme mantığını implemente edin
}

void HistoryWidget::onAddToExclusionsClicked()
{
    // TODO: Seçili öğeyi istisnalara ekleme mantığını implemente edin
}

void HistoryWidget::onSubmitSampleClicked()
{
    // TODO: Örnek gönderme mantığını implemente edin
}

void HistoryWidget::onGenerateReportClicked()
{
    // TODO: Rapor oluşturma mantığını implemente edin
}

void HistoryWidget::onClearAllHistoryClicked()
{
    // TODO: Tüm geçmişi temizleme mantığını implemente edin (onay mekanizması ile)
}
