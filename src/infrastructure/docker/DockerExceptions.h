/**
 * @file DockerExceptions.h
 * @brief Custom exception classes for Docker operations and error handling
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 * 
 * @details This header defines a comprehensive hierarchy of exception classes
 * for handling various Docker-related errors and failure scenarios. The exception
 * hierarchy provides specific error types for different Docker operations, allowing
 * for precise error handling and debugging.
 * 
 * Exception Hierarchy:
 * - DockerException (base class)
 *   - CommandFailureException (CLI command failures)
 *   - ResourceNotFoundException (missing resources)
 *     - ContainerNotFoundException
 *     - ImageNotFoundException
 *   - OperationException (general operation failures)
 *     - ContainerOperationException
 *   - DaemonException (Docker daemon issues)
 *   - ParsingException (output parsing errors)
 * 
 * @note All exceptions inherit from std::runtime_error for standard compatibility
 * @warning Ensure proper exception handling in all Docker operations
 */

#ifndef DOCKER_EXCEPTIONS_H
#define DOCKER_EXCEPTIONS_H

#include <stdexcept> // Required for std::runtime_error, the base class for many standard exceptions.
#include <string>    // Required for using std::string to store messages and other textual data.
#include <utility>   // Required for std::move, used for efficient string transfers.

namespace Docker {

    /**
     * @brief Base exception class for all Docker-related errors
     * 
     * @details Provides a common base for all Docker module exceptions,
     * inheriting from std::runtime_error to maintain compatibility with
     * standard exception handling practices. All Docker-specific exceptions
     * should derive from this class.
     */
    class DockerException : public std::runtime_error {
    public:
        /**
         * @brief Construct a new Docker Exception
         * @param message Descriptive error message explaining the exception
         */
        explicit DockerException(const std::string& message)
            : std::runtime_error(message) {}

        /**
         * @brief Virtual destructor for proper polymorphic destruction
         */
        virtual ~DockerException() noexcept = default;
    };

    /**
     * @brief Exception for Docker CLI command execution failures
     * 
     * @details Thrown when a Docker command line interface command fails
     * to execute successfully. Contains detailed information about the
     * failed command, exit code, and error output for debugging purposes.
     */
    class CommandFailureException : public DockerException {
    public:
        std::string command;     ///< The Docker command that failed to execute
        int         exitCode;    ///< Exit code returned by the failed command
        std::string errorOutput; ///< Error output from stderr of the failed command

        /**
         * @brief Construct a new Command Failure Exception
         * @param cmd The Docker command that failed
         * @param code Exit code from the command execution
         * @param errOut Error output from the command's stderr
         * @param message General error message describing the failure
         */
        CommandFailureException(const std::string& cmd, int code, const std::string& errOut, const std::string& message)
            : DockerException(message),
              command(std::move(cmd)),
              exitCode(code),
              errorOutput(std::move(errOut)) {}
    };

    /**
     * @brief Exception for missing Docker resources
     * 
     * @details Thrown when attempting to access Docker resources (containers,
     * images, volumes, etc.) that do not exist or cannot be found. Provides
     * specific information about the resource type and identifier.
     */
    class ResourceNotFoundException : public DockerException {
    public:
        std::string resourceType;       ///< Type of resource that was not found (e.g., "Container", "Image")
        std::string resourceIdentifier; ///< Name or ID of the missing resource

        /**
         * @brief Construct a new Resource Not Found Exception
         * @param resType Type of the missing resource
         * @param resId Identifier (name or ID) of the missing resource
         */
        explicit ResourceNotFoundException(const std::string& resType, const std::string& resId)
            : DockerException(resType + " not found: " + resId),
              resourceType(resType),
              resourceIdentifier(resId) {}
              
        /**
         * @brief Construct a new Resource Not Found Exception with custom message
         * @param resType Type of the missing resource
         * @param resId Identifier (name or ID) of the missing resource
         * @param errorMsg Custom error message
         */
        explicit ResourceNotFoundException(const std::string& resType, const std::string& resId, const std::string& errorMsg)
            : DockerException(errorMsg),
              resourceType(resType),
              resourceIdentifier(resId) {}
    };

