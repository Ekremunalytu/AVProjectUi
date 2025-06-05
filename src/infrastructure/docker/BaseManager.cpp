#include "BaseManager.h"
#include <stdexcept>
#include <iostream>

namespace Docker {

BaseManager::BaseManager() 
    : dockerManager_(std::make_unique<DockerManager>()) {
}

BaseManager::BaseManager(std::unique_ptr<DockerManager> dockerMgr) 
    : dockerManager_(std::move(dockerMgr)) {
    if (!dockerManager_) {
        throw std::invalid_argument("DockerManager cannot be null");
    }
}

BaseManager::~BaseManager() {
    cleanupAllContainers();
}

bool BaseManager::isDaemonRunning() const {
    if (!dockerManager_) {
        return false;
    }
    try {
        return dockerManager_->isDaemonRunning();
    } catch (const std::exception& e) {
        std::cerr << "Error checking Docker daemon: " << e.what() << std::endl;
        return false;
    }
}

DockerManager* BaseManager::getDockerManager() const {
    return dockerManager_.get();
}

std::string BaseManager::createContainer(const ContainerRunConfiguration& config, const std::string& containerName) {
    std::lock_guard<std::mutex> lock(containerMutex_);
    
    if (!dockerManager_) {
        throw std::runtime_error("Docker manager not initialized");
    }
    
    // Check if container already exists
    auto it = activeContainers_.find(containerName);
    if (it != activeContainers_.end()) {
        // Container already exists, check if it's running
        if (isContainerRunning(containerName)) {
            return it->second;
        } else {
            // Container exists but not running, remove it first
            try {
                dockerManager_->removeContainer(it->second, true);
            } catch (const std::exception& e) {
                std::cerr << "Warning: Failed to remove existing container: " << e.what() << std::endl;
            }
            activeContainers_.erase(it);
        }
    }
    
    // Create new container
    auto result = dockerManager_->runNewContainer(config);
    if (result.containerId.empty()) {
        throw std::runtime_error("Failed to create container: " + containerName);
    }
    
    // Store container mapping
    activeContainers_[containerName] = result.containerId;
    
    return result.containerId;
}

std::string BaseManager::getContainerIdByName(const std::string& containerName) const {
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = activeContainers_.find(containerName);
    return (it != activeContainers_.end()) ? it->second : "";
}

bool BaseManager::isContainerRunning(const std::string& containerName) const {
    if (!dockerManager_) {
        return false;
    }
    
    std::string containerId = getContainerIdByName(containerName);
    if (containerId.empty()) {
        return false;
    }
    
    try {
        auto containerInfo = dockerManager_->getContainerDetails(containerId);
        return containerInfo.containerStatus == "running";
    } catch (const std::exception&) {
        return false;
    }
}

void BaseManager::cleanupContainer(const std::string& containerName) {
    std::lock_guard<std::mutex> lock(containerMutex_);
    
    auto it = activeContainers_.find(containerName);
    if (it == activeContainers_.end()) {
        return;
    }
    
    try {
        if (dockerManager_) {
            dockerManager_->stopContainer(it->second, 5);
            dockerManager_->removeContainer(it->second, true, true);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error cleaning up container " << containerName << ": " << e.what() << std::endl;
    }
    
    activeContainers_.erase(it);
}

void BaseManager::copyFileToContainer(const std::string& containerName, 
                                    const std::string& hostPath, 
                                    const std::string& containerPath) {
    if (!dockerManager_) {
        throw std::runtime_error("Docker manager not initialized");
    }
    
    std::string containerId = getContainerIdByName(containerName);
    if (containerId.empty()) {
        throw std::runtime_error("Container not found: " + containerName);
    }
    
    dockerManager_->copyFileToContainer(containerId, hostPath, containerPath);
}

void BaseManager::copyFileFromContainer(const std::string& containerName,
                                      const std::string& containerPath,
                                      const std::string& hostPath) {
    if (!dockerManager_) {
        throw std::runtime_error("Docker manager not initialized");
    }
    
    std::string containerId = getContainerIdByName(containerName);
    if (containerId.empty()) {
        throw std::runtime_error("Container not found: " + containerName);
    }
    
    dockerManager_->copyFileFromContainer(containerId, containerPath, hostPath);
}

std::string BaseManager::executeCommandInContainer(const std::string& containerName,
                                                 const std::vector<std::string>& commandArgs,
                                                 const std::string& user) {
    if (!dockerManager_) {
        throw std::runtime_error("Docker manager not initialized");
    }
    
    std::string containerId = getContainerIdByName(containerName);
    if (containerId.empty()) {
        throw std::runtime_error("Container not found: " + containerName);
    }
    
    return dockerManager_->executeCommandInContainer(containerId, commandArgs, user);
}

std::string BaseManager::getContainerLogs(const std::string& containerName, int tailLines) {
    if (!dockerManager_) {
        throw std::runtime_error("Docker manager not initialized");
    }
    
    std::string containerId = getContainerIdByName(containerName);
    if (containerId.empty()) {
        throw std::runtime_error("Container not found: " + containerName);
    }
    
    return dockerManager_->getContainerLogs(containerId, tailLines);
}

void BaseManager::cleanupAllContainers() {
    std::lock_guard<std::mutex> lock(containerMutex_);
    
    for (const auto& container : activeContainers_) {
        try {
            if (dockerManager_) {
                dockerManager_->stopContainer(container.second, 5);
                dockerManager_->removeContainer(container.second, true, true);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error cleaning up container " << container.first << ": " << e.what() << std::endl;
        }
    }
    
    activeContainers_.clear();
}

} // namespace Docker
