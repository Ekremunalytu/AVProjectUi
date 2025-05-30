#include "MainWindow.h"
#include "cdr/CdrSanitizer.h" // Add missing include for CdrSanitizer
#include <QDir> // Provides directory handling functionalities.
#include <QProcess> // Provides a way to start external processes.
#include <QFileInfo> // Provides information about files.
#include <QStandardPaths> // Provides standard platform-specific locations.
#include <QDebug> // Provides debugging output stream.
#include <QtConcurrent/QtConcurrent> // Add for thread support
#include <QDateTime> // Add for unique directory naming
#include <iostream> // Standard C++ I/O stream.
#include <thread> // Add for hardware_concurrency
#include <QTextEdit> // FIX: Changed to standard Qt include for QTextEdit

// Constructor for the MainWindow class.
// Initializes the DockerManager and sets up the UI.
// @param parent: Optional parent widget.
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , dockerManager(std::make_unique<Docker::DockerManager>()) // Initialize DockerManager using smart pointer.
    , cdrManager(std::make_unique<CDR::CdrManager>()) // Initialize CdrManager using smart pointer.
{
    setupUi(); // Call to set up the user interface elements.
    
    // Connect signals from UI elements to their respective slots (handler functions).
    connect(selectFileButton, &QPushButton::clicked, this, &MainWindow::onSelectFileButtonClicked);
    connect(startContainerButton, &QPushButton::clicked, this, &MainWindow::onStartContainerButtonClicked);
    connect(refreshContainersButton, &QPushButton::clicked, this, &MainWindow::refreshContainerList);
    
    // Connect CDR-related signals
    connect(startCdrAnalysisButton, &QPushButton::clicked, this, &MainWindow::onStartCdrAnalysisButtonClicked);
    connect(checkCdrStatusButton, &QPushButton::clicked, this, &MainWindow::onCheckCdrStatusButtonClicked);
    connect(viewCdrResultsButton, &QPushButton::clicked, this, &MainWindow::onViewCdrResultsButtonClicked);
    
    // Perform an initial refresh of the container list when the application starts.
    refreshContainerList();
    
    setWindowTitle(tr("Docker File Processor")); // Set the main window title.
    resize(600, 400); // Set the initial size of the main window.
}