    /**
     * @brief Specific exception for missing Docker containers
     * 
     * @details Specialized exception thrown when a requested container
     * cannot be found in the Docker environment.
     */
    class ContainerNotFoundException : public ResourceNotFoundException {
    public:
        /**
         * @brief Construct a new Container Not Found Exception
         * @param containerIdOrName Name or ID of the missing container
         */
        explicit ContainerNotFoundException(const std::string& containerIdOrName)
            : ResourceNotFoundException("Container", containerIdOrName) {}
    };

    /**
     * @brief Specific exception for missing Docker images
     * 
     * @details Specialized exception thrown when a requested Docker image
     * cannot be found in the local repository or registry.
     */
    class ImageNotFoundException : public ResourceNotFoundException {
    public:
        /**
         * @brief Construct a new Image Not Found Exception
         * @param imageIdOrName Name or ID of the missing image
         */
        explicit ImageNotFoundException(const std::string& imageIdOrName)
            : ResourceNotFoundException("Image", imageIdOrName) {}
        
        /**
         * @brief Construct a new Image Not Found Exception with custom message
         * @param imageIdOrName Name or ID of the missing image
         * @param errorMessage Custom error message
         */
        ImageNotFoundException(const std::string& imageIdOrName, const std::string& errorMessage)
            : ResourceNotFoundException("Image", imageIdOrName, errorMessage) {}
    };

    /**
     * @brief Exception for general Docker operation failures
     * 
     * @details Thrown when Docker operations (start, stop, remove, etc.) fail
     * for reasons other than resource not found or direct command failures.
     * Provides context about the specific operation that failed.
     */
    class OperationException : public DockerException {
    public:
        std::string operationName; ///< Name of the Docker operation that failed

        /**
         * @brief Construct a new Operation Exception
         * @param opName Name of the failed operation
         * @param message Descriptive error message
         */
        explicit OperationException(const std::string& opName, const std::string& message)
            : DockerException(message),
              operationName(std::move(opName)) {}
    };

    /**
     * @brief Exception for Docker container operation failures
     * 
     * @details Specialized exception for operations that fail on specific
     * containers, providing both operation context and container identification.
     */
    class ContainerOperationException : public OperationException {
    public:
        std::string containerIdentifier; ///< Name or ID of the container involved in the failed operation

        /**
         * @brief Construct a new Container Operation Exception
         * @param opName Name of the failed operation
         * @param containerIdOrName Name or ID of the container
         * @param message Descriptive error message
         */
        explicit ContainerOperationException(const std::string& opName, const std::string& containerIdOrName, const std::string& message)
            : OperationException(opName, message),
              containerIdentifier(std::move(containerIdOrName)) {}
    };

    /**
     * @brief Exception for Docker daemon connectivity and service issues
     * 
     * @details Thrown when there are problems communicating with the Docker
     * daemon or when the Docker service is unavailable or misconfigured.
     */
    class DaemonException : public DockerException {
    public:
        /**
         * @brief Construct a new Daemon Exception
         * @param message Descriptive error message about the daemon issue
         */
        explicit DaemonException(const std::string& message)
            : DockerException(message) {}
    };

    /**
     * @brief Exception for Docker command output parsing failures
     * 
     * @details Thrown when Docker CLI command output cannot be parsed correctly,
     * such as malformed JSON responses or unexpected text formats.
     */
    class ParsingException : public DockerException {
    public:
        /**
         * @brief Construct a new Parsing Exception
         * @param detailedInformation Detailed information about the parsing failure
         */
        explicit ParsingException(const std::string& detailedInformation)
            : DockerException("Failed to parse Docker output. Details: " + detailedInformation) {}
    };

} // namespace Docker

#endif // DOCKER_EXCEPTIONS_H
