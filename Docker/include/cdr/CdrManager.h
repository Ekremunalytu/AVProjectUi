#ifndef CDRMANAGER_H
#define CDRMANAGER_H

#include "docker/DockerManager.h"
#include "docker/DockerTypes.h"
#include "docker/DockerExceptions.h"
#include "CdrTypes.h" // Moved CdrTypes.h include to the top

#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <array>

namespace CDR {

    // File type detection and sanitization information
    struct FileTypeInfo {
        FileType type;
        std::string extension;
        std::vector<std::string> commonExtensions;
        std::vector<std::string> supportedTools;
        std::string description;
        bool requiresSpecialHandling;
        std::vector<std::string> activeContentTypes; // Active content types found in this file type
    };

    // Recovered file information
    struct RecoveredFileInfo {
        std::string fileName;            // File name
        std::string originalPath;        // Original file path
        std::string recoveredPath;       // Recovered file path
        std::string fileExtension;       // File extension
        long long fileSize = 0;          // File size
        std::string md5Hash;             // MD5 hash value
        std::string status;              // Status (recovered, corrupted, failed)
        double confidenceScore = 0.0;    // Confidence score (0.0-1.0)
        std::chrono::system_clock::time_point recoveryTime; // Recovery time
        std::string recoveryMethod;      // Recovery method
        std::vector<std::string> recoveredFiles; // List of recovered files (for CdrAnalysisResult)
    };

    // CDR analysis result
    struct CdrAnalysisResult {
        std::string analysisId;          // Unique analysis ID
        AnalysisType type;               // Analysis type
        std::string sourceDirectory;    // Source directory
        std::string status;              // Analysis status (running, completed, failed)
        int progressPercentage = 0;      // Progress percentage
        std::chrono::system_clock::time_point startTime;  // Start time
        std::chrono::system_clock::time_point endTime;    // End time
        std::string errorMessage;        // Error message (if any)
        bool isSafe = true;              // Indicates if the scanned content as a whole is safe
        bool allFilesProcessed = false;  // Indicates if all files in an analysis were processed

        // Processing results
        std::vector<std::string> processedFiles;   // List of processed files
        std::vector<std::string> sanitizedFiles;   // List of sanitized files
        std::vector<std::string> quarantinedFiles; // Quarantined files
        std::vector<std::string> cleanFiles;       // Clean files
        std::vector<std::string> recoveredFileItems;   // List of recovered files // Renamed from recoveredFiles to avoid conflict with RecoveredFileInfo::recoveredFiles
        std::map<std::string, std::string> metadata; // Additional metadata

        long long totalFilesScanned = 0;    // Total number of files scanned
        long long threatsDetected = 0;      // Number of threats detected
        long long filesQuarantined = 0;     // Number of files quarantined
        long long filesSanitized = 0;       // Number of files sanitized
        long long totalBytes = 0;           // Total bytes processed
        long long sanitizedBytes = 0;       // Sanitized bytes
    };

    // Sanitized file information
    struct SanitizedFileInfo {
        std::string originalPath;        // Original file path
        std::string sanitizedPath;       // Sanitized file path
        std::string quarantinePath;      // Quarantine path (if suspicious)
        std::string fileName;            // File name
        std::string fileExtension;       // File extension
        long long originalSize = 0;      // Original file size (initialized)
        long long sanitizedSize = 0;     // Sanitized file size (initialized)
        std::string md5Original;         // Original MD5 hash value
        std::string md5Sanitized;        // Sanitized MD5 hash value
        std::string status;              // Status (clean, sanitized, quarantined, failed, deleted)
        double threatScore = 0.0;        // Threat score (0.0-1.0) (initialized)
        std::vector<std::string> threatsFound; // Types of threats found
        std::vector<std::string> removedElements; // Removed active elements
        std::chrono::system_clock::time_point processTime = std::chrono::system_clock::now(); // Processing time (initialized)
        std::string processingLog;       // Processing log
        std::string rawScanOutput;       // Raw output from the container scan
        bool isSafe = true;              // Indicates if this specific file is safe
        bool isDeleted = false;          // Indicates if this specific file was deleted

        /**
         * @brief Get a summary of file processing results.
         * @return Human-readable summary
         */
        std::string getSummary() const {
            std::string summary = fileName + " (" + std::to_string(originalSize) + " bytes): " + status;
            if (!isSafe) {
                summary += " - Threat Score: " + std::to_string(threatScore);
            }
            if (threatsFound.size() > 0) {
                summary += " - Threats: " + std::to_string(threatsFound.size());
            }
            return summary;
        }

        /**
         * @brief Validate file info structure.
         * @return true if all required fields are properly set
         */
        bool isValid() const {
            return !originalPath.empty() && !fileName.empty() && !status.empty() && originalSize >= 0;
        }
    };

