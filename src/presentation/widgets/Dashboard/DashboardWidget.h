/**
 * @file DashboardWidget.h
 * @brief Main dashboard interface for malware scanning operations
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 */

#ifndef  DASHBOARDWIDGET_H
#define  DASHBOARDWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QString>
#include <memory>
#include "security/scanning/BasicScanner.h"
#include "infrastructure/network/VirusTotal/VirusTotalManager.h"
#include "infrastructure/network/Monitor/NetworkMonitor.h"
#include "security/cdr/CDRScanner.h"
#include "infrastructure/docker/DockerManager.h"
#include "security/cdr/CdrManager.h"
#include "../../../security/sandbox/SandboxManager.h" // Added for Sandbox management
#include "../../../infrastructure/docker/DockerTypes.h" // Added for Docker types
#include "../../../security/cdr/CdrTypes.h" // Added for CDR types

QT_BEGIN_NAMESPACE
namespace Ui { class DashboardWidget; }
QT_END_NAMESPACE

/**
 * @brief The DashboardWidget class provides the main dashboard interface for scanning operations.
 * 
 * @details This widget serves as the central hub for all malware scanning operations in the application.
 * It provides a tabbed interface that organizes different scanning methods and displays their results.
 * The dashboard integrates multiple scanning engines including basic hash-based scanning, 
 * VirusTotal cloud scanning, CDR (Content Disarm and Reconstruction), sandbox analysis, 
 * and network monitoring.
 * 
 * Key features:
 * - Tabbed interface for different scan types
 * - Real-time scan result display
 * - Network activity monitoring
 * - Integration with multiple scanning engines
 * - User-friendly scan configuration
 * 
 * @note This widget coordinates between multiple scanning services and provides
 *       a unified interface for malware analysis operations.
 */
class DashboardWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructs a DashboardWidget.
     * 
     * Initializes the dashboard interface, sets up scanning components,
     * configures the tabbed interface, and connects signal-slot relationships
     * between scanning engines and result display widgets.
     * 
     * @param parent The parent widget (nullptr by default)
     */
    explicit DashboardWidget(QWidget *parent = nullptr);
    
    /**
     * @brief Destroys the DashboardWidget and frees resources.
     * 
     * Cleans up scanning engines, stops any running operations,
     * and properly deallocates resources used by the dashboard.
     */
    ~DashboardWidget();

public slots:
    /**
     * @brief Handle results from VirusTotal analysis
     * 
     * Processes and displays analysis results received from the VirusTotal
     * cloud scanning service.
     * 
     * @param results Formatted string containing VirusTotal analysis results
     */
    void handleVirusTotalResults(const QString& results);
    
    /**
     * @brief Append new network log messages to the network monitoring display
     * 
     * Updates the network monitoring interface with new log messages
     * from the network monitor component.
     * 
     * @param logMessage New network log message to display
     */
    void appendNetworkLog(const QString& logMessage); // New slot for network logs

private slots:
    /**
     * @brief Handles click on the Basic Scan button in the Basic Scan tab.
     */
    void onBasicScanButtonClicked();
    
    /**
     * @brief Handles click on the Advanced Scan button in the Advanced Scan tab.
     * 
     * Initiates advanced scanning operations using multiple scanning engines
     * and enhanced analysis techniques.
     */
    void onAdvancedScanButtonClicked();
    
    /**
     * @brief Handles click on the CDR Scan button in the CDR tab.
     * 
     * Starts Content Disarm and Reconstruction scanning process
     * for sanitizing potentially malicious files.
     */
    void onCdrScanButtonClicked();
    
    /**
     * @brief Handles click on the Sandbox Scan button in the Sandbox tab.
     * 
     * Initiates sandbox analysis by executing files in an isolated
     * environment for behavioral analysis.
     */
    void onSandboxScanButtonClicked();
    
    /**
     * @brief Handles click on the Network Monitor button in the Network tab.
     * 
     * Starts or stops network monitoring to track network activity
     * and detect suspicious network behavior.
     */
    void onNetworkMonitorButtonClicked();
    
    /**
     * @brief Handles click on the Configuration button at the bottom of the interface.
     * 
     * Opens configuration dialog for customizing scanning settings,
     * API keys, and other application preferences.
     */
    void onConfigButtonClicked();
    
    /**
     * @brief Handles click on the Refresh button at the bottom of the interface.
     * 
     * Refreshes the dashboard state, updates scan results, and
     * synchronizes the interface with current scanning status.
     */
    void onRefreshButtonClicked();
    
    /**
     * @brief Handles the file selection for Basic Scan.
     * 
     * Opens a file dialog and initiates scanning of the selected file.
     */
    void onBasicScanSelectFile();
    
    /**
     * @brief Handles the file selection for Advanced Scan.
     * 
     * Opens a file dialog and processes the selected file for advanced scanning.
     */
    void onAdvancedScanSelectFile();
    
    /**
     * @brief Processes and displays scan results.
     * @param results String containing the scan results.
     */
    void onBasicScanResultsReady(const QString& results);
    
    /**
     * @brief Handles and displays scanner errors.
     * @param errorCode The error code.
     * @param errorMessage A descriptive error message.
     */
    void onBasicScanError(ScannerErrorCode errorCode, const QString& errorMessage);
    
    /**
     * @brief Handles file selection for advanced scan.
     */
    void onSelectFileClicked();
    
    /**
     * @brief Handles directory selection for advanced scan.
     */
    void onSelectDirectoryClicked();
    
    /**
     * @brief Starts advanced scanning with the specified path and selection type.
     * 
     * Initiates an advanced scan operation using the provided path and type.
     * This function is called internally after path selection in advanced scan.
     * 
     * @param path The file or directory path to scan
     * @param type The selection type (file or directory)
     */
    void startAdvancedScanWithPath(const QString &path, const QString &type);

private:
   

    Ui::DashboardWidget *ui; ///< Pointer to the UI form
    std::unique_ptr<BasicScanner> m_basicScanner; ///< Scanner for basic file scanning
    std::unique_ptr<VirusTotalManager> m_virusTotalManager; ///< Manager for VirusTotal API integration
    std::unique_ptr<CDRScanner> m_cdrScanner; ///< Scanner for CDR operations
    NetworkMonitor *m_networkMonitor; ///< Network monitor instance
    std::unique_ptr<Docker::DockerManager> m_dockerManager; ///< Docker container management
    std::unique_ptr<CDR::CdrManager> m_cdrManager; ///< CDR analysis and sanitization management
    std::unique_ptr<Sandbox::SandboxManager> m_sandboxManager; ///< Sandbox analysis and execution management
};

#endif // DASHBOARDWIDGET_H