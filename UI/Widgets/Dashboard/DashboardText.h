#ifndef DASHBOARDTEXT_H
#define DASHBOARDTEXT_H

#include <QString>

namespace DashboardText {
    // File operations
    const QString SELECTING_FILE = QStringLiteral("Dosya seçiliyor...");
    const QString FILE_SELECTED = QStringLiteral("Seçilen dosya: %1");
    const QString FILE_SELECTION_CANCELED = QStringLiteral("Dosya seçimi iptal edildi.");
    
    // Scan operations
    const QString SCANNING_FILE = QStringLiteral("Dosya taranıyor...");
    const QString BASIC_SCAN = QStringLiteral("Temel tarama başlatılıyor...");
    const QString ADVANCED_SCAN = QStringLiteral("Gelişmiş tarama başlatılıyor...");
    const QString SCAN_INITIATION_FAILED = QStringLiteral("Tarama başlatılamadı.");
    
    // VirusTotal operations
    const QString VT_SUBMITTING = QStringLiteral("VirusTotal'a gönderiliyor...");
    const QString VT_SUBMIT_STATUS = QStringLiteral("VirusTotal durumu: %1");
    const QString VT_SUBMIT_ERROR = QStringLiteral("VirusTotal'a gönderim hatası.");
    
    // Scan results
    const QString SCAN_COMPLETE = QStringLiteral("Tarama tamamlandı.");
    const QString THREAT_DETECTED = QStringLiteral("DURUM: TEHDİT TESPİT EDİLDİ");
    const QString FILE_CLEAN = QStringLiteral("DURUM: TEMİZ");
    
    // Directory scan
    const QString DIRECTORY_SCAN_STARTED = QStringLiteral("Klasör taraması başlatıldı: %1");
    const QString DIRECTORY_SCAN_FINISHED = QStringLiteral("Klasör taraması tamamlandı. Taranan dosyalar: %1, Bulunan tehditler: %2");
    const QString DIRECTORY_SCAN_CANCELED = QStringLiteral("Klasör taraması iptal edildi. İşlenen dosyalar: %1");
    
    // Network monitoring
    const QString NETWORK_MONITORING_STARTED = QStringLiteral("Ağ izleme başlatıldı.");
    const QString NETWORK_MONITORING_STOPPED = QStringLiteral("Ağ izleme durduruldu.");
    const QString START_NETWORK_MONITORING = QStringLiteral("Ağ İzlemeyi Başlat");
    const QString STOP_NETWORK_MONITORING = QStringLiteral("Ağ İzlemeyi Durdur");
    
    // Error messages
    const QString SCAN_ERROR = QStringLiteral("Tarama hatası: %1");
    const QString NO_FILE_SELECTED = QStringLiteral("Lütfen önce bir dosya seçin.");
    const QString SCANNER_NOT_AVAILABLE = QStringLiteral("Tarayıcı kullanılamıyor.");
}

#endif // DASHBOARDTEXT_H
