//
// Created by Ekrem Ünal on 10.05.2025.
//

#include "VirusTotalManager.h"
#include <QCoreApplication>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QFileDialog>
#include <QApplication>
#include <QTimer>
#include <QPointer>
#include <QString>
#include <QRegularExpression>

// Define error code namespace for better error handling
namespace VTErrorCodes {
    // Status codes
    const QString NOT_SUBMITTED = QStringLiteral("not_submitted");
    const QString UPLOADING = QStringLiteral("uploading");
    const QString SUBMITTED_SUCCESSFULLY = QStringLiteral("submitted_successfully");
    const QString SCAN_CANCELLED = QStringLiteral("scan_cancelled");
    
    // Error codes
    const QString ERROR_INVALID_API_KEY = QStringLiteral("error_invalid_api_key");
    const QString ERROR_NO_FILE_SELECTED = QStringLiteral("error_no_file_selected");
    const QString ERROR_FILE_ACCESS = QStringLiteral("error_file_access");
    const QString ERROR_UNEXPECTED_RESPONSE = QStringLiteral("error_unexpected_response");
    const QString ERROR_UNEXPECTED_FORMAT = QStringLiteral("error_unexpected_format");
    const QString ERROR_INVALID_JSON = QStringLiteral("error_invalid_json");
    const QString ERROR_NETWORK = QStringLiteral("error_network");
    const QString ERROR_INVALID_REPORT_JSON = QStringLiteral("error_invalid_report_json");
    const QString ERROR_NETWORK_REPORT = QStringLiteral("error_network_report");
}

/**
 * @brief Validates if the provided API key has a valid format
 * @param apiKey The API key to validate
 * @return True if the API key format appears valid, false otherwise
 */
bool VirusTotalManager::isValidApiKeyFormat(const QString& apiKey) const {
    // VirusTotal API keys are typically 64 characters long
    // and contain only hexadecimal characters
    const QRegularExpression hexRegex(QStringLiteral("^[a-fA-F0-9]+$"));
    
    // Basic validation - length and character check
    if (apiKey.length() < 32) {
        return false;
    }
    
    // Check if API key contains only valid characters
    return hexRegex.match(apiKey).hasMatch();
}

/**
 * @brief Constructor for VirusTotalManager
 * @param apiKey Optional API key override. If empty, will be loaded from configuration
 * 
 * Initializes the manager with the provided API key or loads it from the configuration.
 * Sets up the initial status and scanning state.
 */
VirusTotalManager::VirusTotalManager(const QString& apiKey)
    : QObject(nullptr), // Initialize QObject base class
      m_apiKey(apiKey), 
      m_lastSubmissionStatus(VTErrorCodes::NOT_SUBMITTED),
      m_isScanning(false) {
    
    // If no API key provided, load it from AppConfig
    if (m_apiKey.isEmpty()) {
        m_apiKey = AppConfig::getInstance().getVirusTotalApiKey();
    }
    
    // Log warning if API key looks invalid
    if (m_apiKey.isEmpty()) {
        qWarning() << "VirusTotal API anahtarı ayarlanmamış! config.ini dosyasında [VirusTotal] bölümüne ApiKey ekleyin.";
    } else if (m_apiKey.length() < 32) {
        qWarning() << "VirusTotal API anahtarı geçersiz görünüyor! VirusTotal API anahtarları genellikle 64 karakter uzunluğundadır.";
    }
}

/**
 * @brief Scans the currently selected file
 * @return True if scan was successfully initiated, false otherwise
 * 
 * Initiates a scan for the currently selected file.
 * If no file has been selected, returns false and logs a warning.
 */
bool VirusTotalManager::scan() {
    if (m_selectedFile.exists()) {
        return scanFile(m_selectedFile.absoluteFilePath());
    }
    
    qWarning() << "Error: No file selected for scanning. Use selectFile() first.";
    return false;
}

/**
 * @brief Opens a file dialog for the user to select a file for scanning
 * @return True if a file was successfully selected, false otherwise
 * 
 * Opens a file dialog allowing the user to select a file for scanning.
 * The selected file is stored in m_selectedFile for later use.
 */
bool VirusTotalManager::selectFile() {
    QString filePath = QFileDialog::getOpenFileName(
        QApplication::activeWindow(),
        QStringLiteral("Select File to Scan"),
        QDir::homePath(),
        QStringLiteral("All Files (*.*)"));
    
    if (filePath.isEmpty()) {
        return false;
    }
    
    m_selectedFile = QFileInfo(filePath);
    return m_selectedFile.exists();
}

