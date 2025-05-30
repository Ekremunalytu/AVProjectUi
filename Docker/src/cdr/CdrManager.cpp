#include "cdr/CdrManager.h"
#include "cdr/CdrSanitizer.h"
#include "cdr/CdrTypes.h" // For CdrConfigurationException, CdrFileException etc.
#include "docker/DockerExceptions.h" // Include for Docker::OperationException etc.
#include "cdr/FilesystemCompat.h"
#include <random>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <thread>
#include <unordered_map>
#include <cstring> // for memcmp

// Use CDR filesystem compatibility layer
namespace fs = CDR::FileSystem;

namespace CDR {

// --- Constructor & Destructor ---

CdrManager::CdrManager() 
    : dockerManager(std::make_unique<Docker::DockerManager>()),
      cdrContainerImage("testdisk:latest"),  // Default CDR image
      sandboxContainerImage("ubuntu:20.04"),  // Default sandbox image
      keepThreadsJoined_(true) // Initialize to join threads by default
{
    // Default configuration
    std::cout << "CdrManager initialized with default Docker manager" << std::endl;
}

CdrManager::CdrManager(std::unique_ptr<Docker::DockerManager> dockerMgr)
    : dockerManager(std::move(dockerMgr)),
      cdrContainerImage("testdisk:latest"),
      sandboxContainerImage("ubuntu:20.04"),
      keepThreadsJoined_(true) // Initialize to join threads by default
{
    if (!dockerManager) {
        // throw std::invalid_argument("DockerManager cannot be null");
        throw CDR::CdrConfigurationException("DockerManager cannot be null in CdrManager constructor");
    }
    std::cout << "CdrManager initialized with custom Docker manager" << std::endl;
}

CdrManager::~CdrManager() {
    keepThreadsJoined_ = false; // Signal threads to stop
    for (auto& pair : activeAnalysesThreads_) {
        if (pair.second.joinable()) {
            pair.second.join();
        }
    }
    // Sandbox cleanup threads are detached and should self-terminate.
    // If more robust cleanup is needed, a similar mechanism to activeAnalysesThreads_ can be implemented.
}

// --- Private Helper Methods ---

std::string CdrManager::generateAnalysisId() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "cdr_";
    for (int i = 0; i < 8; ++i) {
        ss << std::hex << dis(gen);
    }
    ss << "_" << std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return ss.str();
}

std::string CdrManager::prepareCdrContainer(const CdrConfiguration& config) {
    if (!dockerManager) {
        // throw std::runtime_error("Docker manager not initialized");
        throw CDR::CdrConfigurationException("Docker manager not initialized in prepareCdrContainer");
    }

    // Thread-safe container configuration
    std::lock_guard<std::mutex> lock(containerMutex_);

    // CDR konteyner konfigürasyonu hazırla
    Docker::ContainerRunConfiguration containerConfig;
    containerConfig.imageName = cdrContainerImage;
    containerConfig.containerName = "cdr_container_" + generateAnalysisId();
    
    // Mount points - input, output ve quarantine dizinleri
    Docker::MountPoint inputMount;
    inputMount.mountType = "bind";
    inputMount.source = config.inputDirectory;
    inputMount.destination = "/cdr/input";
    inputMount.readOnly = true; // Input dizini sadece okunabilir
    containerConfig.hostConfig.mounts.push_back(inputMount);
    
    Docker::MountPoint outputMount;
    outputMount.mountType = "bind";
    outputMount.source = config.outputDirectory;
    outputMount.destination = "/cdr/output";
    outputMount.readOnly = false;
    containerConfig.hostConfig.mounts.push_back(outputMount);

    Docker::MountPoint quarantineMount;
    quarantineMount.mountType = "bind";
    quarantineMount.source = config.quarantineDirectory;
    quarantineMount.destination = "/cdr/quarantine";
    quarantineMount.readOnly = false;
    containerConfig.hostConfig.mounts.push_back(quarantineMount);

    // SECURITY FIX: Remove privileged mode for security
    containerConfig.hostConfig.privileged = false;
    
    // Security hardening: Drop all capabilities and add only necessary ones
    containerConfig.hostConfig.capDrop = {"ALL"};
    containerConfig.hostConfig.capAdd = {"CHOWN", "DAC_OVERRIDE", "SETGID", "SETUID"};
    
    // Read-only root filesystem for security
    containerConfig.hostConfig.readOnlyRootfs = true;
    
    // Add tmpfs for temporary files
    Docker::MountPoint tmpMount;
    tmpMount.mountType = "tmpfs";
    tmpMount.destination = "/tmp";
    tmpMount.readOnly = false;
    containerConfig.hostConfig.mounts.push_back(tmpMount);

    // Memory limit based on configuration
    containerConfig.hostConfig.memoryLimit = config.maxMemoryMB * 1024 * 1024;
    
    // CPU limit for resource control
    containerConfig.hostConfig.cpuQuota = 50000; // 0.5 CPU
    containerConfig.hostConfig.cpuPeriod = 100000;

    // Environment variables with input validation
    std::string analysisTypeStr = getAnalysisTypeString(config.analysisType);
    if (analysisTypeStr.empty() || analysisTypeStr == "unknown") { // Added unknown check
        // throw std::invalid_argument("Invalid analysis type");
        throw CDR::CdrConfigurationException("Invalid analysis type specified in CdrConfiguration");
    }
    
    containerConfig.environmentVariables.push_back("CDR_ANALYSIS_TYPE=" + analysisTypeStr);
    containerConfig.environmentVariables.push_back(std::string("CDR_AUTO_SANITIZE=") + (config.autoSanitize ? "true" : "false"));
    containerConfig.environmentVariables.push_back("CDR_MAX_THREADS=" + std::to_string(std::min(config.maxThreads, 8))); // Limit max threads
    containerConfig.environmentVariables.push_back("CDR_SECURITY_LEVEL=" + std::to_string(static_cast<int>(config.securityLevel)));
    containerConfig.environmentVariables.push_back("CDR_TIMEOUT=" + std::to_string(config.timeoutSeconds));
    
    // Network isolation for security
    containerConfig.hostConfig.networkMode = "none";
    
    // Konteyner başlat
    try {
        auto result = dockerManager->runNewContainer(containerConfig);
        if (result.status == "Error") {
            // throw CDR::DockerOperationException("Failed to start CDR container: " + result.errorMessage);
            throw Docker::OperationException("runNewContainer", "Failed to start CDR container: " + result.errorMessage);
        }
        return result.containerId;
    } catch (const Docker::DockerException& e) { // Catch specific Docker exceptions from Docker namespace
        throw; // Re-throw if it's already a Docker::DockerException
    } catch (const std::exception& e) {
        // throw CDR::DockerOperationException("Failed to prepare CDR container: " + std::string(e.what()));
        throw Docker::OperationException("prepareCdrContainer", "Failed to prepare CDR container: " + std::string(e.what()));
    }
}

