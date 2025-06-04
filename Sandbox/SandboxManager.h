#ifndef SANDBOX_MANAGER_H
#define SANDBOX_MANAGER_H

#include "Docker/include/docker/DockerManager.h"
#include "Docker/include/docker/DockerTypes.h"
#include "Docker/include/docker/DockerExceptions.h"
#include "Docker/include/cdr/CdrManager.h"
#include "SandboxTypes.h"
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <map>

namespace Sandbox {

// SandboxManager uses DockerManager via composition to provide
// specialized sandbox functionality for malware analysis
class SandboxManager {
private:
    std::unique_ptr<Docker::DockerManager> dockerManager_;  // Composition instead of inheritance
    
public:
    // Constructor
    SandboxManager();
    explicit SandboxManager(std::unique_ptr<Docker::DockerManager> dockerMgr);
    
    // Destructor - cleanup any active sandbox environments
    ~SandboxManager();

    // Docker functionality access
    Docker::DockerManager& getDockerManager() { return *dockerManager_; }
    const Docker::DockerManager& getDockerManager() const { return *dockerManager_; }

    // Check if Docker daemon is running
    bool isDaemonRunning() const { return dockerManager_->isDaemonRunning(); }

    // --- Sandbox Core Methods ---
    
    // Analyzes a file for threats in an isolated sandbox
    SandboxAnalysisResult analyzeFileForThreats(const std::string& filePath, MonitoringLevel level = MonitoringLevel::STANDARD);
    
    // Creates and starts a sandbox environment
    std::string createSandbox(const SandboxConfiguration& config);
    
    // Executes a file in an existing sandbox
    bool executeFileInSandbox(const std::string& sandboxId, const std::string& filePath);
    
    // Collects analysis results from sandbox
    SandboxAnalysisResult collectAnalysisResults(const std::string& sandboxId);
    
    // Destroys a sandbox environment
    bool destroySandbox(const std::string& sandboxId);
    
    // --- File Management Methods ---
    
    // Processes file based on user action
    bool processFileWithUserChoice(const std::string& filePath, UserAction action);
    
    // Quarantines a suspicious file
    bool quarantineFile(const std::string& filePath);
    
    // Permanently deletes a file
    bool deleteFile(const std::string& filePath);
    
    // Allows file to be used normally
    bool allowFile(const std::string& filePath);
    
    // --- Configuration and Status Methods ---
    
    // Gets current default configuration
    SandboxConfiguration getDefaultSandboxConfiguration() const;
    
    // Validates sandbox configuration
    bool validateSandboxConfiguration(const SandboxConfiguration& config) const;
    
    // --- Utility Methods ---
    
    // Ensures required sandbox images are available
    bool ensureSandboxImagesAvailable();
    
    // Gets list of available sandbox environments
    std::vector<std::string> getActiveSandboxes();
    
    // Cleanup orphaned sandbox containers
    void cleanupOrphanedSandboxes();

private:
    // --- Private Helper Methods ---
    
    // Builds Docker run configuration for sandbox
    Docker::ContainerRunConfiguration buildSandboxRunConfiguration(const SandboxConfiguration& config);
    
    // Sets up monitoring infrastructure in the sandbox
    bool setupSandboxMonitoring(const std::string& containerId, const MonitoringConfig& monitoringConfig);
    
    // Collects behavioral analysis data
    BehaviorAnalysisData collectBehaviorAnalysis(const std::string& containerId);
    
    // Analyzes network traffic from sandbox
    NetworkAnalysisResult analyzeNetworkTraffic(const std::string& containerId);
    
    // Analyzes file system changes
    FileSystemAnalysisResult analyzeFileSystemChanges(const std::string& containerId);
    
    // Analyzes process behavior
    ProcessAnalysisResult analyzeProcessBehavior(const std::string& containerId);
    
    // Analyzes system calls and API usage
    SystemCallAnalysisResult analyzeSystemCalls(const std::string& containerId);
    
    // Generates IOCs (Indicators of Compromise)
    std::vector<std::string> generateIndicatorsOfCompromise(const SandboxAnalysisResult& analysisResult);
    
    // Scores threat level based on analysis
    ThreatLevel calculateThreatLevel(const SandboxAnalysisResult& analysisResult);
    
    // Validates sandbox image integrity
    bool validateSandboxImage(const std::string& imageName);
    
    // Sets up sandbox network isolation
    bool setupNetworkIsolation(const std::string& containerId, const SandboxConfiguration& config);
    
    // Sets up sandbox file system isolation  
    bool setupFileSystemIsolation(const std::string& containerId, const SandboxConfiguration& config);
    
    // Gets sandbox execution logs
    std::string getSandboxExecutionLogs(const std::string& containerId);
    
    // Parses monitoring data from sandbox
    MonitoringData parseSandboxMonitoringData(const std::string& rawData);
    
    // Helper method to parse delimited strings
    std::vector<std::string> parseDelimitedString(const std::string& input, char delimiter);
};

} // namespace Sandbox

#endif // SANDBOX_MANAGER_H