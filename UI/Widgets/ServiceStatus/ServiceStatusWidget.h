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
     * @brief Update core protection services status
     * 
     * Checks and displays the status of essential security services
     * including scanning engines and real-time protection.
     */
    void updateCoreProtectionStatus();
    
    /**
     * @brief Update technical components status
     * 
     * Monitors and displays the status of technical infrastructure
     * components like databases, network services, and file systems.
     */
    void updateTechnicalComponentsStatus();
    
    /**
     * @brief Update other additional components status
     * 
     * Checks the status of auxiliary services and optional components
     * that enhance system functionality.
     */
    void updateOtherAdditionsStatus();
    
    /**
     * @brief Populate dynamic API keys information
     * 
     * Dynamically loads and displays the status of configured API keys
     * for external services like VirusTotal, threat intelligence feeds, etc.
     */
    void populateDynamicApiKeys();
    
    /**
     * @brief Populate dynamic Docker containers information
     * 
     * Loads and displays the current status of all Docker containers
     * used for scanning operations (CDR, Sandbox, etc.).
     */
    void populateDynamicContainers();
    
    /**
     * @brief Populate dynamic Docker images information
     * 
     * Displays information about available Docker images and their
     * readiness for scanning operations.
     */
    void populateDynamicImages();
    
    /**
     * @brief Update the last refresh timestamp
     * 
     * Updates the display to show when the status information
     * was last refreshed.
     */
    void updateLastRefreshTime();
};

#endif // SERVICESTATUSWIDGET_H