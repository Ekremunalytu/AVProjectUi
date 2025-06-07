#include <iostream>
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "src/security/cdr/CDRScanner.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    std::cout << "=== Testing CDR Scanner Fix ===" << std::endl;
    
    // Create CDRScanner instance
    CDRScanner scanner;
    
    // Test file path
    QString testFile = "/Volumes/Crucial/AVProjectUi/malicious_test.pdf";
    
    if (!QFile::exists(testFile)) {
        std::cerr << "❌ Test file not found: " << testFile.toStdString() << std::endl;
        return 1;
    }
    
    std::cout << "✅ Found test file: " << testFile.toStdString() << std::endl;
    
    // Set up signal handlers
    bool completed = false;
    QString result;
    QString error;
    
    QObject::connect(&scanner, &CDRScanner::scanResultsReady, 
                     [&](const QString& results) {
        std::cout << "\n🎉 SCAN COMPLETED SUCCESSFULLY!" << std::endl;
        result = results;
        completed = true;
        app.quit();
    });
    
    QObject::connect(&scanner, QOverload<ScannerErrorCode, const QString&>::of(&CDRScanner::scanError),
                     [&](ScannerErrorCode code, const QString& errorMsg) {
        std::cout << "\n❌ SCAN FAILED!" << std::endl;
        std::cout << "Error Code: " << static_cast<int>(code) << std::endl;
        std::cout << "Error Message: " << errorMsg.toStdString() << std::endl;
        error = errorMsg;
        completed = true;
        app.quit();
    });
    
    // Set up timeout
    QTimer::singleShot(30000, [&]() {
        std::cout << "\n⏰ TIMEOUT: Scan took longer than 30 seconds" << std::endl;
        app.quit();
    });
    
    // Start scan
    std::cout << "\n🔍 Starting CDR scan..." << std::endl;
    bool started = scanner.scanFile(testFile);
    
    if (!started) {
        std::cerr << "❌ Failed to start scan: " << scanner.getLastError().toStdString() << std::endl;
        return 1;
    }
    
    std::cout << "⏳ Scan started, waiting for completion..." << std::endl;
    
    // Run event loop
    app.exec();
    
    // Print results
    if (!completed) {
        std::cout << "\n❌ Scan did not complete (timeout)" << std::endl;
        return 1;
    }
    
    if (!error.isEmpty()) {
        std::cout << "\n❌ FINAL RESULT: FAILED" << std::endl;
        return 1;
    }
    
    if (!result.isEmpty()) {
        std::cout << "\n✅ FINAL RESULT: SUCCESS" << std::endl;
        std::cout << "\nScan Results:" << std::endl;
        std::cout << result.toStdString() << std::endl;
        return 0;
    }
    
    std::cout << "\n⚠️ FINAL RESULT: UNKNOWN (no result received)" << std::endl;
    return 1;
}
