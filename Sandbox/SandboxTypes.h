#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <map>
#include <memory>

namespace Sandbox {

// Forward declarations
class SandboxManager;
class SandboxDaemon;

// Enums
enum class MonitoringLevel {
    LOW,
    MEDIUM,
    HIGH,
    MAXIMUM,
    STANDARD,  // Used as default in DashboardWidget
    DEEP       // Used for deep analysis in DashboardWidget
};

enum class ThreatLevel {
    NONE,
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

enum class ThreatType {
    MALWARE,
    SUSPICIOUS_BEHAVIOR,
    NETWORK_ACTIVITY,
    FILE_SYSTEM_MODIFICATION,
    REGISTRY_MODIFICATION,
    PROCESS_INJECTION,
    UNKNOWN,
    VIRUS,                       // Used in DashboardWidget
    TROJAN,                      // Used in DashboardWidget
    RANSOMWARE,                  // Used in DashboardWidget
    SPYWARE,                     // Used in DashboardWidget
    ADWARE,                      // Used in DashboardWidget
    POTENTIALLY_UNWANTED_PROGRAM // Used in DashboardWidget
};

enum class UserAction {
    QUARANTINE,
    DELETE,
    ALLOW,
    DEEP_ANALYSIS
};

enum class FileOperation {
    READ,
    WRITE,
    DELETE,
    EXECUTE,
    MODIFY_ATTRIBUTES,
    CREATE,     // Used in DashboardWidget
    MODIFY      // Used in DashboardWidget
};

enum class NetworkOperation {
    CONNECT,
    LISTEN,
    SEND_DATA,
    RECEIVE_DATA,
    DNS_QUERY
};

enum class ProcessOperation {
    CREATE,
    TERMINATE,
    INJECT,
    MODIFY_MEMORY
};

// Structs
struct NetworkActivity {
    NetworkOperation operation;
    std::string remoteAddress;
    int remotePort;
    std::string protocol;
    size_t bytesTransferred;
    std::chrono::system_clock::time_point timestamp;
};

struct FileSystemActivity {
    FileOperation operation;
    std::string filePath;
    size_t bytesTransferred;
    std::chrono::system_clock::time_point timestamp;
};

struct ProcessActivity {
    ProcessOperation operation;
    std::string processName;
    int processId;
    std::string commandLine;
    std::chrono::system_clock::time_point timestamp;
};

struct NetworkConnection {
    std::string destinationHost;
    int destinationPort;
    std::string protocol;
    size_t bytesTransferred;
    std::chrono::system_clock::time_point timestamp;
};

struct ThreatInfo {
    ThreatType type;
    ThreatLevel level;
    std::string description;
    std::string detectedAt;
    std::chrono::system_clock::time_point timestamp;
    std::map<std::string, std::string> additionalInfo;
    std::string details;  // Used in DashboardWidget for additional details
};

struct BehaviorAnalysisData {
    std::vector<NetworkActivity> networkActivities;
    std::vector<FileSystemActivity> fileSystemActivities;
    std::vector<ProcessActivity> processActivities;
    std::vector<NetworkConnection> networkConnections; // Used in DashboardWidget
    std::vector<FileSystemActivity> fileSystemActivity; // Used in DashboardWidget
    ThreatLevel overallThreatLevel;
    std::vector<std::string> suspiciousPatterns;
    std::chrono::system_clock::time_point analysisTime;
};

// Additional types used in SandboxManager.h
struct MonitoringConfig {
    MonitoringLevel level;
    bool enableNetworkMonitoring;
    bool enableFileSystemMonitoring;
    bool enableProcessMonitoring;
    bool enableRegistryMonitoring;
    int monitoringDurationSeconds;
    std::vector<std::string> monitoredProcesses;
    
