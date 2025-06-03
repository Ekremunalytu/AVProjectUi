#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <stdexcept>
#include <algorithm> // For std::transform

namespace CDR {

// Forward declarations
class CdrManager;

// Custom Exception Base Class
class CdrBaseException : public std::runtime_error {
public:
    explicit CdrBaseException(const std::string& message)
        : std::runtime_error(message) {}

    virtual const char* getType() const noexcept {
        return "CdrBaseException";
    }
};

// Exception class for CDR configuration errors
class CdrConfigurationException : public CdrBaseException {
public:
    explicit CdrConfigurationException(const std::string& message) 
        : CdrBaseException(message) {}

    const char* getType() const noexcept override {
        return "CdrConfigurationException";
    }
};

// Specific Exception for Sanitization Failures
class CdrSanitizationException : public CdrBaseException {
public:
    CdrSanitizationException(const std::string& message, const std::string& filePath = "")
        : CdrBaseException(message), filePath_(filePath) {}

    const std::string& getFilePath() const noexcept {
        return filePath_;
    }

    const char* getType() const noexcept override {
        return "CdrSanitizationException";
    }

private:
    std::string filePath_;
};

// Specific Exception for File Operations
class CdrFileException : public CdrBaseException {
public:
    CdrFileException(const std::string& message, const std::string& filePath = "")
        : CdrBaseException(message), filePath_(filePath) {}

    const std::string& getFilePath() const noexcept {
        return filePath_;
    }

    const char* getType() const noexcept override {
        return "CdrFileException";
    }

private:
    std::string filePath_;
};

// Supported file types and sanitization strategies
enum class FileType {
    NOT_SET,            // Default unset value
    OFFICE_DOCUMENT,    // .docx, .xlsx, .pptx (Microsoft Office)
    PDF_DOCUMENT,       // .pdf (Adobe PDF)
    HTML_DOCUMENT,      // .html, .htm (Web pages)
    XML_DOCUMENT,       // .xml (XML files)
    RTF_DOCUMENT,       // .rtf (Rich Text Format)
    TEXT_DOCUMENT,      // .txt, .csv (Plain text)
    ARCHIVE_FILE,       // .zip, .rar, .7z (Archive files)
    IMAGE_FILE,         // .jpg, .png, .gif (Image files - metadata)
    EXECUTABLE_FILE,    // .exe, .dll, .so (Executable files)
    SCRIPT_FILE,        // .js, .vbs, .ps1 (Script files)
    EMAIL_FILE,         // .eml, .msg (Email files)
    UNKNOWN_FILE        // Unknown file type
};

// CDR analysis types (Content Detection & Remediation)
enum class AnalysisType {
    ACTIVE_CONTENT_SCAN,    // Active content scanning
    MALWARE_DETECTION,      // Malware detection
    SCRIPT_SANITIZATION,    // Script sanitization
    MACRO_REMOVAL,          // Macro removal
    EXECUTABLE_ANALYSIS,    // Executable file analysis
    COMPREHENSIVE_SCAN      // Comprehensive scan and sanitization
};

// CDR configuration (Content Detection & Remediation)
struct CdrConfiguration {
    // Security levels
    enum class SecurityLevel {
        LOW,        // Only known threats
        MEDIUM,     // Medium level, suspicious content
        HIGH,       // High, aggressive cleaning
        VERY_HIGH,  // Very high security, may block more content
        PARANOID    // Remove all active content
    };
    
    SecurityLevel securityLevel = SecurityLevel::MEDIUM;

    std::string inputDirectory;      // Directory of files to be scanned
    std::string outputDirectory;     // Output directory for cleaned files
    std::string quarantineDirectory; // Quarantine directory (for suspicious files)
    AnalysisType analysisType = AnalysisType::COMPREHENSIVE_SCAN; // Default analysis type
    bool preserveOriginal = true;    // Preserve original files
    bool autoSanitize = true;        // Is automatic cleaning active
    bool preserveTimestamps = true;  // Preserve timestamps
    std::vector<std::string> targetExtensions; // Target file extensions (.docx, .pdf, .xlsx etc.)
    std::vector<std::string> excludePatterns;  // File patterns to exclude
    int maxThreads = 4;              // Maximum number of threads
    long long maxFileSizeMB = 100;     // Maximum file size (MB)
    long long maxMemoryMB = 1024;    // Maximum memory usage (MB)
    int timeoutSeconds = 300;        // Timeout in seconds

    // New policy flags based on usage in CdrSanitizer.cpp
    bool blockOfficeMacros = false;
    bool blockPdfScripts = false;
    bool blockAllScripts = false;
    bool blockArchives = false;
    bool blockImagesWithMetadata = false; // For ImageSanitizer
    bool blockExecutables = true; // Default to blocking executables
    bool allowUnknownTypes = false; // Default to not allowing unknown types
    
    /**
     * @brief Validate configuration settings.
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
     * @brief Check if a file extension is in the target list.
     * @param extension File extension to check (with or without dot)
     * @return true if extension should be processed, false otherwise
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

// Structure for sanitization result
struct SanitizationResult {
    bool success = false;
    std::string errorMessage;
    std::string inputPath;        // Path to the original input file
    std::string outputPath;       // Path to the sanitized output file (if successful)
    std::string originalPath;     // Often same as inputPath, for clarity
    std::string sanitizedPath;    // Often same as outputPath, for clarity
    long long originalSize = 0;
    long long sanitizedSize = 0;
    FileType fileType = FileType::UNKNOWN_FILE; // Type of the processed file
    std::vector<std::string> threatsDetected;
    std::vector<std::string> actionsPerformed;
    bool requiresQuarantine = false;
    std::string quarantineReason;
    std::string quarantinePath;   // Path where the file was quarantined
    std::string md5Hash;          // MD5 hash of the sanitized file (or original if no change)
    std::string sanitizationDetails; // Additional details about sanitization process
    std::chrono::system_clock::time_point processTime = std::chrono::system_clock::now(); // When processing occurred
    
    /**
     * @brief Check if sanitization was successful and safe.
     * @return true if file was processed successfully and is safe to use
     */
    bool isSafeAndSuccess() const {
        return success && !requiresQuarantine && threatsDetected.empty();
    }
    
    /**
     * @brief Get a summary of the sanitization result.
     * @return Human-readable summary string
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

// Utility function to convert FileType enum to string
std::string getFileTypeName(FileType fileType);

} // namespace CDR
