# Proje Durumu: AVProjectUi

**Son Güncelleme:** 30 Mayıs 2025

Bu belge, AVProjectUi antivirüs projesinin mevcut geliştirme durumunu, tamamlanan özellikleri, devam eden çalışmaları, gelecek planlarını ve bilinen sorunları kapsamlı bir şekilde özetlemektedir.

## 1. Proje Genel Bakış

AVProjectUi, öncelikli olarak Windows 10 ve Windows 11 (x64) platformlarını hedefleyen, modern bir kullanıcı arayüzüne sahip bir antivirüs uygulamasıdır. Geliştirme süreci macOS üzerinde Qt ve C++ kullanılarak yürütülmektedir.

## 2. Mevcut Durum ve İlerleme

### 2.1. Temel Altyapı ve Çerçeve

-   **Derleme Sistemi:** `CMake` (Sürüm 3.19+) tabanlı, platformlar arası derlemeyi destekleyecek şekilde yapılandırılmıştır. (`CMakeLists.txt`)
    -   macOS için Xcode SDK ve libc++ ayarları yapılmıştır.
    -   C++17 standardı zorunlu kılınmıştır.
    -   Qt çeviri fonksiyonları (`tr()`) devre dışı bırakılarak varsayılan dil İngilizce olarak ayarlanmıştır.
-   **Qt Entegrasyonu:** Qt 6 (spesifik olarak 6.9) ile tam entegrasyon sağlanmıştır. (`CMakeLists.txt`)
    -   Gerekli modüller: `Core`, `Widgets`, `Sql`, `Network`, `Test`.
    -   Otomatik `moc`, `uic`, `rcc` işlemleri etkinleştirilmiştir.
-   **Test Altyapısı:** `CTest` entegrasyonu ile birim testleri ve entegrasyon testleri için altyapı kurulmuştur. (`tests/CMakeLists.txt`, `tests/TestDataFactory.h`)
    -   `build_and_test` özel hedefi ile tüm projeyi derleyip testleri çalıştırma imkanı.
-   **Uygulama Yapılandırması:** `Core/AppConfig.h` sınıfı ve `Core/config.ini` dosyası aracılığıyla uygulama ayarları (veritabanı yolu, VirusTotal API anahtarı vb.) yönetilmektedir.
    -   Singleton tasarım deseni ile global erişim.
    -   Varsayılan ayarlar ve ayar yükleme/kaydetme mekanizmaları.
-   **Kaynak Yönetimi:** `.qrc` dosyası (`resources.qrc`) aracılığıyla stil dosyaları gibi kaynaklar uygulamaya dahil edilmiştir.

### 2.2. Kullanıcı Arayüzü (UI)

-   **Ana Pencere:** `UI/Mainwindow/mainwindow.h/cpp/ui` dosyalarında tanımlanan ana uygulama penceresi.
    -   Temel menü, durum çubuğu ve ana içerik alanı.
-   **Widget'lar:**
    -   `UI/Widgets/Dashboard/DashboardWidget.h/cpp/ui`: Temel tarama kontrollerini ve özet bilgileri içeren bir gösterge paneli.
    -   `UI/Widgets/History/HistoryWidget.h/cpp/ui`: Tarama geçmişini görüntülemek ve yönetmek için tasarlanan widget (geliştirme aşamasında, `TODO` işaretleri mevcut).
-   **Stil:** Ana stil dosyası `UI/Resources/Styles/main.qss` aracılığıyla temel bir görünüm sağlanmıştır.

### 2.3. Veritabanı Yönetimi

-   **Arayüz:** `Interface/IDbManager.h` ile veritabanı işlemleri için soyut bir arayüz tanımlanmıştır.
    -   Bağlantı kurma, bağlantı durumu kontrolü, SHA256 varlık kontrolü, imza sayısı sorgulama.
-   **Uygulama:** `Database/DbManager/DbManager.h/cpp` sınıfları `IDbManager` arayüzünü uygular.
    -   SQLite veritabanı (`identifier.sqlite`) ile etkileşim.
-   **Servis Katmanı:** `Database/DatabaseService/DatabaseService.h/cpp` ile veritabanı erişimi için bir servis katmanı oluşturulmuştur. (`main.cpp` içinde kullanılır)

### 2.4. Tarama Motorları ve Yetenekleri

