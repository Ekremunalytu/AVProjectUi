/**
 * @file CdrTypes.h
 * @brief Type definitions and data structures for Content Disarm and Reconstruction (CDR) operations
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 * 
 * @details This header defines comprehensive data structures, enumerations, and exception
 * classes for the CDR (Content Disarm and Reconstruction) system. The CDR module provides
 * advanced file sanitization, threat detection, and content reconstruction capabilities
 * for various file types and formats.
 * 
 * Key Components:
 * - File type identification and classification
 * - Sanitization strategy definitions
 * - Analysis and scanning configuration
 * - Exception hierarchy for error handling
 * - Security level and policy definitions
 * - File processing result structures
 * 
 * @note All CDR operations should be performed in isolated environments
 * @warning Proper validation and sandboxing required for all file operations
 */

#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <stdexcept>
#include <algorithm> // For std::transform

namespace CDR {

// Forward declarations
class CdrManager;

/**
 * @brief Base exception class for all CDR-related errors
 * 
 * @details Provides a common foundation for all CDR exception types,
 * enabling consistent error handling throughout the CDR module.
 */
class CdrBaseException : public std::runtime_error {
public:
    /**
     * @brief Construct a new CDR Base Exception
     * @param message Descriptive error message
     */
    explicit CdrBaseException(const std::string& message)
        : std::runtime_error(message) {}

    /**
     * @brief Get the exception type name
     * @return Exception type identifier string
     */
    virtual const char* getType() const noexcept {
        return "CdrBaseException";
    }
};

/**
 * @brief Exception for CDR configuration and setup errors
 * 
 * @details Thrown when CDR configuration parameters are invalid,
 * missing, or incompatible with the current environment.
 */
class CdrConfigurationException : public CdrBaseException {
public:
    /**
     * @brief Construct a new CDR Configuration Exception
     * @param message Descriptive error message about the configuration issue
     */
    explicit CdrConfigurationException(const std::string& message) 
        : CdrBaseException(message) {}

    /**
     * @brief Get the exception type name
     * @return Exception type identifier string
     */
    const char* getType() const noexcept override {
        return "CdrConfigurationException";
    }
};

/**
 * @brief Exception for file sanitization operation failures
 * 
 * @details Thrown when CDR sanitization processes fail, including
 * cases where files cannot be properly cleaned or reconstructed.
 */
class CdrSanitizationException : public CdrBaseException {
public:
    /**
     * @brief Construct a new CDR Sanitization Exception
     * @param message Descriptive error message about the sanitization failure
     * @param filePath Path to the file that failed sanitization (optional)
     */
    CdrSanitizationException(const std::string& message, const std::string& filePath = "")
        : CdrBaseException(message), filePath_(filePath) {}

    /**
     * @brief Get the file path associated with the sanitization failure
     * @return File path string, empty if not specified
     */
    const std::string& getFilePath() const noexcept {
        return filePath_;
    }

    /**
     * @brief Get the exception type name
     * @return Exception type identifier string
     */
    const char* getType() const noexcept override {
        return "CdrSanitizationException";
    }

private:
    std::string filePath_; ///< Path to the file that failed sanitization
};

/**
 * @brief Exception for file system and I/O operation failures
 * 
 * @details Thrown when CDR operations encounter file system errors,
 * access permission issues, or I/O failures during processing.
 */
class CdrFileException : public CdrBaseException {
public:
    /**
     * @brief Construct a new CDR File Exception
     * @param message Descriptive error message about the file operation failure
     * @param filePath Path to the file that caused the error (optional)
     */
    CdrFileException(const std::string& message, const std::string& filePath = "")
        : CdrBaseException(message), filePath_(filePath) {}

    /**
     * @brief Get the file path associated with the file operation failure
     * @return File path string, empty if not specified
     */
    const std::string& getFilePath() const noexcept {
        return filePath_;
    }

