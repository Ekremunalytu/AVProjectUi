#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextEdit> // Required for displaying logs to the user.
#include <QVBoxLayout> // Used for vertical layout of UI elements.
#include <QHBoxLayout> // Used for horizontal layout of UI elements.
#include <QPushButton> // Required for user interaction buttons.
#include <QLineEdit> // Used for text input fields, like the file path.
#include <QComboBox> // Used for dropdown selection, e.g., for containers.
#include <QLabel> // Used for displaying static text labels.
#include <QWidget> // Base class for all user interface objects.
#include <memory> // Required for std::unique_ptr to manage DockerManager lifetime.

#include "DockerManager.h" // Manages Docker interactions.
#include "cdr/CdrManager.h" // Manages CDR operations.
#include "cdr/CdrTypes.h" // CDR type definitions.

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; } // Forward declaration for UI class (if using .ui files).
QT_END_NAMESPACE

// MainWindow class: Defines the main application window and its functionalities.
class MainWindow : public QMainWindow
{
    Q_OBJECT // Macro required for classes that define signals or slots.

public:
    // Constructor: Initializes the main window.
    // @param parent: Optional parent widget.
    explicit MainWindow(QWidget *parent = nullptr);

    // Destructor: Cleans up resources, especially the DockerManager.
    ~MainWindow();

private slots:
    // Slot for handling the "Select File" button click.
    // Opens a file dialog for the user to choose a file.
    void onSelectFileButtonClicked();

    // Slot for handling the "Start Container" button click.
    // Starts the selected Docker container and copies the selected file to it.
    void onStartContainerButtonClicked();

    // Slot for refreshing the list of available Docker containers/images.
    void refreshContainerList();

    // CDR-related slots
    // Slot for starting CDR analysis
    void onStartCdrAnalysisButtonClicked();
    
    // Slot for checking CDR analysis status
    void onCheckCdrStatusButtonClicked();
    
    // Slot for viewing CDR results
    void onViewCdrResultsButtonClicked();

private:
    void displaySanitizationResult(const CDR::SanitizationResult& result); // Added declaration
    void displayCdrAnalysisResult(const CDR::CdrAnalysisResult& result); // For overall analysis
    void reportFileScanResult(const CDR::SanitizedFileInfo& result); // For single file scan result
    void showContainerLogs(const QString& containerId);
    // Sets up the user interface elements of the main window.
    void setupUi();

    // Updates the status bar message.
    // @param message: The message to display in the status bar.
    void updateStatus(const QString& message);

    // Copies the specified file to the given Docker container.
    // @param filePath: Path of the file on the host machine.
    // @param containerId: ID of the target Docker container.
    void copyFileToContainer(const QString& filePath, const QString& containerId);

    std::unique_ptr<Docker::DockerManager> dockerManager; // Manages Docker operations.
    std::unique_ptr<CDR::CdrManager> cdrManager; // Manages CDR operations.
    QString selectedFilePath; // Stores the path of the file selected by the user.
    QString currentAnalysisId; // Stores the current CDR analysis ID.
    QString tempDirPath_; // Stores the temporary directory path for cleanup

    // UI Elements
    QWidget *centralWidget; // The central widget of the main window.
    QVBoxLayout *mainLayout; // Main vertical layout for the central widget.
    
    QHBoxLayout *fileSelectionLayout; // Layout for file selection components.
    QPushButton *selectFileButton; // Button to open the file dialog.
    QLineEdit *filePathLineEdit; // Line edit to display the selected file path.
    
    QHBoxLayout *containerSelectionLayout; // Layout for container selection components.
    QLabel *containerLabel; // Label for the container selection combo box.
    QComboBox *containerComboBox; // Combo box to list and select Docker images/containers.
    QPushButton *refreshContainersButton; // Button to refresh the container list.
    
    QHBoxLayout *actionButtonLayout; // Layout for action buttons.
    QPushButton *startContainerButton; // Button to start the container and copy the file.
    
    QLabel *statusLabel; // Label to display status messages to the user.

    QLabel *logsLabel; // Label for the logs display area.
    QTextEdit *logsTextEdit; // Text edit area to display container logs.
    
    // CDR-related UI elements
    QHBoxLayout *cdrControlLayout; // Layout for CDR control buttons.
    QPushButton *startCdrAnalysisButton; // Button to start CDR analysis.
    QPushButton *checkCdrStatusButton; // Button to check CDR analysis status.
    QPushButton *viewCdrResultsButton; // Button to view CDR results.
    QPushButton *viewContainerLogsButton; // Button to view container logs
    
    QLabel *cdrStatusLabel; // Label to display CDR analysis status.
    QTextEdit *cdrResultsTextEdit; // Text edit area to display CDR analysis results.
};
#endif // MAINWINDOW_H