std::string CdrManager::prepareSandboxContainer() {
    if (!dockerManager) {
        throw CDR::CdrConfigurationException("Docker manager not initialized in prepareSandboxContainer");
    }

    // Thread-safe container preparation
    std::lock_guard<std::mutex> lock(containerMutex_);

    Docker::ContainerRunConfiguration containerConfig;
    containerConfig.imageName = sandboxContainerImage;
    containerConfig.containerName = "sandbox_" + generateAnalysisId();

    // Enhanced security settings for sandbox
    containerConfig.hostConfig.readOnlyRootfs = true;
    containerConfig.hostConfig.privileged = false; // Never use privileged mode
    
    // Network isolation for maximum security
    containerConfig.hostConfig.networkMode = "none";

    // Drop ALL capabilities and add only absolutely necessary ones
    containerConfig.hostConfig.capDrop = {"ALL"};
    containerConfig.hostConfig.capAdd = {"SETUID", "SETGID"}; // Minimal required capabilities

    // Strict resource limits
    containerConfig.hostConfig.memoryLimit = 256 * 1024 * 1024; // 256MB limit for sandbox
    containerConfig.hostConfig.cpuQuota = 25000; // 0.25 CPU max
    containerConfig.hostConfig.cpuPeriod = 100000;
    
    // Limit number of processes (if supported)
    containerConfig.hostConfig.pidsLimit = 64;

    // tmpfs mount for sandbox - ephemeral storage only
    Docker::MountPoint tmpMount;
    tmpMount.mountType = "tmpfs";
    tmpMount.destination = "/tmp";
    tmpMount.readOnly = false;
    tmpMount.tmpfsOptions = "noexec,nosuid,nodev,size=64m"; // 64MB tmpfs with security options
    containerConfig.hostConfig.mounts.push_back(tmpMount);
    
    // Add a working directory tmpfs
    Docker::MountPoint workMount;
    workMount.mountType = "tmpfs";
    workMount.destination = "/sandbox";
    workMount.readOnly = false;
    workMount.tmpfsOptions = "noexec,nosuid,nodev,size=32m"; // 32MB workspace
    containerConfig.hostConfig.mounts.push_back(workMount);
    
    // Security-hardened container command with timeout
    containerConfig.command = {"/bin/sh", "-c", "sleep 300"}; // 5 minute max lifetime
    
    // Add security labels (if using SELinux/AppArmor)
    containerConfig.hostConfig.securityOpt = {"no-new-privileges:true"};
    
    try {
        auto result = dockerManager->runNewContainer(containerConfig);
        if (result.status == "Error") {
            // throw CDR::DockerOperationException("Failed to start sandbox container: " + result.errorMessage);
            throw Docker::OperationException("runNewContainer", "Failed to start sandbox container: " + result.errorMessage);
        }
        
        // Schedule automatic cleanup after 5 minutes
        std::thread([this, containerId = result.containerId]() {
            std::this_thread::sleep_for(std::chrono::minutes(5));
            try {
                if (dockerManager && keepThreadsJoined_) { // Check keepThreadsJoined_ before accessing dockerManager
                    dockerManager->stopContainer(containerId);
                    dockerManager->removeContainer(containerId);
                }
            } catch (const Docker::DockerException& e) { // Catch specific Docker exceptions
                std::cerr << "Error during sandbox cleanup: " << e.what() << " (Type: DockerException)" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error during sandbox cleanup: " << e.what() << std::endl;
            }
        }).detach();
        
        return result.containerId;
    } catch (const Docker::DockerException& e) { // Catch specific Docker exceptions
        throw;
    } catch (const std::exception& e) {
        // throw CDR::DockerOperationException("Failed to prepare sandbox container: " + std::string(e.what()));
        throw Docker::OperationException("prepareSandboxContainer", "Failed to prepare sandbox container: " + std::string(e.what()));
    }
}

void CdrManager::copyFileToContainer(const std::string& containerId, 
                                    const std::string& hostPath, 
                                    const std::string& containerPath) {
    if (!dockerManager) {
        // throw std::runtime_error("Docker manager not initialized");
        throw CDR::CdrConfigurationException("Docker manager not initialized in copyFileToContainer");
    }
    if (!fs::exists(hostPath)) {
        throw CDR::CdrFileException("Source file for copy does not exist", hostPath);
    }
    
    try {
        dockerManager->copyFileToContainer(containerId, hostPath, containerPath);
    } catch (const Docker::DockerException& e) { // Catch specific Docker exceptions
        throw;
    } catch (const std::exception& e) {
        // throw CDR::DockerOperationException("File copy to container failed for " + hostPath + ": " + std::string(e.what()));
        throw Docker::OperationException("copyFileToContainer", "File copy to container failed for " + hostPath + ": " + std::string(e.what()));
    }
}

void CdrManager::copyFileFromContainer(const std::string& containerId,
                                      const std::string& containerPath,
                                      const std::string& hostPath) {
    if (!dockerManager) {
        // throw std::runtime_error("Docker manager not initialized");
        throw CDR::CdrConfigurationException("Docker manager not initialized in copyFileFromContainer");
    }
    
    try {
        dockerManager->copyFileFromContainer(containerId, containerPath, hostPath);
    } catch (const Docker::DockerException& e) { // Catch specific Docker exceptions
        throw;
    } catch (const std::exception& e) {
        // throw CDR::DockerOperationException("File copy from container '" + containerPath + "' failed: " + std::string(e.what()));
        throw Docker::OperationException("copyFileFromContainer", "File copy from container '" + containerPath + "' failed: " + std::string(e.what()));
    }
}

