// docker_exceptions.h

// Standard include guard to prevent multiple inclusions of this header file
// within a single translation unit. This is crucial for avoiding compilation errors.
#ifndef DOCKER_EXCEPTIONS_H
#define DOCKER_EXCEPTIONS_H

#include <stdexcept> // Required for std::runtime_error, the base class for many standard exceptions.
#include <string>    // Required for using std::string to store messages and other textual data.
#include <utility>   // Required for std::move, used for efficient string transfers.

// The 'Docker' namespace encapsulates all Docker-module-specific declarations,
// preventing naming conflicts with other parts of the larger Antivirus project
// or other libraries.
namespace Docker {

    // Base class for all custom exceptions thrown by the Docker module.
    // It inherits from std::runtime_error, allowing it to be caught by handlers
    // for standard exceptions and to utilize the .what() method for retrieving
    // an explanatory message.
    class DockerException : public std::runtime_error {
    public:
        // Constructor for DockerException.
        // 'explicit' prevents unintentional implicit conversions from std::string.
        // It takes an error message and passes it to the std::runtime_error base class.
        explicit DockerException(const std::string& message)
            : std::runtime_error(message) {} // Call the base class constructor.

        // Virtual destructor.
        // Good practice for base classes intended for polymorphic use (i.e., if derived
        // class objects are deleted via a base class pointer).
        // 'noexcept' specifies that this destructor will not throw exceptions.
        // '= default' tells the compiler to generate the default implementation.
        virtual ~DockerException() noexcept = default;
    };

    // Exception thrown when a Docker Command Line Interface (CLI) command fails to execute correctly.
    // It inherits from DockerException and carries additional information about the failed command.
    class CommandFailureException : public DockerException {
    public:
        // Public member variables to store details about the command failure.
        // While private members with getters are sometimes preferred for encapsulation,
        // for simple data-carrying exception classes, public members are often acceptable.
        std::string command;     // The Docker command string that was executed and failed.
        int         exitCode;    // The exit code returned by the command upon termination.
        std::string errorOutput; // The error message output (typically from stderr) by the failed command.

        // Constructor for CommandFailureException.
        // Initializes the exception with details of the failed command and a general error message.
        CommandFailureException(const std::string& cmd, int code, const std::string& errOut, const std::string& message)
            : DockerException(message),          // Call the base DockerException constructor with the primary message.
              command(std::move(cmd)),           // Initialize 'command' member (std::move for efficiency).
              exitCode(code),                    // Initialize 'exitCode'.
              errorOutput(std::move(errOut)) {}  // Initialize 'errorOutput' (std::move for efficiency).
    };

    // A general exception thrown when a Docker resource (e.g., container, image, volume) cannot be found.
    // It inherits from DockerException.
    class ResourceNotFoundException : public DockerException {
    public:
        // Public member variables to identify the resource that was not found.
        std::string resourceType;       // The type of the resource (e.g., "Container", "Image").
        std::string resourceIdentifier; // The name or ID of the resource that was not found.

        // Constructor for ResourceNotFoundException.
        // Takes the type and identifier of the missing resource to formulate a descriptive error message.
        explicit ResourceNotFoundException(const std::string& resType, const std::string& resId)
            : DockerException(resType + " not found: " + resId), // Construct message and call base constructor.
              resourceType(resType),                  // Initialize 'resourceType'.
              resourceIdentifier(resId) {}            // Initialize 'resourceIdentifier'.
              
        // Constructor with custom error message
        explicit ResourceNotFoundException(const std::string& resType, const std::string& resId, const std::string& errorMsg)
            : DockerException(errorMsg), // Use the provided error message
              resourceType(resType),     // Initialize 'resourceType'.
              resourceIdentifier(resId) {} // Initialize 'resourceIdentifier'.
    };

    // Specific exception thrown when a Docker container cannot be found.
    // Inherits from ResourceNotFoundException for a more specific error classification.
    class ContainerNotFoundException : public ResourceNotFoundException {
    public:
        // Constructor for ContainerNotFoundException.
        // Takes the name or ID of the container and informs the base ResourceNotFoundException
        // that the resource type is "Container".
        explicit ContainerNotFoundException(const std::string& containerIdOrName)
            : ResourceNotFoundException("Container", containerIdOrName) {} // Call base constructor.
    };

    // Specific exception thrown when a Docker image cannot be found.
    // Inherits from ResourceNotFoundException for a more specific error classification.
    class ImageNotFoundException : public ResourceNotFoundException {
    public:
        // Constructor for ImageNotFoundException.
        // Takes the name or ID of the image and informs the base ResourceNotFoundException
        // that the resource type is "Image".
        explicit ImageNotFoundException(const std::string& imageIdOrName)
            : ResourceNotFoundException("Image", imageIdOrName) {} // Call base constructor.
        
        // Constructor with error message
        ImageNotFoundException(const std::string& imageIdOrName, const std::string& errorMessage)
            : ResourceNotFoundException("Image", imageIdOrName, errorMessage) {} // Call base constructor with error message.
    };

    // Exception thrown when a general Docker operation (like start, stop, remove) fails
    // for reasons other than a resource not being found or a direct command failure
    // (though it can also wrap such scenarios if a more generic operation error is suitable).
    // It inherits from DockerException.
    class OperationException : public DockerException {
    public:
        // Public member variable to store the name of the failed operation.
        std::string operationName; // The name of the Docker operation that failed (e.g., "start", "remove").

        // Constructor for OperationException.
        // Takes the operation name and a descriptive error message.
        explicit OperationException(const std::string& opName, const std::string& message)
            : DockerException(message),            // Call base DockerException constructor.
              operationName(std::move(opName)) {}  // Initialize 'operationName'.
    };

    // Specific exception thrown when an operation on a Docker container fails.
    // Inherits from OperationException to provide more context.
    class ContainerOperationException : public OperationException {
    public:
        // Public member variable to identify the container involved in the failed operation.
        std::string containerIdentifier; // The name or ID of the container.

        // Constructor for ContainerOperationException.
        // Takes the operation name, container identifier, and a descriptive error message.
        explicit ContainerOperationException(const std::string& opName, const std::string& containerIdOrName, const std::string& message)
            : OperationException(opName, message),                   // Call base OperationException constructor.
              containerIdentifier(std::move(containerIdOrName)) {} // Initialize 'containerIdentifier'.
    };

    // Exception thrown when there are issues related to the Docker daemon (service) itself,
    // such as an inability to connect to it.
    // It inherits from DockerException.
    class DaemonException : public DockerException {
    public:
        // Constructor for DaemonException.
        // Takes a descriptive error message.
        explicit DaemonException(const std::string& message)
            : DockerException(message) {} // Call base DockerException constructor.
    };

    // Exception thrown when an error occurs while parsing the output from Docker CLI commands
    // (e.g., malformed JSON or unexpected text format).
    // It inherits from DockerException.
    class ParsingException : public DockerException {
    public:
        // Constructor for ParsingException.
        // Takes a string containing details about the parsing failure.
        explicit ParsingException(const std::string& detailedInformation)
            : DockerException("Failed to parse Docker output. Details: " + detailedInformation) {} // Call base constructor.
    };

} // namespace Docker

#endif // DOCKER_EXCEPTIONS_H
