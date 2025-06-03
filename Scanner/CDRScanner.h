// Scanner/CDRScanner.h
#ifndef CDR_SCANNER_H
#define CDR_SCANNER_H

#include "Interface/IDockerScanner.h"
#include "Interface/ScannerTypes.h"
#include "Docker/include/docker/DockerManager.h" // Assuming DockerManager.h is the main header for your Docker library
#include <QString>
#include <memory> // For std::unique_ptr
#include <QFileInfo>

// Include CDR specific headers
#include "Docker/include/cdr/CdrManager.h"
#include "Docker/include/cdr/CdrTypes.h"

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
    QString getContainerStatus() const override;    // CDRScanner specific methods
    QString getSanitizedFilePath() const; // Get the path of the processed file
    bool getThreatsDetected() const { return threatsDetected; }
    bool getWasSanitized() const { return wasSanitized; }
    QString getAnalysisDetails() const { return analysisDetails; }
    
    // CDR configuration methods
    void setCdrConfiguration(const CDR::CdrConfiguration& config) { cdrConfig = config; }
    CDR::CdrConfiguration getCdrConfiguration() const { return cdrConfig; }
    void setOutputDirectory(const QString& outputDir) { cdrConfig.outputDirectory = outputDir.toStdString(); }
    void setQuarantineDirectory(const QString& quarantineDir) { cdrConfig.quarantineDirectory = quarantineDir.toStdString(); }

private:
    std::unique_ptr<Docker::DockerManager> dockerManager;
    std::unique_ptr<CDR::CdrManager> cdrManager; // CDR manager for proper CDR operations
    
    QString currentFilePath;
    QString lastError;
    ScanStatus currentStatus;
    QString sanitizedFilePath; // Store path to the sanitized file
    QString cdrContainerName;
    QString cdrImageName;
    
    // CDR configuration
    CDR::CdrConfiguration cdrConfig;
    
    // CDR analysis results
    bool threatsDetected = false;
    bool wasSanitized = false;
    QString analysisDetails;
    
    // Helper methods
    void initializeCdrComponents();
    bool ensureCdrContainerRunning();
    bool createCdrContainer();
    bool isCdrContainerRunning() const;
};

#endif // CDR_SCANNER_H
