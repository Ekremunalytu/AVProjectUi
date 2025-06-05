#include "CdrManager.h"
#include "CdrSanitizer.h"
#include "CdrTypes.h" // For CdrConfigurationException, CdrFileException etc.
#include "../../infrastructure/docker/DockerExceptions.h" // Include for Docker::OperationException etc.
#include "FilesystemCompat.h"
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

// Security configuration constants
namespace Security {
    const std::string CDR_IMAGE_NAME = "testdisk:v1.2.3";  // Version pinned
    const std::string SANDBOX_IMAGE_NAME = "ubuntu:20.04"; // LTS version
    const std::string ALLOWED_MOUNT_PREFIX = "/tmp/cdr_";   // Restricted mount paths
    const size_t MAX_PATH_LENGTH = 4096;                   // Increased from 255 to 4096 for longer paths
    
    // Enhanced path validation function
    bool isSecurePath(const std::string& path) {
        if (path.empty() || path.length() > MAX_PATH_LENGTH) return false;
        
        // Prevent path traversal attacks
        if (path.find("..") != std::string::npos) return false;
        if (path.find("//") != std::string::npos) return false;
        
        // Check for null bytes
        if (path.find('\0') != std::string::npos) return false;
        
        // For absolute paths, ensure they start with allowed prefix
        if (path[0] == '/') {
            return path.find(ALLOWED_MOUNT_PREFIX) == 0;
        }
        
        // For relative paths, check they don't contain dangerous patterns
        const std::vector<std::string> dangerous_patterns = {
            "../", "./", "~/", "$HOME", "%USERPROFILE%"
        };
        
        for (const auto& pattern : dangerous_patterns) {
            if (path.find(pattern) != std::string::npos) {
                return false;
            }
        }
        
        return true;
    }
    
    // Validate and normalize directory paths
    std::string normalizePath(const std::string& path) {
        if (path.empty()) return path;
        
        std::string normalized = path;
        
        // Remove trailing slashes except for root
        while (normalized.length() > 1 && normalized.back() == '/') {
            normalized.pop_back();
        }
        
        // Ensure directory paths exist or can be created
        try {
            CDR::FileSystem::Path pathObj(normalized);
            if (!CDR::FileSystem::exists(pathObj)) {
                CDR::FileSystem::create_directories(pathObj);
            }
        } catch (const CDR::FileSystem::FilesystemError& e) {
            throw CDR::CdrConfigurationException("Cannot create or access directory: " + normalized + " - " + e.what());
        }
        
        return normalized;
    }
}


// --- Constructor & Destructor ---

CdrManager::CdrManager() 
    : Docker::DockerManager(),  // Call parent constructor
      cdrContainerImage(Security::CDR_IMAGE_NAME),
      sandboxContainerImage(Security::SANDBOX_IMAGE_NAME),
      keepThreadsJoined_(true)
{
    std::cout << "CdrManager initialized with default Docker manager" << std::endl;
}

CdrManager::CdrManager(std::unique_ptr<Docker::DockerManager> dockerMgr)
    : Docker::DockerManager(),  // Call parent constructor
      cdrContainerImage(Security::CDR_IMAGE_NAME),
      sandboxContainerImage(Security::SANDBOX_IMAGE_NAME),
      keepThreadsJoined_(true)
{
    // Copy the passed DockerManager's state if needed
    // For now, just use the parent's functionality
    std::cout << "CdrManager initialized with custom Docker manager" << std::endl;
}

