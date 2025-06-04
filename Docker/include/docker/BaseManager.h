#ifndef BASE_MANAGER_H
#define BASE_MANAGER_H

#include "DockerManager.h"
#include "DockerTypes.h"
#include "DockerExceptions.h"
#include <memory>
#include <string>
#include <map>
#include <mutex>

namespace Docker {

/**
 * @brief Base class for all specialized managers that need Docker functionality
 * 
 * This class provides a common interface and shared Docker management capabilities
 * for specialized managers like CDR and Sandbox managers. It uses composition
 * instead of inheritance to provide Docker functionality.
 */
class BaseManager {
protected:
    std::unique_ptr<DockerManager> dockerManager_;
    mutable std::mutex containerMutex_;
    std::map<std::string, std::string> activeContainers_; // name -> containerId
    
public:
    /**
     * @brief Constructor - initializes Docker manager
     */
    BaseManager();
    
    /**
     * @brief Constructor with custom Docker manager (for testing)
     */
    explicit BaseManager(std::unique_ptr<DockerManager> dockerMgr);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~BaseManager();
    
    /**
     * @brief Check if Docker daemon is running
     */
    bool isDaemonRunning() const;
    
    /**
     * @brief Get the underlying Docker manager (for advanced operations)
     */
    DockerManager* getDockerManager() const;

protected:
    /**
     * @brief Create and prepare a container with given configuration
     * @param config Container configuration
     * @param containerName Name to assign to the container
     * @return Container ID
     */
    std::string createContainer(const ContainerRunConfiguration& config, const std::string& containerName);
    
    /**
     * @brief Get container ID by name
     */
    std::string getContainerIdByName(const std::string& containerName) const;
    
    /**
     * @brief Check if container exists and is running
     */
    bool isContainerRunning(const std::string& containerName) const;
    
    /**
     * @brief Stop and remove container by name
     */
    void cleanupContainer(const std::string& containerName);
    
    /**
     * @brief Copy file to container
     */
    void copyFileToContainer(const std::string& containerName, 
                           const std::string& hostPath, 
                           const std::string& containerPath);
    
    /**
     * @brief Copy file from container
     */
    void copyFileFromContainer(const std::string& containerName,
                             const std::string& containerPath,
                             const std::string& hostPath);
    
    /**
     * @brief Execute command in container
     */
    std::string executeCommandInContainer(const std::string& containerName,
                                        const std::vector<std::string>& commandArgs,
                                        const std::string& user = "");
    
    /**
     * @brief Get container logs
     */
    std::string getContainerLogs(const std::string& containerName, int tailLines = 0);
    
    /**
     * @brief Cleanup all active containers (called by destructor)
     */
    void cleanupAllContainers();
};

} // namespace Docker

#endif // BASE_MANAGER_H
