#ifndef  DASHBOARDWIDGET_H
#define  DASHBOARDWIDGET_H

#include <QWidget>
#include <QMenu>
#include <memory>
#include "Scanner/Dashboard/BasicScanner/BasicScanner.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DashboardWidget; }
QT_END_NAMESPACE

/**
 * @brief The DashboardWidget class provides the main dashboard interface for scanning operations.
 * 
 * This widget contains buttons for various scan types (basic, advanced, CDR, sandbox)
 * and displays the scan results. It serves as the primary user interface for initiating
 * scans and viewing their results.
 */
class DashboardWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructs a DashboardWidget.
     * @param parent The parent widget.
     */
    explicit DashboardWidget(QWidget *parent = nullptr);
    
    /**
     * @brief Destroys the DashboardWidget and frees resources.
     */
    ~DashboardWidget();

private slots:
    /**
     * @brief Handles click on the Basic Scan button.
     * 
     * Opens a menu with scan options.
     */
    void onBasicScanClicked();
    
    /**
     * @brief Handles click on the Advanced Scan button.
     */
    void onAdvancedScanClicked();
    
    /**
     * @brief Handles click on the CDR Scan button.
     */
    void onCdrScanClicked();
    
    /**
     * @brief Handles click on the Sandbox Scan button.
     */
    void onSandboxScanClicked();
    
    /**
     * @brief Handles the file selection for Basic Scan.
     * 
     * Opens a file dialog and initiates scanning of the selected file.
     */
    void onBasicScanSelectFile();
    
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

private:
    Ui::DashboardWidget *ui; ///< Pointer to the UI form
    std::unique_ptr<BasicScanner> m_basicScanner; ///< Scanner for basic file scanning
    QMenu* m_basicScanMenu; ///< Menu for basic scan options
};

#endif // DASHBOARDWIDGET_H