// Destructor for the MainWindow class.
// Performs proper cleanup of resources and temporary files.
MainWindow::~MainWindow()
{
    // Stop any ongoing CDR analysis
    if (!currentAnalysisId.isEmpty()) {
        try {
            cdrManager->stopAnalysis(currentAnalysisId.toStdString());
        } catch (const std::exception& e) {
            std::cerr << "Error stopping CDR analysis during cleanup: " << e.what() << std::endl;
        }
    }
    
    // Clean up temporary directories
    if (!tempDirPath_.isEmpty()) {
        try {
            QDir tempDir(tempDirPath_);
            if (tempDir.exists()) {
                if (!tempDir.removeRecursively()) {
                    std::cerr << "Warning: Failed to remove temporary directory: " 
                              << tempDirPath_.toStdString() << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error during temporary directory cleanup: " << e.what() << std::endl;
        }
    }
    
    // std::unique_ptr for dockerManager and cdrManager handles their deletion automatically.
}

// Sets up the user interface elements of the main window.
void MainWindow::setupUi()
{
    centralWidget = new QWidget(this); // Create the central widget.
    setCentralWidget(centralWidget); // Set it as the main window's central widget.
    
    mainLayout = new QVBoxLayout(centralWidget); // Create the main vertical layout.
    
    // --- File selection section --- 
    fileSelectionLayout = new QHBoxLayout(); // Layout for file selection components.
    selectFileButton = new QPushButton(tr("Select File"), this); // Button to trigger file selection dialog.
    filePathLineEdit = new QLineEdit(this); // Line edit to display the path of the selected file.
    filePathLineEdit->setReadOnly(true); // Make the line edit read-only.
    filePathLineEdit->setPlaceholderText(tr("Please select a file...")); // Placeholder text.
    
    fileSelectionLayout->addWidget(selectFileButton); // Add button to the layout.
    fileSelectionLayout->addWidget(filePathLineEdit); // Add line edit to the layout.
    
    // --- Container selection section --- 
    containerSelectionLayout = new QHBoxLayout(); // Layout for container selection components.
    containerLabel = new QLabel(tr("Container:"), this); // Label for the combo box.
    containerComboBox = new QComboBox(this); // Combo box for selecting Docker images/containers.
    refreshContainersButton = new QPushButton(tr("Refresh"), this); // Button to refresh the list.
    
    containerSelectionLayout->addWidget(containerLabel);
    containerSelectionLayout->addWidget(containerComboBox);
    containerSelectionLayout->addWidget(refreshContainersButton);
    
    // --- Action buttons section --- 
    actionButtonLayout = new QHBoxLayout(); // Layout for action buttons.
    startContainerButton = new QPushButton(tr("Start Container and Copy File"), this); // Button to start container and copy file.
    startContainerButton->setEnabled(false); // Initially disabled until a file and container are selected.
    
    actionButtonLayout->addWidget(startContainerButton);
    
    // --- Status section --- 
    statusLabel = new QLabel(tr("Ready"), this); // Label to display status messages.
    statusLabel->setAlignment(Qt::AlignCenter); // Center-align the status text.
    
    // Add all sections to the main layout.
    mainLayout->addLayout(fileSelectionLayout);
    mainLayout->addLayout(containerSelectionLayout);
    mainLayout->addLayout(actionButtonLayout);
    mainLayout->addWidget(statusLabel);

    // --- Logs section --- 
    logsLabel = new QLabel(tr("Container Logs:"), this); // Label for the logs display area.
    logsTextEdit = new QTextEdit(this); // Text area to display container logs.
    logsTextEdit->setReadOnly(true); // Make the logs area read-only.
    logsTextEdit->setPlaceholderText(tr("Container logs will appear here...")); // Placeholder text.
    
    mainLayout->addWidget(logsLabel);
    mainLayout->addWidget(logsTextEdit);
    
    // --- CDR section ---
    cdrControlLayout = new QHBoxLayout(); // Layout for CDR control buttons.
    startCdrAnalysisButton = new QPushButton(tr("Start CDR Analysis"), this);
    checkCdrStatusButton = new QPushButton(tr("Check CDR Status"), this);
    viewCdrResultsButton = new QPushButton(tr("View CDR Results"), this);
    
    // Initially disable CDR buttons until a file is selected
    startCdrAnalysisButton->setEnabled(false);
    checkCdrStatusButton->setEnabled(false);
    viewCdrResultsButton->setEnabled(false);
    
    cdrControlLayout->addWidget(startCdrAnalysisButton);
    cdrControlLayout->addWidget(checkCdrStatusButton);
    cdrControlLayout->addWidget(viewCdrResultsButton);
    
    cdrStatusLabel = new QLabel(tr("CDR Status: Ready"), this);
    cdrStatusLabel->setAlignment(Qt::AlignCenter);
    
    cdrResultsTextEdit = new QTextEdit(this);
    cdrResultsTextEdit->setReadOnly(true);
    cdrResultsTextEdit->setPlaceholderText(tr("CDR analysis results will appear here..."));
    
    mainLayout->addLayout(cdrControlLayout);
    mainLayout->addWidget(cdrStatusLabel);
    mainLayout->addWidget(cdrResultsTextEdit);
    
    // Add some stretching to the layout for better visual spacing.
    mainLayout->addStretch();
}

// Refreshes the list of available Docker images/containers in the combo box.
void MainWindow::refreshContainerList()
{
    try {
        // Clear the existing items in the combo box.
        containerComboBox->clear();
        
        // Check if the Docker daemon is running before attempting to list images.
        if (!dockerManager->isDaemonRunning()) {
            QMessageBox::critical(this, tr("Error"), tr("Docker daemon is not running."));
            updateStatus(tr("Error: Docker daemon is not running"));
            return;
        }
        
        // Retrieve the list of available Docker images.
        std::vector<Docker::ImageInfo> images = dockerManager->listImages();
        
        // Populate the combo box with the retrieved image information.
        for (const auto& image : images) {
            QString displayName;
            if (!image.repository.empty() && image.repository != "<none>") {
                displayName = QString::fromStdString(image.repository);
                if (!image.tag.empty() && image.tag != "<none>") {
                    displayName += ":" + QString::fromStdString(image.tag);
                }
            } else {
                displayName = QString::fromStdString(image.id).left(12); // Use short ID if repository/tag is not available.
            }
            
            containerComboBox->addItem(displayName, QString::fromStdString(image.id)); // Add display name and image ID to combo box.
        }
        
        updateStatus(tr("Container list updated"));
        
    } catch (const Docker::DockerException& e) { // Catch specific Docker exceptions.
        QMessageBox::critical(this, tr("Error"), tr("Could not retrieve container list: ") + QString::fromStdString(e.what()));
        updateStatus(tr("Error: Could not retrieve container list"));
    } catch (const std::exception& e) { // Catch any other standard exceptions.
        QMessageBox::critical(this, tr("Error"), tr("Could not retrieve container list: ") + e.what());
        updateStatus(tr("Error: Could not retrieve container list"));
    }
}

// Slot for handling the "Select File" button click.
// Opens a file dialog for the user to choose a file.
void MainWindow::onSelectFileButtonClicked()
{
    // Open a file dialog to allow the user to select a file.
    QString filePath = QFileDialog::getOpenFileName(
        this, 
        tr("Select File"), // Dialog title.
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), // Initial directory.
        tr("All Files (*.*)") // File filter.
    );
    
    // If a file was selected (i.e., filePath is not empty).
    if (!filePath.isEmpty()) {
        filePathLineEdit->setText(filePath); // Display the selected file path.
        selectedFilePath = filePath; // Store the selected file path.
        // Enable the start container button if there are containers in the list.
        startContainerButton->setEnabled(containerComboBox->count() > 0);
        // Enable CDR analysis button when a file is selected.
        startCdrAnalysisButton->setEnabled(true);
        updateStatus(tr("File selected: ") + QFileInfo(filePath).fileName());
    }
}

// Slot for handling the "Start Container" button click.
// Starts the selected Docker container and copies the selected file to it.
void MainWindow::onStartContainerButtonClicked()
{
    // Ensure a file has been selected.
    if (selectedFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a file first."));
        return;
    }
    
    // Ensure a container/image has been selected from the combo box.
    if (containerComboBox->currentIndex() == -1) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a container first."));
        return;
    }
    
    QString imageId = containerComboBox->currentData().toString(); // Get the ID of the selected image.
    logsTextEdit->clear(); // Clear any previous logs from the text area.
    
    try {
        updateStatus(tr("Starting container..."));
        
        // Prepare the configuration for running the new container.
        Docker::ContainerRunConfiguration config;
        config.imageName = imageId.toStdString();
        // Example command: echo a message, sleep, then echo another message.
        config.command = {"/bin/sh", "-c", "echo 'Container started, logs are now active. Test output.' && sleep 2 && echo 'Test complete, exiting.'"};
        
        config.hostConfig.autoremove = false; // Do not automatically remove the container when it stops.
        
        // Run the new container using DockerManager.
        Docker::ContainerExecutionResult result = dockerManager->runNewContainer(config);
        QString containerId = QString::fromStdString(result.containerId);
        
        // Check if the container started successfully.
        if (!containerId.isEmpty() && result.exitCode == 0) {
            updateStatus(tr("Container started: ") + containerId.left(12)); // Display short container ID.
            
            // Display initial logs from the container execution, if any.
            if (!result.logs.empty()) {
                logsTextEdit->append(QString::fromStdString(result.logs)); 
            }

            // Copy the selected file to the newly started container.
            copyFileToContainer(selectedFilePath, containerId);

            // Attempt to get and display more logs from the container.
            try {
                std::string logs = dockerManager->getContainerLogs(containerId.toStdString());
                logsTextEdit->setText(QString::fromStdString(logs)); 
                updateStatus(tr("Container logs retrieved."));
            } catch (const Docker::DockerException& e) { // Catch specific Docker exceptions for logging.
                logsTextEdit->setText(tr("Could not retrieve logs: ") + QString::fromStdString(e.what()));
                updateStatus(tr("Error: Could not retrieve container logs."));
                QMessageBox::warning(this, tr("Log Error"), tr("Error while retrieving container logs: ") + QString::fromStdString(e.what()));
            } catch (const std::exception& e) { // Catch any other standard exceptions for logging.
                logsTextEdit->setText(tr("Could not retrieve logs: ") + e.what());
                updateStatus(tr("Error: Could not retrieve container logs."));
                QMessageBox::warning(this, tr("Log Error"), tr("A general error occurred while retrieving container logs: ") + e.what());
            }
            
        } else {
            // Handle container start failure.
            updateStatus(tr("Container could not be started or stopped immediately. Error: ") + QString::fromStdString(result.errorMessage));
            QMessageBox::critical(this, tr("Error"), tr("Container could not be started or stopped immediately. Exit code: %1, Status: %2, Message: %3")
                                              .arg(result.exitCode)
                                              .arg(QString::fromStdString(result.status))
                                              .arg(QString::fromStdString(result.errorMessage)));
            if(!result.logs.empty()){ // Display logs from the failed attempt if available.
                 logsTextEdit->setText(QString::fromStdString(result.logs));
            }
        }
        
    } catch (const Docker::DockerException& e) { // Catch specific Docker exceptions for container operations.
        updateStatus(tr("Docker Error: ") + QString::fromStdString(e.what()));
        QMessageBox::critical(this, tr("Docker Error"), tr("Error during Docker operation: ") + QString::fromStdString(e.what()));
    } catch (const std::exception& e) { // Catch any other standard exceptions for container operations.
        updateStatus(tr("Error: ") + e.what());
        QMessageBox::critical(this, tr("Error"), tr("Error during container operation: ") + e.what());
    }
}