CdrManager::~CdrManager() {
    keepThreadsJoined_ = false; // Signal threads to stop cooperatively
    
    // Stop all active analyses
    std::vector<std::string> activeAnalysisIds;
    {
        std::lock_guard<std::mutex> lock(analysesMutex);
        for (const auto& pair : activeAnalyses) {
            activeAnalysisIds.push_back(pair.first);
        }
    }
    
    // Stop analyses without holding the mutex
    for (const auto& analysisId : activeAnalysisIds) {
        try {
            stopAnalysis(analysisId);
        } catch (const std::exception& e) {
            std::cerr << "Error stopping analysis " << analysisId << " during cleanup: " << e.what() << std::endl;
        }
    }
    
    // Final cleanup of any remaining threads with timeout
    const auto timeout = std::chrono::seconds(5);
    for (auto& pair : activeAnalysesThreads_) {
        if (pair.second.joinable()) {
            try {
                // Use detach as a fallback if join times out
                pair.second.join();
            } catch (const std::exception& e) {
                std::cerr << "Error joining thread " << pair.first << ": " << e.what() << std::endl;
                // Detach as last resort to prevent resource leaks
                try {
                    pair.second.detach();
                } catch (...) {
                    // Thread may have already finished
                }
            }
        }
    }
    activeAnalysesThreads_.clear();
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
    // Enhanced security validation for all directory paths
    std::vector<std::pair<std::string, std::string>> pathsToValidate = {
        {"input", config.inputDirectory},
        {"output", config.outputDirectory},
        {"quarantine", config.quarantineDirectory}
    };
    
    for (const auto& pathPair : pathsToValidate) {
        if (!Security::isSecurePath(pathPair.second)) {
            throw CDR::CdrConfigurationException("Invalid or insecure " + pathPair.first + " directory path: " + pathPair.second);
        }
    }

    // Normalize and validate directory paths
    std::string normalizedInputDir = Security::normalizePath(config.inputDirectory);
    std::string normalizedOutputDir = Security::normalizePath(config.outputDirectory);
    std::string normalizedQuarantineDir = Security::normalizePath(config.quarantineDirectory);

    // Thread-safe container configuration
    std::lock_guard<std::mutex> lock(containerMutex_);

    // CDR konteyner konfigürasyonu hazırla
    Docker::ContainerRunConfiguration containerConfig;
    containerConfig.imageName = cdrContainerImage;
    containerConfig.containerName = "cdr_container_" + generateAnalysisId();
    
    // Mount points with normalized paths
    Docker::MountPoint inputMount;
    inputMount.mountType = "bind";
    inputMount.source = normalizedInputDir;
    inputMount.destination = "/cdr/input";
    inputMount.readOnly = true; // Input dizini sadece okunabilir
    containerConfig.hostConfig.mounts.push_back(inputMount);
    
    Docker::MountPoint outputMount;
    outputMount.mountType = "bind";
    outputMount.source = normalizedOutputDir;
    outputMount.destination = "/cdr/output";
    outputMount.readOnly = false;
    containerConfig.hostConfig.mounts.push_back(outputMount);

    Docker::MountPoint quarantineMount;
    quarantineMount.mountType = "bind";
    quarantineMount.source = normalizedQuarantineDir;
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
        auto result = runNewContainer(containerConfig);
        if (result.status == "Error") {
            throw Docker::OperationException("runNewContainer", "Failed to start CDR container: " + result.errorMessage);
        }
        return result.containerId;
    } catch (const Docker::DockerException& e) {
        throw;
    } catch (const std::exception& e) {
        throw Docker::OperationException("prepareCdrContainer", "Failed to prepare CDR container: " + std::string(e.what()));
    }
}

std::string CdrManager::prepareSandboxContainer() {
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
        auto result = runNewContainer(containerConfig);  // Use inherited method
        if (result.status == "Error") {
            throw Docker::OperationException("runNewContainer", "Failed to start sandbox container: " + result.errorMessage);
        }
        
        // Schedule automatic cleanup with better error handling
        std::thread([this, containerId = result.containerId]() {
            const auto cleanupDelay = std::chrono::minutes(5);
            std::this_thread::sleep_for(cleanupDelay);
            
            if (!keepThreadsJoined_) {
                return; // Manager is being destroyed
            }
            
            try {
                std::lock_guard<std::mutex> lock(containerMutex_);
                if (keepThreadsJoined_) {
                    // Force stop container with timeout
                    stopContainer(containerId);
                    // Remove container and its volumes
                    removeContainer(containerId, true);
                    std::cout << "Sandbox container " << containerId << " cleaned up successfully" << std::endl;
                }
            } catch (const Docker::DockerException& e) {
                std::cerr << "Docker error during sandbox cleanup for " << containerId 
                         << ": " << e.what() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error during sandbox cleanup for " << containerId 
                         << ": " << e.what() << std::endl;
            }
        }).detach();
        
        return result.containerId;
    } catch (const Docker::DockerException& e) {
        throw;
    } catch (const std::exception& e) {
        throw Docker::OperationException("prepareSandboxContainer", "Failed to prepare sandbox container: " + std::string(e.what()));
    }
}

namespace Validation {
    const size_t MAX_FILE_SIZE = 100 * 1024 * 1024;  // 100MB max file size
    const size_t MIN_FILE_SIZE = 1;                   // 1 byte minimum
    
    bool isValidFilePath(const std::string& filePath) {
        if (filePath.empty() || filePath.length() > Security::MAX_PATH_LENGTH) {
            return false;
        }
        
        // Check for dangerous characters
        const std::string dangerous_chars = "<>:\"|?*";
        for (char c : dangerous_chars) {
            if (filePath.find(c) != std::string::npos) {
                return false;
            }
        }
        
        return Security::isSecurePath(filePath);
    }
    