/**
 * @brief Gets information about the currently selected file
 * @return QFileInfo object for the selected file
 * 
 * Returns a QFileInfo object representing the currently selected file.
 * If no file is selected, the returned QFileInfo will be invalid.
 */
QFileInfo VirusTotalManager::getSelectedFile() const {
    return m_selectedFile;
}

/**
 * @brief Checks if a scan is currently in progress
 * @return True if scanning, false otherwise
 * 
 * Returns whether the scanner is currently processing a file.
 * Thread-safe through the use of std::atomic.
 */
bool VirusTotalManager::isScanning() const {
    return m_isScanning;
}

/**
 * @brief Gets the scan results of the last completed scan
 * @return JSON string containing the scan results
 * 
 * Returns a JSON-formatted string containing the results of the
 * most recently completed scan. Returns empty string if no scan
 * has been performed.
 */
QString VirusTotalManager::getResults() const {
    return m_lastResults;
}

/**
 * @brief Cancels an ongoing scan operation
 * @return True if scan was successfully canceled, false if no active scan
 * 
 * Attempts to cancel any ongoing scan operation by aborting the current
 * network request. Updates the status and scanning flag accordingly.
 */
bool VirusTotalManager::cancelScan() {
    if (!m_isScanning || !m_currentReply) {
        return false;
    }
    
    // Cancel current network request
    m_currentReply->abort();
    m_currentReply = nullptr;
    m_isScanning = false;
    m_lastSubmissionStatus = VTErrorCodes::SCAN_CANCELLED;
    return true;
}

/**
 * @brief Submits the selected file to the VirusTotal service
 * @param apiKey Optional API key to use for this submission
 * @return True if submission was successful, false otherwise
 * 
 * Submits the currently selected file to the VirusTotal API for analysis.
 * Uses the provided API key or falls back to the stored one.
 * Performs validation and handles file upload asynchronously.
 */
