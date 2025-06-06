/**
 * @file CdrManager.h
 * @brief Content Disarm and Reconstruction (CDR) Manager - Advanced file sanitization and threat detection
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 * 
 * @details This header defines the CDR (Content Disarm and Reconstruction) management system
 * which provides comprehensive file analysis, sanitization, and threat detection capabilities.
 * The system integrates with Docker containers to provide isolated and secure processing
 * environments for handling potentially malicious content.
 * 
 * Key Components:
 * - File type detection and analysis
 * - Content sanitization and reconstruction  
 * - Active content detection and removal
 * - Sandbox execution environments
 * - File recovery and restoration
 * - Quarantine management
 * 
 * @note This module requires Docker to be installed and properly configured
 * @warning Handle all file paths and content with appropriate security measures
 */
/**
 * @file CdrManager.h
 * @brief Content Disarm and Reconstruction (CDR) Manager - Advanced file sanitization and threat detection
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 * 
 * @details This header defines the CDR (Content Disarm and Reconstruction) management system
 * which provides comprehensive file analysis, sanitization, and threat detection capabilities.
 * The system integrates with Docker containers to provide isolated and secure processing
 * environments for handling potentially malicious content.
 * 
 * Key Components:
 * - File type detection and analysis
 * - Content sanitization and reconstruction  
 * - Active content detection and removal
 * - Sandbox execution environments
 * - File recovery and restoration
 * - Quarantine management
 * 
 * @note This module requires Docker to be installed and properly configured
 * @warning Handle all file paths and content with appropriate security measures
 */
#ifndef CDRMANAGER_H
#define CDRMANAGER_H

