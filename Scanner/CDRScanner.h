// Scanner/CDRScanner.h
#ifndef CDR_SCANNER_H
#define CDR_SCANNER_H

#include "Interface/IDockerScanner.h"
#include "Interface/ScannerTypes.h"
#include "Docker/include/docker/DockerManager.h" // Assuming DockerManager.h is the main header for your Docker library
#include <QString>
#include <memory> // For std::unique_ptr
#include <QFileInfo>

// Forward declarations for CDR specific classes if their headers are not included directly
// namespace Cdr {
// class CdrManager; 
// class CdrSanitizer;
// }

class CDRScanner : public IDockerScanner {
public:
    CDRScanner();
    ~CDRScanner() override = default;

    // IScanner interface methods
    bool selectFile() override;
    bool scanFile(const QString& filePath) override;
    QFileInfo getSelectedFile() const override;
    QString getResults() const override;
    bool isScanning() const override;
    bool cancelScan() override;
    QString getLastError() const override;
    ScannerType getType() const;
    ScanStatus getStatus() const;
    void setFile(const QString& filePath) override;
    QString getFile() const override;

    // IDockerScanner interface methods
    bool submitToContainer(const QString& containerName) override;
    bool isContainerReady() const override;
    QString getContainerStatus() const override;

    // CDRScanner specific methods
    QString getSanitizedFilePath() const; // Example method to get the path of the sanitized file

private:
    std::unique_ptr<Docker::DockerManager> dockerManager;
    // std::unique_ptr<Cdr::CdrManager> cdrManager; // If CdrManager is used
    // std::unique_ptr<Cdr::CdrSanitizer> cdrSanitizer; // If CdrSanitizer is used
    
    QString currentFilePath;
    QString lastError;
        ScanStatus currentStatus;
    QString sanitizedFilePath; // Store path to the sanitized file
    QString cdrContainerName;
    QString cdrImageName;
    
    // Helper methods
    void initializeCdrComponents();
    bool ensureCdrContainerRunning();
    bool createCdrContainer();
    bool isCdrContainerRunning() const;
};

#endif // CDR_SCANNER_H
