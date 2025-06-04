/**
 * @file HistoryWidget.h
 * @brief Scan history management and display widget
 * @author Ekrem Ünal
 * @version 1.0
 * @date 11.05.2025
 */

#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QWidget>
#include <QModelIndex>

// Forward declarations
namespace Ui {
class HistoryWidget;
}

class ScanHistoryModel;
class ScanHistoryFilterModel;
class DbManager;

/**
 * @brief The HistoryWidget class provides scan history management and display functionality
 * 
 * @details This widget displays a comprehensive history of all scan operations performed
 * by the application. It provides filtering, searching, and management capabilities for
 * scan records including threat detection results, file information, and user actions.
 * 
 * Key features:
 * - Chronological display of scan history
 * - Advanced filtering by scan type, status, risk level, and date
 * - Search functionality for files and threats
 * - Export capabilities for scan reports
 * - File management actions (quarantine, restore, delete)
 * - Integration with malware database for threat information
 * 
 * @note This widget relies on the ScanHistoryModel for data management
 *       and requires a valid DbManager for database operations.
 */
class HistoryWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a HistoryWidget
     * 
     * Initializes the history widget with database connection and sets up
     * the UI components including table view, filters, and action buttons.
     * 
     * @param dbManager Pointer to database manager for scan history operations
     * @param parent Parent widget (nullptr by default)
     */
    explicit HistoryWidget(DbManager* dbManager = nullptr, QWidget *parent = nullptr);
    
    /**
     * @brief Destructor for HistoryWidget
     * 
     * Cleans up resources and ensures proper cleanup of database connections
     * and model instances.
     */
    ~HistoryWidget();

    /**
     * @brief Add a new scan record to the history
     * 
     * Adds a completed scan record to the history database and updates
     * the display to show the new entry.
     * 
     * @param fileName Name of the scanned file
     * @param filePath Full path to the scanned file
     * @param fileHash SHA256 hash of the file
     * @param threatName Name of detected threat (empty if clean)
     * @param isMalicious true if file was identified as malicious
     * @param scanType Type of scan performed (0=Basic, 1=Advanced, 2=CDR, 3=Sandbox)
     */
    void addScanRecord(const QString& fileName, const QString& filePath, 
                      const QString& fileHash, const QString& threatName,
                      bool isMalicious, int scanType = 0);