    bool isValidFileSize(const std::string& filePath) {
        try {
            if (!fs::exists(filePath)) return false;
            
            auto size = fs::file_size(filePath);
            return size >= MIN_FILE_SIZE && size <= MAX_FILE_SIZE;
        } catch (...) {
            return false;
        }
    }
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
    std::unique_lock<std::mutex> lock(analysesMutex);
    
    auto it = activeAnalyses.find(analysisId);
    if (it == activeAnalyses.end()) {
        throw CDR::CdrBaseException("Analysis ID not found for stopping: " + analysisId);
    }

    // Check if already stopping or stopped
    if (it->second.status == "stopping" || it->second.status == "stopped") {
        return; // Already in process or completed
    }

    // Mark for stopping
    it->second.status = "stopping";
    
    // Find and extract the thread
    std::thread threadToJoin;
    auto threadIt = activeAnalysesThreads_.find(analysisId);
    if (threadIt != activeAnalysesThreads_.end()) {
        threadToJoin = std::move(threadIt->second);
        activeAnalysesThreads_.erase(threadIt);
    }
    
    // Release lock before joining thread to prevent deadlock
    lock.unlock();    // Join thread if it exists
    if (threadToJoin.joinable()) {
        try {
            threadToJoin.join();
        } catch (const std::exception& e) {
            std::cerr << "Error joining analysis thread: " << e.what() << std::endl;
        }
    }

    // Re-acquire lock for final status update
    lock.lock();
    
