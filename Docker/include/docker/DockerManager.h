/**
 * @file DockerManager.h
 * @brief Docker container and image management interface
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 */

#ifndef DOCKER_MANAGER_H
#define DOCKER_MANAGER_H

#include <string>
#include <vector>
#include <utility> // Required for std::pair, used by private helper executeCliCommand

// Custom type definitions for Docker entities (e.g., ContainerInfo, ImageInfoBasic).
#include "DockerTypes.h"
// Custom exception classes for Docker-specific errors.
#include "DockerExceptions.h"

// Namespace for all Docker-related components of the project.
namespace Docker {

/**
 * @brief Provides an interface to manage Docker resources such as containers and images
 * 
 * @details The DockerManager class encapsulates Docker CLI command execution and data parsing,
 * offering a structured way to interact with Docker infrastructure. It provides high-level
 * methods for container lifecycle management, image operations, and system monitoring.
 * 
 * Key features:
 * - Container lifecycle management (create, start, stop, remove)
 * - Image management and repository operations
 * - Network configuration and management
 * - Volume management for persistent storage
 * - Resource monitoring and logging
 * - Docker daemon health checks
 * 
 * @note This class is designed to return data objects; presentation of this data 
 *       is left to the client code.
 * 
 * @warning Requires Docker to be installed and the Docker daemon to be running
 *          on the host system.
 */
class DockerManager {
public:
    /**
     * @brief Constructor: Initializes a new DockerManager instance
     * 
     * Performs preliminary checks to ensure Docker is available and accessible.
     * May configure default settings and validate Docker daemon connectivity.
     */
    DockerManager();

    /**
     * @brief Destructor: Cleans up resources if any were acquired by the DockerManager
     * 
     * Ensures proper cleanup of any resources or connections established during
     * the DockerManager's lifetime. Default implementation is suitable if no 
     * specific cleanup is needed.
     */
    ~DockerManager() = default;

    // --- Docker Daemon Utilities ---

    // Checks if the Docker daemon is running and responsive.
    // Returns true if the daemon is accessible, otherwise may throw.
    // Throws: Docker::DaemonException if connection to the daemon fails or if the daemon is not operational.
    bool isDaemonRunning() const;

    // --- Container Lifecycle and Management ---

    // Creates and runs a new container based on the provided configuration.
    // config: Specifies image name, command, environment variables, mounts, etc.
    // Returns: Docker::ContainerExecutionResult containing the ID, exit code, and logs of the executed container.
    // Throws: Docker::ImageNotFoundException, Docker::OperationException, Docker::CommandFailureException on errors.
    ContainerExecutionResult runNewContainer(const ContainerRunConfiguration& config);

    // Retrieves a list of Docker containers on the system.
    // includeAll: If true (default), lists all containers (running and stopped).
    //             If false, lists only currently running containers.
    // Returns: A vector of Docker::ContainerInfo objects.
    // Throws: Docker::CommandFailureException, Docker::ParsingException on errors.
    std::vector<ContainerInfo> listContainers(bool includeAll = true) const;

    // Fetches detailed information for a specific container.
    // containerIdOrName: The ID or name of the container to inspect.
    // Returns: A Docker::ContainerInfo object populated with the container's details.
    // Throws: Docker::ContainerNotFoundException, Docker::CommandFailureException, Docker::ParsingException on errors.
    ContainerInfo getContainerDetails(const std::string& containerIdOrName) const;

    // Starts a previously created (and currently stopped) container.
    // containerIdOrName: The ID or name of the container to start.
    // Throws: Docker::ContainerNotFoundException, Docker::ContainerOperationException, Docker::CommandFailureException on errors.
    void startContainer(const std::string& containerIdOrName);

    // Stops a running container.
    // containerIdOrName: The ID or name of the container to stop.
    // timeoutSeconds: Optional. Seconds to wait for the container to stop before killing it. Docker's default is used if <= 0.
    // Throws: Docker::ContainerNotFoundException, Docker::ContainerOperationException, Docker::CommandFailureException on errors.
    void stopContainer(const std::string& containerIdOrName, int timeoutSeconds = 10);

    // Removes a container from the system.
    // containerIdOrName: The ID or name of the container to remove.
    // force: If true, forcibly removes the container even if it is running.
    // removeVolumes: If true, removes anonymous volumes associated with the container.
    // Throws: Docker::ContainerNotFoundException, Docker::ContainerOperationException, Docker::CommandFailureException on errors.
    void removeContainer(const std::string& containerIdOrName, bool force = false, bool removeVolumes = false);

    // --- Container Operations ---

    // Retrieves logs from a specified container.
    // containerIdOrName: The ID or name of the container.
    // tailLines: Number of lines from the end of the logs to retrieve. 0 or negative for all logs.
    // Returns: A string containing the requested logs.
    // Throws: Docker::ContainerNotFoundException, Docker::CommandFailureException on errors.
    std::string getContainerLogs(const std::string& containerIdOrName, int tailLines = 0) const;