std::vector<RecoveredFileInfo> CdrManager::parseRecoveryResults(const std::string& resultsPath) {
    std::vector<RecoveredFileInfo> recoveredFiles;
    
    try {
        std::ifstream resultsFile(resultsPath);
        if (!resultsFile.is_open()) {
            std::cerr << "Warning: Could not open results file: " << resultsPath << std::endl;
            return recoveredFiles;
        }
        
        std::string line;
        while (std::getline(resultsFile, line)) {
            if (line.empty() || line[0] == '#') continue; // Skip comments and empty lines
            
            // Parse JSON or CSV format results
            // Bu implementation format'a göre customize edilmeli
            RecoveredFileInfo fileInfo;
            
            // Basit parsing örneği (gerçek implementasyonda JSON parser kullanılmalı)
            std::istringstream iss(line);
            std::string token;
            
            if (std::getline(iss, token, ',')) fileInfo.originalPath = token;
            if (std::getline(iss, token, ',')) fileInfo.recoveredPath = token;
            if (std::getline(iss, token, ',')) fileInfo.fileName = token;
            if (std::getline(iss, token, ',')) fileInfo.fileExtension = token;
            if (std::getline(iss, token, ',')) fileInfo.fileSize = std::stoll(token);
            if (std::getline(iss, token, ',')) fileInfo.md5Hash = token;
            if (std::getline(iss, token, ',')) fileInfo.status = token;
            if (std::getline(iss, token, ',')) fileInfo.confidenceScore = std::stod(token);
            
            recoveredFiles.push_back(fileInfo);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing recovery results: " << e.what() << std::endl;
    }
    
    return recoveredFiles;
}

// --- Public Methods ---

std::string CdrManager::startAnalysis(const std::string& directoryPath,
                                     const CdrConfiguration& config) {
    // Input validation
    if (directoryPath.empty()) {
        // throw std::invalid_argument("Directory path cannot be empty");
        throw CDR::CdrFileException("Analysis directory path cannot be empty", directoryPath);
    }
    
    if (!fs::exists(directoryPath)) {
        // throw std::invalid_argument("Directory does not exist: " + directoryPath);
        throw CDR::CdrFileException("Analysis directory does not exist", directoryPath);
    }
    
    if (!fs::is_directory(directoryPath)) {
        // throw std::invalid_argument("Path is not a directory: " + directoryPath);
        throw CDR::CdrFileException("Analysis path is not a directory", directoryPath);
    }
    
    // Validate configuration
    if (config.inputDirectory.empty() || config.outputDirectory.empty() || config.quarantineDirectory.empty()) {
        // throw std::invalid_argument("All configuration directories must be specified");
        throw CDR::CdrConfigurationException("All CdrConfiguration directories (input, output, quarantine) must be specified");
    }
    
    if (config.maxMemoryMB < 64 || config.maxMemoryMB > 8192) {
        // throw std::invalid_argument("Memory limit must be between 64MB and 8GB");
        throw CDR::CdrConfigurationException("CdrConfiguration memory limit must be between 64MB and 8GB");
    }
    
    if (config.maxThreads < 1 || config.maxThreads > 16) {
        // throw std::invalid_argument("Thread count must be between 1 and 16");
        throw CDR::CdrConfigurationException("CdrConfiguration thread count must be between 1 and 16");
    }
    
    if (config.timeoutSeconds < 30 || config.timeoutSeconds > 3600) {
        // throw std::invalid_argument("Timeout must be between 30 seconds and 1 hour");
        throw CDR::CdrConfigurationException("CdrConfiguration timeout must be between 30 seconds and 1 hour");
    }
    
    std::string analysisId = generateAnalysisId();
    CdrAnalysisResult result;
    result.analysisId = analysisId;
    result.type = config.analysisType;
    result.sourceDirectory = directoryPath;
    result.status = "initializing";
    result.startTime = std::chrono::system_clock::now();
    result.progressPercentage = 0;

    try {
        // Add analysis to the active list (thread-safe)
        {
            std::lock_guard<std::mutex> lock(analysesMutex);
            activeAnalyses[analysisId] = result;
        }

        // Start analysis in a separate thread with proper error handling
        std::thread analysisThread([this, analysisId, directoryPath, config]() {
            try {
                // Update status to running
                {
                    std::lock_guard<std::mutex> lock(analysesMutex);
                    auto it = activeAnalyses.find(analysisId);
                    if (it == activeAnalyses.end() || !keepThreadsJoined_) { 
                        if (it == activeAnalyses.end()) {
                            std::cerr << "Critical error: Analysis ID " << analysisId << " not found shortly after creation." << std::endl;
                        }
                        return; // Exit if analysis was removed or manager is destructing
                    }
                    it->second.status = "running";
                    it->second.progressPercentage = 5;
                }

                // Create and configure sanitizer
                CdrSanitizer sanitizer;
                std::vector<SanitizationResult> sanitizationResults;

                // Count total files for progress tracking
                size_t totalFiles = 0;
                fs::RecursiveDirectoryIterator dirIter(directoryPath);
                while (dirIter.has_next()) {
                    std::string entryPath = dirIter.current();
                    if (fs::is_regular_file(entryPath)) {
                        totalFiles++;
                    }
                    dirIter.next();
                }

                if (totalFiles == 0) {
                    std::lock_guard<std::mutex> lock(analysesMutex);
                    auto it = activeAnalyses.find(analysisId);
                    if (it != activeAnalyses.end()) {
                        it->second.status = "completed"; // Or "failed" with a specific message
                        it->second.errorMessage = "No files found in the directory.";
                        it->second.progressPercentage = 100;
                        it->second.endTime = std::chrono::system_clock::now();
                        // Consider throwing an exception or setting a specific error code if this is critical
                        // For now, it completes with an error message.
                    }
                    return;
                }

                size_t processedFiles = 0;
                
                // Process files in the directory
                fs::RecursiveDirectoryIterator dirIter2(directoryPath);
                while (dirIter2.has_next()) {
                    std::string entryPath = dirIter2.current();
                    if (fs::is_regular_file(entryPath)) {
                        // Simulate file processing
                        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Placeholder for actual work
                        processedFiles++;
                        {
                            std::lock_guard<std::mutex> lock(analysesMutex);
                            auto it = activeAnalyses.find(analysisId);
                            if (it != activeAnalyses.end()) {
                                it->second.progressPercentage = static_cast<int>((static_cast<double>(processedFiles) / totalFiles) * 90) + 5; // 5% to 95%
                            }
                        }
                    }
                    dirIter2.next();
                }

                // Mark analysis as completed
                {
                    std::lock_guard<std::mutex> lock(analysesMutex);
                    auto it = activeAnalyses.find(analysisId);
                    if (it != activeAnalyses.end()) {
                        it->second.status = "completed";
                        it->second.progressPercentage = 100;
                        it->second.endTime = std::chrono::system_clock::now();
                    }
                }

            } catch (const std::exception& e) {
                // Handle analysis errors
                std::lock_guard<std::mutex> lock(analysesMutex);
                auto it = activeAnalyses.find(analysisId);
                if (it != activeAnalyses.end()) {
                    it->second.status = "failed";
                    it->second.errorMessage = e.what();
                    it->second.endTime = std::chrono::system_clock::now();
                }
                std::cerr << "Analysis failed for ID " << analysisId << ": " << e.what() << std::endl;
            }
        });
        
        // Store the thread to join it later if necessary
        {
            std::lock_guard<std::mutex> lock(analysesMutex); // Protect access to activeAnalysesThreads_
            activeAnalysesThreads_[analysisId] = std::move(analysisThread);
        }
        // Optionally, detach if you absolutely don't want to wait, but joining is safer.
        // analysisThread.detach(); 
        
        return analysisId;
        
    } catch (const std::exception& e) {
        // Remove failed analysis from active list
        {
            std::lock_guard<std::mutex> lock(analysesMutex);
            activeAnalyses.erase(analysisId);
        }
        // throw std::runtime_error("Failed to start analysis: " + std::string(e.what()));
        // Determine if it was a configuration, file, or other type of error
        if (dynamic_cast<const CDR::CdrBaseException*>(&e)) {
            throw; // Re-throw if it's already one of our types
        }
        throw CDR::CdrBaseException("Failed to start analysis: " + std::string(e.what())); // General Cdr exception
    }
}

CDR::CdrAnalysisResult CDR::CdrManager::getAnalysisStatus(const std::string& analysisId) const {
    std::lock_guard<std::mutex> lock(analysesMutex);
    auto it = activeAnalyses.find(analysisId);
    if (it == activeAnalyses.end()) {
        // throw std::invalid_argument("Analysis ID not found: " + analysisId);
        throw CDR::CdrBaseException("Analysis ID not found: " + analysisId);
    }
    return it->second;
}

std::vector<CDR::CdrAnalysisResult> CDR::CdrManager::listActiveAnalyses() const {
    std::vector<CdrAnalysisResult> analyses;
    for (const auto& pair : activeAnalyses) {
        analyses.push_back(pair.second);
    }
    return analyses;
}

void CDR::CdrManager::stopAnalysis(const std::string& analysisId) {
    std::thread* threadToStop = nullptr;
    {
        std::lock_guard<std::mutex> lock(analysesMutex);
        auto it = activeAnalyses.find(analysisId);
        if (it == activeAnalyses.end()) {
            // throw std::invalid_argument("Analysis ID not found: " + analysisId);
            throw CDR::CdrBaseException("Analysis ID not found for stopping: " + analysisId);
        }

        // Mark for stopping, actual thread join/stop happens after releasing lock
        // or in destructor
        it->second.status = "stopping"; 

        auto threadIt = activeAnalysesThreads_.find(analysisId);
        if (threadIt != activeAnalysesThreads_.end()) {
            threadToStop = &threadIt->second;
        }
    }

    // Actual stopping logic for the thread if it exists and is joinable
    // This part needs careful design. Forcing a thread to stop is complex.
    // A cooperative cancellation mechanism is better.
    // For now, we'll rely on the thread checking a flag or the destructor joining.

    try {
        std::lock_guard<std::mutex> lock(analysesMutex); // Re-acquire lock for modifying activeAnalyses
        auto it = activeAnalyses.find(analysisId);
        if (it == activeAnalyses.end()) { // Check again, could have been removed
             // throw std::invalid_argument("Analysis ID not found after attempting to stop: " + analysisId);
             throw CDR::CdrBaseException("Analysis ID not found after attempting to stop: " + analysisId);
        }

        // Stop container
        if (it->second.metadata.count("containerId")) {
            std::string containerId = it->second.metadata["containerId"];
            if (!containerId.empty() && dockerManager) {
                dockerManager->stopContainer(containerId);
                dockerManager->removeContainer(containerId, true);
            }
        }

        // Update status
        it->second.status = "stopped";
        it->second.endTime = std::chrono::system_clock::now();

        // Remove from active threads map if it was managed
        // The thread itself should exit cleanly upon seeing the 'stopping' status or keepThreadsJoined_ flag
        auto threadIt = activeAnalysesThreads_.find(analysisId);
        if (threadIt != activeAnalysesThreads_.end()) {
            if (threadIt->second.joinable()) {
                // Ideally, the thread checks a flag and exits. 
                // If not, join might block. Consider a timeout or a more robust cancellation.
                // For now, let's assume threads are designed to check keepThreadsJoined_ or status.
            }
            activeAnalysesThreads_.erase(threadIt);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error stopping analysis: " << e.what() << std::endl;
        // Potentially revert status if stopping failed critically
        std::lock_guard<std::mutex> lock(analysesMutex);
        auto it = activeAnalyses.find(analysisId);
        if (it != activeAnalyses.end() && it->second.status == "stopping"){
            it->second.status = "failed_to_stop";
        }
        throw;
    }
}

std::vector<CDR::RecoveredFileInfo> CDR::CdrManager::getRecoveredFiles(const std::string& analysisId) const {
    auto it = activeAnalyses.find(analysisId);
    if (it == activeAnalyses.end()) {
        // throw std::invalid_argument("Analysis ID not found: " + analysisId);
        throw CDR::CdrBaseException("Analysis ID not found for getRecoveredFiles: " + analysisId);
    }
    
    // Gerçek implementasyonda parse edilen dosyalar dönülmeli
    std::vector<RecoveredFileInfo> files;
    
    // Placeholder - gerçek implementasyonda dosya bilgileri parse edilmeli
    for (const auto& filename : it->second.recoveredFileItems) {
        RecoveredFileInfo info;
        info.fileName = filename;
        info.recoveredPath = "/cdr/output/" + filename;
        info.status = "recovered";
        info.confidenceScore = 0.95;
        files.push_back(info);
    }
    
    return files;
}

bool CDR::CdrManager::exportRecoveredFile(const std::string& analysisId, 
                                    const std::string& fileId,
                                    const std::string& outputPath) {
    auto it = activeAnalyses.find(analysisId);
    if (it == activeAnalyses.end()) {
        // return false; // Consider throwing an exception for consistency
        throw CDR::CdrBaseException("Analysis ID not found for exportRecoveredFile: " + analysisId);
    }
    
    try {
        std::string containerId = it->second.metadata["containerId"];
        if (containerId.empty()) {
            return false;
        }
        
        // Dosyayı konteynerden kopyala
        std::string containerPath = "/cdr/output/" + fileId;
        copyFileFromContainer(containerId, containerPath, outputPath);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Export failed: " << e.what() << std::endl;
        return false;
    }
}

// --- Sandbox Methods ---

std::string CDR::CdrManager::createSandboxEnvironment() {
    try {
        std::string containerId = prepareSandboxContainer();
        return containerId;
    } catch (const std::exception& e) {
        // throw std::runtime_error("Failed to create sandbox: " + std::string(e.what()));
        throw Docker::OperationException("createSandboxEnvironment", "Failed to create sandbox: " + std::string(e.what()));
    }
}

bool CDR::CdrManager::executeFileInSandbox(const std::string& sandboxId, 
                                     const std::string& filePath,
                                     std::string& executionResult) {
    if (!dockerManager) {
        // return false;
        throw CDR::CdrConfigurationException("Docker manager not initialized in executeFileInSandbox");
    }
    if (!fs::exists(filePath)) {
        throw CDR::CdrFileException("File to execute in sandbox does not exist", filePath);
    }
    
    try {
        // Dosyayı sandbox'a kopyala
        std::string containerPath = "/tmp/suspicious_file";
        copyFileToContainer(sandboxId, filePath, containerPath);
        
        // Dosyayı güvenli şekilde çalıştır (timeout ile)
        std::vector<std::string> execCommand = {
            "timeout", "30s", // 30 saniye timeout
            "strace", "-e", "trace=file,process,network", // Sistem çağrılarını izle
            containerPath
        };
        
        executionResult = dockerManager->executeCommandInContainer(sandboxId, execCommand);
        
        return true;
    } catch (const std::exception& e) {
        executionResult = "Execution failed: " + std::string(e.what());
        return false;
    }
}

void CDR::CdrManager::destroySandboxEnvironment(const std::string& sandboxId) {
    if (!dockerManager) {
        // return; // Consider throwing or logging
        std::cerr << "Warning: Docker manager not initialized in destroySandboxEnvironment. Cannot destroy sandbox." << std::endl;
        return;
    }
    
    try {
        dockerManager->stopContainer(sandboxId);
        dockerManager->removeContainer(sandboxId, true);
    } catch (const Docker::DockerException& e) {
        std::cerr << "Failed to destroy sandbox '" << sandboxId << "': " << e.what() << " (Type: DockerException)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to destroy sandbox '" << sandboxId << "': " << e.what() << std::endl;
    }
}

// --- Configuration Methods ---

void CDR::CdrManager::setCdrContainerImage(const std::string& imageName) {
    if (imageName.empty()) {
        // throw std::invalid_argument("Image name cannot be empty");
        throw CDR::CdrConfigurationException("CDR container image name cannot be empty");
    }
    cdrContainerImage = imageName;
}

void CDR::CdrManager::setSandboxContainerImage(const std::string& imageName) {
    if (imageName.empty()) {
        // throw std::invalid_argument("Image name cannot be empty");
        throw CDR::CdrConfigurationException("Sandbox container image name cannot be empty");
    }
    sandboxContainerImage = imageName;
}

bool CDR::CdrManager::validateCdrEnvironment() const {
    if (!dockerManager) {
        return false;
    }
    
    try {
        // Docker daemon kontrolü
        if (!dockerManager->isDaemonRunning()) {
            return false;
        }
        
        // CDR image kontrolü
        if (!dockerManager->imageExists(cdrContainerImage)) {
            std::cerr << "CDR image not found: " << cdrContainerImage << std::endl;
            return false;
        }
        
        // Sandbox image kontrolü
        if (!dockerManager->imageExists(sandboxContainerImage)) {
            std::cerr << "Sandbox image not found: " << sandboxContainerImage << std::endl;
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Environment validation error: " << e.what() << std::endl;
        return false;
    }
}

// --- Utility Methods ---

std::string CDR::CdrManager::getAnalysisTypeString(AnalysisType type) {
    switch (type) {
        case AnalysisType::ACTIVE_CONTENT_SCAN:
            return "ACTIVE_CONTENT_SCAN";
        case AnalysisType::MALWARE_DETECTION:
            return "MALWARE_DETECTION";
        case AnalysisType::SCRIPT_SANITIZATION:
            return "SCRIPT_SANITIZATION";
        case AnalysisType::MACRO_REMOVAL:
            return "MACRO_REMOVAL";
        case AnalysisType::EXECUTABLE_ANALYSIS:
            return "EXECUTABLE_ANALYSIS";
        case AnalysisType::COMPREHENSIVE_SCAN:
            return "COMPREHENSIVE_SCAN";
        // Removed cases for non-existent AnalysisType members
        default:
            // It's good practice to have a default, though all defined enums should be covered.
            // This could indicate an unhandled enum value if the enum is extended later.
            return "UNKNOWN_ANALYSIS_TYPE"; // Or handle as an error
    }
}

CDR::AnalysisType CDR::CdrManager::parseAnalysisType(const std::string& typeStr) {
    if (typeStr == "active_content_scan") return AnalysisType::ACTIVE_CONTENT_SCAN;
    if (typeStr == "malware_detection") return AnalysisType::MALWARE_DETECTION;
    if (typeStr == "script_sanitization") return AnalysisType::SCRIPT_SANITIZATION;
    if (typeStr == "macro_removal") return AnalysisType::MACRO_REMOVAL;
    if (typeStr == "executable_analysis") return AnalysisType::EXECUTABLE_ANALYSIS;
    if (typeStr == "comprehensive_scan") return AnalysisType::COMPREHENSIVE_SCAN;
    
    // throw std::invalid_argument("Unknown analysis type: " + typeStr);
    throw CDR::CdrConfigurationException("Unknown analysis type string: " + typeStr);
}

// Dosya tipi algılama ve yönetim methodları
FileType CdrManager::detectFileType(const std::string& filePath) {
    fs::Path path(filePath);
    std::string extension = path.extension();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    // Office documents
    if (extension == ".docx" || extension == ".xlsx" || extension == ".pptx" ||
        extension == ".doc" || extension == ".xls" || extension == ".ppt") {
        return FileType::OFFICE_DOCUMENT;
    }

    // PDF documents
    if (extension == ".pdf") {
        return FileType::PDF_DOCUMENT;
    }

    // HTML/Web documents
    if (extension == ".html" || extension == ".htm") {
        return FileType::HTML_DOCUMENT;
    }

    // XML documents
    if (extension == ".xml") {
        return FileType::XML_DOCUMENT;
    }

    // RTF documents
    if (extension == ".rtf") {
        return FileType::RTF_DOCUMENT;
    }

    // Text documents
    if (extension == ".txt" || extension == ".csv") {
        return FileType::TEXT_DOCUMENT;
    }

    // Archive files
    if (extension == ".zip" || extension == ".rar" || extension == ".7z" ||
        extension == ".tar" || extension == ".gz") {
        return FileType::ARCHIVE_FILE;
    }

    // Image files
    if (extension == ".jpg" || extension == ".jpeg" || extension == ".png" ||
        extension == ".gif" || extension == ".bmp" || extension == ".svg") {
        return FileType::IMAGE_FILE;
    }

    // Executable files
    if (extension == ".exe" || extension == ".dll" || extension == ".so" ||
        extension == ".msi" || extension == ".app") {
        return FileType::EXECUTABLE_FILE;
    }

    // Script files
    if (extension == ".js" || extension == ".vbs" || extension == ".ps1" ||
        extension == ".bat" || extension == ".sh" || extension == ".py") {
        return FileType::SCRIPT_FILE;
    }

    // Email files
    if (extension == ".eml" || extension == ".msg") {
        return FileType::EMAIL_FILE;
    }

    return FileType::UNKNOWN_FILE;
}

FileTypeInfo CdrManager::getFileTypeInfo(FileType type) {
    FileTypeInfo info;
    info.type = type;
    info.requiresSpecialHandling = false; // Default

    switch (type) {
        case FileType::OFFICE_DOCUMENT:
            info.extension = ".docx"; // Primary extension
            info.commonExtensions = {".doc", ".docx", ".xls", ".xlsx", ".ppt", ".pptx"}; // .rtf is separate
            info.supportedTools = {"OfficeSanitizer", "MetadataExtractor"};
            info.description = "Microsoft Office Document (Word, Excel, PowerPoint)";
            info.activeContentTypes = {"Macros", "Embedded Objects", "External Links"};
            info.requiresSpecialHandling = true;
            break;
        case FileType::PDF_DOCUMENT:
            info.extension = ".pdf";
            info.commonExtensions = {".pdf"};
            info.supportedTools = {"PdfSanitizer", "ScriptDetector"};
            info.description = "Adobe PDF Document";
            info.activeContentTypes = {"JavaScript", "Forms", "Embedded Files", "Annotations"};
            info.requiresSpecialHandling = true;
            break;
        case FileType::HTML_DOCUMENT:
            info.extension = ".html";
            info.commonExtensions = {".html", ".htm", ".xhtml"};
            info.supportedTools = {"HtmlSanitizer", "ScriptRemover"};
            info.description = "HTML Web Document";
            info.activeContentTypes = {"Scripts", "Iframes", "Event Handlers", "Dangerous CSS"};
            info.requiresSpecialHandling = true;
            break;
        case FileType::XML_DOCUMENT:
            info.extension = ".xml";
            info.commonExtensions = {".xml", ".xsd", ".xsl", ".xslt", ".kml", ".svg"}; // SVG is XML-based
            info.supportedTools = {"XmlSanitizer", "DoctypeValidator"};
            info.description = "XML Document";
            info.activeContentTypes = {"External Entities (XXE)", "DTD Exploits", "SVG Scripts"};
             // SVG is often handled by ImageSanitizer, but XML_DOCUMENT can be a base for it.
            info.requiresSpecialHandling = true;
            break;
        case FileType::RTF_DOCUMENT:
            info.extension = ".rtf";
            info.commonExtensions = {".rtf"};
            info.supportedTools = {"RtfSanitizer", "OleObjectDetector"};
            info.description = "Rich Text Format Document";
            info.activeContentTypes = {"Embedded Objects", "OLE Exploits"};
            info.requiresSpecialHandling = true;
            break;
        case FileType::TEXT_DOCUMENT: // Corrected from TEXT_FILE to match enum
            info.extension = ".txt";
            info.commonExtensions = {".txt", ".log", ".csv", ".md", ".json"}; // Removed .xml as it has its own type
            info.supportedTools = {"TextSanitizer", "EncodingValidator"};
            info.description = "Plain Text File";
            info.activeContentTypes = {}; // Generally none, but JSON can be complex
            break;
        case FileType::ARCHIVE_FILE:
            info.extension = ".zip";
            info.commonExtensions = {".zip", ".rar", ".7z", ".tar", ".gz", ".jar", ".war"};
            info.supportedTools = {"ArchiveSanitizer", "ArchiveExtractor"};
            info.description = "Archive File (ZIP, RAR, 7Z, etc.)";
            info.activeContentTypes = {"Nested Archives", "Contained Executables", "Password Protection"};
            info.requiresSpecialHandling = true;
            break;
        case FileType::IMAGE_FILE:
            info.extension = ".jpg";
            info.commonExtensions = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".tiff", ".webp"}; // .svg handled by XML or separately
            info.supportedTools = {"ImageSanitizer", "MetadataStripper", "SteganographyDetector"};
            info.description = "Image File (JPEG, PNG, GIF, etc.)";
            info.activeContentTypes = {"EXIF Data", "Steganography"};
            // SVG is more complex and might be its own FileType or handled by XML/HTML sanitizers
            // For now, keeping scripts out of here unless specifically an SVG sanitizer is called.
            break;
        case FileType::EXECUTABLE_FILE:
            info.extension = ".exe";
            info.commonExtensions = {".exe", ".dll", ".com", ".bat", ".sh", ".app", ".dmg", ".msi", ".elf"};
            info.supportedTools = {"ExecutableAnalyzer", "SandboxRunner", "SignatureScanner"};
            info.description = "Executable File or Application";
            info.activeContentTypes = {"Binary Code", "Packed Code"};
            info.requiresSpecialHandling = true;
            break;
        case FileType::SCRIPT_FILE:
            info.extension = ".js"; 
            info.commonExtensions = {".js", ".vbs", ".ps1", ".bat", ".sh", ".py", ".php", ".rb", ".pl"};
            info.supportedTools = {"ScriptAnalyzer", "ObfuscationDetector", "BehavioralAnalyzer"};
            info.description = "Script File (JavaScript, PowerShell, VBScript, etc.)";
            info.activeContentTypes = {"Executable Code", "Remote Connections", "Filesystem Access"};
            info.requiresSpecialHandling = true;
            break;
        case FileType::EMAIL_FILE:
            info.extension = ".eml";
            info.commonExtensions = {".eml", ".msg"};
            info.supportedTools = {"EmailSanitizer", "AttachmentExtractor", "HeaderAnalyzer"};
            info.description = "Email File (.eml, .msg)";
            info.activeContentTypes = {"Attachments", "HTML Body Scripts", "Tracking Pixels", "Phishing Links"};
            info.requiresSpecialHandling = true;
            break;
        // Removed cases for non-existent FileType members like VIDEO_FILE, AUDIO_FILE, FONT_FILE etc.
        // Add them to CdrTypes.h if they are needed.
        case FileType::NOT_SET: // Fallthrough
        case FileType::UNKNOWN_FILE: // Fallthrough
        default:
            info.extension = ".dat"; 
            info.commonExtensions = {};
            info.supportedTools = {"GenericAnalyzer", "HeuristicScanner"};
            info.description = "Unknown or Uncategorized File Type";
            info.activeContentTypes = {}; // Could be anything
            info.requiresSpecialHandling = true; 
            break;
    }
    return info;
}

// Private helper method for sanitization logic
SanitizationResult CdrManager::performSanitization(const std::string& fileTypeDescription,
                                                   const std::string& inputPath,
                                                   const std::string& outputPath,
                                                   const CdrConfiguration& config,
                                                   FileType fileType) {
    try {
        // CdrSanitizer should be instantiated once if it holds state or is expensive to create.
        // For now, creating it per call as it was, but consider making it a member of CdrManager.
        CdrSanitizer sanitizer; 

        SanitizationResult result = sanitizer.sanitizeFile(inputPath, outputPath, config, fileType);

        // Log based on the result from CdrSanitizer
        if (result.requiresQuarantine) {
            std::cout << fileTypeDescription << " file ('" << inputPath << "') requires quarantine." << std::endl;
            if (!result.quarantineReason.empty()) {
                std::cout << "Quarantine reason: " << result.quarantineReason << std::endl;
            }
            // Actual quarantine file operation (move file) should be handled here or by CdrSanitizer.
            // For example, if CdrSanitizer only sets the flag, CdrManager could do the move:
            // if (!config.quarantineDirectory.empty()) {
            //     std::filesystem::path qPath = std::filesystem::path(config.quarantineDirectory) / std::filesystem::path(inputPath).filename();
            //     std::filesystem::rename(inputPath, qPath); // Or copy then delete
            //     result.quarantinePath = qPath.string();
            // }
        }

        if (!result.success) {
            std::cerr << fileTypeDescription << " sanitization failed for '" << inputPath << "': " << result.errorMessage << std::endl;
        } else {
            std::cout << fileTypeDescription << " ('" << inputPath << "') processed. Output: '" << result.sanitizedPath << "'." << std::endl;
            if (!result.threatsDetected.empty()) {
                std::cout << "Threats detected: " << result.threatsDetected.size() << std::endl;
            }
            if (!result.actionsPerformed.empty()) {
                std::cout << "Actions performed: ";
                for (size_t i = 0; i < result.actionsPerformed.size(); ++i) {
                    std::cout << result.actionsPerformed[i] << (i < result.actionsPerformed.size() - 1 ? ", " : "");
                }
                std::cout << std::endl;
            }
        }
        return result;

    } catch (const CdrBaseException& cdrEx) { // Catch specific CDR exceptions
        std::cerr << "CdrException during " << fileTypeDescription << " sanitization for '" << inputPath << "': " << cdrEx.what() << " (Type: " << cdrEx.getType() << ")" << std::endl;
        SanitizationResult failedResult;
        failedResult.success = false;
        failedResult.errorMessage = std::string(cdrEx.getType()) + ": " + cdrEx.what();
        failedResult.inputPath = inputPath;
        failedResult.originalPath = inputPath;
        failedResult.fileType = fileType;
        return failedResult;
    } catch (const std::exception& e) {
        std::cerr << "Std::exception during " << fileTypeDescription << " sanitization for '" << inputPath << "': " << e.what() << std::endl;
        SanitizationResult failedResult;
        failedResult.success = false;
        failedResult.errorMessage = "Exception: " + std::string(e.what());
        failedResult.inputPath = inputPath;
        failedResult.originalPath = inputPath;
        failedResult.fileType = fileType;
        return failedResult;
    }
}

SanitizationResult CdrManager::sanitizeOfficeDocument(const std::string& inputPath, const std::string& outputPath,
                                      const CdrConfiguration& config) {
    return performSanitization("Office Document", inputPath, outputPath, config, FileType::OFFICE_DOCUMENT);
}

SanitizationResult CdrManager::sanitizePdfDocument(const std::string& inputPath, const std::string& outputPath, 
                                   const CdrConfiguration& config) {
    return performSanitization("PDF Document", inputPath, outputPath, config, FileType::PDF_DOCUMENT);
}

SanitizationResult CdrManager::sanitizeHtmlDocument(const std::string& inputPath, const std::string& outputPath, 
                                    const CdrConfiguration& config) {
    return performSanitization("HTML Document", inputPath, outputPath, config, FileType::HTML_DOCUMENT);
}

SanitizationResult CdrManager::sanitizeArchiveFile(const std::string& inputPath, const std::string& outputPath, 
                                   const CdrConfiguration& config) {
    return performSanitization("Archive File", inputPath, outputPath, config, FileType::ARCHIVE_FILE);
}

SanitizationResult CdrManager::sanitizeScriptFile(const std::string& inputPath, const std::string& outputPath, 
                                  const CdrConfiguration& config) {
    return performSanitization("Script File", inputPath, outputPath, config, FileType::SCRIPT_FILE);
}

bool CdrManager::isFileTypeSupported(const std::string& filePath) {
    try {
        // Validate file exists and is readable
        if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
            return false;
        }
        
        // Check file size limits (prevent processing of extremely large files)
        auto fileSize = fs::file_size(filePath);
        constexpr size_t MAX_FILE_SIZE = 500 * 1024 * 1024; // 500MB limit
        if (fileSize > MAX_FILE_SIZE) {
            std::cerr << "File too large for processing: " << fileSize << " bytes" << std::endl;
            return false;
        }
        
        fs::Path path(filePath);
        std::string extension = path.extension();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        // Remove the dot
        if (!extension.empty() && extension[0] == '.') {
            extension = extension.substr(1);
        }
        
        // Enhanced file type detection with magic numbers
        if (extension.empty()) {
            // Try to detect by file content (magic numbers)
            return CDR::CdrManager::detectFileTypeByContent(filePath) != FileType::UNKNOWN_FILE;
        }
        
        // Supported file types with enhanced validation
        static const std::unordered_map<std::string, std::vector<std::string>> supportedTypes = {
            {"office", {"docx", "xlsx", "pptx", "doc", "xls", "ppt", "odt", "ods", "odp"}},
            {"pdf", {"pdf"}},
            {"web", {"html", "htm", "xml", "xhtml"}},
            {"text", {"txt", "csv", "rtf", "md"}},
            {"archive", {"zip", "rar", "7z", "tar", "gz", "bz2", "xz"}},
            {"image", {"jpg", "jpeg", "png", "gif", "bmp", "tiff", "svg"}},
            {"executable", {"exe", "dll", "com", "bat", "cmd", "msi", "scr"}},
            {"script", {"js", "vbs", "ps1", "sh", "py", "pl", "rb", "php"}},
            {"email", {"eml", "msg", "pst"}},
            {"media", {"mp3", "mp4", "avi", "wav", "wmv", "mov"}}
        };
        
        // Check if extension is in any supported category
        for (const auto& [category, extensions] : supportedTypes) {
            if (std::find(extensions.begin(), extensions.end(), extension) != extensions.end()) {
                // Additional validation for executable files (security concern)
                if (category == "executable") {
                    return CDR::CdrManager::validateExecutableFile(filePath);
                }
                return true;
            }
        }
        
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error checking file type support: " << e.what() << std::endl;
        return false;
    }
}

FileType CdrManager::detectFileTypeByContent(const std::string& filePath) {
    try {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return FileType::UNKNOWN_FILE;
        }
        
        // Read first few bytes for magic number detection
        char buffer[16];
        file.read(buffer, sizeof(buffer));
        auto bytesRead = file.gcount();
        
        if (bytesRead < 4) {
            return FileType::UNKNOWN_FILE;
        }
        
        // Check magic numbers
        if (bytesRead >= 4) {
            // PDF: %PDF
            if (std::memcmp(buffer, "%PDF", 4) == 0) {
                return FileType::PDF_DOCUMENT;
            }
            
            // ZIP-based formats (Office documents): PK
            if (buffer[0] == 'P' && buffer[1] == 'K') {
                return FileType::OFFICE_DOCUMENT; // Could be Office doc
            }
            
            // HTML: <!DO or <html
            if (std::memcmp(buffer, "<!DO", 4) == 0 || std::memcmp(buffer, "<htm", 4) == 0) {
                return FileType::HTML_DOCUMENT;
            }
            
            // Executable: MZ
            if (buffer[0] == 'M' && buffer[1] == 'Z') {
                return FileType::EXECUTABLE_FILE;
            }
        }
        
        return FileType::UNKNOWN_FILE;
    } catch (const std::exception&) {
        return FileType::UNKNOWN_FILE;
    }
}

