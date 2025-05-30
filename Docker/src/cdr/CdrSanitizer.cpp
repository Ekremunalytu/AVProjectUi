#include "cdr/CdrSanitizer.h"
#include "cdr/CdrTypes.h"
#include "cdr/FileSanitizer.h"
#include "cdr/FilesystemCompat.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <cstdio>
#include <ctime>

// Use CDR filesystem compatibility layer
namespace fs = CDR::FileSystem;

namespace CDR {

// === Helper: File Type Detection (Internal to this CPP) ===
FileType detectFileTypeInternal(const std::string& filePath) {
    try {
        fs::Path pathObj(filePath); // Renamed to avoid conflict with function parameter
        if (!fs::exists(pathObj)) {
            // Optional: Log or handle non-existent file before trying to get extension
            // std::cerr << "detectFileTypeInternal: File does not exist - " << filePath << std::endl;
            return FileType::UNKNOWN_FILE; // Or some other appropriate default/error type
        }
        std::string extension = pathObj.extension();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

        if (!extension.empty() && extension[0] == '.') {
            extension = extension.substr(1);
        }

        if (extension == "docx" || extension == "xlsx" || extension == "pptx" ||
            extension == "docm" || extension == "xlsm" || extension == "pptm" ||
            extension == "doc" || extension == "xls" || extension == "ppt") {
            return FileType::OFFICE_DOCUMENT;
        }
        if (extension == "html" || extension == "htm") {
            return FileType::HTML_DOCUMENT;
        }
        if (extension == "pdf") {
            return FileType::PDF_DOCUMENT;
        }
        if (extension == "exe" || extension == "dll" || extension == "com" ||
            extension == "bat" || extension == "cmd" || extension == "msi" || extension == "app" || extension == "dmg") {
            return FileType::EXECUTABLE_FILE;
        }
        if (extension == "js" || extension == "vbs" || extension == "ps1" ||
            extension == "sh" || extension == "py" || extension == "rb" || extension == "pl") {
            return FileType::SCRIPT_FILE;
        }
        if (extension == "zip" || extension == "rar" || extension == "7z" ||
            extension == "tar" || extension == "gz" || extension == "bz2") {
            return FileType::ARCHIVE_FILE;
        }
        if (extension == "jpg" || extension == "jpeg" || extension == "png" ||
            extension == "gif" || extension == "bmp" || extension == "tiff" || extension == "svg" || extension == "webp") {
            return FileType::IMAGE_FILE;
        }
        if (extension == "xml") {
            return FileType::XML_DOCUMENT;
        }
        if (extension == "rtf") {
            return FileType::RTF_DOCUMENT;
        }
        if (extension == "txt" || extension == "csv" || extension == "log") {
            return FileType::TEXT_DOCUMENT;
        }
         if (extension == "eml" || extension == "msg") {
            return FileType::EMAIL_FILE;
        }

        return FileType::UNKNOWN_FILE;
    } catch (const fs::FilesystemError& fs_err) {
        std::cerr << "Filesystem error in detectFileTypeInternal for " << filePath << ": " << fs_err.what() << std::endl;
        return FileType::UNKNOWN_FILE;
    }
     catch (const std::exception& e) {
        std::cerr << "Generic error in detectFileTypeInternal for " << filePath << ": " << e.what() << std::endl;
        return FileType::UNKNOWN_FILE;
    }
}

// === Utility function to convert FileType enum to string ===
std::string getFileTypeName(FileType fileType) {
    switch (fileType) {
        case FileType::NOT_SET:
            return "Not Set";
        case FileType::OFFICE_DOCUMENT:
            return "Office Document";
        case FileType::PDF_DOCUMENT:
            return "PDF Document";
        case FileType::HTML_DOCUMENT:
            return "HTML Document";
        case FileType::XML_DOCUMENT:
            return "XML Document";
        case FileType::RTF_DOCUMENT:
            return "RTF Document";
        case FileType::TEXT_DOCUMENT:
            return "Text Document";
        case FileType::ARCHIVE_FILE:
            return "Archive File";
        case FileType::IMAGE_FILE:
            return "Image File";
        case FileType::EXECUTABLE_FILE:
            return "Executable File";
        case FileType::SCRIPT_FILE:
            return "Script File";
        case FileType::EMAIL_FILE:
            return "Email File";
        case FileType::UNKNOWN_FILE:
            return "Unknown File";
        default:
            return "Unknown Type";
    }
}

// === FileSanitizer Base Class Protected Method Implementations ===

std::string FileSanitizer::calculateMD5(const std::string& filePath) {
    // Placeholder for MD5 calculation if needed locally in the future.
    // For now, as OpenSSL is removed, this function will return a dummy hash
    // or can be adapted to use a different cross-platform library if hashing is critical.
    // If hashing is not strictly required for the current project scope, this can be simplified.
    return "dummy-md5-hash-for-" + filePath; // Placeholder
}

bool FileSanitizer::copyFile(const std::string& src, const std::string& dst) {
    try {
        if (!fs::exists(src)) {
            std::cerr << "copyFile: Source file " << src << " does not exist." << std::endl;
            return false;
        }
        fs::Path dstPathObj(dst);
        if (!dstPathObj.parent_path().empty() && !fs::exists(dstPathObj.parent_path())) {
            fs::create_directories(dstPathObj.parent_path());
        }
        fs::copy_file(src, dstPathObj.string());
        return true;
    } catch (const fs::FilesystemError& e) {
        std::cerr << "Failed to copy file from " << src << " to " << dst << ": " << e.what() << std::endl;
        return false;
    }
}

std::string FileSanitizer::readFileContent(const std::string& filePath) {
    try {
        if (!fs::exists(filePath)) {
             std::cerr << "readFileContent: File " << filePath << " does not exist." << std::endl;
            return "";
        }
        auto fileSize = fs::file_size(filePath);
        constexpr size_t MAX_FILE_SIZE_READ = 50 * 1024 * 1024;
        if (fileSize > MAX_FILE_SIZE_READ) {
            std::cerr << "File too large for content reading: " << filePath << ", size: " << fileSize << " bytes" << std::endl;
            return "";
        }

        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading: " << filePath << std::endl;
            return "";
        }
        std::streamsize stream_size = file.tellg(); // Use std::streamsize
        file.seekg(0, std::ios::beg);

        std::string content(static_cast<size_t>(stream_size), '\0'); // Cast to size_t for string constructor
        if (file.read(&content[0], stream_size)) {
            return content;
        }
        return "";
    } catch (const fs::FilesystemError& e) {
        std::cerr << "Filesystem error reading file content from " << filePath << ": " << e.what() << std::endl;
        return "";
    }
     catch (const std::exception& e) {
        std::cerr << "Error reading file content from " << filePath << ": " << e.what() << std::endl;
        return "";
    }
}

