#ifndef MOCKDBMANAGER_H
#define MOCKDBMANAGER_H

#include "Interface/IDbManager.h"
#include <QStringView>
#include <map>
#include <string>

/**
 * @class MockDbManager
 * @brief IDbManager arayüzünün mock implementasyonu
 * 
 * Bu sınıf, veritabanı bağlantısını taklit eder ve testlerde 
 * gerçek veritabanı olmadan kullanılabilir.
 */
class MockDbManager : public IDbManager {
public:
    MockDbManager() : connected(false) {}
    ~MockDbManager() override = default;

    std::error_code connectDatabase(QStringView dbPath) noexcept override {
        lastDbPath = dbPath.toString().toStdString();
        connected = true;
        return {};
    }

    bool isDatabaseConnected() const noexcept override {
        return connected;
    }

    bool isSha256Exists(QStringView sha256Hash, std::error_code& ec) override {
        if (!connected) {
            ec = std::make_error_code(std::errc::not_connected);
            return false;
        }
        auto it = hashStore.find(sha256Hash.toString().toStdString());
        ec.clear();
        return it != hashStore.end();
    }

    long getSignatureCount(std::error_code& ec) override {
        if (!connected) {
            ec = std::make_error_code(std::errc::not_connected);
            return -1;
        }
        ec.clear();
        return static_cast<long>(hashStore.size());
    }

    // Mock için özel metodlar
    void addHashToStore(const QString& hash) {
        hashStore.insert(hash.toStdString());
    }

    void clearHashStore() {
        hashStore.clear();
    }

    void setConnected(bool state) {
        connected = state;
    }

    std::string getLastDbPath() const {
        return lastDbPath;
    }

private:
    bool connected;
    std::string lastDbPath;
    std::set<std::string> hashStore;
};

#endif // MOCKDBMANAGER_H 