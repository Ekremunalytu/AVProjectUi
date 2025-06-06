/**
 * @file CdrSanitizer.h
 * @brief CDR (Content Disarm and Reconstruction) Sanitizer - File sanitization engine
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 * 
 * @details This header defines the main sanitization engine for the CDR system.
 * The CdrSanitizer class provides a unified interface for sanitizing various file
 * types by removing or neutralizing potentially malicious active content while
 * preserving the file's legitimate functionality.
 * 
 * Supported file types:
 * - Microsoft Office documents (Word, Excel, PowerPoint)
 * - PDF documents
 * - HTML files
 * - Archive files (ZIP, RAR, 7z)
 * - Script files (JavaScript, PowerShell, VBScript)
 * - Image files (with embedded metadata)
 * 
 * @note This sanitizer uses pluggable FileSanitizer implementations for different file types
 * @warning All sanitization operations should be performed in isolated environments
 */
#ifndef CDRSANITIZER_H
#define CDRSANITIZER_H

#include "CdrTypes.h" // Include CdrTypes.h for SanitizationResult and CdrConfiguration
#include "FileSanitizer.h" // Include FileSanitizer base class
#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <mutex>

// Forward declaration if DockerManager is used by CdrSanitizer, otherwise remove
// namespace Docker {
//     class DockerManager;
// }

namespace CDR {

// struct SanitizationResult { // Bu tanım CdrTypes.h dosyasına taşındı
//     bool success = false;
//     std::string errorMessage;
//     std::vector<std::string> threatsDetected;
//     std::vector<std::string> actionsPerformed;
//     bool requiresQuarantine = false;
//     std::string quarantineReason;
//     std::string quarantinePath; // Path where the file was quarantined
//     std::string inputPath;      // Path to the original input file
//     std::string outputPath;     // Path to the sanitized output file (if successful)
//     std::string originalPath;   // Often same as inputPath, for clarity
//     std::string sanitizedPath;  // Often same as outputPath, for clarity
//     long long originalSize = 0;
//     long long sanitizedSize = 0;
//     FileType fileType = FileType::UNKNOWN_FILE; // Type of the processed file
//     std::string md5Hash;        // MD5 hash of the sanitized file (or original if no change)
// };

class CdrSanitizer {
public:
    CdrSanitizer();
    ~CdrSanitizer();

    void registerSanitizer(std::unique_ptr<FileSanitizer> sanitizer);
    
    /**
     * @brief Sanitize a file with proper validation and error handling.
     * @param inputPath Path to input file
     * @param outputPath Path for sanitized output
     * @param config Sanitization configuration
     * @param fileType Detected file type
     * @return SanitizationResult with detailed results
     */
    SanitizationResult sanitizeFile(const std::string& inputPath, 
                                  const std::string& outputPath, 
                                  const CdrConfiguration& config,
                                  FileType fileType);
    
    /**
     * @brief Validate file before sanitization.
     * @param filePath Path to file to validate
     * @param config Configuration to use for validation
     * @return true if file is valid for sanitization
     */
    bool validateFile(const std::string& filePath, const CdrConfiguration& config) const;
    
    /**
     * @brief Detect and validate file type.
     * @param filePath Path to file
     * @return Detected FileType, UNKNOWN_FILE if cannot be determined
     */
    static FileType detectAndValidateFileType(const std::string& filePath);

    // File-specific sanitization methods with validation
    SanitizationResult sanitizeOfficeFile(const std::string& inputPath, const std::string& outputPath, const CdrConfiguration& config);
    SanitizationResult sanitizePdfFile(const std::string& inputPath, const std::string& outputPath, const CdrConfiguration& config);
    SanitizationResult sanitizeHtmlFile(const std::string& inputPath, const std::string& outputPath, const CdrConfiguration& config);
    SanitizationResult sanitizeArchiveFile(const std::string& inputPath, const std::string& outputPath, const CdrConfiguration& config);
    SanitizationResult sanitizeScriptFile(const std::string& inputPath, const std::string& outputPath, const CdrConfiguration& config);
    SanitizationResult sanitizeImageFile(const std::string& inputPath, const std::string& outputPath, const CdrConfiguration& config);

    // Information methods
    std::vector<std::string> getAvailableSanitizers() const;
    std::vector<std::string> getSupportedFileTypes() const;
    
    /**
     * @brief Get sanitizer statistics.
     * @return Map of sanitizer names to usage statistics
     */
    std::map<std::string, size_t> getSanitizerStats() const;

private:
    std::vector<std::unique_ptr<FileSanitizer>> sanitizers_;
    mutable std::mutex stats_mutex_;
    std::map<std::string, size_t> sanitizer_usage_stats_;
    
    /**
     * @brief Find appropriate sanitizer for file type.
     * @param fileType Type of file to sanitize
     * @return Pointer to sanitizer or nullptr if none available
     */
    FileSanitizer* findSanitizerForType(FileType fileType) const;
    
    /**
     * @brief Update usage statistics.
     * @param sanitizerName Name of sanitizer used
     */    void updateStats(const std::string& sanitizerName);
};

// Helper functions for file content detection
bool detectPdfJavaScript(const std::string& filePath);
bool detectOfficeMacros(const std::string& filePath);
bool sanitizePdfJavaScript(const std::string& inputPath, const std::string& outputPath);

} // namespace CDR

#endif // CDRSANITIZER_H
