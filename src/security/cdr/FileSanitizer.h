#ifndef FILESANITIZER_H
#define FILESANITIZER_H

#include "CdrTypes.h"
#include "FilesystemCompat.h"
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>

// Use CDR filesystem compatibility layer
namespace fs = CDR::FileSystem;

namespace CDR {

/**
 * @brief Abstract base class for file sanitizers.
 * 
 * This class provides the interface that all file type-specific sanitizers must implement.
 * It also provides common utility methods for file operations.
 */
class FileSanitizer {
public:
    virtual ~FileSanitizer() = default;

    /**
     * @brief Sanitize a file according to the given configuration.
     * @param inputPath Path to the input file
     * @param outputPath Path where the sanitized file should be written
     * @param config Configuration settings for sanitization
     * @return SanitizationResult containing the results of the operation
     */
    virtual SanitizationResult sanitize(const std::string& inputPath,
                                       const std::string& outputPath,
                                       const CdrConfiguration& config) = 0;

    /**
     * @brief Check if this sanitizer can handle the given file type.
     * @param type The file type to check
     * @return true if this sanitizer can handle the file type, false otherwise
     */
    virtual bool canHandle(FileType type) const = 0;

    /**
     * @brief Get a list of threats this sanitizer can detect.
     * @return Vector of threat names this sanitizer can detect
     */
    virtual std::vector<std::string> getDetectableThreats() const = 0;

    /**
     * @brief Validate file before sanitization.
     * @param filePath Path to the file to validate
     * @param config Configuration settings
     * @return true if file is valid for sanitization, false otherwise
     */
    virtual bool validateFile(const std::string& filePath, const CdrConfiguration& config) const {
        // Default implementation - can be overridden by derived classes
        if (filePath.empty() || !fs::exists(filePath)) return false;
        
        try {
            auto fileSize = fs::file_size(filePath);
            if (fileSize > static_cast<size_t>(config.maxFileSizeMB * 1024 * 1024)) return false;
            if (fileSize == 0) return false; // Empty files
            
            // Check file readability
            std::ifstream file(filePath, std::ios::binary);
            return file.good();
        } catch (...) {
            return false;
        }
    }

    // Public utility methods
    static std::string calculateMD5(const std::string& filePath);
    static long long getFileSize(const std::string& filePath);

protected:
    // Utility methods for derived classes
    static bool copyFile(const std::string& src, const std::string& dst);
    static std::string readFileContent(const std::string& filePath);
    static bool writeFileContent(const std::string& filePath, const std::string& content);
    
    /**
     * @brief Validate output path and create directories if needed.
     * @param outputPath Path where output should be written
     * @return true if output path is valid and accessible, false otherwise
     */
    static bool validateOutputPath(const std::string& outputPath) {
        if (outputPath.empty()) return false;
        
        try {
            fs::Path pathObj(outputPath);
            if (!pathObj.parent_path().empty() && !fs::exists(pathObj.parent_path())) {
                fs::create_directories(pathObj.parent_path());
            }
            
            // Test write permissions by creating a temporary file
            std::string testFile = outputPath + ".tmp_test";
            std::ofstream test(testFile);
            bool canWrite = test.good();
            test.close();
            
            if (canWrite) {
                fs::remove(testFile); // Clean up test file
            }
            
            return canWrite;
        } catch (...) {
            return false;
        }
    }
};

/**
 * @brief Sanitizer for Microsoft Office documents (Word, Excel, PowerPoint).
 * 
 * Handles detection and removal of macros, external links, embedded objects,
 * and other potentially dangerous content in Office documents.
 */
class OfficeSanitizer : public FileSanitizer {
public:
    SanitizationResult sanitize(const std::string& inputPath,
                               const std::string& outputPath,
                               const CdrConfiguration& config) override;
    
    bool canHandle(FileType type) const override;
    std::vector<std::string> getDetectableThreats() const override;

private:
    bool containsMacros(const std::string& zipPath);
    bool removeMacros(const std::string& zipPath);
    bool removeExternalLinks(const std::string& zipPath);
    bool removeEmbeddedObjects(const std::string& zipPath);
    bool sanitizeXMLContent(const std::string& xmlContent, std::string& sanitized);
    std::vector<std::string> extractZipFiles(const std::string& zipPath, const std::string& tempDir);
    bool recreateZipFile(const std::vector<std::string>& files, const std::string& outputPath);
    std::string removeHyperlinksFromXml(const std::string& xmlContent);
    std::string removeOleObjectsFromXml(const std::string& xmlContent);
};

/**
 * @brief Sanitizer for PDF documents.
 * 
 * Handles detection and removal of JavaScript, forms, embedded files,
 * annotations, and other potentially dangerous content in PDF files.
 */
class PdfSanitizer : public FileSanitizer {
public:
    SanitizationResult sanitize(const std::string& inputPath,
                               const std::string& outputPath,
                               const CdrConfiguration& config) override;
    
