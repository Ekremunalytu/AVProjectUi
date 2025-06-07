#include "YaraRuleManager.h"
#include <fstream>
#include <cstdio>
#include <iostream>
#include <memory>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDirIterator>

// Include generated configuration if available
#ifdef CMAKE_CURRENT_BINARY_DIR
#include "YaraRulesPaths.h"
#endif

#ifdef _WIN32
#include <windows.h>
#endif

extern "C" {
    #include <yara.h>
}

// Custom error category implementation for YARA errors
namespace {
    class YaraErrorCategory : public std::error_category {
    public:
        const char* name() const noexcept override {
            return "YARA";
        }
        
        std::string message(int ev) const override {
            switch (static_cast<YaraErrorCodes>(ev)) {
                case YaraErrorCodes::success:
                    return "Success";
                case YaraErrorCodes::AlreadyInitialized:
                    return "YARA is already initialized";
                case YaraErrorCodes::NotInitialized:
                    return "YARA is not initialized";
                case YaraErrorCodes::InternalError:
                    return "YARA internal error";
                case YaraErrorCodes::FileNotFound:
                    return "File not found or not accessible";
                case YaraErrorCodes::CompilerError:
                    return "Error compiling YARA rules";
                case YaraErrorCodes::RulesNotCompiled:
                    return "YARA rules are not compiled";
                case YaraErrorCodes::ScanError:
                    return "Error scanning with YARA";
                default:
                    return "Unknown YARA error";
            }
        }
    };
    
    const YaraErrorCategory yaraErrorCategory{};
}

// make_error_code implementation for YaraErrorCodes
std::error_code make_error_code(YaraErrorCodes e) {
    return {static_cast<int>(e), yaraErrorCategory};
}

// Static callback function
static int yara_callback(
    YR_SCAN_CONTEXT* context,
    int message,
    void* message_data,
    void* user_data)
{
    auto* manager = reinterpret_cast<YaraRuleManager*>(user_data);
    // CALLBACK_MSG_RULE_MATCHING, YARA eşleşme mesajı
    if (message == CALLBACK_MSG_RULE_MATCHING && manager && manager->getCallback()) {
        YR_RULE* rule = static_cast<YR_RULE*>(message_data);
        manager->getCallback()(rule->identifier);
    }
    return CALLBACK_CONTINUE;
}

// Constructor & Destructor
YaraRuleManager::YaraRuleManager() : rules(nullptr), compiler(nullptr), initialized(false) {}

YaraRuleManager::~YaraRuleManager() {
    finalize();
}

// Initialize / Finalize
std::error_code YaraRuleManager::initialize() {
    if (initialized)
        return make_error_code(YaraErrorCodes::AlreadyInitialized);

    int result = yr_initialize();
    if (result != ERROR_SUCCESS) {
        qDebug() << "YARA initialization failed with code:" << result;
        return make_error_code(YaraErrorCodes::InternalError);
    }

    initialized = true;
    // Initialize statistics
    m_stats.totalRulesLoaded = 0;
    m_stats.filesScanned = 0;
    m_stats.matchesFound = 0;
    m_stats.averageScanTime = std::chrono::milliseconds{0};
    m_stats.lastRuleUpdate = std::chrono::system_clock::now();
    
    qDebug() << "YARA successfully initialized";
    return make_error_code(YaraErrorCodes::success);
}

std::error_code YaraRuleManager::finalize() {
    if (!initialized)
        return make_error_code(YaraErrorCodes::NotInitialized);

    unloadRules();
    yr_finalize();
    initialized = false;
    return make_error_code(YaraErrorCodes::success);
}

#ifdef _WIN32
// Windows için geniş karakter dosya açma yardımcı fonksiyonu
FILE* win32_fopen(const QString& path, const char* mode) {
    FILE* fp = nullptr;
    std::wstring wPath = path.toStdWString();
    std::wstring wMode;
    
    // mode stringini wstring'e dönüştür
    for (const char* c = mode; *c; ++c) {
        wMode.push_back(static_cast<wchar_t>(*c));
    }
    
    _wfopen_s(&fp, wPath.c_str(), wMode.c_str());
    return fp;
}
#endif