    /**
     * @brief Get the exception type name
     * @return Exception type identifier string
     */
    const char* getType() const noexcept override {
        return "CdrFileException";
    }

private:
    std::string filePath_; ///< Path to the file that caused the error
};

/**
 * @brief Enumeration of supported file types for CDR processing
 * 
 * @details Defines all file types that the CDR system can process,
 * including documents, archives, executables, and multimedia files.
 * Each type has specific sanitization strategies and security considerations.
 */
enum class FileType {
    NOT_SET,            ///< Default unset value for uninitialized file type
    OFFICE_DOCUMENT,    ///< Microsoft Office documents (.docx, .xlsx, .pptx)
    PDF_DOCUMENT,       ///< Adobe PDF documents (.pdf)
    HTML_DOCUMENT,      ///< Web pages and HTML documents (.html, .htm)
    XML_DOCUMENT,       ///< XML structured documents (.xml)
    RTF_DOCUMENT,       ///< Rich Text Format documents (.rtf)
    TEXT_DOCUMENT,      ///< Plain text and CSV files (.txt, .csv)
    ARCHIVE_FILE,       ///< Compressed archive files (.zip, .rar, .7z)
    IMAGE_FILE,         ///< Image files with metadata processing (.jpg, .png, .gif)
    EXECUTABLE_FILE,    ///< Executable and library files (.exe, .dll, .so)
    SCRIPT_FILE,        ///< Script files requiring analysis (.js, .vbs, .ps1)
    EMAIL_FILE,         ///< Email message files (.eml, .msg)
    UNKNOWN_FILE        ///< Unknown or unsupported file type
};

/**
 * @brief Types of CDR analysis and scanning operations
 * 
 * @details Defines the various analysis strategies available for content
 * detection, threat identification, and remediation processes.
 */
enum class AnalysisType {
    ACTIVE_CONTENT_SCAN,    ///< Scan for active content (scripts, macros, embedded objects)
    MALWARE_DETECTION,      ///< Malware and virus detection analysis
    SCRIPT_SANITIZATION,    ///< JavaScript and script sanitization
    MACRO_REMOVAL,          ///< Microsoft Office macro detection and removal
    EXECUTABLE_ANALYSIS,    ///< Binary executable file analysis
    COMPREHENSIVE_SCAN      ///< Complete analysis with all available techniques
};

/**
 * @brief Configuration structure for CDR operations
 * 
 * @details Comprehensive configuration object that controls all aspects
 * of CDR processing, including security levels, analysis types, and
 * processing parameters.
 */
struct CdrConfiguration {
    /**
     * @brief Security levels for CDR processing
     * 
     * @details Defines the aggressiveness of content sanitization,
     * from basic threat detection to complete active content removal.
     */
    enum class SecurityLevel {
        LOW,        ///< Only process known threats and malware
        MEDIUM,     ///< Process suspicious content with moderate cleaning
        HIGH,       ///< Aggressive cleaning with high security standards
        VERY_HIGH,  ///< Very high security, may block more legitimate content
        PARANOID    ///< Remove all active content regardless of legitimacy
    };
    
    SecurityLevel securityLevel = SecurityLevel::MEDIUM; ///< Current security level setting

    std::string inputDirectory;      ///< Directory containing files to be scanned
    std::string outputDirectory;     ///< Output directory for cleaned files
    std::string quarantineDirectory; ///< Quarantine directory for suspicious files
    AnalysisType analysisType = AnalysisType::COMPREHENSIVE_SCAN; ///< Type of analysis to perform
    bool preserveOriginal = true;    ///< Whether to preserve original files
    bool autoSanitize = true;        ///< Enable automatic sanitization
    bool preserveTimestamps = true;  ///< Preserve file timestamps during processing
    std::vector<std::string> targetExtensions; ///< Target file extensions (.docx, .pdf, etc.)
    std::vector<std::string> excludePatterns;  ///< File patterns to exclude from processing
    int maxThreads = 4;              ///< Maximum number of processing threads
    long long maxFileSizeMB = 100;   ///< Maximum file size limit in MB
    long long maxMemoryMB = 1024;    ///< Maximum memory usage limit in MB
    int timeoutSeconds = 300;        ///< Processing timeout in seconds

    // Policy flags for specific content types
    bool blockOfficeMacros = false;          ///< Block Microsoft Office macros
    bool blockPdfScripts = false;            ///< Block PDF embedded scripts
    bool blockAllScripts = false;            ///< Block all script content
    bool blockArchives = false;              ///< Block archive file processing
    bool blockImagesWithMetadata = false;    ///< Block images with embedded metadata
    bool blockExecutables = true;            ///< Block executable files (default: true)
    bool allowUnknownTypes = false;          ///< Allow processing of unknown file types
    