-   **Temel Tarayıcı Arayüzü:** `Interface/IScanner.h` ile genel tarayıcı işlevleri (dosya seçme, tarama, sonuç alma, iptal etme) için bir sözleşme tanımlanmıştır.
-   **Temel Yerel Tarayıcı:** `Scanner/Dashboard/BasicScanner/BasicScanner.h/cpp` (`IScanner` arayüzünü uygular, temel dosya tarama mantığı için başlangıç noktası).
-   **VirusTotal Entegrasyonu:**
    -   Arayüz: `Interface/IVirusTotalScanner.h` (`IScanner`'ı genişletir).
    -   Uygulama: `Network/VirusTotal/VirusTotalManager.h/cpp` ile VirusTotal API v3'e dosya gönderme ve analiz sonuçlarını alma.
    -   API anahtarı `AppConfig` üzerinden yönetilir.
    -   API endpoint'leri (`getVirusTotalFilesUrl`, `getVirusTotalAnalysesUrl`) `AppConfig` içinde tanımlıdır.
-   **Docker Tabanlı Tarama Arayüzü:** `Interface/IDockerScanner.h` (`IScanner`'ı genişletir), Docker konteynerleri ile tarama yapacak modüller için bir arayüz sunar (örn: CDR, sandbox).
    -   Konteynere dosya gönderme, konteyner durumu kontrolü.

### 2.5. Ağ İşlemleri

-   **VirusTotal Yöneticisi:** Yukarıda bahsedilen `Network/VirusTotal/VirusTotalManager`.
-   **Ağ İzleme (Planlanan):** `Network/Monitor/NetworkMonitor.cpp` dosyası mevcut, ancak işlevselliği henüz tam olarak geliştirilmemiştir.

## 3. Tamamlanan Özellikler

-   CMake tabanlı proje yapısı ve derleme sistemi (macOS odaklı başlangıç).
-   Temel Qt kullanıcı arayüzü çerçevesi ve ana pencere.
-   `AppConfig` ile merkezi yapılandırma yönetimi (`config.ini` dosyası).
-   SQLite veritabanı bağlantısı ve temel sorgulama yetenekleri (`DbManager`, `DatabaseService`).
-   `IScanner`, `IVirusTotalScanner`, `IDockerScanner` arayüzleri ile modüler tarayıcı altyapısı.
-   VirusTotal API'sine dosya yükleme ve temel analiz sorgulama için altyapı.
-   Temel CTest test altyapısı.

## 4. Geliştirilmekte Olan Özellikler ve TODO'lar

-   **`UI/Widgets/History/HistoryWidget.cpp`:** Bu widget'ın tam işlevselliği (`TODO` olarak işaretlenmiş kısımlar).
    -   Tarama geçmişi verilerinin veritabanından okunması ve tabloda gösterilmesi.
    -   Filtreleme (tarih, tarama tipi, sonuç) ve arama işlevleri.
    -   Detay görüntüleme, olay silme, dışa aktarma.
    -   Karantina, dosya geri yükleme, kalıcı silme, dışlamalara ekleme gibi eylemler için buton bağlantıları ve işlevleri.
-   **`Scanner/Dashboard/BasicScanner/BasicScanner.cpp`:** Yerel dosya sistemi tarama mantığının detaylandırılması.
    -   İmza tabanlı tarama.
    -   Sezgisel analiz (heuristic) için temel algoritmalar.
-   **`Network/Monitor/NetworkMonitor.cpp`:** Ağ trafiği izleme ve analiz özelliklerinin geliştirilmesi.
-   **`IDockerScanner` Uygulamaları:** Docker tabanlı tarayıcıların (örn: bir CDR çözümü veya sandbox) somut uygulamalarının geliştirilmesi.
-   **Kullanıcı Arayüzü İyileştirmeleri:**
    -   Daha detaylı tarama ilerleme göstergeleri ve geri bildirimler.
    -   Ayarlar ve yapılandırma için kullanıcı arayüzü.
    -   Tehdit algılandığında kullanıcıya sunulacak bildirimler ve eylem seçenekleri.
-   **Hata Yönetimi ve Loglama:** Daha kapsamlı hata yakalama ve detaylı loglama mekanizmaları.
-   **Windows Platformuna Özel Derleme ve Dağıtım Ayarları:** `CMakeLists.txt` dosyasında Windows (MSVC) için derleme ve `windeployqt` ile dağıtım süreçlerinin iyileştirilmesi.

## 5. Gelecek Planları ve Vizyon

-   **Tam Windows Desteği:** Windows 10 ve 11 için kararlı ve tam özellikli bir sürüm sunmak.
-   **Gelişmiş Tehdit Algılama:**
    -   Davranışsal analiz modülü.
    -   Makine öğrenimi tabanlı tehdit algılama.
    -   Bulut tabanlı tehdit istihbaratı ile entegrasyon.
-   **Performans Optimizasyonu:** Tarama hızını artırmak ve sistem kaynak kullanımını minimize etmek.
-   **Karantina Yönetimi:** Güvenli bir karantina alanı ve kullanıcı dostu yönetim arayüzü.
-   **Gerçek Zamanlı Koruma:** Dosya sistemi olaylarını izleyerek anlık koruma sağlama.
-   **Otomatik Güncellemeler:** Uygulama ve imza veritabanı için otomatik güncelleme mekanizması.
-   **Detaylı Raporlama:** Kullanıcıların tarama sonuçlarını ve sistem güvenliği durumunu inceleyebileceği kapsamlı raporlar.
-   **Çoklu Dil Desteği:** (Şu an `QT_NO_TRANSLATION` ile devre dışı, ileride eklenebilir).

## 6. Bilinen Sorunlar ve Kısıtlamalar

-   **Platform Odaklı Geliştirme:** Şu anki geliştirme ve testler ağırlıklı olarak macOS üzerinde yapılmaktadır. Windows'a özel sorunlar derleme ve test aşamalarında ortaya çıkabilir.
-   **`HistoryWidget` Eksik İşlevsellik:** Yukarıda belirtildiği gibi, geçmiş widget'ı henüz tamamlanmamıştır.
-   **Docker Entegrasyonu:** `IDockerScanner` için henüz somut bir uygulama bulunmamaktadır.
-   **Kullanıcı Deneyimi (UX):** Mevcut UI temel düzeydedir ve daha fazla iyileştirme gerektirmektedir.
-   **Güvenlik Açıkları:** Geliştirme aşamasında olduğu için potansiyel güvenlik açıkları bulunabilir; kapsamlı test ve denetim gereklidir.

Bu belge, projenin ilerleyişine paralel olarak düzenli olarak güncellenecektir.