    try {
        auto it = activeAnalyses.find(analysisId);
        if (it != activeAnalyses.end()) {
            // Stop container if it exists
            if (it->second.metadata.count("containerId")) {
                std::string containerId = it->second.metadata["containerId"];
                if (!containerId.empty()) {
                    stopContainer(containerId);
                    removeContainer(containerId, true);
                }
            }

            // Update final status
            it->second.status = "stopped";
            it->second.endTime = std::chrono::system_clock::now();
        }

    } catch (const std::exception& e) {
        std::cerr << "Error stopping analysis containers: " << e.what() << std::endl;
        // Revert status if stopping failed critically
        auto it = activeAnalyses.find(analysisId);
        if (it != activeAnalyses.end() && it->second.status == "stopping") {
            it->second.status = "failed_to_stop";
            it->second.errorMessage = e.what();
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
        throw CDR::CdrBaseException("Analysis ID not found for exportRecoveredFile: " + analysisId);
    }
    
    try {
        std::string containerId = it->second.metadata["containerId"];
        if (containerId.empty()) {
            return false;
        }
        
        // Use inherited copyFileFromContainer method
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
    if (!fs::exists(filePath)) {
        throw CDR::CdrFileException("File to execute in sandbox does not exist", filePath);
    }
    
    try {
        // Use inherited copyFileToContainer method
        std::string containerPath = "/tmp/suspicious_file";
        copyFileToContainer(sandboxId, filePath, containerPath);
        
        // Execute file with inherited executeCommandInContainer method
        std::vector<std::string> execCommand = {
            "timeout", "30s",
            "strace", "-e", "trace=file,process,network",
            containerPath
        };
        
        executionResult = executeCommandInContainer(sandboxId, execCommand);
        
        return true;
    } catch (const std::exception& e) {
        executionResult = "Execution failed: " + std::string(e.what());
        return false;
    }
}

void CDR::CdrManager::destroySandboxEnvironment(const std::string& sandboxId) {
    try {
        stopContainer(sandboxId);        // Use inherited method
        removeContainer(sandboxId, true); // Use inherited method
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
    try {
        // Use inherited methods
        if (!isDaemonRunning()) {
            return false;
        }
        
        if (!imageExists(cdrContainerImage)) {
            std::cerr << "CDR image not found: " << cdrContainerImage << std::endl;
            return false;
        }
        
        if (!imageExists(sandboxContainerImage)) {
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
        case FileType::TEXT_DOCUMENT:
            info.extension = ".txt";
            info.commonExtensions = {".txt", ".log", ".csv", ".md", ".json"};
            info.supportedTools = {"TextSanitizer", "EncodingValidator"};
            info.description = "Plain Text File";
            info.activeContentTypes = {};
            break;
        case FileType::EMAIL_FILE:
            info.extension = ".eml";
            info.commonExtensions = {".eml", ".msg"};
            info.supportedTools = {"EmailSanitizer", "AttachmentExtractor", "HeaderAnalyzer"};
            info.description = "Email File (.eml, .msg)";
            info.activeContentTypes = {"Attachments", "HTML Body Scripts", "Tracking Pixels", "Phishing Links"};
            info.requiresSpecialHandling = true;
            break;
        case FileType::NOT_SET:
        case FileType::UNKNOWN_FILE:
        default:
            info.extension = ".dat"; 
            info.commonExtensions = {};
            info.supportedTools = {"GenericAnalyzer", "HeuristicScanner"};
            info.description = "Unknown or Uncategorized File Type";
            info.activeContentTypes = {};
            info.requiresSpecialHandling = true; 
            break;
    }
    return info;
}

// --- Scanning Files in Containers ---

SanitizedFileInfo CdrManager::scanFileInContainer(const std::string& filePath, 
                                                  const std::string& containerId, 
                                                  const CdrConfiguration& config, 
                                                  bool deleteIfUnsafe) {
    SanitizedFileInfo fileInfo;
    
    try {
        // Initialize file info structure with proper validation
        fileInfo.originalPath = filePath;
        fileInfo.fileName = fs::Path(filePath).filename();
        fileInfo.fileExtension = fs::Path(filePath).extension();
        fileInfo.originalSize = 0;
        fileInfo.sanitizedSize = 0;
        fileInfo.threatScore = 0.0;
        fileInfo.isSafe = true;
        fileInfo.isDeleted = false;
        fileInfo.processTime = std::chrono::system_clock::now();

        // Validate input file exists and is accessible
        if (!fs::exists(filePath)) {
            fileInfo.status = "failed";
            fileInfo.processingLog = "File not found: " + filePath;
            fileInfo.isSafe = false;
            return fileInfo;
        }

        // Check if file is readable
        std::ifstream testFile(filePath);
        if (!testFile.good()) {
            fileInfo.status = "failed";
            fileInfo.processingLog = "File not readable: " + filePath;
            fileInfo.isSafe = false;
            return fileInfo;
        }
        testFile.close();

        fileInfo.originalSize = static_cast<long long>(fs::file_size(filePath));
        fileInfo.md5Original = FileSanitizer::calculateMD5(filePath);

        // Detect file type BEFORE processing
        FileType fileType = detectFileTypeByContent(filePath);
        if (fileType == FileType::UNKNOWN_FILE && !config.allowUnknownTypes) {
            fileInfo.status = "blocked";
            fileInfo.isSafe = false;
            fileInfo.threatScore = 0.5;
            fileInfo.processingLog = "Unknown file type blocked by policy";
            
            if (deleteIfUnsafe) {
                try {
                    fs::remove(filePath);
                    fileInfo.isDeleted = true;
                    fileInfo.processingLog += " | Original file deleted";
                } catch (const fs::FilesystemError& e) {
                    fileInfo.processingLog += " | Failed to delete file: " + std::string(e.what());
                }
            }
            return fileInfo;
        }

        // Validate file size before processing
        if (fileInfo.originalSize > config.maxFileSizeMB * 1024 * 1024) {
            fileInfo.status = "blocked";
            fileInfo.isSafe = false;
            fileInfo.threatScore = 0.3;
            fileInfo.processingLog = "File size exceeds limit: " + std::to_string(fileInfo.originalSize) + " bytes";
            
            if (deleteIfUnsafe) {
                try {
                    fs::remove(filePath);
                    fileInfo.isDeleted = true;
                    fileInfo.processingLog += " | Original file deleted due to size";
                } catch (const fs::FilesystemError& e) {
                    fileInfo.processingLog += " | Failed to delete file: " + std::string(e.what());
                }
            }
            return fileInfo;
        }
        
        // Prepare container paths with proper validation
        std::string containerInputPath = "/input/" + fileInfo.fileName;
        std::string containerOutputPath = "/output/" + fileInfo.fileName;
        std::string hostOutputPath = config.outputDirectory + "/" + fileInfo.fileName;

        // Ensure output directory exists
        fs::Path outputDir(config.outputDirectory);
        if (!fs::exists(outputDir)) {
            try {
                fs::create_directories(outputDir);
            } catch (const fs::FilesystemError& e) {
                fileInfo.status = "failed";
                fileInfo.processingLog = "Cannot create output directory: " + std::string(e.what());
                fileInfo.isSafe = false;
                return fileInfo;
            }
        }

        // Copy file to container with error handling
        try {
            copyFileToContainer(containerId, filePath, containerInputPath);
        } catch (const std::exception& e) {
            fileInfo.status = "failed";
            fileInfo.processingLog = "Failed to copy file to container: " + std::string(e.what());
            fileInfo.isSafe = false;
            return fileInfo;
        }

        // Create sanitization configuration for container
        CdrConfiguration containerConfig = config;
        containerConfig.inputDirectory = "/input";
        containerConfig.outputDirectory = "/output";
        containerConfig.quarantineDirectory = "/quarantine";

        // Execute sanitization in container with proper file type
        CdrSanitizer sanitizer;
        SanitizationResult result = sanitizer.sanitizeFile(containerInputPath, containerOutputPath, containerConfig, fileType);

        // Process results with enhanced validation
        fileInfo.threatsFound = result.threatsDetected;
        fileInfo.removedElements = result.actionsPerformed;
        fileInfo.processingLog = result.errorMessage.empty() ? "Processing completed successfully" : result.errorMessage;

        if (result.success) {
            if (result.requiresQuarantine) {
                // Handle quarantine case with proper path construction
                fileInfo.status = "quarantined";
                fileInfo.quarantinePath = config.quarantineDirectory + "/" + fileInfo.fileName;
                fileInfo.isSafe = false;
                fileInfo.threatScore = std::min(1.0, static_cast<double>(result.threatsDetected.size()) * 0.3);

                // Ensure quarantine directory exists
                fs::Path quarantineDir(config.quarantineDirectory);
                if (!fs::exists(quarantineDir)) {
                    try {
                        fs::create_directories(quarantineDir);
                    } catch (const fs::FilesystemError& e) {
                        fileInfo.processingLog += " | Failed to create quarantine directory: " + std::string(e.what());
                    }
                }

                try {
                    copyFileFromContainer(containerId, "/quarantine/" + fileInfo.fileName, fileInfo.quarantinePath);
                } catch (const std::exception& e) {
                    fileInfo.processingLog += " | Failed to copy file from quarantine: " + std::string(e.what());
                }

                if (deleteIfUnsafe) {
                    try {
                        fs::remove(filePath);
                        fileInfo.isDeleted = true;
                        fileInfo.processingLog += " | Original file deleted due to quarantine";
                    } catch (const fs::FilesystemError& e) {
                        fileInfo.processingLog += " | Failed to delete original file: " + std::string(e.what());
                    }
                }
            } else {
                // Handle successful sanitization with proper output path
                fileInfo.status = result.threatsDetected.empty() ? "clean" : "sanitized";
                fileInfo.sanitizedPath = hostOutputPath;
                fileInfo.isSafe = true;
                fileInfo.threatScore = static_cast<double>(result.threatsDetected.size()) * 0.1;

                try {
                    copyFileFromContainer(containerId, containerOutputPath, hostOutputPath);
                    
                    if (fs::exists(hostOutputPath)) {
                        fileInfo.sanitizedSize = static_cast<long long>(fs::file_size(hostOutputPath));
                        fileInfo.md5Sanitized = FileSanitizer::calculateMD5(hostOutputPath);
                    } else {
                        fileInfo.processingLog += " | Warning: Sanitized file not found in output";
                        fileInfo.md5Sanitized = fileInfo.md5Original;
                        fileInfo.sanitizedSize = fileInfo.originalSize;
                    }
                } catch (const std::exception& e) {
                    fileInfo.status = "failed";
                    fileInfo.processingLog += " | Failed to retrieve sanitized file: " + std::string(e.what());
                    fileInfo.isSafe = false;
                }
            }
        } else {
            // Handle failed sanitization
            fileInfo.status = "failed";
            fileInfo.isSafe = false;
            fileInfo.threatScore = 0.5;
            fileInfo.processingLog = "Sanitization failed: " + result.errorMessage;

            if (deleteIfUnsafe) {
                try {
                    fs::remove(filePath);
                    fileInfo.isDeleted = true;
                    fileInfo.processingLog += " | Original file deleted due to processing failure";
                } catch (const fs::FilesystemError& e) {
                    fileInfo.processingLog += " | Failed to delete original file: " + std::string(e.what());
                }
            }
        }

        // Capture enhanced scan output
        fileInfo.rawScanOutput = "Container ID: " + containerId + "\n" +
                                "File Type: " + getFileTypeName(fileType) + "\n" +
                                "Original Size: " + std::to_string(fileInfo.originalSize) + " bytes\n" +
                                "Threats Detected: " + std::to_string(result.threatsDetected.size()) + "\n" +
                                "Actions Performed: " + std::to_string(result.actionsPerformed.size()) + "\n" +
                                "Processing Status: " + fileInfo.status;

    } catch (const CdrBaseException& cdrEx) {
        fileInfo.status = "failed";
        fileInfo.isSafe = false;
        fileInfo.threatScore = 0.7;
        fileInfo.processingLog = std::string(cdrEx.getType()) + ": " + cdrEx.what();
        
        if (deleteIfUnsafe) {
            try {
                fs::remove(filePath);
                fileInfo.isDeleted = true;
                fileInfo.processingLog += " | Original file deleted due to CDR exception";
            } catch (const fs::FilesystemError& e) {
                fileInfo.processingLog += " | Failed to delete original file: " + std::string(e.what());
            }
        }
    } catch (const std::exception& e) {
        fileInfo.status = "failed";
        fileInfo.isSafe = false;
        fileInfo.threatScore = 0.8;
        fileInfo.processingLog = "Exception during container processing: " + std::string(e.what());
        
        if (deleteIfUnsafe) {
            try {
                fs::remove(filePath);
                fileInfo.isDeleted = true;
                fileInfo.processingLog += " | Original file deleted due to exception";
            } catch (const fs::FilesystemError& e) {
                fileInfo.processingLog += " | Failed to delete original file: " + std::string(e.what());
            }
        }
    }

    return fileInfo;
}

// --- Parsing Sanitization Results ---

std::vector<SanitizedFileInfo> CdrManager::parseSanitizationResults(const std::string& resultsPath) {
    std::vector<SanitizedFileInfo> results;
    
    try {
        if (!fs::exists(resultsPath)) {
            std::cerr << "Results file not found: " << resultsPath << std::endl;
            return results;
        }

        std::ifstream resultsFile(resultsPath);
        if (!resultsFile.is_open()) {
            std::cerr << "Failed to open results file: " << resultsPath << std::endl;
            return results;
        }

        std::string line;
        SanitizedFileInfo currentFile;
        bool inFileBlock = false;

        while (std::getline(resultsFile, line)) {
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty()) continue;            // Parse different types of result lines
            if (line.substr(0, 11) == "FILE_START:") {
                if (inFileBlock) {
                    // Save previous file info
                    results.push_back(currentFile);
                }
                // Start new file block
                currentFile = SanitizedFileInfo();
                currentFile.originalPath = line.substr(11); // Remove "FILE_START:"
                currentFile.fileName = fs::Path(currentFile.originalPath).filename();
                currentFile.fileExtension = fs::Path(currentFile.originalPath).extension();
                currentFile.processTime = std::chrono::system_clock::now();
                inFileBlock = true;            }
            else if (line.substr(0, 8) == "FILE_END" && inFileBlock) {
                results.push_back(currentFile);
                inFileBlock = false;
                currentFile = SanitizedFileInfo();
            }
            else if (inFileBlock) {
                // Parse file attributes
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string key = line.substr(0, colonPos);
                    std::string value = line.substr(colonPos + 1);
                    
                    // Remove leading/trailing whitespace from value
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);

                    if (key == "SANITIZED_PATH") {
                        currentFile.sanitizedPath = value;
                    }
                    else if (key == "QUARANTINE_PATH") {
                        currentFile.quarantinePath = value;
                    }
                    else if (key == "ORIGINAL_SIZE") {
                        try {
                            currentFile.originalSize = std::stoll(value);
                        } catch (const std::exception&) {
                            currentFile.originalSize = 0;
                        }
                    }
                    else if (key == "SANITIZED_SIZE") {
                        try {
                            currentFile.sanitizedSize = std::stoll(value);
                        } catch (const std::exception&) {
                            currentFile.sanitizedSize = 0;
                        }
                    }
                    else if (key == "MD5_ORIGINAL") {
                        currentFile.md5Original = value;
                    }
                    else if (key == "MD5_SANITIZED") {
                        currentFile.md5Sanitized = value;
                    }
                    else if (key == "STATUS") {
                        currentFile.status = value;
                        currentFile.isSafe = (value == "clean" || value == "sanitized");
                        currentFile.isDeleted = (value == "deleted");
                    }
                    else if (key == "THREAT_SCORE") {
                        try {
                            currentFile.threatScore = std::stod(value);
                        } catch (const std::exception&) {
                            currentFile.threatScore = 0.0;
                        }
                    }
                    else if (key == "THREATS_FOUND") {
                        // Parse comma-separated list
                        std::stringstream ss(value);
                        std::string threat;
                        while (std::getline(ss, threat, ',')) {
                            threat.erase(0, threat.find_first_not_of(" \t"));
                            threat.erase(threat.find_last_not_of(" \t") + 1);
                            if (!threat.empty()) {
                                currentFile.threatsFound.push_back(threat);
                            }
                        }
                    }
                    else if (key == "REMOVED_ELEMENTS") {
                        // Parse comma-separated list
                        std::stringstream ss(value);
                        std::string element;
                        while (std::getline(ss, element, ',')) {
                            element.erase(0, element.find_first_not_of(" \t"));
                            element.erase(element.find_last_not_of(" \t") + 1);
                            if (!element.empty()) {
                                currentFile.removedElements.push_back(element);
                            }
                        }
                    }
                    else if (key == "PROCESSING_LOG") {
                        currentFile.processingLog = value;
                    }
                    else if (key == "RAW_SCAN_OUTPUT") {
                        currentFile.rawScanOutput = value;
                    }
                    else if (key == "PROCESS_TIME") {
                        // Parse ISO 8601 timestamp (basic implementation)
                        // For simplicity, just set to current time
                        currentFile.processTime = std::chrono::system_clock::now();
                    }
                }
            }
        }

        // Handle last file if file doesn't end with FILE_END
        if (inFileBlock) {
            results.push_back(currentFile);
        }

        resultsFile.close();
        
        std::cout << "Parsed " << results.size() << " file results from " << resultsPath << std::endl;

    } catch (const fs::FilesystemError& fsErr) {
        std::cerr << "Filesystem error parsing results: " << fsErr.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing sanitization results: " << e.what() << std::endl;
    }

    return results;
}

// --- File Operations (Add these methods) ---

void CdrManager::copyFileToContainer(const std::string& containerId,
                                   const std::string& hostPath,
                                   const std::string& containerPath) {
    // Use inherited DockerManager functionality
    Docker::DockerManager::copyFileToContainer(containerId, hostPath, containerPath);
}

void CdrManager::copyFileFromContainer(const std::string& containerId,
                                     const std::string& containerPath,
                                     const std::string& hostPath) {
    // Use inherited DockerManager functionality  
    Docker::DockerManager::copyFileFromContainer(containerId, containerPath, hostPath);
}

CDR::FileType CdrManager::detectFileTypeByContent(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return FileType::UNKNOWN_FILE;
    }
    