    // Executes a command inside an already running container.
    // containerIdOrName: The ID or name of the target container.
    // commandArgs: A vector of strings representing the command and its arguments (e.g., {"echo", "hello"}).
    // user: Optional. The user to run the command as within the container.
    // Returns: The standard output (stdout) from the executed command.
    // Throws: Docker::ContainerNotFoundException, Docker::ContainerOperationException (if not running), Docker::CommandFailureException.
    std::string executeCommandInContainer(const std::string& containerIdOrName, const std::vector<std::string>& commandArgs, const std::string& user = "") const;

    // Opens an interactive shell session (e.g., /bin/sh or /bin/bash) in a running container.
    // Note: This method typically hijacks the current terminal for the shell session.
    // containerIdOrName: The ID or name of the container.
    // Throws: Docker::ContainerNotFoundException, Docker::ContainerOperationException (if not running or shell fails).
    void enterInteractiveShell(const std::string& containerIdOrName);

    // Copies a file from the host to a container.
    // containerIdOrName: The ID or name of the target container.
    // hostPath: Absolute path to the source file on the host.
    // containerPath: Absolute path to the destination in the container.
    // Throws: Docker::ContainerNotFoundException, Docker::OperationException, Docker::CommandFailureException.
    void copyFileToContainer(const std::string& containerIdOrName, const std::string& hostPath, const std::string& containerPath) const;

    // Copies a file from a container to the host.
    // containerIdOrName: The ID or name of the source container.
    // containerPath: Absolute path to the source file in the container.
    // hostPath: Absolute path to the destination on the host.
    // Throws: Docker::ContainerNotFoundException, Docker::OperationException, Docker::CommandFailureException.
    void copyFileFromContainer(const std::string& containerIdOrName, const std::string& containerPath, const std::string& hostPath) const;

    // --- Image Management ---

    // Checks if a Docker image exists locally.
    // imageNameWithTag: The full name and tag of the image (e.g., "ubuntu:latest").
    // Returns: True if the image is found locally, false otherwise.
    // Throws: Docker::CommandFailureException on underlying command errors.
    bool imageExists(const std::string& imageNameWithTag) const;

    // Pulls (downloads) a Docker image from a registry.
    // imageNameWithTag: The full name and tag of the image to pull.
    // Throws: Docker::OperationException (on pull failure), Docker::CommandFailureException.
    void pullImage(const std::string& imageNameWithTag);

    // Lists Docker images available on the local system.
    // Returns: A vector of Docker::ImageInfo objects.
    // Throws: Docker::CommandFailureException, Docker::ParsingException on errors.
    std::vector<ImageInfo> listImages() const;

    // Removes a Docker image from the local system.
    // imageNameOrId: The name (with tag) or ID of the image to remove.
    // force: If true, forcibly removes the image even if it is in use by containers.
    // Throws: Docker::ImageNotFoundException, Docker::OperationException (if removal fails, e.g., image in use without force), Docker::CommandFailureException.
    void removeImage(const std::string& imageNameOrId, bool force = false);

private:
    // Helper to execute a Docker CLI command and return its output and exit code.
    // dockerSubCommand: The Docker command to execute (e.g., "ps -a", "inspect container_id").
    // treatNonZeroExitAsError: If true, throws CommandFailureException for non-zero exit codes.
    // Returns: A pair containing the command's stdout and its exit code.
    // Throws: Docker::CommandFailureException if popen/pclose fails or if treatNonZeroExitAsError is true and exit code is non-zero.
    std::pair<std::string, int> executeCliCommand(const std::string& dockerSubCommand, bool treatNonZeroExitAsError = true) const;

    // Helper to build the full 'docker run' command string from a configuration object.
    std::string buildDockerRunCommand(const ContainerRunConfiguration& config) const;

    // Helper to populate parts of ContainerExecutionResult using 'docker inspect' and 'docker logs' after a container run.
    void populateExecutionResultFromInspectAndLogs(ContainerExecutionResult& execResult, const std::string& containerId, const ContainerRunConfiguration& config) const;
    
    // Helper to build a command string from a vector of arguments.
    // commandArgs: A vector of strings, where each string is a part of the command or an argument.
    // Returns: A single string with arguments concatenated, suitable for execution.
    std::string buildCommandString(const std::vector<std::string>& commandArgs) const;

    // Helper to parse container ID from 'docker run -d' output
    std::string parseContainerIdFromRunOutput(const std::string& output) const;

    // Helper to parse JSON output from 'docker inspect' into a ContainerInfo object
    ContainerInfo parseInspectOutputToContainerInfo(const std::string& inspectJson, const std::string& containerIdOrName) const;

    // --- New private helper method declarations ---
    void fetchContainerLogs(const std::string& containerId, ContainerExecutionResult& execResult) const;
    void handleSuccessfulContainerRun(ContainerExecutionResult& execResult, const std::string& output, const ContainerRunConfiguration& config);
    void handleFailedContainerRun(ContainerExecutionResult& execResult, const std::string& command, const std::pair<std::string, int>& resultPair);
    ContainerInfo inspectContainer(const std::string& containerIdOrName) const;
};

}
#endif //DOCKER_MANAGER_H
