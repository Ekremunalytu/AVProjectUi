#ifndef DOCKERTYPES_H
#define DOCKERTYPES_H

#include <string>
#include <vector>
#include <map>     // For potential future use (e.g., Labels, PortBindings)
#include <cstdint> // For types like int64_t

namespace Docker {

    // Defines a mount point for a Docker container (bind, volume, or tmpfs).
    struct MountPoint {
        std::string mountType;      // Type of mount, e.g., "bind", "volume".
        std::string source;         // Path on host (for bind) or name of volume.
        std::string destination;    // Path inside the container.
        bool readOnly = false;      // If true, mount is read-only in container.
        std::string tmpfsOptions;   // Options for tmpfs mounts
    };

    // Specifies host-side configuration options for a Docker container.
    struct HostConfigurationOptions {
        std::vector<MountPoint> mounts; // List of mount points.

        bool autoremove = false;         // Automatically remove container when it exits.
        bool privileged = false;        // Grant extended privileges to the container (use with caution).
        bool readOnlyRootfs = false;    // Mount container's root filesystem as read-only.

        std::vector<std::string> capDrop;           // List of Linux capabilities to drop.
        std::vector<std::string> capAdd;            // List of Linux capabilities to add.
        std::vector<std::string> securityOptions;   // Security options (e.g., AppArmor, Seccomp).
        std::vector<std::string> securityOpt;       // Security options for compatibility
        std::string networkMode;                    // Network mode (e.g., "none", "bridge")
        int pidsLimit = 0;                          // Process limit (0 = no limit)

        long long memoryLimit = 0;      // Memory limit in bytes (0 = no limit).
        long long cpuPeriod = 0;        // CPU CFS period in microseconds.
        long long cpuQuota = 0;         // CPU CFS quota in microseconds.
    };

    // Configuration for running a new Docker container.
    struct ContainerRunConfiguration {
        std::string imageName;          // Name or ID of the Docker image.
        std::vector<std::string> command; // Command and arguments to run in container.
        std::vector<std::string> environmentVariables; // Environment variables ("KEY=VALUE" format).
        std::string containerName;      // Optional name for the container.
        std::string userName;           // Optional username or UID to run as in container.

        //bool attachStdOut = true;       // Attach to container's STDOUT.
        //bool attachStdErr = true;       // Attach to container's STDERR.

        HostConfigurationOptions hostConfig; // Host-specific configurations.
    };

    // Represents the result of a Docker container execution.
    struct ContainerExecutionResult {
        std::string containerId;    // ID of the executed container.
        int exitCode = -1;          // Exit code from the container's main process.
        std::string logs;           // Combined STDOUT and STDERR from the container.
        std::string status;         // Final status of the container (e.g., "exited").
        std::string errorMessage;   // Error message if Docker operation failed.
    };

    // Basic information about a Docker image.
    struct ImageInfoBasic {
        std::string id;                     // Unique ID of the image (SHA256 hash).
        std::vector<std::string> repoTags;  // List of repository tags (e.g., "ubuntu:latest").
    };
    
    // Detailed information about a Docker image.
    struct ImageInfo {
        std::string id;                     // Unique ID of the image (SHA256 hash).
        std::string repository;             // Repository name (e.g., "ubuntu").
        std::string tag;                    // Tag name (e.g., "latest").
        std::string size;                   // Size of the image in human-readable format.
        std::string created;                // Creation timestamp.
    };

    // Basic information about a running or stopped Docker container.
    struct ContainerInfo {
        std::string containerId;            // Unique ID of the container.
        std::string containerName;          // Name of the container.
        std::string containerStatus;        // Current status (e.g., "running", "exited").
        std::string containerImage;         // Image used by the container.
        std::string containerCreationTime;  // Creation timestamp (often ISO 8601 string or Unix epoch).
    };

} // namespace Docker

#endif // DOCKERTYPES_H