// Basitleştirilmiş YARA kuralı oluşturma fonksiyonu
bool createSimpleYaraRule(const QString& rulePath) {
    // Basit bir YARA kuralı oluşturalım
    const char* simpleRule = 
        "rule simple_test_rule {\n"
        "    meta:\n"
        "        description = \"Simple test rule\"\n"
        "        author = \"Auto-generated\"\n"
        "    strings:\n"
        "        $a = \"test string\" nocase\n"
        "    condition:\n"
        "        $a\n"
        "}\n";
    
    QFile file(rulePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Basit YARA kuralı dosyası oluşturulamadı:" << rulePath;
        return false;
    }
    
    QTextStream out(&file);
    out << simpleRule;
    file.close();
    
    qDebug() << "Basit YARA kuralı oluşturuldu:" << rulePath;
    return true;
}

// Load & Unload Rules - Updated to handle directories
std::error_code YaraRuleManager::loadRules(const std::string& rulesPath) {
    if (!initialized)
        return make_error_code(YaraErrorCodes::NotInitialized);

    unloadRules();

    QString qRulesPath = QString::fromStdString(rulesPath);
    QFileInfo pathInfo(qRulesPath);
    
    // If the provided path doesn't exist, try to find it relative to application directory
    // If the provided path doesn't exist, try to find it relative to application directory
    if (!pathInfo.exists()) {
        QString exePath = QCoreApplication::applicationDirPath();
        
        // Try different relative paths
        QStringList searchPaths = {
            // Source directory relative to project root
            "/Volumes/Crucial/AVProjectUi/src/security/scanning/yara/rules",
            // Relative paths from executable
            exePath + "/" + qRulesPath,
            exePath + "/rules",
            exePath + "/../rules",
            exePath + "/../../src/security/scanning/yara/rules",
            exePath + "/../../../src/security/scanning/yara/rules",
            exePath + "/../../../../src/security/scanning/yara/rules",
            exePath + "/../../../../../../../src/security/scanning/yara/rules"
        };
        
        for (const QString& searchPath : searchPaths) {
            QFileInfo searchInfo(searchPath);
            if (searchInfo.exists()) {
                // If it's a directory, check for .yar files
                if (searchInfo.isDir()) {
                    QDir dir(searchPath);
                    QStringList yarFiles = dir.entryList(QStringList() << "*.yar", QDir::Files, QDir::Name);
                    if (!yarFiles.isEmpty()) {
                        qRulesPath = searchPath;
                        pathInfo = searchInfo;
                        qDebug() << "Found rules directory with" << yarFiles.size() << ".yar files at:" << qRulesPath;
                        break;
                    }
                } else if (searchInfo.isFile() && searchPath.endsWith(".yar")) {
                    // Single .yar file
                    qRulesPath = searchPath;
                    pathInfo = searchInfo;
                    qDebug() << "Found single rule file at:" << qRulesPath;
                    break;
                }
            }
        }
        
        if (!pathInfo.exists()) {
            qDebug() << "YARA rules not found in any expected location. Searched paths:";
            for (const QString& path : searchPaths) {
                qDebug() << "  -" << path;
            }
            return make_error_code(YaraErrorCodes::FileNotFound);
        }
    }
    
    // Create compiler
    YR_COMPILER* rawCompiler = nullptr;
    int cres = yr_compiler_create(&rawCompiler);
    if (cres != ERROR_SUCCESS) {
        qDebug() << "YARA compiler creation failed with code:" << cres;
        return make_error_code(YaraErrorCodes::InternalError);
    }
    compiler = rawCompiler;

    // Set error callback to handle compiler warnings/errors gracefully
    yr_compiler_set_callback(rawCompiler, 
        [](int error_level, const char* file_name, int line_number, const YR_RULE* rule, const char* message, void* user_data) {
            if (error_level == YARA_ERROR_LEVEL_ERROR) {
                qDebug() << "YARA compiler ERROR:" << message << "in file:" << (file_name ? file_name : "unknown");
            } else {
                qDebug() << "YARA compiler WARNING:" << message << "in file:" << (file_name ? file_name : "unknown");
            }
        },
        nullptr);

    bool foundRules = false;
    
    if (pathInfo.isDir()) {
        // Handle directory - load all .yar files recursively
        QDir rulesDir(qRulesPath);
        
        // Get all .yar files in the main directory
        QStringList yaraFiles = rulesDir.entryList(QStringList() << "*.yar", QDir::Files, QDir::Name);
        
        // Also search in subdirectories recursively
        QDirIterator it(qRulesPath, QStringList() << "*.yar", QDir::Files, QDirIterator::Subdirectories);
        QStringList allYaraFiles;
        
        // Add files from main directory
        for (const QString& fileName : yaraFiles) {
            allYaraFiles << rulesDir.absoluteFilePath(fileName);
        }
        
        // Add files from subdirectories
        while (it.hasNext()) {
            QString filePath = it.next();
            if (!allYaraFiles.contains(filePath)) {  // Avoid duplicates
                allYaraFiles << filePath;
            }
        }
        
        qDebug() << "Found" << allYaraFiles.size() << ".yar files in directory (including subdirectories):" << qRulesPath;
        
        for (const QString& ruleFile : allYaraFiles) {
            qDebug() << "Loading YARA rule file:" << ruleFile;
            
            QFile file(ruleFile);
            if (!file.open(QIODevice::ReadOnly)) {
                qDebug() << "Cannot open rule file:" << ruleFile;
                continue;
            }
            
            // Create a temporary file for YARA (it needs FILE*)
            QString tempPath = QDir::temp().filePath(QFileInfo(ruleFile).fileName());
            QFile tempFile(tempPath);
            if (tempFile.open(QIODevice::WriteOnly)) {
                tempFile.write(file.readAll());
                tempFile.close();
                
                FILE* fp = fopen(tempPath.toLocal8Bit().constData(), "r");
                if (fp) {
                    cres = yr_compiler_add_file(rawCompiler, fp, nullptr, tempPath.toLocal8Bit().constData());
                    fclose(fp);
                    QFile::remove(tempPath); // Clean up temp file
                    
                    if (cres == ERROR_SUCCESS) {
                        foundRules = true;
                        qDebug() << "Successfully loaded rule:" << ruleFile;
                    } else {
                        qDebug() << "Failed to compile rule:" << ruleFile << "with error:" << cres;
                        qDebug() << "Compiler has" << rawCompiler->errors << "errors";
                        
                        // Reset compiler to continue with other files
                        yr_compiler_destroy(rawCompiler);
                        rawCompiler = nullptr;
                        yr_compiler_create(&rawCompiler);
                        
                        // Continue with other files instead of stopping completely
                    }
                }
            }
            file.close();
        }
        
    } else if (pathInfo.isFile()) {
        // Handle single file
        FILE* ruleFile = fopen(qRulesPath.toLocal8Bit().constData(), "r");
        if (ruleFile) {
            cres = yr_compiler_add_file(rawCompiler, ruleFile, nullptr, qRulesPath.toLocal8Bit().constData());
            fclose(ruleFile);
            
            if (cres == ERROR_SUCCESS) {
                foundRules = true;
                qDebug() << "Successfully loaded single rule file:" << qRulesPath;
            } else {
                qDebug() << "Failed to compile single rule file:" << qRulesPath << "with error:" << cres;
                qDebug() << "Compiler has" << rawCompiler->errors << "errors";
                
                // Reset compiler to prevent issues
                yr_compiler_destroy(rawCompiler);
                rawCompiler = nullptr;
                yr_compiler_create(&rawCompiler);
            }
        }
    }
    
    if (!foundRules) {
        qDebug() << "No valid YARA rules found in:" << qRulesPath;
        yr_compiler_destroy(rawCompiler);
        compiler = nullptr;
        return make_error_code(YaraErrorCodes::FileNotFound);
    }
    
    // Compile rules
    YR_RULES* rawRules = nullptr;
    cres = yr_compiler_get_rules(rawCompiler, &rawRules);
    if (cres != ERROR_SUCCESS) {
        qDebug() << "Failed to get compiled rules with code:" << cres;
        yr_compiler_destroy(rawCompiler);
        compiler = nullptr;
        return make_error_code(YaraErrorCodes::CompilerError);
    }
    
    rules = rawRules;
    yr_compiler_destroy(rawCompiler);
    compiler = nullptr;
    
    // Update statistics
    m_stats.totalRulesLoaded = getLoadedRuleNames().size();
    m_stats.lastRuleUpdate = std::chrono::system_clock::now();
    
    qDebug() << "YARA rules successfully loaded and compiled from:" << qRulesPath;
    qDebug() << "Total rules loaded:" << m_stats.totalRulesLoaded;
    return make_error_code(YaraErrorCodes::success);
}