bool VirusTotalManager::submitToRemoteService(const QString& apiKey) {
    // Use provided API key or the one set in constructor
    QString effectiveApiKey = apiKey.isEmpty() ? m_apiKey : apiKey;
    
    // Validate API key
    if (effectiveApiKey.isEmpty()) {
        qWarning() << "Error: VirusTotal API key is not set or invalid.";
        m_lastSubmissionStatus = VTErrorCodes::ERROR_INVALID_API_KEY;
        return false;
    }
    
    // Additional API key validation (basic format check)
    // VirusTotal API keys are typically 64 hexadecimal characters
    if (effectiveApiKey.length() < 32 || !isValidApiKeyFormat(effectiveApiKey)) {
        qWarning() << "Error: VirusTotal API key has invalid format. Check configuration.";
        m_lastSubmissionStatus = VTErrorCodes::ERROR_INVALID_API_KEY;
        return false;
    }
    
    // Validate file selection
    if (!m_selectedFile.exists()) {
        qWarning() << "Error: No file selected for scanning. Use selectFile() first.";
        m_lastSubmissionStatus = VTErrorCodes::ERROR_NO_FILE_SELECTED;
        return false;
    }
    
    // Perform the actual file submission asynchronously
    QFile* file = new QFile(m_selectedFile.absoluteFilePath());
    if (!file->open(QIODevice::ReadOnly)) {
        qWarning() << "Error: Could not open file:" << m_selectedFile.absoluteFilePath();
        delete file;
        m_lastSubmissionStatus = VTErrorCodes::ERROR_FILE_ACCESS;
        return false;
    }
    
    // Prepare multipart HTTP request for file upload
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                      QVariant(QStringLiteral("form-data; name=\"file\"; filename=\"%1\"").arg(m_selectedFile.fileName())));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QStringLiteral("application/octet-stream")));
    filePart.setBodyDevice(file);
    file->setParent(multiPart); // QHttpMultiPart takes ownership of the file
    multiPart->append(filePart);
    
    // Prepare network request with API endpoint and headers
    QUrl apiUrl(AppConfig::getInstance().getVirusTotalFilesUrl());
    QNetworkRequest request(apiUrl);
    request.setRawHeader("accept", "application/json");
    request.setRawHeader("x-apikey", effectiveApiKey.toUtf8());
    
    // Log upload attempt and update status
    qDebug() << "Uploading file to VirusTotal:" << m_selectedFile.absoluteFilePath();
    
    m_isScanning = true;
    m_lastSubmissionStatus = VTErrorCodes::UPLOADING;
    
    // Store the reply for potential cancellation
    m_currentReply = m_networkManager.post(request, multiPart);
    multiPart->setParent(m_currentReply); // QNetworkReply takes ownership
    
    // Connect signals to handle response using weak pointer pattern to prevent dangling pointers
    QPointer<VirusTotalManager> weakThis = this;
    QObject::connect(m_currentReply, &QNetworkReply::finished, [this, weakThis]() {
        // Prevent processing if the object has been deleted
        if (!weakThis) return; 
        
        // Store reply pointer and reset member variables
        QNetworkReply* reply = m_currentReply;
        m_currentReply = nullptr;
        m_isScanning = false;
        
        if (reply->error() == QNetworkReply::NoError) {
            // Parse response JSON
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
            
            if (!jsonResponse.isNull() && jsonResponse.isObject()) {
                QJsonObject jsonObject = jsonResponse.object();
                
                // Successful response with data object
                if (jsonObject.contains(QStringLiteral("data")) && jsonObject[QStringLiteral("data")].isObject()) {
                    QJsonObject dataObject = jsonObject[QStringLiteral("data")].toObject();
                    
                    // Validate analysis ID and type
                    if (dataObject.contains(QStringLiteral("id")) && dataObject[QStringLiteral("id")].isString() &&
                        dataObject.contains(QStringLiteral("type")) && dataObject[QStringLiteral("type")].toString() == QStringLiteral("analysis")) {
                        QString analysisId = dataObject[QStringLiteral("id")].toString();
                        qInfo() << "File successfully uploaded. Analysis ID:" << analysisId;
                        m_lastAnalysisId = analysisId;
                        m_lastSubmissionStatus = VTErrorCodes::SUBMITTED_SUCCESSFULLY;
                        
                        // Start polling for results - first attempt after 5 seconds
                        m_isScanning = true; // Keep scanning flag on during polling
                        startPollingForResults(analysisId);
                    } else {
                        qWarning() << "Error: Expected 'data.id' or 'data.type' not found in response.";
                        m_lastSubmissionStatus = VTErrorCodes::ERROR_UNEXPECTED_RESPONSE;
                    }
                } 
                // Error response
                else if (jsonObject.contains(QStringLiteral("error")) && jsonObject[QStringLiteral("error")].isObject()) {
                    QJsonObject errorObject = jsonObject[QStringLiteral("error")].toObject();
                    QString errorCode = errorObject.value(QStringLiteral("code")).toString(QStringLiteral("Unknown Code"));
                    QString errorMessage = errorObject.value(QStringLiteral("message")).toString(QStringLiteral("Unknown error message."));
                    qWarning() << "VirusTotal API Error:" << errorCode << "-" << errorMessage;
                    m_lastSubmissionStatus = VTErrorCodes::ERROR_UNEXPECTED_RESPONSE + QStringLiteral("_") + errorCode;
                } 
                // Unexpected response format
                else {
                    qWarning() << "Error: Response format is different than expected.";
                    m_lastSubmissionStatus = VTErrorCodes::ERROR_UNEXPECTED_FORMAT;
                }
            } else {
                qWarning() << "Error: Invalid JSON response received.";
                m_lastSubmissionStatus = VTErrorCodes::ERROR_INVALID_JSON;
            }
        } else {
            qWarning() << "Network Error:" << reply->errorString();
            m_lastSubmissionStatus = VTErrorCodes::ERROR_NETWORK;
        }
        
        reply->deleteLater();
    });
    
    return true;
}

/**
 * @brief Retrieves the current status of the file submission
 * @return Status string describing the current state of the submission
 * 
 * Returns a string describing the current status of the file submission.
 * Possible values include "not_submitted", "uploading", "submitted_successfully",
 * "scan_cancelled", "completed", and various error status codes.
 */
QString VirusTotalManager::getSubmissionStatus() const {
    return m_lastSubmissionStatus;
}

/**
 * @brief Scans a specific file using the VirusTotal service
 * @param filePath The path to the file that should be scanned
 * @return True if the scan was successfully initiated, false otherwise
 * 
 * Updates the selected file and submits it to the VirusTotal service for scanning.
 * Verifies that the file exists before attempting to submit it.
 */
bool VirusTotalManager::scanFile(const QString& filePath) {
    // Update the selected file
    m_selectedFile = QFileInfo(filePath);
    
    if (!m_selectedFile.exists()) {
        qWarning() << "Error: File does not exist:" << filePath;
        return false;
    }
    
    // Submit the file to VirusTotal for scanning
    return submitToRemoteService();
}