// Copies the specified file from the host to the given Docker container.
// @param filePath: Path of the file on the host machine.
// @param containerId: ID of the target Docker container.
void MainWindow::copyFileToContainer(const QString& filePath, const QString& containerId)
{
    // Validate input parameters.
    if (filePath.isEmpty() || containerId.isEmpty()) {
        updateStatus(tr("File path or container ID cannot be empty."));
        QMessageBox::warning(this, tr("Missing Information"), tr("File path and container ID are required to copy the file."));
        return;
    }
    
    try {
        updateStatus(tr("Copying file to container..."));
        
        QFileInfo fileInfo(filePath); // Get file information.
        QString fileName = fileInfo.fileName(); // Extract the file name.
        std::string hostPath = filePath.toStdString(); // Convert host path to std::string.
        std::string contPath = "/tmp/" + fileName.toStdString(); // Define the destination path inside the container.
        
        // Use DockerManager to copy the file.
        dockerManager->copyFileToContainer(containerId.toStdString(), hostPath, contPath);
        
        updateStatus(tr("File successfully copied to: %1").arg(QString::fromStdString(contPath)));
        QMessageBox::information(
            this, 
            tr("Success"), 
            tr("File successfully copied to container.\\nContainer ID: %1\\nFile path: %2")
            .arg(containerId.left(12)) // Show short container ID.
            .arg(QString::fromStdString(contPath))
        );
        
    } catch (const Docker::DockerException& e) { // Catch specific Docker exceptions for file copying.
        updateStatus(tr("File copy error: Docker error"));
        QMessageBox::critical(this, tr("Copy Error"), tr("Could not copy file (Docker): ") + QString::fromStdString(e.what()));
    } catch (const std::exception& e) { // Catch any other standard exceptions for file copying.
        updateStatus(tr("File copy error"));
        QMessageBox::critical(this, tr("Copy Error"), tr("Could not copy file: ") + QString::fromStdString(e.what()));
    }
}

// Updates the status label with the given message.
// @param message: The message to display.
void MainWindow::updateStatus(const QString& message)
{
    statusLabel->setText(message); // Set the text of the status label.
    statusLabel->repaint(); // Ensure the label is repainted immediately to show the update.
}

// CDR-related slot implementations