    bool canHandle(FileType type) const override;
    std::vector<std::string> getDetectableThreats() const override;

private:
    bool containsJavaScript(const std::string& pdfPath);
    bool removeJavaScript(const std::string& pdfPath, const std::string& outputPath);
    bool removeForms(const std::string& pdfPath, const std::string& outputPath);
    bool removeEmbeddedFiles(const std::string& pdfPath, const std::string& outputPath);
    bool removeAnnotations(const std::string& pdfPath, const std::string& outputPath);
    bool hasSuspiciousStructure(const std::string& pdfPath);
};

/**
 * @brief Sanitizer for HTML documents.
 * 
 * Handles removal of scripts, event handlers, iframes, dangerous CSS,
 * and other potentially malicious content in HTML files.
 */
class HtmlSanitizer : public FileSanitizer {
public:
    SanitizationResult sanitize(const std::string& inputPath,
                               const std::string& outputPath,
                               const CdrConfiguration& config) override;
    
    bool canHandle(FileType type) const override;
    std::vector<std::string> getDetectableThreats() const override;

private:
    std::string removeScripts(const std::string& htmlContent);
    std::string removeIframes(const std::string& htmlContent);
    std::string sanitizeCSS(const std::string& htmlContent);
    std::string removeEventHandlers(const std::string& htmlContent);
    std::string removeDangerousTags(const std::string& htmlContent);
    bool containsSuspiciousContent(const std::string& htmlContent);
};

/**
 * @brief Analyzer for script files (JavaScript, PowerShell, VBScript, etc.).
 * 
 * Performs static analysis to detect obfuscation, suspicious API calls,
 * and known malware signatures in script files.
 */
class ScriptAnalyzer : public FileSanitizer {
public:
    SanitizationResult sanitize(const std::string& inputPath,
                               const std::string& outputPath,
                               const CdrConfiguration& config) override;
    
    bool canHandle(FileType type) const override;
    std::vector<std::string> getDetectableThreats() const override;

private:
    bool analyzeJavaScript(const std::string& content, std::vector<std::string>& threats);
    bool analyzePowerShell(const std::string& content, std::vector<std::string>& threats);
    bool analyzeVBScript(const std::string& content, std::vector<std::string>& threats);
    bool analyzeBatchScript(const std::string& content, std::vector<std::string>& threats);
    bool containsObfuscation(const std::string& content);
    bool containsSuspiciousAPIs(const std::string& content);
};

/**
 * @brief Sanitizer for archive files (ZIP, RAR, 7Z, etc.).
 * 
 * Extracts archives, sanitizes individual files within them,
 * and recreates clean archives without malicious content.
 */
class ArchiveSanitizer : public FileSanitizer {
public:
    SanitizationResult sanitize(const std::string& inputPath,
                               const std::string& outputPath,
                               const CdrConfiguration& config) override;
    
    bool canHandle(FileType type) const override;
    std::vector<std::string> getDetectableThreats() const override;

private:
    bool extractAndScanArchive(const std::string& archivePath, const std::string& tempDir);
    bool sanitizeExtractedFiles(const std::string& tempDir, const CdrConfiguration& config);
    bool recreateCleanArchive(const std::string& tempDir, const std::string& outputPath);
    bool hasNestedArchives(const std::string& archivePath);
    bool containsExecutables(const std::string& archivePath);
};

/**
 * @brief Sanitizer for image files (JPEG, PNG, GIF, SVG, etc.).
 * 
 * Strips metadata (EXIF data), removes embedded scripts from SVG files,
 * and checks for steganography patterns and malformed image data.
 */
class ImageSanitizer : public FileSanitizer {
public:
    SanitizationResult sanitize(const std::string& inputPath,
                               const std::string& outputPath,
                               const CdrConfiguration& config) override;
    
    bool canHandle(FileType type) const override;
    std::vector<std::string> getDetectableThreats() const override;

private:
    bool containsExcessiveMetadata(const std::string& filePath);
    bool stripMetadata(const std::string& inputPath, const std::string& outputPath);
    bool sanitizeSVG(const std::string& svgContent, std::string& sanitized);
    bool checkSteganography(const std::string& filePath);
    bool validateImageStructure(const std::string& filePath);
};

} // namespace CDR

#endif // FILESANITIZER_H