/**
 * @brief Retrieves the analysis report for a submitted file
 * @param analysisId The ID of the analysis to retrieve
 * @return Analysis report as a JSON string or error message
 * 
 * Fetches a detailed analysis report from VirusTotal using the provided analysis ID.
 * Returns a JSON string containing the analysis results or an appropriate error message.
 */
QString VirusTotalManager::getAnalysisReport(const QString& analysisId) {
    // Validate required parameters
    if (m_apiKey.isEmpty()) {
        qWarning() << "Error: VirusTotal API key is not set or invalid.";
        return QStringLiteral("{\"error\": \"Invalid or missing API key.\"}");
    }
    
    if (analysisId.isEmpty()) {
        qWarning() << "Error: Analysis ID cannot be empty.";
        return QStringLiteral("{\"error\": \"Analysis ID cannot be empty.\"}");
    }

    // Create URL with analysis ID using the template from AppConfig
    QUrl apiUrl(QString(AppConfig::getInstance().getVirusTotalAnalysesUrl()).arg(analysisId));
    QNetworkRequest request(apiUrl);
    request.setRawHeader("accept", "application/json");
    request.setRawHeader("x-apikey", m_apiKey.toUtf8());

    qDebug() << "Retrieving analysis report from VirusTotal, ID:" << analysisId;

    // Set up a promise/future mechanism to make this synchronous but without blocking event loop
    QSharedPointer<QString> resultPtr = QSharedPointer<QString>::create(QStringLiteral("{\"status\": \"pending\"}"));
    QSharedPointer<bool> completedPtr = QSharedPointer<bool>::create(false);
    QSharedPointer<QEventLoop> loopPtr = QSharedPointer<QEventLoop>::create();
    
    // Use the class member network manager to send the request
    QNetworkReply* reply = m_networkManager.get(request);
    
    // Handle the asynchronous response with no captured references to stack variables
    QPointer<VirusTotalManager> weakThis = this;
    QObject::connect(reply, &QNetworkReply::finished, [weakThis, resultPtr, completedPtr, loopPtr, reply]() {
        // Ensure object still exists
        if (!weakThis) {
            *completedPtr = true;
            loopPtr->quit();
            return;
        }
        
        // Process successful response
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
            
            if (!jsonResponse.isNull()) {
                qDebug() << "Report response received. Size:" << responseData.size() << "bytes.";
                *resultPtr = QString::fromUtf8(jsonResponse.toJson(QJsonDocument::Compact));
                
                // Update status based on report
                QJsonObject jsonObject = jsonResponse.object();
                if (jsonObject.contains(QStringLiteral("data")) && jsonObject[QStringLiteral("data")].isObject()) {
                    QJsonObject dataObject = jsonObject[QStringLiteral("data")].toObject();
                    if (dataObject.contains(QStringLiteral("attributes")) && dataObject[QStringLiteral("attributes")].isObject()) {
                        QJsonObject attrsObject = dataObject[QStringLiteral("attributes")].toObject();
                        if (attrsObject.contains(QStringLiteral("status"))) {
                            weakThis->m_lastSubmissionStatus = attrsObject[QStringLiteral("status")].toString();
                        }
                    }
                }
                
                // Store the results in the class member
                weakThis->m_lastResults = *resultPtr;
            } else {
                qWarning() << "Error: Invalid JSON response received (Analysis Report).";
                *resultPtr = QStringLiteral("{\"error\": \"Invalid JSON response for report.\"}");
                if (weakThis) {
                    weakThis->m_lastSubmissionStatus = VTErrorCodes::ERROR_INVALID_REPORT_JSON;
                    weakThis->m_lastResults = *resultPtr;
                }
            }
        } else {
            qWarning() << "Network Error (Analysis Report):" << reply->errorString();
            if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).isValid()) {
                qWarning() << "HTTP Status Code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            }
            *resultPtr = QStringLiteral("{\"error\": \"Network error getting report.\"}");
            if (weakThis) {
                weakThis->m_lastSubmissionStatus = VTErrorCodes::ERROR_NETWORK_REPORT;
                weakThis->m_lastResults = *resultPtr;
            }
        }
        
        *completedPtr = true;
        loopPtr->quit();
    });
    
    // Set timeout to avoid hanging indefinitely
    QTimer::singleShot(30000, [completedPtr, loopPtr]() {
        if (!(*completedPtr)) {
            *completedPtr = true;
            loopPtr->quit();
        }
    });
    
    // Wait for completion
    loopPtr->exec();
    
    // Clean up resources
    if (reply->isFinished()) {
        reply->deleteLater();
    } else {
        reply->abort();
        reply->deleteLater();
        return QStringLiteral("{\"error\": \"Request timed out.\"}");
    }
    
    return *resultPtr;
}

