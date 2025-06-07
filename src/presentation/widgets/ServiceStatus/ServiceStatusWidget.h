/**
 * @file ServiceStatusWidget.h
 * @brief Service status monitoring and display widget
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 */

#ifndef SERVICESTATUSWIDGET_H
#define SERVICESTATUSWIDGET_H

#include <QWidget>
#include <QDateTime>

namespace Ui {
class ServiceStatusWidget; // Corrected class name
}

/**
 * @brief The ServiceStatusWidget class provides real-time monitoring of system services and components
 * 
 * @details This widget displays the operational status of all critical components in the malware
 * analysis system including core protection services, Docker containers, API services, and 
 * technical components. It provides both summary and detailed views of system health.
 * 
 * Key features:
 * - Real-time service status monitoring
 * - Core protection system status display
 * - Docker container and image management monitoring
 * - API service connectivity checks
 * - Technical component health assessment
 * - Detailed vs. summary view toggle
 * - Automatic and manual refresh capabilities
 * 
 * @note This widget requires appropriate permissions to query system services
 *       and Docker daemon status.
 */
class ServiceStatusWidget : public QWidget // Corrected class name
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a ServiceStatusWidget
     * 
     * Initializes the service status widget, sets up monitoring components,
     * and begins initial status assessment of all system services.
     * 
     * @param parent Parent widget (nullptr by default)
     */
    explicit ServiceStatusWidget(QWidget *parent = nullptr); // Corrected constructor name
    
    /**
     * @brief Destructor for ServiceStatusWidget
     * 
     * Cleans up monitoring resources and stops any running status checks.
     */
    ~ServiceStatusWidget(); // Corrected destructor name

private slots:
    /**
     * @brief Handle refresh button click
     * 
     * Manually triggers a refresh of all service status information
     * and updates the display with current system state.
     */
    void on_buttonRefresh_clicked();
    
    /**
     * @brief Handle detailed view toggle
     * 
     * Switches between detailed and summary view modes, showing
     * or hiding additional technical information about services.
     * 
     * @param checked true for detailed view, false for summary view
     */
    void on_checkBoxDetailedView_toggled(bool checked);

private:
    Ui::ServiceStatusWidget *ui; ///< Pointer to the UI form

    /**
     * @brief Perform initial UI setup and configuration
     * 
     * Configures the initial state of all UI components including
     * status indicators, tables, and display elements.
     */
    void initialUISetup();
    
    /**
     * @brief Update the overall system status indicator
     * 
     * Calculates and displays the overall health status based on
     * the status of all individual components.
     */
    void updateOverallStatus();
    
    /**
     * @brief Update core scanning engines status
     * 
     * Checks and displays the status of essential scanning services
     * including YARA engine, Basic Scanner, and VirusTotal integration.
     */
    void updateCoreScanningEnginesStatus();
    
    /**
     * @brief Update technical components status
     * 
     * Monitors and displays the status of technical infrastructure
     * components like SQLite database, CDR service, Sandbox analysis,
     * and Docker infrastructure.
     */
    void updateTechnicalComponentsStatus();
    
    /**
     * @brief Update other additional components status
     * 
     * Checks the status of network monitoring, connectivity status,
     * system resources, and application logging.
     */
    void updateOtherAdditionsStatus();
    
    /**
     * @brief Update the last refresh timestamp
     * 
     * Updates the display to show when the status information
     * was last refreshed.
     */
    void updateLastRefreshTime();
    
    /**
     * @brief Check Docker daemon status
     * 
     * Verifies if Docker daemon is running and accessible.
     * 
     * @return true if Docker is running, false otherwise
     */
    bool checkDockerStatus();
    
    /**
     * @brief Get number of running Docker containers
     * 
     * Queries Docker daemon for the count of currently running containers.
     * 
     * @return Number of running containers
     */
    int getDockerContainerCount();
    
    /**
     * @brief Get number of Docker images
     * 
     * Queries Docker daemon for the total count of available images.
     * 
     * @return Number of Docker images
     */
    int getDockerImageCount();
    
    /**
     * @brief Check internet connectivity status
     * 
     * Tests connectivity to external services using ping.
     * 
     * @return true if connectivity is available, false otherwise
     */
    bool checkConnectivityStatus();
    
    /**
     * @brief Update system resource monitoring
     * 
     * Updates CPU usage, memory usage, and disk space information.
     */
    void updateSystemResourceStatus();
};

#endif // SERVICESTATUSWIDGET_H