// Slot for starting CDR analysis
void MainWindow::onStartCdrAnalysisButtonClicked()
{
    // Ensure a file has been selected.
    if (selectedFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a file first."));
        return;
    }

    QFileInfo selectedFileInfo(selectedFilePath);
    if (selectedFileInfo.isDir()) {
        // Existing directory-based analysis logic
        // ... (original code for directory analysis) ...
    } else if (selectedFileInfo.isFile()) {
        // Single file sanitization logic
        startCdrAnalysisButton->setEnabled(false);
        startContainerButton->setEnabled(false);

        class CdrSingleFileGuard {
        public:
            CdrSingleFileGuard(MainWindow* window) : window_(window), shouldReEnable_(true) {}
            ~CdrSingleFileGuard() {
                if (shouldReEnable_) {
                    window_->startCdrAnalysisButton->setEnabled(true);
                    // Ensure selectedFilePath is checked for enabling startContainerButton
                    window_->startContainerButton->setEnabled(window_->containerComboBox->count() > 0 && !window_->selectedFilePath.isEmpty());
                }
            }
            void disableReEnable() { shouldReEnable_ = false; }
        private:
            MainWindow* window_;
            bool shouldReEnable_;
        };
        CdrSingleFileGuard guard(this);

        try {
            cdrStatusLabel->setText(tr("CDR Status: Preparing single file sanitization..."));
            cdrResultsTextEdit->clear();

            CDR::CdrConfiguration config; // Use default or allow user to configure
            config.securityLevel = CDR::CdrConfiguration::SecurityLevel::HIGH;
            config.autoSanitize = true;

            // Determine output path
            QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            QString outputDir = tempDir + "/cdr_sanitized_" + QString::number(QDateTime::currentMSecsSinceEpoch());
            if (!QDir().mkpath(outputDir)) {
                throw std::runtime_error("Failed to create temporary output directory for sanitization.");
            }
            tempDirPath_ = outputDir; // Store for cleanup
            QString outputFilePath = outputDir + "/" + selectedFileInfo.fileName();

            CDR::FileType fileType = cdrManager->detectFileType(selectedFilePath.toStdString());

            // Run sanitization in a separate thread
            QString originalInputPath = selectedFilePath; // Capture for lambda
            QString currentTempDirPath = tempDirPath_;   // Capture for lambda

            QFuture<void> future = QtConcurrent::run([this, config, fileType, inputPath = selectedFilePath.toStdString(), outputPath = outputFilePath.toStdString(), capturedOriginalPath = originalInputPath, capturedTempDir = currentTempDirPath]() {
                CDR::SanitizationResult result;
                result.inputPath = inputPath; // Initialize basic fields
                result.originalPath = inputPath;
                result.outputPath = outputPath;
                result.fileType = fileType;
                QFileInfo originalFileInfoForSize(QString::fromStdString(inputPath));
                if (originalFileInfoForSize.exists()) {
                    result.originalSize = originalFileInfoForSize.size();
                }

                try {
                    // All sanitization calls are now routed through performSanitization in CdrManager
                    // which internally uses CdrSanitizer.
                    // The switch statement here is to call the correct CdrManager public sanitize<Type>Document method.
                    switch (fileType) {
                        case CDR::FileType::PDF_DOCUMENT:
                            result = cdrManager->sanitizePdfDocument(inputPath, outputPath, config);
                            break;
                        case CDR::FileType::ARCHIVE_FILE:
                            result = cdrManager->sanitizeArchiveFile(inputPath, outputPath, config);
                            break;
                        case CDR::FileType::HTML_DOCUMENT:
                            result = cdrManager->sanitizeHtmlDocument(inputPath, outputPath, config);
                            break;
                        case CDR::FileType::SCRIPT_FILE:
                            result = cdrManager->sanitizeScriptFile(inputPath, outputPath, config);
                            break;
                        case CDR::FileType::OFFICE_DOCUMENT:
                            result = cdrManager->sanitizeOfficeDocument(inputPath, outputPath, config);
                            break;
                        // For types not having a direct sanitize<Type>Document method on CdrManager,
                        // or if we want to use a generic approach:
                        // We can call a generic sanitizeFile method on CdrManager if it exists,
                        // or rely on CdrSanitizer directly if CdrManager doesn't wrap all types.
                        // For now, we assume CdrManager has specific methods for common types.
                        // For other types, CdrSanitizer called by performSanitization will handle them (e.g. copy or block).
                        case CDR::FileType::XML_DOCUMENT:
                        case CDR::FileType::RTF_DOCUMENT:
                        case CDR::FileType::TEXT_DOCUMENT:
                        case CDR::FileType::IMAGE_FILE:
                        case CDR::FileType::EMAIL_FILE:
                        case CDR::FileType::EXECUTABLE_FILE: // Executables are typically blocked or handled by policy
                        case CDR::FileType::UNKNOWN_FILE:
                        case CDR::FileType::NOT_SET: // Should ideally be resolved before this point
                            // Use a generic approach if CdrManager has one, or let CdrSanitizer handle it via performSanitization
                            // The performSanitization in CdrManager already calls CdrSanitizer.sanitizeFile
                            // which has logic for unknown/unhandled types based on config.allowUnknownTypes.
                            // So, we can call a generic CdrManager method or directly use performSanitization logic here if needed.
                            // Let's assume for these types, we can call a generic CdrManager::sanitizeFile if it existed,
                            // or rely on the fact that CdrManager's performSanitization (called by specific methods)
                            // will use CdrSanitizer which has a fallback.
                            // This implies CdrManager might need a more generic public sanitizeFile method or this switch needs to be more exhaustive
                            // or the performSanitization logic needs to be robust enough for all types passed to it.

                            // The current structure is: MainWindow -> CdrManager::sanitize<Type>Document -> CdrManager::performSanitization -> CdrSanitizer::sanitizeFile
                            // So, if a type like XML doesn't have sanitizeXmlDocument, we need a way to call performSanitization for it.
                            // Let's add a generic call for unlisted types for now, assuming performSanitization can be called directly or via a generic wrapper.
                            // This is a bit of a structural refinement point.
                            // For now, we'll simulate this by calling performSanitization directly if we were inside CdrManager.
                            // Since we are in MainWindow, we'd ideally have a CdrManager method for all types or a generic one.
                            // Let's assume the existing specific methods will call performSanitization which handles all FileType enum values.
                            // If a specific CdrManager::sanitize<Type>Document doesn't exist, this is a gap.
                            // We will add a catch-all that relies on a generic sanitization path if one existed or default to error.
                            {
                                // This is a conceptual call. In reality, CdrManager should expose a method for this.
                                // CdrSanitizer sanitizer; // This would be wrong here, MainWindow uses CdrManager
                                // result = sanitizer.sanitizeFile(inputPath, outputPath, config, fileType);
                                // This will be handled by the default case in CdrSanitizer if no specific sanitizer is found.
                                // The existing specific CdrManager methods (sanitizePdfDocument etc.) already cover the main types.
                                // For truly unhandled types by CdrManager's public API, we'd get an error or default behavior from CdrSanitizer.
                                // The current code calls specific CdrManager methods. If a FileType doesn't match one of these cases,
                                // it will fall through the switch. We need a default for the switch or ensure all types are handled.

                                // Correcting the switch structure:
                                std::string typeDesc = "Unknown/Other";
                                switch(fileType) {
                                    case CDR::FileType::XML_DOCUMENT: typeDesc = "XML Document"; break;
                                    case CDR::FileType::RTF_DOCUMENT: typeDesc = "RTF Document"; break;
                                    case CDR::FileType::TEXT_DOCUMENT: typeDesc = "Text Document"; break;
                                    case CDR::FileType::IMAGE_FILE: typeDesc = "Image File"; break;
                                    case CDR::FileType::EXECUTABLE_FILE: typeDesc = "Executable File"; break;
                                    case CDR::FileType::EMAIL_FILE: typeDesc = "Email File"; break;
                                    default: break;
                                }
                                // This is where we'd call a generic CdrManager::sanitizeAnyFile(inputPath, outputPath, config, fileType)
                                // if it existed. Since it doesn't, we'll rely on CdrSanitizer's default handling when called by
                                // one of the existing CdrManager::sanitize<Type>Document methods if we map it to one, or error out.
                                // The current CdrManager::performSanitization is generic enough. The issue is calling it from MainWindow for *any* type.
                                // For now, we'll assume that if it's not one of the explicitly handled cases, it's an error at this level,
                                // or we need a generic CdrManager entry point.
                                // Let's make it an error for types not explicitly handled by a CdrManager public method here.
                                result.success = false;
                                result.errorMessage = "Sanitization for file type '" + typeDesc + "' is not directly available via a dedicated CdrManager method in MainWindow.";
                                result.actionsPerformed.push_back("BLOCKED_NO_UI_HANDLER");
                            }
                            break;
                        // default: // This default was inside the inner switch, should be for the outer one.
                        //     result.success = false;
                        //     result.errorMessage = "Unsupported file type for single sanitization in MainWindow switch.";
                        //     result.actionsPerformed.push_back("BLOCKED_INTERNAL_ERROR");
                        //     break;
                    }
                    // Populate original file info into result if not already done by sanitize methods
                    // This is now done at the start of the lambda.
                    // if (result.originalPath.empty()) result.originalPath = inputPath;
                    // QFileInfo originalFileInfo(QString::fromStdString(inputPath));
                    // if (originalFileInfo.exists()) result.originalSize = originalFileInfo.size();

                    // Ensure sanitizedPath is populated in the result if sanitization was successful and an output path was used.
                    // The sanitize methods in CdrManager (via performSanitization and CdrSanitizer) should now correctly set this.
                    // if (result.success && !outputPath.empty() && result.sanitizedPath.empty()) {
                    //    result.sanitizedPath = outputPath;
                    // }

                    QMetaObject::invokeMethod(this, [this, result, capturedOriginalPath, capturedTempDir, outputPathString = outputPath]() {
                        // bool cdrProcessFailed = !result.success; // Kaldırıldı
                        bool threatIndicatedByCDR = false; // Varsayılan olarak tehdit yok

                        // threatIndicatedByCDR için yeni mantık:
                        // Sadece işlem başarılıysa ve eylemler boş değilse tehdit olasılığını değerlendir.
                        if (result.success && !result.actionsPerformed.empty()) {
                            for (const std::string& actionStd : result.actionsPerformed) {
                                // Eğer eylem "COPIED_AS_IS" ile başlamıyorsa, gerçek bir tehdit eylemi bulunmuştur.
                                if (!QString::fromStdString(actionStd).startsWith("COPIED_AS_IS", Qt::CaseInsensitive)) {
                                    threatIndicatedByCDR = true;
                                    break;
                                }
                            }
                        }

                        cdrResultsTextEdit->clear(); // Önceki sonuçları temizle

                        if (result.success) { // Doğrudan result.success kullan
                            if (threatIndicatedByCDR) {
                                // Harmful content detected
                                cdrStatusLabel->setText(tr("CDR Status: Harmful content detected in %1").arg(QFileInfo(capturedOriginalPath).fileName()));
                                QMessageBox::StandardButton reply;
                                reply = QMessageBox::warning(this, tr("Harmful Content Detected"),
                                                             tr("Harmful content was detected in '%1'.\\n"
                                                                "Do you want to delete the original file or replace it with a cleaned version (if available)?\\n\\n"
                                                                "Details:\\n%2")
                                                             .arg(QFileInfo(capturedOriginalPath).fileName())
                                                             .arg(QString::fromStdString(result.success ? "Success" : "Failed: " + result.errorMessage)), // Manual string construction
                                                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Save); // Yes = Delete, No = Keep, Save = Replace

                                if (reply == QMessageBox::Yes) { // User chose to delete
                                    cdrResultsTextEdit->append(tr("Original File: %1").arg(capturedOriginalPath));
                                    cdrResultsTextEdit->append(tr("Status: Harmful content detected. Original file deleted by user."));
                                    if (result.originalSize > 0) cdrResultsTextEdit->append(tr("Original Size: %1 bytes").arg(result.originalSize));
                                    for (const std::string& actionStd : result.actionsPerformed) {
                                        cdrResultsTextEdit->append(tr("CDR Action: %1").arg(QString::fromStdString(actionStd))); // FIX: Convert to QString
                                    }

                                    QFile originalFile(capturedOriginalPath);
                                    if (originalFile.exists()) {
                                        if (originalFile.remove()) {
                                            cdrResultsTextEdit->append(tr("Action: Original file deleted successfully."));
                                            updateStatus(tr("Original file deleted: %1").arg(QFileInfo(capturedOriginalPath).fileName()));
                                        } else {
                                            QString errorMsg = tr("Action: Failed to delete original file. Error: %1").arg(originalFile.errorString());
                                            cdrResultsTextEdit->append(errorMsg);
                                            QMessageBox::critical(this, tr("Deletion Error"), errorMsg);
                                        }
                                    } else {
                                        cdrResultsTextEdit->append(tr("Action: Original file (%1) not found for deletion.").arg(capturedOriginalPath));
                                    }
                                } else if (reply == QMessageBox::Save) { // User chose to replace
                                    QString sanitizedFileResultPath = QString::fromStdString(result.sanitizedPath);
                                    cdrResultsTextEdit->append(tr("Original File: %1 (Replaced with cleaned version)").arg(capturedOriginalPath));
                                    cdrResultsTextEdit->append(tr("Status: File cleaned. Original replaced."));
                                    if (result.originalSize > 0) cdrResultsTextEdit->append(tr("Original Size: %1 bytes").arg(result.originalSize));
                                    if (QFileInfo(sanitizedFileResultPath).exists()) {
                                        cdrResultsTextEdit->append(tr("Cleaned Size: %1 bytes").arg(QFileInfo(sanitizedFileResultPath).size()));
                                    }
                                    for (const std::string& actionStd : result.actionsPerformed) {
                                        cdrResultsTextEdit->append(tr("CDR Action: %1").arg(QString::fromStdString(actionStd))); // FIX: Convert to QString
                                    }

                                    QFile originalFile(capturedOriginalPath);
                                    if (originalFile.exists()) originalFile.remove(); // Remove original first

                                    if (QFile::copy(sanitizedFileResultPath, capturedOriginalPath)) {
                                        cdrResultsTextEdit->append(tr("Action: Original file replaced with cleaned version."));
                                        updateStatus(tr("Original file replaced with cleaned version: %1").arg(QFileInfo(capturedOriginalPath).fileName()));
                                        // İsteğe bağlı: Geçici temizlenmiş dosyayı sil (eğer orijinal yoldan farklıysa ve mevcutsa)
                                        // Genel geçici dizin 'capturedTempDir' daha sonra yine de silinecek, bu daha güvenli.
                                        if (sanitizedFileResultPath != capturedOriginalPath && QFileInfo(capturedOriginalPath).exists() && QFile::exists(sanitizedFileResultPath)) {
                                            QFile(sanitizedFileResultPath).remove();
                                        }
                                    } else {
                                        QString copyErrorMsg = tr("Failed to copy '%1' to '%2'.").arg(sanitizedFileResultPath, capturedOriginalPath);
                                        if (!QFile::exists(sanitizedFileResultPath)) {
                                            copyErrorMsg += tr(" Source file does not exist.");
                                        }
                                        cdrResultsTextEdit->append(tr("Action: Failed to replace original file with cleaned version. Error: %1. Cleaned file may be in %2").arg(copyErrorMsg).arg(sanitizedFileResultPath));
                                        QMessageBox::critical(this, tr("Replacement Error"), tr("Failed to replace original file. Cleaned file saved at: %1. Error: %2").arg(sanitizedFileResultPath).arg(copyErrorMsg));
                                    }
                                } else { // User chose to keep or closed dialog
                                    cdrResultsTextEdit->append(tr("Original File: %1").arg(capturedOriginalPath));
                                    cdrResultsTextEdit->append(tr("Status: Harmful content detected. Original file preserved by user decision."));
                                    if (result.originalSize > 0) cdrResultsTextEdit->append(tr("Original Size: %1 bytes").arg(result.originalSize));
                                    for (const std::string& actionStd : result.actionsPerformed) {
                                        cdrResultsTextEdit->append(tr("CDR Action: %1").arg(QString::fromStdString(actionStd))); // FIX: Convert to QString
                                    }
                                    updateStatus(tr("Harmful content detected. Original file preserved: %1").arg(QFileInfo(capturedOriginalPath).fileName()));
                                }
                            } else {
                                // No harmful content or all "copied as is"
                                cdrStatusLabel->setText(tr("CDR Status: File '%1' processed. No direct threats found or all elements copied as is.").arg(QFileInfo(capturedOriginalPath).fileName()));
                                cdrResultsTextEdit->append(tr("Original File: %1").arg(capturedOriginalPath));
                                cdrResultsTextEdit->append(tr("Status: File processed. No direct threats found or all elements copied as is."));
                                if (result.originalSize > 0) cdrResultsTextEdit->append(tr("Original Size: %1 bytes").arg(result.originalSize));

                                QString sanitizedFileResultPath = QString::fromStdString(result.sanitizedPath);
                                if (!sanitizedFileResultPath.isEmpty() && QFileInfo(sanitizedFileResultPath).exists()) {
                                     cdrResultsTextEdit->append(tr("Sanitized File: %1").arg(sanitizedFileResultPath));
                                     cdrResultsTextEdit->append(tr("Sanitized Size: %1 bytes").arg(QFileInfo(sanitizedFileResultPath).size()));
                                } else if (!outputPathString.empty() && QFileInfo(QString::fromStdString(outputPathString)).exists()) {
                                    // If result.sanitizedPath was empty but we had an outputPath and it exists
                                    sanitizedFileResultPath = QString::fromStdString(outputPathString);
                                    cdrResultsTextEdit->append(tr("Output File: %1").arg(sanitizedFileResultPath));
                                    cdrResultsTextEdit->append(tr("Output Size: %1 bytes").arg(QFileInfo(sanitizedFileResultPath).size()));
                                }


                                if (result.actionsPerformed.empty()) {
                                    cdrResultsTextEdit->append(tr("CDR Actions: No specific actions performed (e.g., file type might be benign or already clean)."));
                                } else {
                                    for (const std::string& actionStd : result.actionsPerformed) {
                                        cdrResultsTextEdit->append(tr("CDR Action: %1").arg(QString::fromStdString(actionStd)));
                                    }
                                }
                                updateStatus(tr("CDR processing complete for %1. No direct threats found.").arg(QFileInfo(capturedOriginalPath).fileName()));
                                QMessageBox::information(this, tr("CDR Complete"), tr("CDR processing for '%1' is complete. No direct threats were found, or all elements were copied as is.").arg(QFileInfo(capturedOriginalPath).fileName()));
                            }
                        } else {
                            // CDR process failed
                            QString errorDetails = QString::fromStdString(result.errorMessage);
                            if (errorDetails.isEmpty()) errorDetails = "Unknown error during CDR process.";
                            cdrStatusLabel->setText(tr("CDR Status: Failed for %1").arg(QFileInfo(capturedOriginalPath).fileName()));
                            cdrResultsTextEdit->append(tr("Original File: %1").arg(capturedOriginalPath));
                            cdrResultsTextEdit->append(tr("Status: CDR process failed."));
                            cdrResultsTextEdit->append(tr("Error: %1").arg(errorDetails));
                            updateStatus(tr("CDR process failed for %1.").arg(QFileInfo(capturedOriginalPath).fileName()));
                            QMessageBox::critical(this, tr("CDR Error"), tr("CDR process for '%1' failed: %2").arg(QFileInfo(capturedOriginalPath).fileName()).arg(errorDetails));
                        }

                        // Re-enable buttons after processing
                        startCdrAnalysisButton->setEnabled(true);
                        startContainerButton->setEnabled(containerComboBox->count() > 0 && !selectedFilePath.isEmpty());
                        checkCdrStatusButton->setEnabled(false); // Status is now final
                        viewCdrResultsButton->setEnabled(true); // Results are available

                        // Clean up the specific temporary directory for this analysis if it exists
                        if (!capturedTempDir.isEmpty()) {
                            QDir tempDirToRemove(capturedTempDir);
                            if (tempDirToRemove.exists()) {
                                tempDirToRemove.removeRecursively();
                            }
                        }
                         // Reset tempDirPath_ if it was the one just cleaned.
                        if (tempDirPath_ == capturedTempDir) {
                            tempDirPath_.clear();
                        }


                    }, Qt::QueuedConnection);
                } catch (const std::exception& e) {
                    QMetaObject::invokeMethod(this, [this, e_what = std::string(e.what()), capturedOriginalPath, capturedTempDir]() {
                        cdrStatusLabel->setText(tr("CDR Status: Exception during analysis for %1").arg(QFileInfo(capturedOriginalPath).fileName()));
                        cdrResultsTextEdit->clear();
                        cdrResultsTextEdit->append(tr("Exception during CDR analysis for %1:").arg(capturedOriginalPath));
                        cdrResultsTextEdit->append(QString::fromStdString(e_what));
                        startCdrAnalysisButton->setEnabled(true);
                        startContainerButton->setEnabled(containerComboBox->count() > 0 && !selectedFilePath.isEmpty());
                        checkCdrStatusButton->setEnabled(false);
                        viewCdrResultsButton->setEnabled(true);
                        QMessageBox::critical(this, tr("CDR Exception"), tr("An exception occurred during CDR analysis for '%1': %2").arg(QFileInfo(capturedOriginalPath).fileName()).arg(QString::fromStdString(e_what)));
                        
                        // Clean up the specific temporary directory for this analysis if it exists
                        if (!capturedTempDir.isEmpty()) {
                            QDir tempDirToRemove(capturedTempDir);
                            if (tempDirToRemove.exists()) {
                                tempDirToRemove.removeRecursively();
                            }
                        }
                        // Reset tempDirPath_ if it was the one just cleaned.
                        if (tempDirPath_ == capturedTempDir) {
                            tempDirPath_.clear();
                        }
                    }, Qt::QueuedConnection);
                }
            });

            // If QtConcurrent::run successfully launched the task,
            // responsibility for re-enabling buttons is passed to the async task's completion/error lambdas.
            // So, disarm the guard.
            guard.disableReEnable();

        } catch (const std::exception& e) {
            cdrStatusLabel->setText(tr("CDR Status: Error preparing analysis"));
            cdrResultsTextEdit->setText(tr("Error preparing CDR analysis: ") + QString::fromStdString(e.what()));
            QMessageBox::critical(this, tr("CDR Setup Error"), tr("Could not set up CDR analysis: ") + QString::fromStdString(e.what()));
            startCdrAnalysisButton->setEnabled(true); // Re-enable button on setup failure
            startContainerButton->setEnabled(containerComboBox->count() > 0 && !selectedFilePath.isEmpty());
        }
    }
}