/**
 * @brief Starts polling for VirusTotal analysis results.
 * @param analysisId The ID of the analysis to poll for.
 * 
 * Initiates a polling mechanism that periodically checks if the analysis is complete.
 * Will attempt up to 5 times with increasing delay between attempts.
 */
void VirusTotalManager::startPollingForResults(const QString& analysisId) {
    // Store the analysis ID and attempt count in static variables
    static int pollingAttempt = 0;
    static QString currentAnalysisId;
    
    // Reset attempt count if this is a new analysis
    if (currentAnalysisId != analysisId) {
        currentAnalysisId = analysisId;
        pollingAttempt = 0;
    }
    
    // Increment attempt counter
    pollingAttempt++;
    
    // Calculate delay with increasing backoff (5s, 10s, 20s, 30s, 60s)
    int delay = 5000;
    if (pollingAttempt == 2) delay = 10000;
    else if (pollingAttempt == 3) delay = 20000;
    else if (pollingAttempt == 4) delay = 30000;
    else if (pollingAttempt >= 5) delay = 60000;
    
    // Maximum 5 polling attempts
    if (pollingAttempt <= 5) {
        qDebug() << "Polling for VirusTotal results: attempt" << pollingAttempt 
                 << "for analysis" << analysisId << "with delay" << delay/1000 << "seconds";
        
        QTimer::singleShot(delay, this, [this, analysisId]() {
            if (!this) return; // Check if the object still exists
            
            // Fetch the analysis report
            QString results = getAnalysisReport(analysisId);
            qDebug() << "Analysis results obtained in polling. Emitting signal.";
            m_lastResults = results;
            
            // Check if analysis is completed
            QJsonDocument jsonDoc = QJsonDocument::fromJson(results.toUtf8());
            bool isCompleted = false;
            
            if (!jsonDoc.isNull() && jsonDoc.isObject()) {
                QJsonObject rootObj = jsonDoc.object();
                if (rootObj.contains(QLatin1String("data")) && rootObj[QLatin1String("data")].isObject()) {
                    QJsonObject dataObj = rootObj[QLatin1String("data")].toObject();
                    if (dataObj.contains(QLatin1String("attributes")) && dataObj[QLatin1String("attributes")].isObject()) {
                        QJsonObject attrsObj = dataObj[QLatin1String("attributes")].toObject();
                        if (attrsObj.contains(QLatin1String("status"))) {
                            QString status = attrsObj[QLatin1String("status")].toString();
                            isCompleted = (status == QLatin1String("completed"));
                        }
                    }
                }
            }
            
            // Emit results regardless of completion status to update UI
            emit analysisResultsReady(m_lastResults);
            
            // If not completed, continue polling unless we reached max attempts
            if (!isCompleted) {
                startPollingForResults(analysisId);
            } else {
                // Reset scanning flag when completed
                m_isScanning = false;
                pollingAttempt = 0;
            }
        });
    } else {
        // Max attempts reached, stop polling
        qDebug() << "Max polling attempts reached for analysis" << analysisId;
        m_isScanning = false;
        pollingAttempt = 0;
        
        // Emit one final signal with the last results we have
        emit analysisResultsReady(m_lastResults + QStringLiteral("\n\nAnaliz zaman aşımına uğradı. Sonuç tam olmayabilir."));
    }
}

/**
 * VirusTotal Integration Usage Example
 * 
 * This class replaces the previous free functions like uploadFileToVirusTotal and getVirusTotalFileReport.
 * Below is an example of how to use this class in your application:
 * 
 * Example usage in main.cpp or another file:
 * 
 * ```cpp
 * // Create the VirusTotal manager (API key loaded from configuration)
 * auto vtManager = new VirusTotalManager();
 * 
 * // Select a file for scanning (shows file dialog)
 * if (vtManager->selectFile()) {
 *     // Submit the file to VirusTotal
 *     if (vtManager->submitToRemoteService()) {
 *         qDebug() << "File submitted to VirusTotal";
 *         
 *         // Check submission status
 *         qDebug() << "Status:" << vtManager->getSubmissionStatus();
 *         
 *         // Wait for results (in a real app, use signals/slots instead of polling)
 *         while (vtManager->isScanning()) {
 *             QThread::msleep(1000);
 *             QCoreApplication::processEvents();
 *         }
 *         
 *         // Display results
 *         qDebug() << "Results:" << vtManager->getResults();
 *     }
 * }
 * ```
 * 
 * For hash-based lookups, a separate method could be added in the future.
 */