#include "../../infrastructure/docker/DockerManager.h"
#include "../../infrastructure/docker/DockerTypes.h"
#include "../../infrastructure/docker/DockerExceptions.h"
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

    /**
     * @brief File type detection and sanitization information
     * @details Contains comprehensive metadata about supported file types,
     * including detection methods, sanitization tools, and security considerations.
     */
    struct FileTypeInfo {
        FileType type;                              ///< Enumerated file type identifier
        std::string extension;                      ///< Primary file extension
        std::vector<std::string> commonExtensions;  ///< List of all supported extensions for this type
        std::vector<std::string> supportedTools;    ///< Available sanitization tools
        std::string description;                    ///< Human-readable description of file type
        bool requiresSpecialHandling;               ///< Whether this file type needs special security measures
        std::vector<std::string> activeContentTypes; ///< Types of active content that can be found in this file type
    };

    /**
     * @brief Information about files recovered during CDR analysis
     * @details Stores comprehensive metadata about files that have been recovered
     * from corrupted or partially damaged sources during the CDR process.
     */
    struct RecoveredFileInfo {
        std::string fileName;            ///< Name of the recovered file
        std::string originalPath;        ///< Path to the original file before processing
        std::string recoveredPath;       ///< Path where the recovered file is stored
        std::string fileExtension;       ///< File extension of the recovered file
        long long fileSize = 0;          ///< Size of the recovered file in bytes
        std::string md5Hash;             ///< MD5 hash for file integrity verification
        std::string status;              ///< Recovery status (recovered, corrupted, failed)
        double confidenceScore = 0.0;    ///< Confidence level of successful recovery (0.0-1.0)
        std::chrono::system_clock::time_point recoveryTime; ///< Timestamp when recovery was completed
        std::string recoveryMethod;      ///< Method or tool used for recovery
        std::vector<std::string> recoveredFiles; ///< List of all files recovered in this operation
    };

    /**
     * @brief Results and status information for CDR analysis operations
     * @details Comprehensive container for all data related to a CDR analysis session,
     * including progress tracking, results, and metadata.
     */
    struct CdrAnalysisResult {
        std::string analysisId;          ///< Unique identifier for this analysis session
        AnalysisType type;               ///< Type of analysis performed
        std::string sourceDirectory;    ///< Source directory
        std::string status;              ///< Analysis status (running, completed, failed)
        int progressPercentage = 0;      ///< Progress percentage
        std::chrono::system_clock::time_point startTime;  ///< Start time
        std::chrono::system_clock::time_point endTime;    ///< End time
        std::string errorMessage;        ///< Error message (if any)
        bool isSafe = true;              ///< Indicates if the scanned content as a whole is safe
        bool allFilesProcessed = false;  ///< Indicates if all files in an analysis were processed

        // Processing results
        std::vector<std::string> processedFiles;   ///< List of processed files
        std::vector<std::string> sanitizedFiles;   ///< List of sanitized files
        std::vector<std::string> quarantinedFiles; ///< Quarantined files
        std::vector<std::string> cleanFiles;       ///< Clean files
        std::vector<std::string> recoveredFileItems;   ///< List of recovered files // Renamed from recoveredFiles to avoid conflict with RecoveredFileInfo::recoveredFiles
        std::map<std::string, std::string> metadata; ///< Additional metadata

        long long totalFilesScanned = 0;    ///< Total number of files scanned
        long long threatsDetected = 0;      ///< Number of threats detected
        long long filesQuarantined = 0;     ///< Number of files quarantined
        long long filesSanitized = 0;       ///< Number of files sanitized
        long long totalBytes = 0;           ///< Total bytes processed
        long long sanitizedBytes = 0;       ///< Sanitized bytes
    };

    /**
     * @brief Information about files that have been sanitized
     * @details Contains complete metadata about the sanitization process for individual files,
     * including original and sanitized file information, threat analysis, and processing logs.
     */
    struct SanitizedFileInfo {
        std::string originalPath;        ///< Path to the original file before sanitization
        std::string sanitizedPath;       ///< Path to the sanitized output file
        std::string quarantinePath;      ///< Path where file is quarantined if deemed unsafe
        std::string fileName;            ///< Name of the file being processed
        std::string fileExtension;       ///< File extension
        long long originalSize = 0;      ///< Size of original file in bytes
        long long sanitizedSize = 0;     ///< Size of sanitized file in bytes
        std::string md5Original;         ///< MD5 hash of original file
        std::string md5Sanitized;        ///< MD5 hash of sanitized file
        std::string status;              ///< Processing status (clean, sanitized, quarantined, failed, deleted)
        double threatScore = 0.0;        ///< Calculated threat level (0.0-1.0)
        std::vector<std::string> threatsFound; ///< List of specific threats detected
        std::vector<std::string> removedElements; ///< List of active content elements that were removed
        std::chrono::system_clock::time_point processTime = std::chrono::system_clock::now(); ///< When processing occurred
        std::string processingLog;       ///< Detailed log of processing steps
        std::string rawScanOutput;       ///< Raw output from container scanning process
        bool isSafe = true;              ///< Whether the file is considered safe after processing
        bool isDeleted = false;          ///< Whether the file was deleted during processing

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

    /**
     * @brief Main CDR (Content Disarm and Reconstruction) Manager class
     * @details Provides comprehensive file sanitization, threat detection, and content analysis
     * capabilities using containerized processing environments for security isolation.
     * 
     * This class manages:
     * - File type detection and validation
     * - Content sanitization and reconstruction
     * - Active content detection and removal
     * - Sandbox execution environments
     * - File recovery operations
     * - Threat analysis and quarantine management
     * 
     * @note Inherits from Docker::DockerManager to leverage container management capabilities
     * @warning All file operations should be performed in isolated environments
     */
    class CdrManager : public Docker::DockerManager {
    private:
        std::string cdrContainerImage;                          ///< Docker image for CDR operations
        std::string sandboxContainerImage;                      ///< Docker image for sandbox environments
        std::map<std::string, CdrAnalysisResult> activeAnalyses; ///< Currently running analyses
        mutable std::mutex analysesMutex;                       ///< Thread safety for analyses map
        mutable std::mutex containerMutex_;                     ///< Thread safety for container operations
        std::map<std::string, std::thread> activeAnalysesThreads_; ///< Background analysis threads
        std::atomic<bool> keepThreadsJoined_;                   ///< Flag for thread management

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
        
        // Enhanced threat detection helper methods
        bool detectPdfActiveContent(const std::string& filePath);
        bool detectHtmlScript(const std::string& filePath);
        bool detectSuspiciousContent(const std::string& filePath);

    public:
        /**
         * @brief Default constructor
         * @details Initializes CdrManager with default Docker manager and container images
         */
        CdrManager();
        
        /**
         * @brief Constructor with custom Docker manager
         * @param dockerMgr Unique pointer to a Docker::DockerManager instance
         * @details Allows injection of a custom Docker manager for testing or specialized configurations
         */
        explicit CdrManager(std::unique_ptr<Docker::DockerManager> dockerMgr);
        
        /**
         * @brief Destructor
         * @details Ensures proper cleanup of active analyses and container resources
         */
        ~CdrManager();

        /**
         * @brief Detect file type by examining file content
         * @param filePath Path to the file to analyze
         * @return Detected FileType enumeration value
         * @details Uses file content analysis rather than just extension to determine type
         */
        static FileType detectFileTypeByContent(const std::string& filePath);

        /**
         * @brief Start a new CDR analysis session
         * @param directoryPath Directory containing files to analyze
         * @param config Configuration parameters for the analysis
         * @return Unique analysis ID for tracking progress
         * @throws Docker::DockerException if container setup fails
         */
        std::string startAnalysis(const std::string& directoryPath,
                                 const CdrConfiguration& config);

        /**
         * @brief Get current status of an analysis session
         * @param analysisId Unique identifier of the analysis to check
         * @return CdrAnalysisResult containing current status and results
         * @throws std::invalid_argument if analysis ID is not found
         */
        CdrAnalysisResult getAnalysisStatus(const std::string& analysisId) const;

        /**
         * @brief List all currently active analysis sessions
         * @return Vector of CdrAnalysisResult for all active analyses
         */
        std::vector<CdrAnalysisResult> listActiveAnalyses() const;

        /**
         * @brief Stop a running analysis session
         * @param analysisId Unique identifier of the analysis to stop
         * @details Gracefully terminates the analysis and cleans up resources
         */
        void stopAnalysis(const std::string& analysisId);

        /**
         * @brief Scan a single file within a container environment
         * @param filePath Path to the file to scan
         * @param containerId ID of the container to use for scanning
         * @param config Configuration for the scanning process
         * @param deleteIfUnsafe Whether to delete the file if deemed unsafe
         * @return SanitizedFileInfo with scan results and processing details
         */
        SanitizedFileInfo scanFileInContainer(const std::string& filePath, 
                                            const std::string& containerId, 
                                            const CdrConfiguration& config, 
                                            bool deleteIfUnsafe);

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
        /**
         * @brief Perform file sanitization with specified parameters
         * @param fileTypeDescription Human-readable description of file type
         * @param inputPath Path to input file
         * @param outputPath Path for sanitized output
         * @param config Sanitization configuration
         * @param fileType Specific file type for specialized handling
         * @return SanitizationResult with operation results
         * @details Helper method to reduce code duplication in sanitization operations
         */
        SanitizationResult performSanitization(const std::string& fileTypeDescription,
                                           const std::string& inputPath,
                                           const std::string& outputPath,
                                           const CdrConfiguration& config,
                                           FileType fileType);

    public:
        /**
         * @brief Sanitize Microsoft Office documents
         * @param filePath Path to the Office document
         * @param outputPath Path for the sanitized output
         * @param config Sanitization configuration
         * @return SanitizationResult with processing details
         */
        SanitizationResult sanitizeOfficeDocument(const std::string& filePath, 
                                                 const std::string& outputPath, 
                                                 const CdrConfiguration& config);

        /**
         * @brief Sanitize PDF documents
         * @param filePath Path to the PDF file
         * @param outputPath Path for the sanitized output
         * @param config Sanitization configuration
         * @return SanitizationResult with processing details
         */
        SanitizationResult sanitizePdfDocument(const std::string& filePath, 
                                              const std::string& outputPath, 
                                              const CdrConfiguration& config);

        /**
         * @brief Sanitize archive files (ZIP, RAR, etc.)
         * @param filePath Path to the archive file
         * @param outputPath Path for the sanitized output
         * @param config Sanitization configuration
         * @return SanitizationResult with processing details
         */
        SanitizationResult sanitizeArchiveFile(const std::string& filePath, 
                                              const std::string& outputPath, 
                                              const CdrConfiguration& config);

        /**
         * @brief Sanitize HTML documents
         * @param filePath Path to the HTML file
         * @param outputPath Path for the sanitized output
         * @param config Sanitization configuration
         * @return SanitizationResult with processing details
         */
        SanitizationResult sanitizeHtmlDocument(const std::string& filePath, 
                                               const std::string& outputPath, 
                                               const CdrConfiguration& config);

        /**
         * @brief Sanitize text documents (.txt, .csv, etc.)
         * @param filePath Path to the text file
         * @param outputPath Path for the sanitized output
         * @param config Sanitization configuration
         * @return SanitizationResult with processing details
         */
        SanitizationResult sanitizeTextDocument(const std::string& filePath, 
                                               const std::string& outputPath, 
                                               const CdrConfiguration& config);

        /**
         * @brief Sanitize script files (JavaScript, PowerShell, etc.)
         * @param filePath Path to the script file
         * @param outputPath Path for the sanitized output
         * @param config Sanitization configuration
         * @return SanitizationResult with processing details
         */
        SanitizationResult sanitizeScriptFile(const std::string& filePath, 
                                             const std::string& outputPath, 
                                             const CdrConfiguration& config);
    };

} // namespace CDR

#endif // CDRMANAGER_H