void MainWindow::displaySanitizationResult(const CDR::SanitizationResult& result)
{
    QString resultText;
    resultText += tr("Original File: %1\n").arg(QString::fromStdString(result.originalPath));
    if (result.success) {
        cdrStatusLabel->setText(tr("CDR Status: Sanitization Successful"));
        resultText += tr("Sanitized File: %1\n").arg(QString::fromStdString(result.sanitizedPath));
        resultText += tr("Status: Successful\n");
        resultText += tr("Original Size: %1 bytes\n").arg(result.originalSize);
        resultText += tr("Sanitized Size: %1 bytes\n").arg(result.sanitizedSize);
        if (!result.threatsDetected.empty()) {
            resultText += tr("Threats Detected:\n");
            for (const auto& threat : result.threatsDetected) {
                resultText += QString::fromStdString(threat) + "\n";
            }
        }
        if (!result.actionsPerformed.empty()) {
            resultText += tr("Actions Performed:\n");
            for (const auto& action : result.actionsPerformed) {
                resultText += QString::fromStdString(action) + "\n";
            }
        }
    } else {
        cdrStatusLabel->setText(tr("CDR Status: Sanitization Failed"));
        resultText += tr("Status: Failed\n");
        resultText += tr("Error: %1\n").arg(QString::fromStdString(result.errorMessage));
    }
    cdrResultsTextEdit->setText(resultText);
    updateStatus(result.success ? tr("Sanitization complete.") : tr("Sanitization failed."));
    checkCdrStatusButton->setEnabled(false); // No ongoing batch analysis
    viewCdrResultsButton->setEnabled(true); // Show the result we just displayed
}

