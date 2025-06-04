/**
 * @file DockerTypes.h
 * @brief Type definitions and data structures for Docker container management
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 * 
 * @details This header defines all data structures, types, and configuration objects
 * used throughout the Docker management system. It provides a comprehensive set of
 * types for container lifecycle management, image operations, and resource configuration.
 * 
 * Key Components:
 * - Container configuration and execution structures
 * - Image metadata and information types
 * - Mount point and volume configuration
 * - Host configuration options for security and resource management
 * - Execution result and status reporting structures
 * 
 * @note All types are designed to be lightweight and easily serializable
 * @warning Proper validation should be performed on all configuration values
 */

#ifndef DOCKERTYPES_H
#define DOCKERTYPES_H

#include <string>
#include <vector>
#include <map>     // For potential future use (e.g., Labels, PortBindings)
#include <cstdint> // For types like int64_t

namespace Docker {

    /**
     * @brief Defines a mount point for Docker container volume and bind mount operations
     * 
     * @details Specifies how host directories, volumes, or temporary filesystems
     * are mounted into Docker containers. Supports bind mounts, named volumes,
     * and tmpfs mounts with configurable read/write permissions.
     */
    struct MountPoint {
        std::string mountType;      ///< Type of mount: "bind", "volume", or "tmpfs"
        std::string source;         ///< Host path for bind mounts or volume name
        std::string destination;    ///< Target path inside the container
        bool readOnly = false;      ///< Mount as read-only if true
        std::string tmpfsOptions;   ///< Options for tmpfs mounts (size, mode, etc.)
    };

    /**
     * @brief Host-side configuration options for Docker container security and resources
     * 
     * @details Comprehensive configuration structure for controlling container
     * behavior, security settings, resource limits, and host integration options.
     * Provides fine-grained control over container privileges and isolation.
     */
    struct HostConfigurationOptions {
        std::vector<MountPoint> mounts; ///< List of mount points for volumes and bind mounts

        bool autoremove = false;         ///< Automatically remove container when it exits
        bool privileged = false;        ///< Grant extended privileges (use with extreme caution)
        bool readOnlyRootfs = false;    ///< Mount container's root filesystem as read-only

        std::vector<std::string> capDrop;           ///< Linux capabilities to drop for security
        std::vector<std::string> capAdd;            ///< Linux capabilities to add if needed
        std::vector<std::string> securityOptions;   ///< Security options (AppArmor, Seccomp profiles)
        std::vector<std::string> securityOpt;       ///< Additional security options for compatibility
        std::string networkMode;                    ///< Network mode ("none", "bridge", "host", etc.)
        int pidsLimit = 0;                          ///< Process limit (0 = unlimited)

        long long memoryLimit = 0;      ///< Memory limit in bytes (0 = unlimited)
        long long cpuPeriod = 0;        ///< CPU CFS period in microseconds
        long long cpuQuota = 0;         ///< CPU CFS quota in microseconds
    };

    /**
     * @brief Complete configuration for creating and running a Docker container
     * 
     * @details Encapsulates all parameters needed to create and execute a container,
     * including image selection, command execution, environment setup, and host
     * configuration options.
     */
    struct ContainerRunConfiguration {
        std::string imageName;          ///< Name or ID of the Docker image to use
        std::vector<std::string> command; ///< Command and arguments to execute in container
        std::vector<std::string> environmentVariables; ///< Environment variables in "KEY=VALUE" format
        std::string containerName;      ///< Optional name for the container instance
        std::string userName;           ///< Optional username or UID to run as in container

        //bool attachStdOut = true;       ///< Attach to container's STDOUT stream
        //bool attachStdErr = true;       ///< Attach to container's STDERR stream

        HostConfigurationOptions hostConfig; ///< Host-specific configuration options
    };

    /**
     * @brief Result data from Docker container execution operations
     * 
     * @details Contains comprehensive information about container execution,
     * including exit status, output logs, and any error conditions that occurred
     * during the container lifecycle.
     */
    struct ContainerExecutionResult {
        std::string containerId;    ///< Unique identifier of the executed container
        int exitCode = -1;          ///< Exit code from container's main process (-1 if not available)
        std::string logs;           ///< Combined STDOUT and STDERR output from container
        std::string status;         ///< Final status of the container ("exited", "running", etc.)
        std::string errorMessage;   ///< Error message if Docker operation failed
    };

    /**
     * @brief Basic metadata information for Docker images
     * 
     * @details Lightweight structure containing essential image identification
     * information, suitable for listing and basic image management operations.
     */
    struct ImageInfoBasic {
        std::string id;                     ///< Unique SHA256 hash identifier of the image
        std::vector<std::string> repoTags;  ///< Repository tags (e.g., "ubuntu:latest", "app:v1.0")
    };
    
    /**
     * @brief Detailed metadata and information for Docker images
     * 
     * @details Comprehensive image information including repository details,
     * size information, and creation timestamps. Used for detailed image
     * inspection and management operations.
     */
    struct ImageInfo {
        std::string id;                     ///< Unique SHA256 hash identifier of the image
        std::string repository;             ///< Repository name (e.g., "ubuntu", "myapp")
        std::string tag;                    ///< Tag name (e.g., "latest", "v1.0", "stable")
        std::string size;                   ///< Human-readable size format (e.g., "125MB")
        std::string created;                ///< ISO 8601 creation timestamp
    };

    /**
     * @brief Information about Docker container instances
     * 
     * @details Contains metadata and status information for both running and
     * stopped containers. Used for container monitoring, management, and
     * lifecycle operations.
     */
    struct ContainerInfo {
        std::string containerId;            ///< Unique identifier of the container
        std::string containerName;          ///< Human-readable name assigned to container
        std::string containerStatus;        ///< Current status ("running", "exited", "paused", etc.)
        std::string containerImage;         ///< Image used to create this container
        std::string containerCreationTime;  ///< ISO 8601 creation timestamp or Unix epoch
    };

} // namespace Docker

#endif // DOCKERTYPES_H