    MonitoringConfig() 
        : level(MonitoringLevel::STANDARD)
        , enableNetworkMonitoring(true)
        , enableFileSystemMonitoring(true)
        , enableProcessMonitoring(true)
        , enableRegistryMonitoring(true)
        , monitoringDurationSeconds(300) {}
};

struct NetworkAnalysisResult {
    std::vector<NetworkConnection> connections;
    std::vector<std::string> suspiciousConnections;
    ThreatLevel threatLevel;
    std::string analysisDetails;
    bool analysisCompleted;
    
    NetworkAnalysisResult() 
        : threatLevel(ThreatLevel::NONE)
        , analysisCompleted(false) {}
};

struct FileSystemAnalysisResult {
    std::vector<FileSystemActivity> activities;
    std::vector<std::string> suspiciousFiles;
    ThreatLevel threatLevel;
    std::string analysisDetails;
    bool analysisCompleted;
    
    FileSystemAnalysisResult() 
        : threatLevel(ThreatLevel::NONE)
        , analysisCompleted(false) {}
};

struct ProcessAnalysisResult {
    std::vector<ProcessActivity> activities;
    std::vector<std::string> suspiciousProcesses;
    ThreatLevel threatLevel;
    std::string analysisDetails;
    bool analysisCompleted;
    
    ProcessAnalysisResult() 
        : threatLevel(ThreatLevel::NONE)
        , analysisCompleted(false) {}
};

struct SystemCallAnalysisResult {
    std::vector<std::string> systemCalls;
    std::vector<std::string> suspiciousCalls;
    ThreatLevel threatLevel;
    std::string analysisDetails;
    bool analysisCompleted;
    
    SystemCallAnalysisResult() 
        : threatLevel(ThreatLevel::NONE)
        , analysisCompleted(false) {}
};

struct MonitoringData {
    std::vector<NetworkConnection> networkConnections;
    std::vector<FileSystemActivity> fileSystemActivity;
    std::vector<ProcessActivity> processActivity;
    std::vector<std::string> systemCalls;
    std::chrono::system_clock::time_point collectionTime;
    std::string rawData;
};

struct SandboxConfiguration {
    MonitoringLevel level;
    std::string sandboxPath;
    std::string tempPath;
    int timeoutSeconds;
    bool enableNetworkMonitoring;
    bool enableFileSystemMonitoring;
    bool enableProcessMonitoring;
    bool enableBehaviorAnalysis;
    std::vector<std::string> allowedNetworkRanges;
    std::vector<std::string> blockedProcesses;
    
    SandboxConfiguration() 
        : level(MonitoringLevel::MEDIUM)
        , timeoutSeconds(300)
        , enableNetworkMonitoring(true)
        , enableFileSystemMonitoring(true)
        , enableProcessMonitoring(true)
        , enableBehaviorAnalysis(true) {}
};

struct SandboxAnalysisResult {
    std::string analysisId;
    std::string filePath;
    bool isSafe;
    ThreatLevel threatLevel;
    ThreatLevel overallThreatLevel;    // Used in DashboardWidget
    std::vector<ThreatInfo> threats;
    std::vector<ThreatInfo> detectedThreats; // Used in DashboardWidget
    BehaviorAnalysisData behaviorData;
    BehaviorAnalysisData behaviorAnalysis;   // Used in DashboardWidget
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    std::string errorMessage;
    bool analysisCompleted;
    bool success;                     // Used in DashboardWidget
    int processExitCode;              // Used in DashboardWidget
    double executionDurationSeconds;  // Used in DashboardWidget
    
    SandboxAnalysisResult() 
        : isSafe(true)
        , threatLevel(ThreatLevel::NONE)
        , overallThreatLevel(ThreatLevel::NONE)
        , analysisCompleted(false)
        , success(false)
        , processExitCode(0)
        , executionDurationSeconds(0.0) {}
};

struct SandboxStats {
    int totalAnalyses;
    int threatsDetected;
    int cleanFiles;
    std::chrono::system_clock::time_point lastUpdate;
    
    SandboxStats() 
        : totalAnalyses(0)
        , threatsDetected(0)
        , cleanFiles(0)
        , lastUpdate(std::chrono::system_clock::now()) {}
};

} // namespace Sandbox