    /**
     * @brief Validate configuration settings for consistency and safety
     * @throws CdrConfigurationException if configuration is invalid
     */
    void validate() const {
        if (inputDirectory.empty()) {
            throw CdrConfigurationException("Input directory cannot be empty");
        }
        if (outputDirectory.empty()) {
            throw CdrConfigurationException("Output directory cannot be empty");
        }
        if (quarantineDirectory.empty()) {
            throw CdrConfigurationException("Quarantine directory cannot be empty");
        }
        if (maxThreads < 1 || maxThreads > 32) {
            throw CdrConfigurationException("Thread count must be between 1 and 32");
        }
        if (maxFileSizeMB < 1 || maxFileSizeMB > 10240) { // Max 10GB
            throw CdrConfigurationException("File size limit must be between 1MB and 10GB");
        }
        if (maxMemoryMB < 64 || maxMemoryMB > 32768) { // Max 32GB
            throw CdrConfigurationException("Memory limit must be between 64MB and 32GB");
        }
        if (timeoutSeconds < 30 || timeoutSeconds > 7200) { // Max 2 hours
            throw CdrConfigurationException("Timeout must be between 30 seconds and 2 hours");
        }
        
        // Validate directory paths are different
        if (inputDirectory == outputDirectory) {
            throw CdrConfigurationException("Input and output directories must be different");
        }
        if (inputDirectory == quarantineDirectory) {
            throw CdrConfigurationException("Input and quarantine directories must be different");
        }
    }
    
    /**
     * @brief Check if a file extension should be processed based on target list
     * @param extension File extension to check (with or without leading dot)
     * @return true if extension should be processed, false otherwise
     * 
     * @details If no target extensions are specified, all extensions are processed.
     * The comparison is case-insensitive and handles extensions with or without dots.
     */
    bool shouldProcessExtension(const std::string& extension) const {
        if (targetExtensions.empty()) return true; // Process all if no specific targets
        
        std::string ext = extension;
        if (!ext.empty() && ext[0] != '.') {
            ext = "." + ext;
        }
        
        std::string extLower = ext;
        std::transform(extLower.begin(), extLower.end(), extLower.begin(), ::tolower);
        
        for (const auto& target : targetExtensions) {
            std::string targetLower = target;
            std::transform(targetLower.begin(), targetLower.end(), targetLower.begin(), ::tolower);
            if (extLower == targetLower) return true;
        }
        
        return false;
    }
};

/**
 * @brief Result structure for file sanitization operations
 * 
 * @details Contains comprehensive information about the sanitization process,
 * including success status, file paths, detected threats, and actions performed.
 * Provides detailed tracking of the entire CDR operation lifecycle.
 */
struct SanitizationResult {
    bool success = false;                ///< Whether sanitization completed successfully
    std::string errorMessage;            ///< Error message if sanitization failed
    std::string inputPath;               ///< Path to the original input file
    std::string outputPath;              ///< Path to the sanitized output file
    std::string originalPath;            ///< Original file path for reference
    std::string sanitizedPath;           ///< Sanitized file path for reference
    long long originalSize = 0;          ///< Size of original file in bytes
    long long sanitizedSize = 0;         ///< Size of sanitized file in bytes
    FileType fileType = FileType::UNKNOWN_FILE; ///< Detected file type
    std::vector<std::string> threatsDetected;   ///< List of detected threats
    std::vector<std::string> actionsPerformed;  ///< List of sanitization actions taken
    bool requiresQuarantine = false;     ///< Whether file should be quarantined
    std::string quarantineReason;        ///< Reason for quarantine if applicable
    std::string quarantinePath;          ///< Path where file was quarantined
    std::string md5Hash;                 ///< MD5 hash of processed file
    std::string sanitizationDetails;     ///< Additional processing details
    std::chrono::system_clock::time_point processTime = std::chrono::system_clock::now(); ///< Processing timestamp
    
    /**
     * @brief Check if sanitization was successful and file is safe to use
     * @return true if file was processed successfully with no threats or quarantine needed
     */
    bool isSafeAndSuccess() const {
        return success && !requiresQuarantine && threatsDetected.empty();
    }
    
    /**
     * @brief Get a human-readable summary of the sanitization result
     * @return Summary string describing the outcome of sanitization
     * 
     * @details Provides a concise description of the sanitization result,
     * including failure reasons, quarantine status, or threat information.
     */
    std::string getSummary() const {
        if (!success) {
            return "Failed: " + errorMessage;
        }
        if (requiresQuarantine) {
            return "Quarantined: " + quarantineReason + " (" + std::to_string(threatsDetected.size()) + " threats)";
        }
        if (threatsDetected.empty()) {
            return "Clean: No threats detected";
        }
        return "Sanitized: " + std::to_string(threatsDetected.size()) + " threats removed";
    }
};

/**
 * @brief Utility function to convert FileType enum to string representation
 * @param fileType The FileType enum value to convert
 * @return Human-readable string name of the file type
 */
std::string getFileTypeName(FileType fileType);

} // namespace CDR