    // CDR Manager class - inherits from Docker::DockerManager
    class CdrManager : public Docker::DockerManager {
    private:
        std::string cdrContainerImage;
        std::string sandboxContainerImage;
        std::map<std::string, CdrAnalysisResult> activeAnalyses;
        mutable std::mutex analysesMutex;
        mutable std::mutex containerMutex_;
        std::map<std::string, std::thread> activeAnalysesThreads_;
        std::atomic<bool> keepThreadsJoined_;

        // Private helper methods
        std::string generateAnalysisId() const;
        std::string prepareCdrContainer(const CdrConfiguration& config);
        std::string prepareSandboxContainer();
        std::vector<SanitizedFileInfo> parseSanitizationResults(const std::string& resultsPath);
        bool scanFileForActiveContent(const std::string& filePath,
                                    std::vector<std::string>& threats);
        bool removeActiveContent(const std::string& inputPath,
                               const std::string& outputPath,
                               const std::vector<std::string>& elementsToRemove);
        static bool validateExecutableFile(const std::string& filePath);
        void copyFileToContainer(const std::string& containerId, 
                               const std::string& hostPath, 
                               const std::string& containerPath);
        void copyFileFromContainer(const std::string& containerId, 
                                 const std::string& containerPath, 
                                 const std::string& hostPath);

    public:
        // Constructor & Destructor
        CdrManager();
        explicit CdrManager(std::unique_ptr<Docker::DockerManager> dockerMgr);
        ~CdrManager();

        // Override detectFileTypeByContent as public method
        static FileType detectFileTypeByContent(const std::string& filePath);

        // Basic CDR operations (Content Detection & Remediation)
        std::string startAnalysis(const std::string& directoryPath,
                                 const CdrConfiguration& config);
        CdrAnalysisResult getAnalysisStatus(const std::string& analysisId) const;
        std::vector<CdrAnalysisResult> listActiveAnalyses() const;
        void stopAnalysis(const std::string& analysisId);
        SanitizedFileInfo scanFileInContainer(const std::string& filePath, const std::string& containerId, const CdrConfiguration& config, bool deleteIfUnsafe); // New method

        // File recovery operations
        std::vector<RecoveredFileInfo> parseRecoveryResults(const std::string& resultsPath);
        std::vector<RecoveredFileInfo> getRecoveredFiles(const std::string& analysisId) const;
        bool exportRecoveredFile(const std::string& analysisId,
                                const std::string& fileId,
                                const std::string& outputPath);

        // File sanitization operations
        std::vector<SanitizedFileInfo> getSanitizedFiles(const std::string& analysisId) const;
        bool exportSanitizedFile(const std::string& analysisId,
                                const std::string& fileId,
                                const std::string& outputPath);

        // Threat analysis
        std::vector<std::string> detectActiveContent(const std::string& filePath);
        bool sanitizeFile(const std::string& inputPath, const std::string& outputPath,
                         const CdrConfiguration& config);
        bool quarantineFile(const std::string& filePath, const std::string& quarantinePath);

        // Sandbox operations
        std::string createSandboxEnvironment();
        bool executeFileInSandbox(const std::string& sandboxId,
                                 const std::string& filePath,
                                 std::string& executionResult);
        void destroySandboxEnvironment(const std::string& sandboxId);

        // Configuration and management
        void setCdrContainerImage(const std::string& imageName);
        void setSandboxContainerImage(const std::string& imageName);
        bool validateCdrEnvironment() const;

        // Utility methods
        static std::string getAnalysisTypeString(AnalysisType type);
        static AnalysisType parseAnalysisType(const std::string& typeStr);

        // File type detection and management methods
        static FileType detectFileType(const std::string& filePath);
        static FileTypeInfo getFileTypeInfo(FileType type);
        static std::vector<FileTypeInfo> getSupportedFileTypes();
        static bool isFileTypeSupported(const std::string& filePath);
        static std::vector<std::string> getActiveContentTypesForFile(const std::string& filePath);

    private:
        // Helper function to reduce code duplication in sanitization methods
        SanitizationResult performSanitization(const std::string& fileTypeDescription,
                                           const std::string& inputPath,
                                           const std::string& outputPath,
                                           const CdrConfiguration& config,
                                           FileType fileType); // Added FileType for specific handling

    public:
        // Public methods for file sanitization
        SanitizationResult sanitizeOfficeDocument(const std::string& filePath, const std::string& outputPath, const CdrConfiguration& config); // Added declaration
        SanitizationResult sanitizePdfDocument(const std::string& filePath, const std::string& outputPath, const CdrConfiguration& config);
        SanitizationResult sanitizeArchiveFile(const std::string& filePath, const std::string& outputPath, const CdrConfiguration& config);
        SanitizationResult sanitizeHtmlDocument(const std::string& filePath, const std::string& outputPath, const CdrConfiguration& config);
        SanitizationResult sanitizeScriptFile(const std::string& filePath, const std::string& outputPath, const CdrConfiguration& config);
    };

} // namespace CDR

#endif // CDRMANAGER_H