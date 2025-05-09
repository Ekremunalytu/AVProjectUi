#ifndef TESTDATAFACTORY_H
#define TESTDATAFACTORY_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <random>
#include <vector>

/**
 * @class TestDataFactory
 * @brief Test verilerini oluşturmak için yardımcı sınıf.
 */
class TestDataFactory {
public:
    /**
     * Rastgele SHA256 hash değeri oluşturur.
     */
    static QString generateSha256Hash() {
        static const char hexChars[] = "0123456789abcdef";
        QString hash;
        hash.reserve(64); // SHA256 hash 64 hex karakter içerir

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15); // 0-15 arası hex karakteri

        for (int i = 0; i < 64; ++i) {
            hash.append(hexChars[dis(gen)]);
        }

        return hash;
    }

    /**
     * Belirtilen sayıda rastgele SHA256 hash listesi oluşturur.
     */
    static QStringList generateSha256HashList(int count) {
        QStringList hashList;
        for (int i = 0; i < count; ++i) {
            hashList.append(generateSha256Hash());
        }
        return hashList;
    }

    /**
     * Örnek dosya yolları oluşturur.
     */
    static QStringList generateSampleFilePaths(int count) {
        static const std::vector<QString> basePaths = {
            "/Users/test/Documents/",
            "/home/user/Downloads/",
            "C:/Program Files/",
            "D:/Projects/",
            "/var/www/",
        };

        static const std::vector<QString> fileNames = {
            "document.pdf",
            "image.jpg",
            "presentation.pptx",
            "archive.zip",
            "executable.exe",
            "script.sh",
            "data.csv",
            "config.json",
        };

        QStringList paths;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> baseDis(0, basePaths.size() - 1);
        std::uniform_int_distribution<> fileDis(0, fileNames.size() - 1);

        for (int i = 0; i < count; ++i) {
            QString path = basePaths[baseDis(gen)] + fileNames[fileDis(gen)];
            paths.append(path);
        }

        return paths;
    }

    /**
     * Belirli bir zaman aralığında rastgele tarih/saat oluşturur.
     */
    static QDateTime generateRandomDateTime(const QDateTime& start, const QDateTime& end) {
        qint64 startMSecs = start.toMSecsSinceEpoch();
        qint64 endMSecs = end.toMSecsSinceEpoch();
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<qint64> dis(startMSecs, endMSecs);
        
        return QDateTime::fromMSecsSinceEpoch(dis(gen));
    }
};

#endif // TESTDATAFACTORY_H 