    std::array<unsigned char, 16> header{};
    file.read(reinterpret_cast<char*>(header.data()), header.size());
    
    // PDF signature
    if (header[0] == '%' && header[1] == 'P' && header[2] == 'D' && header[3] == 'F') {
        return FileType::PDF_DOCUMENT;
    }
    
    // ZIP-based formats (Office documents)
    if (header[0] == 0x50 && header[1] == 0x4B && header[2] == 0x03 && header[3] == 0x04) {
        // Further check for Office documents by extension
        std::string extension = fs::Path(filePath).extension();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        if (extension == ".docx" || extension == ".xlsx" || extension == ".pptx" ||
            extension == ".doc" || extension == ".xls" || extension == ".ppt") {
            return FileType::OFFICE_DOCUMENT;
        }
        return FileType::ARCHIVE_FILE;
    }
    
    // HTML signature
    std::string headerStr(reinterpret_cast<char*>(header.data()), 15);
    std::transform(headerStr.begin(), headerStr.end(), headerStr.begin(), ::tolower);
    if (headerStr.find("<!doc") == 0 || headerStr.find("<html") == 0 || headerStr.find("<?xml") == 0) {
        if (headerStr.find("<html") != std::string::npos) {
            return FileType::HTML_DOCUMENT;
        } else if (headerStr.find("<?xml") == 0) {
            return FileType::XML_DOCUMENT;
        }
    }
    