// Slot for checking CDR analysis status
void MainWindow::onCheckCdrStatusButtonClicked()
{
    if (currentAnalysisId.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No active CDR analysis found."));
        return;
    }
    
    try {
        CDR::CdrAnalysisResult result = cdrManager->getAnalysisStatus(currentAnalysisId.toStdString());
        
        QString statusText = tr("CDR Status: %1 (%2%)")
                             .arg(QString::fromStdString(result.status))
                             .arg(result.progressPercentage);
        cdrStatusLabel->setText(statusText);
        
        // Display basic analysis information
        QString analysisInfo = tr("Analysis ID: %1\n").arg(currentAnalysisId);
        analysisInfo += tr("Status: %1\n").arg(QString::fromStdString(result.status));
        analysisInfo += tr("Progress: %1%\n").arg(result.progressPercentage);
        analysisInfo += tr("Files Processed: %1\n").arg(result.processedFiles.size());
        analysisInfo += tr("Files Sanitized: %1\n").arg(result.filesSanitized);
        analysisInfo += tr("Files Quarantined: %1\n").arg(result.filesQuarantined);
        analysisInfo += tr("Threats Detected: %1\n").arg(result.threatsDetected);
        
        if (!result.errorMessage.empty()) {
            analysisInfo += tr("Error: %1\n").arg(QString::fromStdString(result.errorMessage));
        }
        
        cdrResultsTextEdit->setText(analysisInfo);
        
        updateStatus(tr("CDR status updated: ") + QString::fromStdString(result.status));
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("CDR Error"), tr("Error checking CDR status: ") + QString::fromStdString(e.what()));
        updateStatus(tr("CDR Status Check Error: ") + QString::fromStdString(e.what()));
    }
}

