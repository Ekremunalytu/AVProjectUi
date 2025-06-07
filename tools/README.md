# AVProjectUi Tools

Bu dizin proje geliştirme ve yönetimi için araçlar içerir.

## 🛠️ Ana Araçlar

### `quick.sh` - Hızlı Proje Yönetimi
Proje için en sık kullanılan işlemleri hızlıca yapmanızı sağlar:
```bash
./tools/quick.sh status    # Proje durumu
./tools/quick.sh cleanup   # Build temizliği
./tools/quick.sh docs      # Dokümantasyon oluştur ve serve et
./tools/quick.sh test      # Testleri çalıştır
```

## 📁 Scripts Dizini

### 📚 Dokümantasyon
- `docs.sh` - Ana dokümantasyon yöneticisi
- `serve_docs.py` - Gelişmiş dokümantasyon sunucusu
- `generate_comprehensive_uml.py` - UML diagram oluşturucu
- `verify_uml_coverage.py` - UML coverage kontrolü

### 🧹 Bakım
- `cleanup_build.sh` - Build artifactlarını temizler
- `pre-commit` - Git pre-commit hook'u

### 🔧 Geliştirme
- `reorganize_modules.sh` - Modül reorganizasyon scripti

## 🚀 Hızlı Başlangıç

1. **İlk kurulum:**
   ```bash
   make install-hooks  # Git hooks kurulumu
   ```

2. **Dokümantasyon:**
   ```bash
   ./tools/quick.sh docs
   # veya
   ./tools/scripts/docs.sh build-serve
   ```

3. **Build ve Test:**
   ```bash
   ./tools/quick.sh test
   # veya
   make build-and-test
   ```

4. **Temizlik:**
   ```bash
   ./tools/quick.sh cleanup
   ```

## 📋 VS Code Tasks

Projenizde mevcut VS Code task'larını da kullanabilirsiniz:
- `📚 Generate Documentation`
- `🚀 Serve Documentation`
- `🏗️ Build Project`
- `🧪 Run Tests`

## ⚡ Tips

- `quick.sh` scriptini proje kök dizininden çalıştırabilirsiniz: `./tools/quick.sh`
- Dokümantasyon otomatik olarak browser'da açılır
- Pre-commit hook kodunuzu otomatik kontrol eder
- Build temizliği disk alanı kazandırır
