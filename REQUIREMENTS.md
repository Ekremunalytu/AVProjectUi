# Proje Gereksinimleri

Bu belge, AVProjectUi projesinin geliştirilmesi, derlenmesi ve çalıştırılması için gerekli olan sistem gereksinimlerini, bağımlılıkları ve kurulum adımlarını detaylandırmaktadır.

## 1. Desteklenen Platformlar

-   **Hedef İşletim Sistemleri:**
    -   Windows 10 (x64)
    -   Windows 11 (x64)
-   **Geliştirme Ortamı (Önerilen):**
    -   macOS (Apple Silicon veya Intel)
    -   Windows 10/11 (MSVC ile)
-   **Diğer Platformlar:** Linux desteği gelecekte değerlendirilebilir.

## 2. Yazılım Gereksinimleri

### 2.1. Geliştirme Araçları

-   **C++ Derleyicisi:**
    -   **Windows:** MSVC (Visual Studio 2019 veya üstü ile birlikte gelir)
    -   **macOS:** Clang (Xcode Command Line Tools ile birlikte gelir)
    -   **Genel:** C++17 standardını destekleyen herhangi bir derleyici.
-   **CMake:** Sürüm 3.19 veya üstü.
-   **Qt Framework:** Sürüm 6.2 veya üstü (Proje şu anda `/Volumes/Crucial/QT/6.10.0/macos` yolundaki Qt 6.10.0 sürümünü kullanmaktadır). Gerekli Qt Modülleri:
    -   `Core`
    -   `Widgets`
    -   `Sql` (SQLite veritabanı işlemleri için)
    -   `Network` (Ağ işlemleri, örn: VirusTotal API)
    -   `Test` (Birim testleri için)
-   **Git:** Versiyon kontrol sistemi.

### 2.2. Çalıştırma Zamanı Bağımlılıkları (Hedef Windows Platformu İçin)

-   **Microsoft Visual C++ Redistributable:** Uygulamanın derlendiği MSVC sürümüne uygun.
-   **Qt Runtime Kütüphaneleri:** Uygulama ile birlikte dağıtılması gereken Qt DLL'leri (Core, Widgets, Sql, Network vb.). `windeployqt` aracı bu işlem için kullanılabilir.

## 3. Donanım Gereksinimleri (Minimum)

-   **İşlemci:** x64 mimarisine sahip çift çekirdekli işlemci.
-   **RAM:** 4 GB (8 GB önerilir).
-   **Disk Alanı:** Uygulama ve bağımlılıklar için en az 500 MB boş alan (tarama veritabanı ve loglar için ek alan gerekebilir).

## 4. Kurulum ve Derleme Adımları

### 4.1. Geliştirme Ortamının Hazırlanması

1.  **C++ Derleyicisi ve CMake Kurulumu:**
    -   **Windows:** Visual Studio (C++ geliştirme araçları ile) ve CMake'i kurun.
    -   **macOS:** Xcode Command Line Tools (`xcode-select --install`) ve CMake'i (örn: Homebrew ile `brew install cmake`) kurun.
2.  **Qt Framework Kurulumu:** Qt Online Installer kullanarak belirtilen sürümü ve gerekli modülleri kurun. `CMAKE_PREFIX_PATH` ortam değişkenini veya CMake yapılandırmasında Qt kurulum yolunu doğru şekilde ayarlayın. Mevcut macOS geliştirme ortamında bu yol `/Volumes/Crucial/QT/6.10.0/macos` olarak ayarlanmıştır.
3.  **Depoyu Klonlama:**
    ```bash
    git clone <repository_url>
    cd AVProjectUi
    ```

### 4.2. Projeyi Derleme (Örnek: Komut Satırı)

1.  **Build Dizini Oluşturma:**
    ```bash
    mkdir build
    cd build
    ```
2.  **CMake ile Projeyi Yapılandırma:**
    -   **macOS (Xcode veya Makefiles):**
        ```bash
        cmake .. 
        # Veya belirli bir jeneratör için: cmake -G "Xcode" ..
        ```
    -   **Windows (Visual Studio):**
        ```bash
        cmake .. -G "Visual Studio 17 2022" -A x64 
        # Visual Studio sürümünüze göre ayarlayın
        ```
    *Not: `CMAKE_PREFIX_PATH` Qt kurulum dizininizi gösterecek şekilde ayarlanmalıdır eğer sistem genelinde bulunmuyorsa.*
3.  **Projeyi Derleme:**
    -   **macOS (Makefiles):**
        ```bash
        make
        ```
    -   **macOS (Xcode):** Xcode IDE üzerinden projeyi açıp derleyin.
    -   **Windows (Visual Studio):**
        ```bash
        cmake --build . --config Release
        # Veya Visual Studio IDE üzerinden projeyi açıp derleyin.
        ```
4.  **Çalıştırılabilir Dosya:** Derleme işlemi tamamlandığında, çalıştırılabilir dosya (`AvProjectUi.app` macOS'ta, `AvProjectUi.exe` Windows'ta) build dizini içinde veya platforma özgü bir alt dizinde (örn: `build/Release`) bulunacaktır.

### 4.3. Dağıtım (Deployment)

-   **Windows:** `windeployqt` aracı, gerekli Qt DLL'lerini ve eklentilerini çalıştırılabilir dosyanın yanına kopyalamak için kullanılmalıdır.
-   **macOS:** `macdeployqt` aracı, `.app` paketini dağıtıma hazır hale getirmek için kullanılır. CMake `qt_generate_deploy_app_script` komutu ile bu işlem otomatikleştirilmiştir.

## 5. Opsiyonel Bağımlılıklar ve Entegrasyonlar

-   **Docker:** Docker tabanlı tarama özellikleri (`IDockerScanner`) kullanılacaksa, Docker Desktop'ın kurulu ve çalışır durumda olması gerekir.
-   **İnternet Bağlantısı:** VirusTotal API entegrasyonu (`IVirusTotalScanner`) için aktif bir internet bağlantısı gereklidir.
-   **VirusTotal API Anahtarı:** `config.ini` dosyasında geçerli bir VirusTotal API anahtarının tanımlanması, VirusTotal tarama özelliklerinin çalışması için zorunludur.

Bu gereksinimler, projenin başarılı bir şekilde geliştirilmesi, derlenmesi ve çalıştırılması için temel oluşturur.

