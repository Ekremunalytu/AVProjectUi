/**
 * @file NetworkMonitor.h
 * @brief Network monitoring and logging functionality
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 */

#ifndef NETWORKMONITOR_H
#define NETWORKMONITOR_H

#include <QObject>
#include <QString>
#include <QTimer> // For simulating network logs

/**
 * @brief The NetworkMonitor class provides network activity monitoring and logging
 * 
 * @details This class monitors network traffic and generates log messages for analysis.
 * It can capture real network activity or generate simulated logs for testing purposes.
 * The monitor operates asynchronously and emits signals when new log messages are available.
 * 
 * Key features:
 * - Real-time network traffic monitoring
 * - Simulated log generation for testing
 * - Asynchronous operation with signal-based notifications
 * - Start/stop monitoring control
 * 
 * @note Currently implements simulated logging for development and testing purposes.
 *       Production version should integrate with actual network monitoring tools.
 */
class NetworkMonitor : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructor for NetworkMonitor
     * 
     * Initializes the network monitor with the specified parent object.
     * Sets up internal timers and prepares monitoring infrastructure.
     * 
     * @param parent Parent QObject (nullptr by default)
     */
    explicit NetworkMonitor(QObject *parent = nullptr);
    
    /**
     * @brief Start network monitoring and log generation
     * 
     * Begins monitoring network activity and generates periodic log messages.
     * In simulation mode, generates test log messages at regular intervals.
     */
    void startMonitoring(); // To start generating/capturing logs
    
    /**
     * @brief Stop network monitoring and log generation
     * 
     * Stops monitoring network activity and ceases log message generation.
     * Cleans up monitoring resources and timers.
     */
    void stopMonitoring();  // To stop generating/capturing logs
    
    /**
     * @brief Check if network monitoring is currently active
     * 
     * @return true if monitoring is active, false otherwise
     */
    bool isMonitoring() const; // To check if monitoring is active

signals:
    /**
     * @brief Signal emitted when a new network log message is available
     * 
     * This signal is emitted whenever new network activity is detected
     * or a simulated log message is generated.
     * 
     * @param message The log message containing network activity information
     */
    void newLogMessage(const QString& message);

private slots:
    /**
     * @brief Generate a simulated network log message
     * 
     * This slot is called periodically by the simulation timer to generate
     * test log messages for development and testing purposes.
     */
    void generateSimulatedLog(); // Slot to periodically generate a log

private:
    QTimer *m_simulationTimer; ///< Timer for generating simulated log messages
    int m_logCounter; ///< Counter for tracking generated log messages
};

#endif // NETWORKMONITOR_H
