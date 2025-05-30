//
// Created by Ekrem Ünal on 10.05.2025.
//

#ifndef VIRUSTOTALMANAGER_H
#define VIRUSTOTALMANAGER_H

#include <QString>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject> // Added for QObject inheritance
#include <atomic>
#include "../../Interface/IVirusTotalScanner.h"
#include "../../Core/AppConfig.h"

class VirusTotalManager : public QObject, public IVirusTotalScanner { // Inherit from QObject
    Q_OBJECT // Add Q_OBJECT macro
public:
    // Constructor that takes the API key
    explicit VirusTotalManager(const QString& apiKey = QString());
    virtual ~VirusTotalManager() = default;

    // IVirusTotalScanner interface implementation
    bool submitToRemoteService(const QString& apiKey = QString()) override;
    QString getSubmissionStatus() const override;
    
    // IScanner interface implementation
    bool selectFile() override;
    bool scanFile(const QString& filePath) override;
    QFileInfo getSelectedFile() const override;
    QString getResults() const override;
    bool isScanning() const override;
    bool cancelScan() override;
    QString getLastError() const override;
    void setFile(const QString& filePath) override;
    QString getFile() const override;
    bool scan(); // Not in interface, so no override

    // Original methods
    QString getAnalysisReport(const QString& analysisId);
    void startPollingForResults(const QString& analysisId);

signals:
    // Signal emitted when analysis results are ready
    void analysisResultsReady(const QString& results);

private:
    // Helper function to validate API key format
    bool isValidApiKeyFormat(const QString& apiKey) const;
    
    QString m_apiKey;
    QString m_lastAnalysisId;
    QString m_lastSubmissionStatus;
    QString m_lastResults;
    QString m_lastError;
    QFileInfo m_selectedFile;
    std::atomic<bool> m_isScanning;
    QNetworkAccessManager m_networkManager;
    QNetworkReply* m_currentReply = nullptr;
    QString m_currentPollingAnalysisId; // Added member variable
    int m_pollingAttempt = 0;          // Added member variable
};

#endif //VIRUSTOTALMANAGER_H