std::error_code YaraRuleManager::unloadRules() {
    if (rules) {
        yr_rules_destroy(rules);
        rules = nullptr;
    }
    
    if (compiler) {
        YR_COMPILER* comp = static_cast<YR_COMPILER*>(compiler);
        yr_compiler_destroy(comp);
        compiler = nullptr;
    }
    return make_error_code(YaraErrorCodes::success);
}

// Compile Rules - artık loadRules içinde yapılıyor
std::error_code YaraRuleManager::compileRules() {
    if (!compiler) {
        qDebug() << "No compiler available for rule compilation";
        return make_error_code(YaraErrorCodes::RulesNotCompiled);
    }

    qDebug() << "Attempting to compile YARA rules";
    YR_RULES* rawRules = nullptr;
    YR_COMPILER* comp = static_cast<YR_COMPILER*>(compiler);
    int cres = yr_compiler_get_rules(comp, &rawRules);
    if (cres != ERROR_SUCCESS) {
        qDebug() << "Compilation failed with code:" << cres;
        return make_error_code(YaraErrorCodes::CompilerError);
    }

    qDebug() << "Rules compiled successfully";
    rules = rawRules;
    compiler = nullptr;
    yr_compiler_destroy(comp);
    return make_error_code(YaraErrorCodes::success);
}

