#ifndef TESTDATAFACTORY_H
#define TESTDATAFACTORY_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <random>
#include <vector>
#include <QRandomGenerator>
#include <QCryptographicHash>

/**
 * @brief Utility class for generating test data
 * 
 * This class provides factory methods to create various types of test data
 * for use in unit tests, such as hashes, file paths, and random strings.
 */
class TestDataFactory {
public:
    /**
     * @brief Generates a random SHA256 hash string
     * @return A randomly generated SHA256 hash as a hex string
     */
    static QString generateSha256Hash() {
        // Generate 32 random bytes (256 bits for SHA256)
        QByteArray randomData;
        for (int i = 0; i < 32; i++) {
            randomData.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
        }
        
        // Convert to SHA256 hash string
        QByteArray hash = QCryptographicHash::hash(randomData, QCryptographicHash::Sha256);
        return QString::fromLatin1(hash.toHex());
    }

    /**
     * @brief Generates a random string of specified length
     * @param length Length of the string to generate
     * @return A randomly generated string
     */
    static QString generateRandomString(int length = 10) {
        const QString possibleChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        QString randomString;
        randomString.reserve(length);
        
        for (int i = 0; i < length; i++) {
            int index = QRandomGenerator::global()->bounded(possibleChars.length());
            QChar nextChar = possibleChars.at(index);
            randomString.append(nextChar);
        }
        
        return randomString;
    }

    /**
     * @brief Generates example file paths for testing
     * @return A file path string for testing
     */
    static QString generateFilePath() {
        QString basePath;
        
        // Generate a random platform-appropriate path
        #ifdef _WIN32
            basePath = "C:/Test";
        #else
            basePath = "/tmp/test";
        #endif
        
        // Add random subdirectories and file name
        return QString("%1/%2/%3.txt").arg(
            basePath,
            generateRandomString(5),
            generateRandomString(8)
        );
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