// Slot for viewing CDR results
void MainWindow::onViewCdrResultsButtonClicked()
{
    if (currentAnalysisId.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No active CDR analysis found."));
        return;
    }
    
    try {
        CDR::CdrAnalysisResult result = cdrManager->getAnalysisStatus(currentAnalysisId.toStdString());
        
        QString detailedResults = tr("=== CDR Analysis Results ===\n\n");
        detailedResults += tr("Analysis ID: %1\n").arg(currentAnalysisId);
        detailedResults += tr("Source Directory: %1\n").arg(QString::fromStdString(result.sourceDirectory));
        detailedResults += tr("Status: %1\n").arg(QString::fromStdString(result.status));
        detailedResults += tr("Progress: %1%\n\n").arg(result.progressPercentage);
        
        // Statistics
        detailedResults += tr("=== Statistics ===\n");
        detailedResults += tr("Total Files Scanned: %1\n").arg(result.totalFilesScanned);
        detailedResults += tr("Files Sanitized: %1\n").arg(result.filesSanitized);
        detailedResults += tr("Files Quarantined: %1\n").arg(result.filesQuarantined);
        detailedResults += tr("Clean Files: %1\n").arg(result.cleanFiles.size());
        detailedResults += tr("Threats Detected: %1\n").arg(result.threatsDetected);
        detailedResults += tr("Total Bytes Processed: %1\n").arg(result.totalBytes);
        detailedResults += tr("Sanitized Bytes: %1\n\n").arg(result.sanitizedBytes);
        
        // File lists
        if (!result.sanitizedFiles.empty()) {
            detailedResults += tr("=== Sanitized Files ===\n");
            for (const auto& file : result.sanitizedFiles) {
                detailedResults += tr("- %1\n").arg(QString::fromStdString(file));
            }
            detailedResults += "\n";
        }
        
        if (!result.quarantinedFiles.empty()) {
            detailedResults += tr("=== Quarantined Files ===\n");
            for (const auto& file : result.quarantinedFiles) {
                detailedResults += tr("- %1\n").arg(QString::fromStdString(file));
            }
            detailedResults += "\n";
        }
        
        if (!result.cleanFiles.empty() && result.cleanFiles.size() <= 10) { // Limit display for clean files
            detailedResults += tr("=== Clean Files ===\n");
            for (const auto& file : result.cleanFiles) {
                detailedResults += tr("- %1\n").arg(QString::fromStdString(file));
            }
            detailedResults += "\n";
        } else if (!result.cleanFiles.empty()) {
            detailedResults += tr("=== Clean Files ===\n");
            detailedResults += tr("Total: %1 clean files (not all listed)\n\n").arg(result.cleanFiles.size());
        }
        
        if (!result.errorMessage.empty()) {
            detailedResults += tr("=== Errors ===\n");
            detailedResults += QString::fromStdString(result.errorMessage) + "\n\n";
        }
        
        // Add file export buttons if there are sanitized files
        if (!result.sanitizedFiles.empty()) {
            detailedResults += tr("=== Export Options ===\n");
            detailedResults += tr("Use 'Export Results' button to save sanitized files to a custom location.\n");
        }
        
        cdrResultsTextEdit->setText(detailedResults);
        
        updateStatus(tr("CDR results displayed"));
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("CDR Error"), tr("Error viewing CDR results: ") + QString::fromStdString(e.what()));
        updateStatus(tr("CDR Results Error: ") + QString::fromStdString(e.what()));
    }
}
