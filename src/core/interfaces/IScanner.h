#ifndef ISCANNER_H
#define ISCANNER_H

#include <QString>
#include <QFileInfo>

/**
 * @brief The IScanner interface provides an abstract base for file scanning operations.
 *
 * It defines a common contract for various scanner implementations (e.g., local,
 * Docker-based, remote services). This includes core functionalities such as
 * selecting a file, initiating a scan, retrieving results, checking scan status,
 * and canceling an ongoing scan.
 */
class IScanner {
public:
    virtual ~IScanner() = default;
    
    /**
     * @brief Allows the user to select a file for scanning.
     *
     * Typically, this method will open a file dialog, enabling the user
     * to browse and choose a file from the local filesystem.
     * @return True if a file was successfully selected, false otherwise (e.g., if the user cancels the dialog).
     */
    virtual bool selectFile() = 0;
    
    /**
     * @brief Initiates a scan for the file at the specified path.
     *
     * This function starts the scanning process for the given file path.
     * The scan operation might be asynchronous.
     * @param filePath The absolute or relative path to the file that needs to be scanned.
     * @return True if the scan was successfully initiated, false otherwise (e.g., if the file path is invalid or the scanner is busy).
     */
    virtual bool scanFile(const QString& filePath) = 0;
    
    /**
     * @brief Retrieves information about the currently selected or scanned file.
     *
     * Returns a QFileInfo object for the file last selected via selectFile()
     * or scanned via scanFile(). If no file is selected or if an operation
     * failed, it might return an invalid QFileInfo object.
     * @return QFileInfo object representing the selected file.
     */
    virtual QFileInfo getSelectedFile() const = 0;
    
    /**
     * @brief Fetches the results of a completed scan.
     *
     * After a scan operation is finished, this method returns a QString
     * containing the scan outcome. The format of the results (e.g., plain text,
     * JSON, XML) is implementation-dependent.
     * @return QString containing the scan results. May return an empty QString
     *         if the scan is not completed, failed, or if there are no results.
     */
    virtual QString getResults() const = 0;
    
    /**
     * @brief Checks if the scanner is currently performing a scan operation.
     * @return True if the scanner is actively scanning a file, false otherwise.
     */
    virtual bool isScanning() const = 0;
    
    /**
     * @brief Attempts to cancel an ongoing scan operation.
     *
     * If a scan is in progress, this function tries to stop it.
     * The success of the cancellation depends on the specific scanner
     * implementation and the state of the scan.
     * @return True if the scan was successfully canceled, false otherwise (e.g.,
     *         if there was no active scan or if it could not be canceled).
     */
    virtual bool cancelScan() = 0;
    
    /**
     * @brief Gets the last error that occurred during scanning.
     * @return Error message as a QString.
     */
    virtual QString getLastError() const = 0;
    
    /**
     * @brief Sets the file path for scanning.
     * @param filePath The path to the file.
     */
    virtual void setFile(const QString& filePath) = 0;
    
    /**
     * @brief Gets the currently set file path.
     * @return The file path as a QString.
     */
    virtual QString getFile() const = 0;
};

#endif // ISCANNER_H 