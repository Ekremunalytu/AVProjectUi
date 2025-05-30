#ifndef CDRSANITIZER_H
#define CDRSANITIZER_H

#include "CdrTypes.h" // Include CdrTypes.h for SanitizationResult and CdrConfiguration
#include "FileSanitizer.h" // Include FileSanitizer base class
#include <string>
#include <vector>
#include <filesystem>

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
    SanitizationResult sanitizeFile(const std::string& inputPath, 
                                  const std::string& outputPath, 
                                  const CdrConfiguration& config,
                                  FileType fileType); // Added fileType parameter

    // File-specific sanitization methods
    SanitizationResult sanitizeOfficeFile(const std::string& inputPath, const std::string& outputPath, const CdrConfiguration& config);
    SanitizationResult sanitizePdfFile(const std::string& inputPath, const std::string& outputPath, const CdrConfiguration& config);
    SanitizationResult sanitizeHtmlFile(const std::string& inputPath, const std::string& outputPath, const CdrConfiguration& config);
    SanitizationResult sanitizeArchiveFile(const std::string& inputPath, const std::string& outputPath, const CdrConfiguration& config);
    SanitizationResult sanitizeScriptFile(const std::string& inputPath, const std::string& outputPath, const CdrConfiguration& config);
    SanitizationResult sanitizeImageFile(const std::string& inputPath, const std::string& outputPath, const CdrConfiguration& config);

private:
    std::vector<std::unique_ptr<FileSanitizer>> sanitizers_;
    // Potentially add other members like statistics, logging, etc.
    // SanitizationStats stats_; // Example if you add stats
};

} // namespace CDR

#endif // CDRSANITIZER_H