private slots:
    /**
     * @brief Handle request to view detailed information about a scan
     * 
     * Opens a detailed view dialog showing comprehensive information about
     * the selected scan record including threat analysis and file metadata.
     */
    void onViewDetailsClicked();
    
    /**
     * @brief Handle request to delete a scan record from history
     * 
     * Removes the selected scan record from the history database after
     * user confirmation. This does not affect the actual file.
     */
    void onDeleteEventClicked(); // Renamed from onDeleteClicked
    
    /**
     * @brief Handle request to export scan history data
     * 
     * Opens export dialog and generates reports in various formats
     * (CSV, PDF, etc.) containing filtered scan history data.
     */
    void onExportClicked();
    
    /**
     * @brief Handle changes in the search text field
     * 
     * Filters the scan history display based on the search criteria,
     * searching through file names, paths, and threat names.
     * 
     * @param text Search text entered by the user
     */
    void onSearchTextChanged(const QString &text);
    
    /**
     * @brief Handle changes in scan type filter selection
     * 
     * Filters the history to show only scans of the selected type
     * (Basic, Advanced, CDR, Sandbox, etc.).
     * 
     * @param index Index of the selected scan type filter
     */
    void onScanTypeFilterChanged(int index);
    
    /**
     * @brief Handle changes in date range filter
     * 
     * Updates the display to show only scans within the selected date range.
     * Applies to both start and end date filter changes.
     */
    void onDateFilterChanged(); // Hem başlangıç hem de bitiş tarihi için
    
    /**
     * @brief Handle changes in scan status filter
     * 
     * Filters the history to show scans with specific status
     * (Completed, Failed, In Progress, etc.).
     * 
     * @param index Index of the selected status filter
     */
    void onStatusFilterChanged(int index);
    
    /**
     * @brief Handle changes in risk level filter
     * 
     * Filters the history to show only scans with the selected risk level
     * (Clean, Low, Medium, High, Critical).
     * 
     * @param index Index of the selected risk level filter
     */
    void onRiskLevelFilterChanged(int index); // New slot
    
    /**
     * @brief Handle changes in action status filter
     * 
     * Filters the history based on actions taken on detected threats
     * (Quarantined, Deleted, Allowed, etc.).
     * 
     * @param index Index of the selected action status filter
     */
    void onActionStatusFilterChanged(int index); // New slot
    
    /**
     * @brief Handle clicks on the history table view
     * 
     * Processes user selection in the scan history table and updates
     * the UI to show relevant action buttons and details.
     * 
     * @param index Model index of the clicked item
     */
    void onHistoryTableViewClicked(const QModelIndex &index); // New slot for table view click

    // New slots for action buttons
    /**
     * @brief Handle request to move file to quarantine
     * 
     * Moves the selected file to the quarantine folder for safe isolation
     * and updates the scan record accordingly.
     */
    void onGoToQuarantineClicked();
    
    /**
     * @brief Handle request to restore quarantined file
     * 
     * Restores a quarantined file to its original location or a safe
     * location chosen by the user.
     */
    void onRestoreFileClicked();
    
    /**
     * @brief Handle request to permanently delete file
     * 
     * Permanently removes the file from the system after user confirmation.
     * This action cannot be undone.
     */
    void onPermanentlyDeleteFileClicked();
    
    /**
     * @brief Handle request to add file to exclusions list
     * 
     * Adds the file or its hash to the exclusions list so it won't be
     * flagged in future scans.
     */
    void onAddToExclusionsClicked();
    
    /**
     * @brief Handle request to submit sample for analysis
     * 
     * Submits the file sample to external threat analysis services
     * for further investigation and signature updates.
     */
    void onSubmitSampleClicked();
    
    /**
     * @brief Handle request to generate a report of the scan history
     * 
     * Compiles the current scan history data into a report format
     * and opens it in the default report viewer or saves it to a file.
     */
    void onGenerateReportClicked();
    
    /**
     * @brief Handle request to clear all scan history
     * 
     * Removes all scan records from the history database after
     * user confirmation. This action cannot be undone.
     */
    void onClearAllHistoryClicked();

private:
    /**
     * @brief Set up the table view for displaying scan history
     * 
     * Configures the table view properties, columns, sorting,
     * and selection behavior for optimal user experience.
     */
    void setupTableView();
    
    /**
     * @brief Set up the filter widgets and controls
     * 
     * Initializes all filter components including date pickers,
     * combo boxes, and search fields with appropriate options.
     */
    void setupFilters();
    
    /**
     * @brief Update the details display for the selected scan
     * 
     * Updates the details panel with information about the currently
     * selected scan record.
     */
    void updateDetailsDisplay();
    
    /**
     * @brief Show context menu for table actions
     * 
     * Displays a context menu with available actions for the
     * scan record at the specified position.
     * 
     * @param position Position where the context menu should be shown
     */
    void showContextMenu(const QPoint& position);
    
    /**
     * @brief Format scan details for display
     * 
     * Formats the scan record data into a human-readable string
     * for display in the details panel.
     * 
     * @param record Scan history record to format
     * @return Formatted string containing scan details
     */
    QString formatScanDetails(const struct ScanHistoryRecord& record) const;
    
    Ui::HistoryWidget *ui; ///< Pointer to the UI form
    ScanHistoryModel* m_model; ///< Model for scan history data
    ScanHistoryFilterModel* m_filterModel; ///< Filter model for search and filtering
    DbManager* m_dbManager; ///< Database manager for history operations
};

#endif //HISTORYWIDGET_H