bool FileSanitizer::writeFileContent(const std::string& filePath, const std::string& content) {
    try {
        fs::Path filePathObj(filePath);
        if (!filePathObj.parent_path().empty() && !fs::exists(filePathObj.parent_path())) {
            fs::create_directories(filePathObj.parent_path());
        }
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filePath << std::endl;
            return false;
        }
        file << content;
        return file.good();
    } catch (const fs::FilesystemError& e) {
        std::cerr << "Filesystem exception while writing file " << filePath << ": " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception while writing file " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

long long FileSanitizer::getFileSize(const std::string& filePath) {
    try {
        if (!fs::exists(filePath)) return 0;
        return static_cast<long long>(fs::file_size(filePath));
    } catch (const fs::FilesystemError& e) {
        std::cerr << "Error getting file size for " << filePath << ": " << e.what() << std::endl;
        return 0;
    }
}

// === OfficeSanitizer Implementation ===
SanitizationResult OfficeSanitizer::sanitize(const std::string& inputPath,
                                           const std::string& outputPath,
                                           const CdrConfiguration& config) {
    SanitizationResult result;
    result.inputPath = inputPath;
    result.outputPath = outputPath;
    result.originalPath = inputPath;
    result.sanitizedPath = outputPath;
    result.originalSize = FileSanitizer::getFileSize(inputPath);
    result.fileType = FileType::OFFICE_DOCUMENT;

    if (config.blockOfficeMacros && containsMacros(inputPath)) {
        result.success = false;
        result.errorMessage = "Office file blocked due to macro policy.";
        result.threatsDetected.push_back("MACROS_PRESENT");
        result.requiresQuarantine = true;
        result.quarantineReason = "Contains macros, blocked by policy.";
        return result;
    }

    if (copyFile(inputPath, outputPath)) {
        result.success = true;
        result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
        result.md5Hash = FileSanitizer::calculateMD5(outputPath);
        result.actionsPerformed.push_back("COPIED_AS_IS_PENDING_FULL_SANITIZATION");
    } else {
        result.success = false;
        result.errorMessage = "Failed to copy Office document during sanitization.";
    }
    return result;
}

bool OfficeSanitizer::canHandle(FileType type) const {
    return type == FileType::OFFICE_DOCUMENT;
}

std::vector<std::string> OfficeSanitizer::getDetectableThreats() const {
    return {"MACROS", "EMBEDDED_OBJECTS", "EXTERNAL_LINKS", "MALFORMED_XML"};
}

bool OfficeSanitizer::containsMacros(const std::string& /*zipPath*/) {
    return false;
}
// Stubs for other private OfficeSanitizer methods from header
bool OfficeSanitizer::removeMacros(const std::string&) {return false;} // TASLAK
bool OfficeSanitizer::removeExternalLinks(const std::string&) {return false;} // TASLAK
bool OfficeSanitizer::removeEmbeddedObjects(const std::string& /*zipPath*/) { return false; }
bool OfficeSanitizer::sanitizeXMLContent(const std::string& /*xmlContent*/, std::string& /*sanitized*/) { return false; }
std::vector<std::string> OfficeSanitizer::extractZipFiles(const std::string& /*zipPath*/, const std::string& /*tempDir*/) { return {}; }
bool OfficeSanitizer::recreateZipFile(const std::vector<std::string>& /*files*/, const std::string& /*outputPath*/) { return false; }
std::string OfficeSanitizer::removeHyperlinksFromXml(const std::string& xmlContent) { return xmlContent; }
std::string OfficeSanitizer::removeOleObjectsFromXml(const std::string& xmlContent) { return xmlContent; }


// === PdfSanitizer Implementation ===
SanitizationResult PdfSanitizer::sanitize(const std::string& inputPath,
                                        const std::string& outputPath,
                                        const CdrConfiguration& config) {
    SanitizationResult result;
    result.inputPath = inputPath;
    result.outputPath = outputPath;
    result.originalPath = inputPath;
    result.sanitizedPath = outputPath;
    result.originalSize = FileSanitizer::getFileSize(inputPath);
    result.fileType = FileType::PDF_DOCUMENT;

    std::string contentStart = readFileContent(inputPath);
    if (contentStart.length() >= 4 && contentStart.substr(0, 4) == "%PDF") {
        if (config.blockPdfScripts && containsJavaScript(inputPath)) {
             result.success = false;
             result.errorMessage = "PDF blocked due to JavaScript content policy.";
             result.threatsDetected.push_back("JAVASCRIPT_IN_PDF");
             result.requiresQuarantine = true;
             result.quarantineReason = "Contains JavaScript, blocked by policy.";
             return result;
        }

        if (copyFile(inputPath, outputPath)) {
            result.success = true;
            result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
            result.md5Hash = FileSanitizer::calculateMD5(outputPath);
            result.actionsPerformed.push_back("VALIDATED_PDF_HEADER_AND_COPIED");
        } else {
            result.success = false;
            result.errorMessage = "Failed to copy PDF document.";
        }
    } else {
        result.success = false;
        result.errorMessage = "Invalid PDF format (header mismatch).";
        result.threatsDetected.push_back("INVALID_PDF_FORMAT");
    }
    return result;
}

bool PdfSanitizer::canHandle(FileType type) const {
    return type == FileType::PDF_DOCUMENT;
}

std::vector<std::string> PdfSanitizer::getDetectableThreats() const {
    return {"JAVASCRIPT", "EMBEDDED_FILES", "MALICIOUS_ACTIONS", "ENCRYPTED_PAYLOADS"};
}
bool PdfSanitizer::containsJavaScript(const std::string& /*pdfPath*/) {
    return false;
}
// Stubs for other private PdfSanitizer methods from header
bool PdfSanitizer::removeJavaScript(const std::string& /*pdfPath*/, const std::string& /*outputPath*/) { return false; }
bool PdfSanitizer::removeForms(const std::string& /*pdfPath*/, const std::string& /*outputPath*/) { return false; }
bool PdfSanitizer::removeEmbeddedFiles(const std::string& /*pdfPath*/, const std::string& /*outputPath*/) { return false; }
bool PdfSanitizer::removeAnnotations(const std::string& /*pdfPath*/, const std::string& /*outputPath*/) { return false; }
bool PdfSanitizer::hasSuspiciousStructure(const std::string& /*pdfPath*/) { return false; }


// === HtmlSanitizer Implementation ===
SanitizationResult HtmlSanitizer::sanitize(const std::string& inputPath,
                                         const std::string& outputPath,
                                         const CdrConfiguration& config) {
    SanitizationResult result;
    result.inputPath = inputPath;
    result.outputPath = outputPath;
    result.originalSize = FileSanitizer::getFileSize(inputPath);
    result.fileType = FileType::HTML_DOCUMENT;

    std::string content = readFileContent(inputPath);
    if (content.empty() && result.originalSize > 0) {
        result.success = false;
        result.errorMessage = "Failed to read HTML file content.";
        return result;
    }

    std::string sanitizedContent = content;
    bool modified = false;

    sanitizedContent = removeScripts(sanitizedContent);
    if (sanitizedContent != content) { // A bit simplistic, better to check specific regex matches
        result.actionsPerformed.push_back("REMOVED_SCRIPT_TAGS");
        result.threatsDetected.push_back("JAVASCRIPT_CODE_IN_SCRIPT_TAGS");
        modified = true;
    }
    std::string prevContent = sanitizedContent;

    sanitizedContent = removeEventHandlers(sanitizedContent);
    if (sanitizedContent != prevContent) {
        result.actionsPerformed.push_back("REMOVED_EVENT_HANDLERS");
        result.threatsDetected.push_back("JAVASCRIPT_IN_EVENT_HANDLERS");
        modified = true;
        prevContent = sanitizedContent;
    }
    
    std::regex jsUrlRegex(R"((?:href|src|data|action)\s*=\s*["']?\s*javascript:[^"'>\s]+["']?)", std::regex_constants::icase | std::regex_constants::ECMAScript);
    std::string tempContent = std::regex_replace(sanitizedContent, jsUrlRegex, "href=\"#blocked\"");
    if (tempContent != sanitizedContent) {
        sanitizedContent = tempContent;
        result.actionsPerformed.push_back("NEUTRALIZED_JAVASCRIPT_URLS");
        result.threatsDetected.push_back("JAVASCRIPT_URLS");
        modified = true;
        prevContent = sanitizedContent;
    }

    if (config.securityLevel >= CdrConfiguration::SecurityLevel::MEDIUM) {
        sanitizedContent = removeIframes(sanitizedContent);
         if (sanitizedContent != prevContent) {
            result.actionsPerformed.push_back("REMOVED_IFRAMES");
            result.threatsDetected.push_back("IFRAME_CONTENT");
            modified = true;
            prevContent = sanitizedContent;
        }
    }
    
    if (config.securityLevel >= CdrConfiguration::SecurityLevel::HIGH) {
        std::regex objectEmbedRegex(R"(<(?:object|embed)\b[^<]*(?:(?!<\/(?:object|embed)>)<[^<]*)*<\/(?:object|embed)>)", std::regex_constants::icase | std::regex_constants::ECMAScript);
        tempContent = std::regex_replace(sanitizedContent, objectEmbedRegex, "<!-- object/embed removed -->");
        if (tempContent != sanitizedContent) {
            sanitizedContent = tempContent;
            result.actionsPerformed.push_back("REMOVED_OBJECT_EMBED_TAGS");
            result.threatsDetected.push_back("EMBEDDED_OBJECTS");
            modified = true;
            // prevContent = sanitizedContent; // Not strictly needed for last operation
        }
    }

    if (writeFileContent(outputPath, sanitizedContent)) {
        result.success = true;
        result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
        result.md5Hash = FileSanitizer::calculateMD5(outputPath);
        if (!modified && result.originalSize > 0) {
             result.actionsPerformed.push_back("PASSED_AS_IS_NO_THREATS_FOUND");
        } else if (!modified && result.originalSize == 0) {
            result.actionsPerformed.push_back("EMPTY_HTML_PROCESSED");
        }
    } else {
        result.success = false;
        result.errorMessage = "Failed to write sanitized HTML content.";
    }
    return result;
}

bool HtmlSanitizer::canHandle(FileType type) const {
    return type == FileType::HTML_DOCUMENT;
}

std::vector<std::string> HtmlSanitizer::getDetectableThreats() const {
    return {"JAVASCRIPT_CODE", "IFRAMES", "MALICIOUS_CSS", "EVENT_HANDLERS", "DANGEROUS_TAGS"};
}
// Implementations for HtmlSanitizer private methods
std::string HtmlSanitizer::removeScripts(const std::string& htmlContent) {
    std::regex scriptRegex(R"(<script\b[^<]*(?:(?!<\/script>)<[^<]*)*<\/script>)", std::regex_constants::icase | std::regex_constants::ECMAScript);
    return std::regex_replace(htmlContent, scriptRegex, "<!-- script removed -->");
}
std::string HtmlSanitizer::removeIframes(const std::string& htmlContent) {
    std::regex iframeRegex(R"(<iframe\b[^<]*(?:(?!<\/iframe>)<[^<]*)*<\/iframe>)", std::regex_constants::icase | std::regex_constants::ECMAScript);
    return std::regex_replace(htmlContent, iframeRegex, "<!-- iframe removed -->");
}
std::string HtmlSanitizer::sanitizeCSS(const std::string& htmlContent) { 
    // Basic CSS sanitization - this is complex, placeholder for now
    // Could remove known dangerous properties or use a CSS parser/sanitizer library
    std::regex styleRegex(R"(<style\b[^<]*(?:(?!<\/style>)<[^<]*)*<\/style>)", std::regex_constants::icase | std::regex_constants::ECMAScript);
    // Example: remove all <style> tags for simplicity, or implement more granular logic
    // return std::regex_replace(htmlContent, styleRegex, "<!-- style block removed for safety -->");
    return htmlContent; // Placeholder: no change
}
std::string HtmlSanitizer::removeEventHandlers(const std::string& htmlContent) {
    std::regex eventHandlerRegex(R"(\s+on\w+\s*=\s*(?:"[^"]*"|'[^']*'|[^\s>]+))", std::regex_constants::icase | std::regex_constants::ECMAScript);
    return std::regex_replace(htmlContent, eventHandlerRegex, "");
}
std::string HtmlSanitizer::removeDangerousTags(const std::string& htmlContent) {
    // Example: remove <applet>, <embed>, <object> if not handled by higher security levels
    // std::regex dangerousTagRegex(R"(<(?:applet|object|embed)\b[^>]*>.*?<\/(?:applet|object|embed)>)", std::regex_constants::icase | std::regex_constants::ECMAScript);
    // return std::regex_replace(htmlContent, dangerousTagRegex, "<!-- dangerous tag removed -->");
    return htmlContent; // Placeholder
}
bool HtmlSanitizer::containsSuspiciousContent(const std::string& /*htmlContent*/) {
    // Placeholder for more advanced heuristic checks
    return false;
}


// === ScriptAnalyzer Implementation ===
SanitizationResult ScriptAnalyzer::sanitize(const std::string& inputPath,
                                          const std::string& outputPath,
                                          const CdrConfiguration& config) {
    SanitizationResult result;
    result.inputPath = inputPath;
    result.outputPath = outputPath; 
    result.originalSize = FileSanitizer::getFileSize(inputPath);
    result.fileType = FileType::SCRIPT_FILE;

    if (config.securityLevel >= CdrConfiguration::SecurityLevel::STRICT) { 
        result.success = false;
        result.errorMessage = "Script file quarantined due to strict security policy.";
        result.threatsDetected.push_back("SCRIPT_FILE_TYPE");
        result.actionsPerformed.push_back("QUARANTINED");
        result.requiresQuarantine = true;
        result.quarantineReason = "Script file type blocked by strict policy.";
        return result;
    }
    
    if (config.blockAllScripts) {
        result.success = false;
        result.errorMessage = "Script file blocked by configuration.";
        result.threatsDetected.push_back("SCRIPT_FILE_TYPE");
        result.actionsPerformed.push_back("BLOCKED_BY_POLICY");
        result.requiresQuarantine = true;
        result.quarantineReason = "All scripts are blocked by current policy.";
        return result;
    }

    if (copyFile(inputPath, outputPath)) {
        result.success = true;
        result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
        result.md5Hash = FileSanitizer::calculateMD5(outputPath);
        result.actionsPerformed.push_back("COPIED_AS_IS_SCRIPT_ALLOWED");
        result.sanitizationDetails = "Script file allowed and copied. No content modification performed by default.";
    } else {
        result.success = false;
        result.errorMessage = "Failed to copy script file.";
    }
    return result;
}

bool ScriptAnalyzer::canHandle(FileType type) const {
    return type == FileType::SCRIPT_FILE;
}

std::vector<std::string> ScriptAnalyzer::getDetectableThreats() const {
    return {"OBFUSCATION", "SUSPICIOUS_API_CALLS", "KNOWN_MALWARE_SIGNATURES"};
}
// Stubs for ScriptAnalyzer private methods
bool ScriptAnalyzer::analyzeJavaScript(const std::string& /*content*/, std::vector<std::string>& /*threats*/) { return false; }
bool ScriptAnalyzer::analyzePowerShell(const std::string& /*content*/, std::vector<std::string>& /*threats*/) { return false; }
bool ScriptAnalyzer::analyzeVBScript(const std::string& /*content*/, std::vector<std::string>& /*threats*/) { return false; }
bool ScriptAnalyzer::analyzeBatchScript(const std::string& /*content*/, std::vector<std::string>& /*threats*/) { return false; }
bool ScriptAnalyzer::containsObfuscation(const std::string& /*content*/) { return false; }
bool ScriptAnalyzer::containsSuspiciousAPIs(const std::string& /*content*/) { return false; }


// === ArchiveSanitizer Implementation ===
SanitizationResult ArchiveSanitizer::sanitize(const std::string& inputPath,
                                            const std::string& outputPath,
                                            const CdrConfiguration& config) {
    SanitizationResult result;
    result.inputPath = inputPath;
    result.outputPath = outputPath;
    result.originalSize = FileSanitizer::getFileSize(inputPath);
    result.fileType = FileType::ARCHIVE_FILE;

    if (config.blockArchives) {
        result.success = false;
        result.errorMessage = "Archive file blocked by policy.";
        result.threatsDetected.push_back("ARCHIVE_FILE_TYPE");
        result.actionsPerformed.push_back("BLOCKED_BY_POLICY");
        result.requiresQuarantine = true;
        result.quarantineReason = "Archives are blocked by current policy.";
        return result;
    }
    
    if (copyFile(inputPath, outputPath)) {
        result.success = true;
        result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
        result.md5Hash = FileSanitizer::calculateMD5(outputPath);
        result.actionsPerformed.push_back("COPIED_AS_IS_PENDING_FULL_ARCHIVE_SANITIZATION");
        result.sanitizationDetails = "Archive copied. Deep content sanitization not yet implemented.";
    } else {
        result.success = false;
        result.errorMessage = "Failed to copy archive file.";
    }
    return result;
}

bool ArchiveSanitizer::canHandle(FileType type) const {
    return type == FileType::ARCHIVE_FILE;
}

std::vector<std::string> ArchiveSanitizer::getDetectableThreats() const {
    return {"NESTED_ARCHIVES", "EXECUTABLES_IN_ARCHIVE", "PASSWORD_PROTECTED_ARCHIVE", "MALICIOUS_FILES_IN_ARCHIVE"};
}
// Stubs for ArchiveSanitizer private methods
bool ArchiveSanitizer::extractAndScanArchive(const std::string& /*archivePath*/, const std::string& /*tempDir*/) { return false; }
bool ArchiveSanitizer::sanitizeExtractedFiles(const std::string& /*tempDir*/, const CdrConfiguration& /*config*/) { return false; }
bool ArchiveSanitizer::recreateCleanArchive(const std::string& /*tempDir*/, const std::string& /*outputPath*/) { return false; }
bool ArchiveSanitizer::hasNestedArchives(const std::string& /*archivePath*/) { return false; }
bool ArchiveSanitizer::containsExecutables(const std::string& /*archivePath*/) { return false; }


// === ImageSanitizer Implementation ===
SanitizationResult ImageSanitizer::sanitize(const std::string& inputPath,
                                          const std::string& outputPath,
                                          const CdrConfiguration& config) {
    SanitizationResult result;
    result.inputPath = inputPath;
    result.outputPath = outputPath;
    result.originalSize = FileSanitizer::getFileSize(inputPath);
    result.fileType = FileType::IMAGE_FILE;

    if (config.blockImagesWithMetadata && containsExcessiveMetadata(inputPath)) {
        result.success = false;
        result.errorMessage = "Image blocked due to metadata policy.";
        result.threatsDetected.push_back("EXCESSIVE_METADATA");
        result.requiresQuarantine = true;
        result.quarantineReason = "Image contains metadata blocked by policy.";
        return result;
    }

    if (copyFile(inputPath, outputPath)) {
        result.success = true;
        result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
        result.md5Hash = FileSanitizer::calculateMD5(outputPath);
        result.actionsPerformed.push_back("COPIED_AS_IS_PENDING_METADATA_STRIPPING");
    } else {
        result.success = false;
        result.errorMessage = "Failed to copy image file.";
    }
    return result;
}

bool ImageSanitizer::canHandle(FileType type) const {
    return type == FileType::IMAGE_FILE;
}

std::vector<std::string> ImageSanitizer::getDetectableThreats() const {
    return {"EXIF_METADATA", "EMBEDDED_SCRIPTS_IN_SVG", "STEGANOGRAPHY_PATTERNS", "MALFORMED_IMAGE_DATA"};
}

bool ImageSanitizer::containsExcessiveMetadata(const std::string& /*filePath*/) {
    return false;
}


// === CdrSanitizer (Manager Class) Implementation ===
CdrSanitizer::CdrSanitizer() {
    registerSanitizer(std::make_unique<OfficeSanitizer>());
    registerSanitizer(std::make_unique<PdfSanitizer>());
    registerSanitizer(std::make_unique<HtmlSanitizer>());
    registerSanitizer(std::make_unique<ScriptAnalyzer>());
    registerSanitizer(std::make_unique<ArchiveSanitizer>());
    registerSanitizer(std::make_unique<ImageSanitizer>()); 
}

CdrSanitizer::~CdrSanitizer() = default; 

void CdrSanitizer::registerSanitizer(std::unique_ptr<FileSanitizer> sanitizer) {
    if (sanitizer) {
        sanitizers_.push_back(std::move(sanitizer));
    }
}

SanitizationResult CdrSanitizer::sanitizeFile(const std::string& inputPath, 
                                            const std::string& outputPath, 
                                            const CdrConfiguration& config,
                                            FileType fileType) { // Added fileType parameter
    SanitizationResult result;
    result.inputPath = inputPath;
    result.outputPath = outputPath;
    result.originalPath = inputPath; // Default to inputPath
    result.sanitizedPath = outputPath; // Default to outputPath
    result.fileType = fileType;
    // result.originalSize = 0; // Initialize, will be set if file exists
    // result.sanitizedSize = 0; // Initialize
    // result.success = false; // Default to false

    // Basic file existence check
    if (!fs::exists(inputPath)) {
        result.success = false;
        result.errorMessage = "Input file does not exist: " + inputPath;
        return result;
    }
    result.originalSize = FileSanitizer::getFileSize(inputPath); // Set original size here

    // Placeholder for actual sanitization logic based on fileType
    // For now, find the appropriate sanitizer and call its sanitize method.
    bool handled = false;
    for (const auto& sanitizer : sanitizers_) {
        if (sanitizer->canHandle(fileType)) {
            // Call the specific sanitizer's method.
            // This assumes the individual sanitizers (OfficeSanitizer, PdfSanitizer, etc.)
            // have a public 'sanitize' method that takes inputPath, outputPath, and config.
            // The individual sanitize methods should populate the result structure.
            SanitizationResult specificResult = sanitizer->sanitize(inputPath, outputPath, config);
            // Copy over the specific result. Be careful about overwriting common fields if they were already set.
            result = specificResult; // This will overwrite fields like inputPath, outputPath, etc.
                                     // Ensure specific sanitizers correctly populate all necessary fields.
            // Ensure common fields are preserved or correctly set by specific sanitizers
            result.inputPath = inputPath; 
            result.outputPath = outputPath;
            result.originalPath = inputPath;
            result.fileType = fileType; // Ensure fileType is correctly set
            if (fs::exists(inputPath)) { // Re-check existence for originalSize
                 result.originalSize = FileSanitizer::getFileSize(inputPath);
            }
            if (result.success && fs::exists(result.sanitizedPath)) {
                 result.sanitizedSize = FileSanitizer::getFileSize(result.sanitizedPath);
                 result.md5Hash = FileSanitizer::calculateMD5(result.sanitizedPath); // Use static call
            } else if (result.success && result.sanitizedPath.empty() && fs::exists(inputPath)) {
                // Case: "copied as is" or no modification, output might be same as input
                // or sanitizedPath might not be explicitly set by a simple sanitizer.
                // If outputPath was different, this might need adjustment.
                // For now, if sanitizedPath is empty but success is true, assume input is the 'output'.
                result.sanitizedPath = inputPath;
                result.sanitizedSize = result.originalSize;
                result.md5Hash = FileSanitizer::calculateMD5(inputPath);
            }


            handled = true;
            break;
        }
    }

    if (!handled) {
        // If no specific sanitizer handles this file type, decide on default behavior.
        // For example, copy if allowed by config, or mark as unsupported.
        if (config.allowUnknownTypes) {
            std::cout << "No specific sanitizer for " << getFileTypeName(fileType) << ", copying file: " << inputPath << std::endl;
            try {
                if (inputPath != outputPath) {
                     fs::copy(inputPath, outputPath);
                }
                result.sanitizedPath = outputPath;
                result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
                result.md5Hash = FileSanitizer::calculateMD5(outputPath); // Use static call
                result.success = true;
                result.actionsPerformed.push_back("File copied (unknown type, allowed by policy)");
                result.sanitizationDetails = "File type is unknown but allowed; copied without modification.";
            } catch (const fs::FilesystemError& fs_err) {
                result.success = false;
                result.errorMessage = "Filesystem error copying unknown file type: " + std::string(fs_err.what());
            }
        } else {
            result.success = false;
            result.errorMessage = "Unsupported file type for sanitization: " + getFileTypeName(fileType);
            result.actionsPerformed.push_back("BLOCKED_UNSUPPORTED_TYPE");
            result.requiresQuarantine = true; // Potentially quarantine unsupported types
            result.quarantineReason = "Unsupported file type and not allowed by policy.";
        }
    }
    
    // Simulate threat detection and quarantine for specific types if needed for testing (example from before)
    // This logic should ideally be within the specific sanitizers or based on their results.
    if (fileType == FileType::SCRIPT_FILE && config.securityLevel >= CdrConfiguration::SecurityLevel::HIGH && !result.requiresQuarantine) {
        // This is an example override or additional check.
        // If the ScriptAnalyzer already decided on quarantine, this might be redundant or conflicting.
        // For now, let's assume this is an additional policy layer.
        // However, it's better if ScriptAnalyzer itself handles this based on config.securityLevel.
        // result.threatsDetected.push_back("POTENTIAL_SCRIPT_EXECUTION_HIGH_SECURITY"); // More specific
        // result.requiresQuarantine = true;
        // result.quarantineReason = "Script file detected at high security level, policy dictates quarantine.";
        // result.success = true; // Quarantine can be a 'successful' outcome of a policy.
    }

    return result;
}

} // namespace CDR
