# Basit Makefile yardımcısı - alt testlere yönlendirir
.PHONY: clean build test build-and-test all help coverage benchmark

# Varsayılan değişkenler
BUILD_DIR ?= build
CMAKE_FLAGS ?= -DCMAKE_BUILD_TYPE=Debug

# Varsayılan hedef
all: build-and-test

# CMake konfigürasyonu oluştur
configure:
	@echo "CMake projesini yapılandırıyor..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. $(CMAKE_FLAGS)

# Coverage enabled konfigürasyon
configure-coverage:
	@echo "CMake projesini coverage ile yapılandırıyor..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. $(CMAKE_FLAGS) -DENABLE_COVERAGE=ON

# Projeyi derle
build: configure
	@echo "Projeyi derliyor..."
	@cd $(BUILD_DIR) && cmake --build .

# Sadece testleri çalıştır
test:
	@echo "Testleri çalıştırıyor..."
	@cd $(BUILD_DIR) && ctest --output-on-failure

# Derle ve test et
build-and-test: build
	@echo "Tüm testleri çalıştırıyor..."
	@cd $(BUILD_DIR) && ctest --output-on-failure

# Ayrıntılı test çıktısı
test-verbose:
	@echo "Testleri ayrıntılı çıktı ile çalıştırıyor..."
	@cd $(BUILD_DIR)/tests && ./DashboardTest -v2 && ./DbManagerTest -v2 && ./MainWindowTest -v2

# Belirli bir testi çalıştır
test-dashboard:
	@echo "Dashboard testlerini çalıştırıyor..."
	@cd $(BUILD_DIR)/tests && ./DashboardTest -v2

test-dbmanager:
	@echo "DbManager testlerini çalıştırıyor..."
	@cd $(BUILD_DIR)/tests && ./DbManagerTest -v2

test-mainwindow:
	@echo "MainWindow testlerini çalıştırıyor..."
	@cd $(BUILD_DIR)/tests && ./MainWindowTest -v2

# Test coverage oluştur
coverage: configure-coverage build
	@echo "Test coverage raporu oluşturuluyor..."
	@cd $(BUILD_DIR) && make coverage
	@echo "Coverage raporu hazır: $(BUILD_DIR)/coverage-report/index.html"

# Benchmark testleri çalıştır
benchmark:
	@echo "Benchmark testleri çalıştırılıyor..."
	@cd $(BUILD_DIR)/tests && ./DbManagerTest -silent -functions testSha256Exists
	@cd $(BUILD_DIR)/tests && ./DashboardTest -silent -functions testInitialization

# Temizle
clean:
	@echo "Build dizinini temizliyor..."
	@rm -rf $(BUILD_DIR)/*

# Yardım bilgisi
help:
	@echo "Kullanılabilir hedefler:"
	@echo "  configure    - CMake yapılandırması oluştur"
	@echo "  build        - Projeyi derle"
	@echo "  test         - Testleri çalıştır"
	@echo "  test-verbose - Testleri ayrıntılı çıktı ile çalıştır"
	@echo "  test-dashboard - Sadece Dashboard testlerini çalıştır"
	@echo "  test-dbmanager - Sadece DbManager testlerini çalıştır"
	@echo "  test-mainwindow - Sadece MainWindow testlerini çalıştır"
	@echo "  build-and-test - Derle ve testleri çalıştır"
	@echo "  coverage     - Test kapsam raporlamasını derle ve çalıştır"
	@echo "  benchmark    - Performans testlerini çalıştır"
	@echo "  clean        - Build dizinini temizle"
	@echo "  help         - Bu yardım mesajını göster"
	@echo ""
	@echo "Değişkenler:"
	@echo "  BUILD_DIR=<dizin>   - Build dizini (default: build)"
	@echo "  CMAKE_FLAGS=<flags> - CMake için ekstra bayraklar" 