bool CdrManager::validateExecutableFile(const std::string& filePath) {
    // Security check for executable files - only allow in controlled environments
    // For now, return false to quarantine all executables
    std::cerr << "Executable file detected and quarantined for security: " << filePath << std::endl;
    return false;
}

// === Single File Scanning Method ===
SanitizedFileInfo CdrManager::scanFileInContainer(const std::string& filePath, 
                                                 const std::string& containerId, 
                                                 const CdrConfiguration& config, 
                                                 bool deleteIfUnsafe) {
    SanitizedFileInfo result;
    result.originalPath = filePath;
    result.fileName = fs::Path(filePath).filename();
    result.fileExtension = fs::Path(filePath).extension();
    result.processTime = std::chrono::system_clock::now();
    
    try {
        // Validate inputs
        if (filePath.empty() || containerId.empty()) {
            throw std::invalid_argument("File path and container ID cannot be empty");
        }
        
        if (!fs::exists(filePath)) {
            throw std::invalid_argument("File does not exist: " + filePath);
        }
        
        // Get original file information
        result.originalSize = fs::file_size(filePath);
        
        // Detect file type
        FileType fileType = detectFileType(filePath);
        
        // Check if file type is supported
        if (!isFileTypeSupported(filePath)) {
            result.isSafe = false;
            result.status = "unsupported";
            result.processingLog = "File type not supported for scanning";
            
            if (deleteIfUnsafe) {
                fs::remove(filePath);
                result.isDeleted = true;
                result.status = "deleted";
                result.processingLog += " - File deleted due to unsupported type";
            }
            return result;
        }
        
        // Copy file to container for scanning
        std::string containerPath = "/cdr/scan/" + result.fileName;
        copyFileToContainer(containerId, filePath, containerPath);
        
        // Prepare sanitized output path
        std::string sanitizedFileName = "sanitized_" + result.fileName;
        std::string outputPath = std::filesystem::path(filePath).parent_path() / sanitizedFileName;
        std::string containerOutputPath = "/cdr/output/" + sanitizedFileName;
        
        // Execute CDR scanning based on file type
        std::vector<std::string> scanCommand;
        switch (fileType) {
            case FileType::PDF_DOCUMENT:
                scanCommand = {"/cdr/scan_pdf.sh", containerPath, containerOutputPath};
                break;
            case FileType::OFFICE_DOCUMENT:
                scanCommand = {"/cdr/scan_office.sh", containerPath, containerOutputPath};
                break;
            case FileType::HTML_DOCUMENT:
                scanCommand = {"/cdr/scan_html.sh", containerPath, containerOutputPath};
                break;
            case FileType::SCRIPT_FILE:
                scanCommand = {"/cdr/scan_script.sh", containerPath, containerOutputPath};
                break;
            case FileType::ARCHIVE_FILE:
                scanCommand = {"/cdr/scan_archive.sh", containerPath, containerOutputPath};
                break;
            default:
                scanCommand = {"/cdr/scan_generic.sh", containerPath, containerOutputPath};
                break;
        }
        
        // Execute scanning command with timeout
        std::string scanResult = dockerManager->executeCommandInContainer(containerId, scanCommand);
        result. rawScanOutput = scanResult; // Store raw scan output

        // Parse scan results
        bool threatsFound = scanResult.find("THREAT_DETECTED") != std::string::npos;
        bool sanitized = scanResult.find("SANITIZED") != std::string::npos;
        bool corrupted = scanResult.find("CORRUPTED") != std::string::npos;
        
        if (corrupted) {
            result.isSafe = false;
            result.status = "corrupted";
            result.processingLog = "File appears to be corrupted";
            
            if (deleteIfUnsafe) {
                std::filesystem::remove(filePath);
                result.isDeleted = true;
                result.status = "deleted";
                result.processingLog += " - File deleted due to corruption";
            }
        } else if (threatsFound) {
            result.isSafe = false;
            result.status = sanitized ? "sanitized" : "quarantined";
            // Convert char* to std::string before concatenation
            result.processingLog = std::string("Threats detected and ") + (sanitized ? "removed" : "file quarantined");
            
            if (sanitized) {
                // Copy sanitized file back from container
                copyFileFromContainer(containerId, containerOutputPath, outputPath);
                result.sanitizedPath = outputPath;
                result.sanitizedSize = std::filesystem::file_size(outputPath);
            } else if (deleteIfUnsafe) {
                // If not sanitized (i.e., quarantined) and deleteIfUnsafe is true, delete the original file.
                std::filesystem::remove(filePath);
                result.isDeleted = true;
                result.status = "deleted_quarantined"; // New status to indicate deletion after quarantine
                result.processingLog += " - Original file deleted after being quarantined";
            }
            
        } else {
            // File is clean
            result.isSafe = true;
            result.status = "clean";
            result.processingLog = "File scanned successfully - no threats detected";
        }
        
        // Calculate threat score based on findings
        if (!result.threatsFound.empty()) {
            result.threatScore = std::min(1.0, static_cast<double>(result.threatsFound.size()) * 0.2);
        } else {
            result.threatScore = 0.0;
        }
        // result.filesSanitized = sanitized ? 1 : 0; // This member doesn't exist in SanitizedFileInfo
        
        std::cout << "File scanning completed: " << filePath << " - Status: " << result.status << std::endl;
        
    } catch (const std::exception& e) {
        result.isSafe = false;
        result.status = "error";
        result.processingLog = "Scanning failed: " + std::string(e.what());
        
        if (deleteIfUnsafe) {
            try {
                std::filesystem::remove(filePath);
                result.isDeleted = true;
                result.status = "deleted";
                result.processingLog += " - File deleted due to scanning error";
            } catch (const std::exception& deleteErr) {
                result.processingLog += " - Failed to delete file: " + std::string(deleteErr.what());
            }
        }
        
        std::cerr << "Error scanning file in container: " << e.what() << std::endl;
    }
    
    return result;
}

} // namespace CDR