// Callback setter - remove noexcept to match header
void YaraRuleManager::setCallback(std::function<void(const std::string&)> cb) {
    callback = std::move(cb);
}

// Scanning Methods
std::error_code YaraRuleManager::scanFile(const std::string& filePath, std::vector<std::string>& matches) {
    if (!initialized) {
        qDebug() << "YARA not initialized for scanning";
        return make_error_code(YaraErrorCodes::NotInitialized);
    }
    if (!rules) {
        qDebug() << "No compiled rules available for scanning";
        return make_error_code(YaraErrorCodes::RulesNotCompiled);
    }

    // Dosya varlığını kontrol et
    QString qFilePath = QString::fromStdString(filePath);
    QFileInfo fileInfo(qFilePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        qDebug() << "File does not exist or is not accessible:" << qFilePath;
        return make_error_code(YaraErrorCodes::FileNotFound);
    }
    
    // Dosya izinlerini kontrol et
    QFile file(qFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file for reading:" << qFilePath << "Error:" << file.errorString();
        return make_error_code(YaraErrorCodes::FileNotFound);
    }
    file.close();

    matches.clear();
    setCallback([&matches](const std::string& name) { 
        matches.push_back(name); 
        qDebug() << "Found YARA match:" << QString::fromStdString(name);
    });

    qDebug() << "Scanning file with YARA: " << qFilePath;
    
    // YARA tarama seçeneklerini ayarlayalım
    int scan_flags = 0;
    
    // Hash modülü kullanıldığında SCAN_FLAGS_PROCESS_MEMORY bayrağını etkinleştir
    // Bu "import hash" ile ilgili sorunları önleyebilir
    scan_flags |= SCAN_FLAGS_NO_TRYCATCH;  // Hata yakalama devre dışı bırakılır
    
#ifdef _WIN32
    // Windows'ta dosya yolu için özel karakter desteği
    std::wstring wFilePath = qFilePath.toStdWString();
    
    // Dosyayı manuel olarak açalım ve bellek olarak tarayalım
    try {
        FILE* fp = _wfopen(wFilePath.c_str(), L"rb");
        if (fp) {
            // Dosya boyutunu belirle
            fseek(fp, 0, SEEK_END);
            long fileSize = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            
            if (fileSize > 0) {
                // Dosya içeriğini oku
                std::vector<uint8_t> buffer(fileSize);
                size_t bytesRead = fread(buffer.data(), 1, fileSize, fp);
                fclose(fp);
                
                if (bytesRead > 0) {
                    // Bellek olarak tara
                    int sres = yr_rules_scan_mem(
                        rules,
                        buffer.data(), 
                        bytesRead,
                        scan_flags,
                        yara_callback,
                        this,
                        10000  // 10 sn zaman aşımı
                    );
                    
                    if (sres != ERROR_SUCCESS) {
                        qDebug() << "YARA memory scan error with code:" << sres;
                        return make_error_code(YaraErrorCodes::ScanError);
                    }
                } else {
                    qDebug() << "Failed to read file content";
                    return make_error_code(YaraErrorCodes::ScanError);
                }
            } else {
                qDebug() << "File is empty";
                return make_error_code(YaraErrorCodes::ScanError);
            }
        } else {
            qDebug() << "Failed to open file with _wfopen";
            return make_error_code(YaraErrorCodes::FileNotFound);
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception during file scanning:" << e.what();
        return make_error_code(YaraErrorCodes::ScanError);
    }
#else
    // Standart YARA tarama fonksiyonunu kullanalım
    int sres = yr_rules_scan_file(
        rules,
        filePath.c_str(),
        scan_flags,
        yara_callback,
        this,
        10000  // 10 sn zaman aşımı
    );
    
    if (sres != ERROR_SUCCESS) {
        qDebug() << "YARA file scan error with code:" << sres;
        return make_error_code(YaraErrorCodes::ScanError);
    }
#endif

    // Update statistics
    m_stats.filesScanned++;
    m_stats.matchesFound += matches.size();
    
    qDebug() << "YARA scan completed successfully, found" << matches.size() << "matches";
    return make_error_code(YaraErrorCodes::success);
}

std::error_code YaraRuleManager::scanMemory(const uint8_t* data, size_t size, std::vector<std::string>& matches) {
    if (!initialized)
        return make_error_code(YaraErrorCodes::NotInitialized);
    if (!rules)
        return make_error_code(YaraErrorCodes::RulesNotCompiled);

    matches.clear();
    setCallback([&matches](const std::string& name) { matches.push_back(name); });

    int sres = yr_rules_scan_mem(
        rules,
        data,
        size,
        0,
        yara_callback,
        this,
        0
    );
    if (sres != ERROR_SUCCESS)
        return make_error_code(YaraErrorCodes::ScanError);

    return make_error_code(YaraErrorCodes::success);
}

std::vector<std::string> YaraRuleManager::getLoadedRuleNames() const {
    std::vector<std::string> ruleNames;
    
    if (!rules) {
        return ruleNames; // Return empty vector if no rules loaded
    }
    
    YR_RULE* rule;
    yr_rules_foreach(rules, rule) {
        if (rule->identifier) {
            ruleNames.emplace_back(rule->identifier);
        }
    }
    
    return ruleNames;
}

YaraRuleStats YaraRuleManager::getStatistics() const {
    return m_stats;
}

void YaraRuleManager::resetStatistics() {
    m_stats.filesScanned = 0;
    m_stats.matchesFound = 0;
    m_stats.averageScanTime = std::chrono::milliseconds{0};
    m_stats.ruleMatchCounts.clear();
    // Keep totalRulesLoaded and lastRuleUpdate
}

// Built-in Rules Loading Functions
std::error_code YaraRuleManager::loadBuiltinRules() {
    if (!initialized) {
        return make_error_code(YaraErrorCodes::NotInitialized);
    }

    qDebug() << "Loading built-in YARA rules...";
    
    // Try to find rules directory relative to executable
    QString exePath = QCoreApplication::applicationDirPath();
    QStringList rulePaths = {
        exePath + "/yara_rules",           // Next to executable
        exePath + "/../yara_rules",        // One level up
        exePath + "/../../yara_rules",     // Two levels up (for build structure)
        exePath + "/Contents/Resources/yara_rules",  // macOS app bundle
#ifdef YARA_RULES_PATHS_H
        YaraConfig::RULES_BASE_DIR,        // From generated config
#endif
        "/Volumes/Crucial/AVProjectUi/src/security/scanning/yara"  // Fallback to source
    };

    QString foundRulesDir;
    for (const QString& path : rulePaths) {
        QFileInfo pathInfo(path);
        if (pathInfo.exists() && pathInfo.isDir()) {
            // Check if it contains .yar files
            QDir dir(path);
            QStringList yarFiles = dir.entryList(QStringList() << "*.yar", QDir::Files | QDir::AllDirs, QDir::Name);
            if (!yarFiles.isEmpty() || dir.exists("rules") || dir.exists("maldocs")) {
                foundRulesDir = path;
                qDebug() << "Found YARA rules directory at:" << foundRulesDir;
                break;
            }
        }
    }

    if (foundRulesDir.isEmpty()) {
        qDebug() << "No built-in YARA rules directory found. Searched paths:";
        for (const QString& path : rulePaths) {
            qDebug() << "  -" << path;
        }
        return make_error_code(YaraErrorCodes::FileNotFound);
    }

    // Load all rules from the found directory
    return loadRules(foundRulesDir.toStdString());
}

std::error_code YaraRuleManager::loadRulesByCategory(YaraConfig::RuleCategory category) {
    if (!initialized) {
        return make_error_code(YaraErrorCodes::NotInitialized);
    }

    // Try to find application directory first for built rules
    QString exePath = QCoreApplication::applicationDirPath();
    QString rulesBaseDir = exePath + "/yara_rules";
    
    QString rulePath;
    switch(category) {
        case YaraConfig::RuleCategory::BASIC_MALWARE:
            rulePath = rulesBaseDir + "/rules/malware_basic.yar";
            break;
        case YaraConfig::RuleCategory::ADVANCED_THREATS:
            rulePath = rulesBaseDir + "/rules/advanced_threats.yar";
            break;
        case YaraConfig::RuleCategory::TROJANS:
            rulePath = rulesBaseDir + "/rules/trojans.yar";
            break;
        case YaraConfig::RuleCategory::MALDOCS:
            rulePath = rulesBaseDir + "/maldocs";  // Entire directory
            break;
        case YaraConfig::RuleCategory::CUSTOM:
            rulePath = rulesBaseDir + "/rules/custom.yar";
            break;
        default:
            return loadBuiltinRules();  // Fallback to loading all rules
    }
    
    if (!rulePath.isEmpty()) {
        qDebug() << "Loading rules for category:" << static_cast<int>(category) << "from:" << rulePath;
        return loadRules(rulePath.toStdString());
    }

    qDebug() << "Category-based rule loading failed, falling back to all rules";
    return loadBuiltinRules();  // Fallback to loading all rules
}