    // RTF signature
    if (header[0] == '{' && header[1] == '\\' && header[2] == 'r' && header[3] == 't' && header[4] == 'f') {
        return FileType::RTF_DOCUMENT;
    }
    
    // Executable signatures
    if (header[0] == 0x4D && header[1] == 0x5A) { // MZ header
        return FileType::EXECUTABLE_FILE;
    }
    
    // ELF signature (Linux executables)
    if (header[0] == 0x7F && header[1] == 0x45 && header[2] == 0x4C && header[3] == 0x46) {
        return FileType::EXECUTABLE_FILE;
    }
      // Fallback to extension-based detection
    return detectFileType(filePath);
}

// === Missing Public Method Implementations ===

bool CdrManager::isFileTypeSupported(const std::string& filePath) {
    FileType type = detectFileType(filePath);
    return type != FileType::UNKNOWN_FILE && type != FileType::NOT_SET;
}

std::vector<std::string> CdrManager::detectActiveContent(const std::string& filePath) {
    std::vector<std::string> activeContent;
    
    try {
        FileType type = detectFileType(filePath);
        FileTypeInfo typeInfo = getFileTypeInfo(type);
        
        // Return the known active content types for this file type
        activeContent = typeInfo.activeContentTypes;
        
        // Add specific detection for certain file types
        switch (type) {
            case FileType::PDF_DOCUMENT:
                if (detectPdfJavaScript(filePath)) {
                    activeContent.push_back("JavaScript_Detected");
                }
                break;
            case FileType::OFFICE_DOCUMENT:
                if (detectOfficeMacros(filePath)) {
                    activeContent.push_back("Macros_Detected");
                }
                break;
            case FileType::HTML_DOCUMENT:
                // Basic HTML script detection could be added here
                break;
            default:
                break;
        }
    } catch (const std::exception& e) {
        activeContent.push_back("Detection_Error");
    }
    
    return activeContent;
}

