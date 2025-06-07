#include <iostream>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include "src/security/cdr/CDRScanner.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    std::cout << "Testing CDR Scanner Fix..." << std::endl;
    
    // Create CDRScanner instance
    CDRScanner scanner;
    
    // Set up test file path
    QString testFile = "/Volumes/Crucial/AVProjectUi/malicious_test.pdf";
    
    // Check if test file exists
    if (!QFile::exists(testFile)) {
        std::cerr << "Test file not found: " << testFile.toStdString() << std::endl;
        return 1;
    }
    
    std::cout << "Found test file: " << testFile.toStdString() << std::endl;
    std::cout << "Starting CDR scan..." << std::endl;
    
    // Connect signals to capture results
    bool scanCompleted = false;
    QString scanResults;
    QString scanError;
    
    QObject::connect(&scanner, &CDRScanner::scanResultsReady, [&](const QString& results) {
        std::cout << "✅ SIGNAL RECEIVED: scanResultsReady" << std::endl;
        scanResults = results;
        scanCompleted = true;
        app.quit();
    });
    
    QObject::connect(&scanner, QOverload<ScannerErrorCode, const QString&>::of(&CDRScanner::scanError), 
                     [&](ScannerErrorCode code, const QString& error) {
        std::cout << "❌ SIGNAL RECEIVED: scanError" << std::endl;
        scanError = error;
        scanCompleted = true;
        app.quit();
    });
    
    // Start the scan
    bool started = scanner.scanFile(testFile);
    if (!started) {
        std::cerr << "Failed to start scan: " << scanner.getLastError().toStdString() << std::endl;
        return 1;
    }
    
    std::cout << "Scan started successfully. Waiting for completion..." << std::endl;
    
    // Set up a timeout
    QTimer::singleShot(30000, &app, [&]() {
        std::cout << "⚠️ TIMEOUT: Scan did not complete within 30 seconds" << std::endl;
        app.quit();
    });
    
    // Run the event loop
    app.exec();
    
    // Print results
    std::cout << "\n=== SCAN RESULTS ===" << std::endl;
    std::cout << "Scan completed: " << (scanCompleted ? "YES" : "NO") << std::endl;
    
    if (!scanError.isEmpty()) {
        std::cout << "Error: " << scanError.toStdString() << std::endl;
        return 1;
    }
    
    if (!scanResults.isEmpty()) {
        std::cout << "Results:\n" << scanResults.toStdString() << std::endl;
        std::cout << "\n✅ CDR SCAN COMPLETED SUCCESSFULLY!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ No results received" << std::endl;
        return 1;
    }
}