SanitizationResult CdrManager::sanitizeOfficeDocument(const std::string& filePath, 
                                                     const std::string& outputPath, 
                                                     const CdrConfiguration& config) {
    CdrSanitizer sanitizer;
    return sanitizer.sanitizeOfficeFile(filePath, outputPath, config);
}

SanitizationResult CdrManager::sanitizePdfDocument(const std::string& filePath, 
                                                   const std::string& outputPath, 
                                                   const CdrConfiguration& config) {
    CdrSanitizer sanitizer;
    return sanitizer.sanitizePdfFile(filePath, outputPath, config);
}

SanitizationResult CdrManager::sanitizeHtmlDocument(const std::string& filePath, 
                                                    const std::string& outputPath, 
                                                    const CdrConfiguration& config) {
    CdrSanitizer sanitizer;
    return sanitizer.sanitizeHtmlFile(filePath, outputPath, config);
}

SanitizationResult CdrManager::sanitizeArchiveFile(const std::string& filePath, 
                                                   const std::string& outputPath, 
                                                   const CdrConfiguration& config) {
    CdrSanitizer sanitizer;
    return sanitizer.sanitizeArchiveFile(filePath, outputPath, config);
}

SanitizationResult CdrManager::sanitizeScriptFile(const std::string& filePath, 
                                                  const std::string& outputPath, 
                                                  const CdrConfiguration& config) {
    CdrSanitizer sanitizer;
    return sanitizer.sanitizeScriptFile(filePath, outputPath, config);
}

SanitizationResult CdrManager::performSanitization(const std::string& fileTypeDescription,
                                                   const std::string& inputPath,
                                                   const std::string& outputPath,
                                                   const CdrConfiguration& config,
                                                   FileType fileType) {
    std::cout << "[CdrManager] Performing sanitization for " << fileTypeDescription 
              << ": " << inputPath << " -> " << outputPath << std::endl;
    
    try {
        // Create a CdrSanitizer instance and use it to sanitize the file
        CdrSanitizer sanitizer;
        return sanitizer.sanitizeFile(inputPath, outputPath, config, fileType);
        
    } catch (const std::exception& e) {
        SanitizationResult result;
        result.success = false;
        result.errorMessage = "Sanitization failed: " + std::string(e.what());
        result.inputPath = inputPath;
        result.outputPath = outputPath;
        result.fileType = fileType;
        std::cerr << "[CdrManager] Sanitization error: " << e.what() << std::endl;
        return result;
    }
}

bool CdrManager::sanitizeFile(const std::string& inputPath, const std::string& outputPath,
                              const CdrConfiguration& config) {
    try {
        // Detect file type first
        FileType fileType = detectFileType(inputPath);
        
        // Use the performSanitization helper method
        SanitizationResult result = performSanitization("Generic File", inputPath, outputPath, config, fileType);
        
        // Return true if sanitization was successful
        return result.success;
    } catch (const std::exception& e) {
        std::cerr << "Exception during file sanitization for '" << inputPath << "': " << e.what() << std::endl;
        return false;
    }
}

bool CdrManager::quarantineFile(const std::string& filePath, const std::string& quarantinePath) {
    try {
        // Ensure quarantine directory exists
        fs::Path quarantineDir = fs::Path(quarantinePath).parent_path();
        if (!fs::exists(quarantineDir)) {
            fs::create_directories(quarantineDir);
        }
          // Copy file to quarantine location
        if (fs::exists(filePath)) {
            fs::copy_file(filePath, quarantinePath, true); // true = overwrite existing
            return true;
        }
        return false;
    } catch (const fs::FilesystemError& e) {
        return false;
    } catch (const std::exception& e) {
        return false;
    }
}

} // namespace CDR