#include "DockerManager.h"
#include "DockerExceptions.h" // Docker:: specific exceptions
#include "DockerTypes.h"      // Docker:: type definitions

#include <iostream>  // For std::cerr, std::cout (debugging, or specific methods like enterInteractiveShell)
#include <cstdio>    // For popen, pclose, fgets
#include <cstdlib>   // For system (used carefully or avoided)
#include <array>     // For buffer in executeCliCommand
#include <sstream>   // For std::stringstream
#include <algorithm> // For std::remove, std::transform
#include <iomanip>   // For std::quoted (potentially)

// Platform-specific includes for process management
#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__MINGW64__)
#include <windows.h> // For Windows-specific process handling
#else
#include <sys/wait.h> // For WIFEXITED and WEXITSTATUS (POSIX)
#endif

// Qt6 includes for JSON parsing (ensure these are consistently used)
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonValue>
#include <QtCore/QString>

// Helper function to trim leading/trailing whitespace
std::string trim(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos)
        return ""; // no content
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, (end - start + 1));
}


namespace Docker {

// --- Constructor ---
DockerManager::DockerManager() {
    // In the future, initial checks can be added
    // such as verifying the existence of Docker CLI.
}

// --- Private Helper Methods ---

// Helper to build the 'docker run' command string
std::string DockerManager::buildDockerRunCommand(const ContainerRunConfiguration& config) const {
    std::stringstream cmd;
    cmd << "run ";

    // Add -d for detached mode
    cmd << "-d ";

    if (config.hostConfig.autoremove) {
        cmd << "--rm ";
    }
    if (!config.containerName.empty()) {
        cmd << "--name " << std::quoted(config.containerName) << " ";
    }
    if (!config.userName.empty()) {
        cmd << "--user " << std::quoted(config.userName) << " ";
    }

    for (const auto& envVar : config.environmentVariables) {
        cmd << "-e " << std::quoted(envVar) << " ";
    }

    for (const auto& mount : config.hostConfig.mounts) {
        cmd << "-v " << std::quoted(mount.source + ":" + mount.destination + (mount.readOnly ? ":ro" : "")) << " ";
    }
    
    if (config.hostConfig.privileged){
        cmd << "--privileged ";
    }
    if (config.hostConfig.readOnlyRootfs){
        cmd << "--read-only ";
    }
    for(const auto& capDrop : config.hostConfig.capDrop){
        cmd << "--cap-drop=" << std::quoted(capDrop) << " ";
    }
    for(const auto& capAdd : config.hostConfig.capAdd){
        cmd << "--cap-add=" << std::quoted(capAdd) << " ";
    }
    for(const auto& secOpt : config.hostConfig.securityOptions){
        cmd << "--security-opt " << std::quoted(secOpt) << " ";
    }
    if(config.hostConfig.memoryLimit > 0){
        cmd << "--memory=" << config.hostConfig.memoryLimit << " ";
    }
    if(config.hostConfig.cpuPeriod > 0){
        cmd << "--cpu-period=" << config.hostConfig.cpuPeriod << " ";
    }
    if(config.hostConfig.cpuQuota > 0){
        cmd << "--cpu-quota=" << config.hostConfig.cpuQuota << " ";
    }

    cmd << std::quoted(config.imageName) << " ";

    if (!config.command.empty()) {
        for (size_t i = 0; i < config.command.size(); ++i) {
            cmd << std::quoted(config.command[i]) << (i == config.command.size() - 1 ? "" : " ");
        }
    }
    return cmd.str();
}

// Helper to parse container ID from 'docker run -d' output
std::string DockerManager::parseContainerIdFromRunOutput(const std::string& output) const {
    std::string potentialId = trim(output);
    // Docker IDs are typically 64 hex characters, but short IDs (12 chars) are also common.
    // A simple check for hex characters and reasonable length.
    bool isValidDockerId = (potentialId.length() >= 12 && potentialId.length() <= 64) &&
                             (potentialId.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos);

    if (isValidDockerId) {
        return potentialId;
    }
    throw DockerException("Failed to parse container ID from docker run output: " + output);
}

// Helper to fetch container logs
void DockerManager::fetchContainerLogs(const std::string& containerId, ContainerExecutionResult& execResult) const {
    std::cout << "DEBUG: Getting container logs with docker logs for container ID: " << containerId << std::endl;
    std::stringstream ss;
    ss << "logs " << std::quoted(containerId);
    std::string logsCmd = ss.str();
    try {
        auto logsResult = executeCliCommand(logsCmd, false);
        if (logsResult.second == 0) {
            execResult.logs = logsResult.first; // Overwrite initial logs if new logs are fetched
        } else {
            std::cerr << "Warning: docker logs failed for container " << containerId << ". Output: " << logsResult.first << std::endl;
        }
    } catch (const DockerException& e) {
        std::cerr << "Warning: Could not fetch logs for container " << containerId << ": " << e.what() << std::endl;
    }
}

// Helper to populate execution result using 'docker inspect' and 'docker logs'
void DockerManager::populateExecutionResultFromInspectAndLogs(ContainerExecutionResult& execResult, const std::string& containerId, const ContainerRunConfiguration& config) const {
    if (config.hostConfig.autoremove) {
        // If --rm is used, inspect and logs might not be available after container exits.
        // The initial execResult.logs from `docker run` might be the only logs.
        // Exit code might not be available, or it might be 0 if `docker run` itself succeeded.
        // Logs might have been captured by `docker run` if it wasn't fully detached,
        // but typically for -d --rm, logs are not easily retrievable after exit.
        std::cout << "DEBUG: Container was run with --rm, skipping inspect and logs fetch for container ID: " << containerId << std::endl;
        return;
    }

    try {
        std::cout << "DEBUG: Getting container details with docker inspect for container ID: " << containerId << std::endl;
        std::stringstream ss;
        ss << "inspect " << std::quoted(containerId);
        std::string inspectCmd = ss.str();
        auto inspectResult = executeCliCommand(inspectCmd, false);
                        
        if (inspectResult.second == 0) {
            QByteArray jsonData = QByteArray::fromStdString(inspectResult.first);
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
                            
            if (!jsonDoc.isNull() && jsonDoc.isArray()) {
                QJsonArray containerArray = jsonDoc.array();
                if (!containerArray.isEmpty()) {
                    QJsonObject container = containerArray.first().toObject();
                    // parseContainerStateFromInspect(container, execResult);
                }
            }
        } else {
            std::cerr << "Warning: docker inspect failed for container " << containerId << ". Output: " << inspectResult.first << std::endl;
        }
                        
        // Logları al (inspect başarılı olsa da olmasa da logları almayı dene)
        fetchContainerLogs(containerId, execResult);
    } catch (const DockerException& e) {
        std::cerr << "Warning: Could not fetch container details after run for container " << containerId << ": " 
                  << e.what() << std::endl;
    }
}

std::string DockerManager::buildCommandString(const std::vector<std::string>& commandArgs) const {
    std::stringstream commandStream;
    for (size_t i = 0; i < commandArgs.size(); ++i) {
        commandStream << std::quoted(commandArgs[i]) << (i == commandArgs.size() - 1 ? "" : " ");
        // In a more advanced version, std::quoted can be used for arguments containing spaces
        // or arguments can be handled separately for safer concatenation.
        // Example: commandStream << std::quoted(arg, \'\'\'\'\'\') << " ";
    }
    return commandStream.str();
}

std::pair<std::string, int> DockerManager::executeCliCommand(const std::string& dockerSubCommand, bool treatNonZeroExitAsError) const {
    std::string fullCommand = "docker " + dockerSubCommand;
    std::array<char, 256> buffer; // You can increase the buffer size.
    std::string result;
    int status = 0; // Status for pclose
    int exitCode = 0;

    // Debug command output
    // std::cout << "DEBUG: Executing command: " << fullCommand << std::endl; // Disabled by default

    // Redirect stderr to stdout to capture errors as well
    std::string commandWithStdErrRedirect = fullCommand + " 2>&1";

#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__MINGW64__)
    FILE* pipe = _popen(commandWithStdErrRedirect.c_str(), "r");
#else
    FILE* pipe = popen(commandWithStdErrRedirect.c_str(), "r");
#endif

    if (!pipe) {
        // std::cerr << "DEBUG: popen() failed for command: " << fullCommand << std::endl; // Disabled by default
        throw CommandFailureException(fullCommand, -1, "popen() failed.", "Failed to execute command (popen failed)");
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__MINGW64__)
    status = _pclose(pipe);
    // On Windows, _pclose returns the exit status directly, or -1 on error.
    exitCode = status;
    if (status == -1) {
         // pclose error
        std::cerr << "DEBUG: _pclose() failed for command: " << fullCommand << std::endl;
        throw CommandFailureException(fullCommand, -1, result, "_pclose() failed after command execution.");
    }
#else
    status = pclose(pipe);
    if (status == -1) {
        // pclose error
        std::cerr << "DEBUG: pclose() failed for command: " << fullCommand << std::endl;
        throw CommandFailureException(fullCommand, -1, result, "pclose() failed after command execution.");
    } else {
        if (WIFEXITED(status)) {
            exitCode = WEXITSTATUS(status);
        } else {
            // Command did not terminate normally (signal, etc.)
            exitCode = -1; // Indicate abnormal termination
            std::cerr << "DEBUG: Command did not terminate normally: " << fullCommand << std::endl;
        }
    }
#endif
    // Show output and exit code for debugging
    // std::cout << "DEBUG: Command output: " << result << std::endl; // Disabled by default to reduce noise
    // std::cout << "DEBUG: Command exit code: " << exitCode << std::endl;

    if (treatNonZeroExitAsError && exitCode != 0) {
        throw CommandFailureException(fullCommand, exitCode, result, "Command failed with non-zero exit code.");
    }
    return {result, exitCode};
}

// --- Docker Daemon Utilities ---
bool DockerManager::isDaemonRunning() const {
    try {
        std::cout << "DEBUG: Checking if Docker daemon is running..." << std::endl;
        // "docker info" is generally a good way to check daemon status.
        // If successful (exit code 0), it means the daemon is running.
        // We can also check that the output contains no error messages.
        auto resultPair = executeCliCommand("info", false); // Don't throw exceptions on error, we'll check manually.
        
        if (resultPair.second != 0) {
             // If the exit code is not 0, there's a problem.
             // Error message might be in resultPair.first.
            std::cerr << "DEBUG: Docker daemon not responding. Exit code: " << resultPair.second << std::endl;
            std::cerr << "DEBUG: Output: " << resultPair.first << std::endl;
            throw DaemonException("Docker daemon is not responding or 'docker info' failed. Output: " + resultPair.first);
        }
        // On some systems, "docker info" might return successfully but still contain messages like "Cannot connect to the Docker daemon".
        // We need to check for these cases as well.
        if (resultPair.first.find("Cannot connect to the Docker daemon") != std::string::npos ||
            resultPair.first.find("Is the docker daemon running") != std::string::npos ) {
            std::cerr << "DEBUG: Docker daemon reported as not running in output." << std::endl;
            throw DaemonException("Docker daemon reported it is not running. Output: " + resultPair.first);
        }
        std::cout << "DEBUG: Docker daemon is running correctly." << std::endl;
        return true;
    } catch (const CommandFailureException& e) {
        // executeCliCommand popen hatası gibi bir durumda fırlatabilir.
        std::cerr << "DEBUG: Exception while checking Docker daemon: " << e.what() << std::endl;
        throw DaemonException("Failed to check Docker daemon status: " + std::string(e.what()));
    }
    // Diğer beklenmedik durumlar için genel bir false dönüşü veya daha spesifik bir exception.
    return false; 
}

// --- Container Lifecycle and Management ---

// Helper to handle successful container run
void DockerManager::handleSuccessfulContainerRun(ContainerExecutionResult& execResult, const std::string& output, const ContainerRunConfiguration& config) {
    try {
        execResult.containerId = parseContainerIdFromRunOutput(output);
        std::cout << "DEBUG: Container started with ID: " << execResult.containerId << std::endl;

        if (!config.hostConfig.autoremove) {
            populateExecutionResultFromInspectAndLogs(execResult, execResult.containerId, config);
        } else {
            // If --rm is used, the container is already gone.
            // We can set a status reflecting this.
            execResult.status = "Exited (auto-removed)";
            // Exit code might not be available, or it might be 0 if `docker run` itself succeeded.
            // Logs might have been captured by `docker run` if it wasn't fully detached,
            // but typically for -d --rm, logs are not easily retrievable after exit.
        }
    } catch (const DockerException& e) {
        // This catch is for errors during parsing container ID or during populateExecutionResultFromInspectAndLogs
        execResult.status = "Error";
        execResult.errorMessage = "Post-run processing failed: " + std::string(e.what());
        std::cerr << "ERROR: Post-run processing failed for container: " << e.what() << std::endl;
        // Depending on policy, we might want to re-throw or just log and return.
        // For now, we populate error message and let the caller decide.
    }
}

// Helper to handle failed container run
void DockerManager::handleFailedContainerRun(ContainerExecutionResult& execResult, const std::string& command, const std::pair<std::string, int>& resultPair) {
    execResult.status = "Error";
    execResult.errorMessage = "docker run command failed. Output: " + resultPair.first;
    execResult.exitCode = resultPair.second; // Exit code from the `docker run` command itself
    // This exception will be caught by the caller (runNewContainer) and re-thrown or handled.
    throw CommandFailureException("docker " + command, resultPair.second, resultPair.first, "docker run command failed.");
}

ContainerExecutionResult DockerManager::runNewContainer(const ContainerRunConfiguration& config) {
    if (config.imageName.empty()) {
        std::cerr << "DEBUG: runNewContainer failed - Image name is empty" << std::endl;
        throw OperationException("runNewContainer", "Image name cannot be empty.");
    }

    std::cout << "DEBUG: Starting container with image: " << config.imageName << std::endl;
    
    std::string cmdStr = buildDockerRunCommand(config);

    ContainerExecutionResult execResult;
    try {
        // Execute the 'docker run' command.
        // We set treatNonZeroExitAsError to false because 'docker run -d' might return non-zero
        // for application errors inside the container, but the container ID might still be valid.
        // However, if 'docker run' itself fails (e.g., image not found, daemon error), it will also be non-zero.
        auto resultPair = executeCliCommand(cmdStr, false); 
        
        // Check if the command execution itself was successful and produced output (potential container ID)
        if (resultPair.second == 0 && !resultPair.first.empty()) {
            handleSuccessfulContainerRun(execResult, resultPair.first, config);
        } else {
            // 'docker run' command failed or produced no output.
            // This could be due to various reasons like image not found, daemon issues, or invalid command.
            handleFailedContainerRun(execResult, cmdStr, resultPair);
        }

    } catch (const CommandFailureException& e) {
        // This catches failures from executeCliCommand (e.g., popen failure) or re-thrown from handleFailedContainerRun
        execResult.status = "Error";
        execResult.errorMessage = "Failed to execute docker run: " + std::string(e.what());
        execResult.exitCode = e.exitCode; 
        throw; // Re-throw the original or a new specific exception
    } catch (const DockerException& e) { // Catch other Docker-specific exceptions
        execResult.status = "Error";
        execResult.errorMessage = "Docker operation failed during runNewContainer: " + std::string(e.what());
        throw; // Re-throw
    }
    return execResult;
}


std::vector<ContainerInfo> DockerManager::listContainers(bool includeAll) const {
    std::string command = "ps ";
    if (includeAll) {
        command += "-a ";
    }
    
    // JSON formatında çıktı al - daha güvenli parsing için
    command += "--format \"{{json .}}\"";
    
    auto resultPair = executeCliCommand(command);
    std::vector<ContainerInfo> containers;
    std::stringstream ss(resultPair.first);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        
        try {
            // Her satır bir JSON objesidir
            QByteArray jsonData = QByteArray::fromStdString(line);
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
            
            if (jsonDoc.isNull() || !jsonDoc.isObject()) {
                std::cerr << "Warning: Invalid JSON data in container listing: " << line << std::endl;
                continue;
            }
            
            QJsonObject containerObj = jsonDoc.object();

            ContainerInfo info;
            info.containerId = containerObj[QStringLiteral("ID")].toString().toStdString();
            info.containerName = containerObj[QStringLiteral("Names")].toString().toStdString(); // Fixed
            info.containerImage = containerObj[QStringLiteral("Image")].toString().toStdString(); // Fixed
            info.containerStatus = containerObj[QStringLiteral("Status")].toString().toStdString(); // Fixed
            info.containerCreationTime = containerObj[QStringLiteral("CreatedAt")].toString().toStdString(); // Fixed

            containers.push_back(info);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Failed to parse container JSON: " << e.what() << " for line: " << line << std::endl;
        }
    }

    return containers;
}

// Helper to parse JSON output from 'docker inspect' into a ContainerInfo object
ContainerInfo DockerManager::parseInspectOutputToContainerInfo(const std::string& inspectJson, const std::string& containerIdOrName) const {
    ContainerInfo info;
    QByteArray jsonData = QByteArray::fromStdString(inspectJson);
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);

    if (jsonDoc.isNull() || !jsonDoc.isArray()) {
        throw ParsingException("Failed to parse JSON response from docker inspect for: " + containerIdOrName);
    }

    QJsonArray containerArray = jsonDoc.array();
    if (containerArray.isEmpty()) {
        throw ParsingException("Empty JSON array returned from docker inspect for: " + containerIdOrName);
    }

    QJsonObject container = containerArray.first().toObject();

    info.containerId = container[QStringLiteral("Id")].toString().toStdString();
    QString nameStr = container[QStringLiteral("Name")].toString();
    if (nameStr.startsWith(QLatin1Char('/'))) {
        nameStr.remove(0, 1);
    }
    info.containerName = nameStr.toStdString();
    info.containerImage = container[QStringLiteral("Config")].toObject()[QStringLiteral("Image")].toString().toStdString();
    QJsonObject state = container[QStringLiteral("State")].toObject();
    info.containerStatus = state[QStringLiteral("Status")].toString().toStdString();
    info.containerCreationTime = container[QStringLiteral("Created")].toString().toStdString();

    if (info.containerId.empty()) {
        throw ParsingException("getContainerDetails: Failed to parse essential details (containerId) from inspect output for: " + containerIdOrName);
    }
    return info;
}

// Helper to handle 'docker inspect' command execution and parsing
ContainerInfo DockerManager::inspectContainer(const std::string& containerIdOrName) const {
    std::stringstream ss;
    ss << "inspect " << std::quoted(containerIdOrName);
    std::string command = ss.str();
    auto resultPair = executeCliCommand(command, false); // Don't throw on non-zero, we'll check it.

    if (resultPair.second != 0) {
        // Check for common "not found" messages in the error output.
        if (resultPair.first.find("No such object") != std::string::npos || 
            resultPair.first.find("not found") != std::string::npos) {
            throw ContainerNotFoundException(containerIdOrName);
        }
        // For other errors, throw a generic command failure.
        throw CommandFailureException(command, resultPair.second, resultPair.first, "Failed to inspect container.");
    }
    
    // If command was successful (exit code 0), parse the JSON output.
    return parseInspectOutputToContainerInfo(resultPair.first, containerIdOrName);
}

ContainerInfo DockerManager::getContainerDetails(const std::string& containerIdOrName) const {
    if (containerIdOrName.empty()) {
        throw OperationException("getContainerDetails", "Container ID or name cannot be empty.");
    }
    try {
        return inspectContainer(containerIdOrName);
    } catch (const ContainerNotFoundException&) {
        // Re-throw specific exception if container is not found.
        throw; 
    } catch (const ParsingException&) {
        // Re-throw specific exception if JSON parsing fails.
        throw; 
    } catch (const CommandFailureException& e) {
        // Wrap other command failures in an OperationException for consistent API error reporting.
        throw OperationException("getContainerDetails", "Failed to get container details for '" + containerIdOrName + "': " + e.what());
    } catch (const DockerException& e) { // Catch any other Docker specific errors
        throw OperationException("getContainerDetails", "An unexpected Docker error occurred while getting details for '" + containerIdOrName + "': " + e.what());
    }
}

void DockerManager::startContainer(const std::string& containerIdOrName) {
    if (containerIdOrName.empty()) {
        throw OperationException("startContainer", "Container ID or name cannot be empty.");
    }
    std::stringstream ss;
    ss << "start " << std::quoted(containerIdOrName);
    std::string command = ss.str();
    try {
        auto resultPair = executeCliCommand(command, false); // We'll handle the error condition ourselves.
        if (resultPair.second != 0) {
            if (resultPair.first.find("No such container") != std::string::npos) {
                throw ContainerNotFoundException(containerIdOrName);
            }
            // Other errors (e.g.: already running) can be handled as OperationException.
            throw OperationException("startContainer", "Failed to start container '" + containerIdOrName + "'. Output: " + resultPair.first);
        }
    } catch (const CommandFailureException& e) { // General error from executeCliCommand
         throw OperationException("startContainer", "Command execution failed for starting container '" + containerIdOrName + "': " + e.what());
    }
}

void DockerManager::stopContainer(const std::string& containerIdOrName, int timeoutSeconds) {
     if (containerIdOrName.empty()) {
        throw OperationException("stopContainer", "Container ID or name cannot be empty.");
    }
    std::stringstream cmd;
    cmd << "stop ";
    if (timeoutSeconds > 0) {
        cmd << "-t " << timeoutSeconds << " ";
    }
    cmd << std::quoted(containerIdOrName);
    try {
        auto resultPair = executeCliCommand(cmd.str(), false);
        if (resultPair.second != 0) {
            if (resultPair.first.find("No such container") != std::string::npos) {
                throw ContainerNotFoundException(containerIdOrName);
            }
            throw OperationException("stopContainer", "Failed to stop container '" + containerIdOrName + "'. Output: " + resultPair.first);
        }
    } catch (const CommandFailureException& e) {
         throw OperationException("stopContainer", "Command execution failed for stopping container '" + containerIdOrName + "': " + e.what());
    }
}

void DockerManager::removeContainer(const std::string& containerIdOrName, bool force, bool removeVolumes) {
    if (containerIdOrName.empty()) {
        throw OperationException("removeContainer", "Container ID or name cannot be empty.");
    }
    std::stringstream cmd;
    cmd << "rm ";
    if (force) {
        cmd << "-f ";
    }
    if (removeVolumes) {
        cmd << "-v ";
    }
    cmd << std::quoted(containerIdOrName);
    try {
        auto resultPair = executeCliCommand(cmd.str(), false);
        if (resultPair.second != 0) {
            if (resultPair.first.find("No such container") != std::string::npos) {
                throw ContainerNotFoundException(containerIdOrName);
            }
             // If the container is running and force=false, it will generate an error.
            if (resultPair.first.find("is running") != std::string::npos && !force) {
                 throw OperationException("removeContainer", "Cannot remove running container '" + containerIdOrName + "' without force. Output: " + resultPair.first);
            }
            throw OperationException("removeContainer", "Failed to remove container '" + containerIdOrName + "'. Output: " + resultPair.first);
        }
    } catch (const CommandFailureException& e) {
         throw OperationException("removeContainer", "Command execution failed for removing container '" + containerIdOrName + "': " + e.what());
    }
}

// --- Container Operations ---
std::string DockerManager::getContainerLogs(const std::string& containerIdOrName, int tailLines) const {
    if (containerIdOrName.empty()) {
        throw OperationException("getContainerLogs", "Container ID or name cannot be empty.");
    }
    std::stringstream cmd;
    cmd << "logs ";
    if (tailLines > 0) {
        cmd << "--tail " << tailLines << " ";
    }
    cmd << std::quoted(containerIdOrName);
    try {
        auto resultPair = executeCliCommand(cmd.str(), false);
         if (resultPair.second != 0) {
            if (resultPair.first.find("No such container") != std::string::npos || resultPair.first.find("No such object") != std::string::npos) {
                throw ContainerNotFoundException(containerIdOrName);
            }
            throw CommandFailureException(cmd.str(), resultPair.second, resultPair.first, "Failed to get logs for container '" + containerIdOrName + "'.");
        }
        return resultPair.first;
    } catch (const CommandFailureException& e) {
         throw OperationException("getContainerLogs", "Command execution failed for getting logs of container '" + containerIdOrName + "': " + e.what());
    }
}

std::string DockerManager::executeCommandInContainer(const std::string& containerIdOrName, const std::vector<std::string>& commandArgs, const std::string& user) const {
    if (containerIdOrName.empty()) {
        throw OperationException("executeCommandInContainer", "Container ID or name cannot be empty.");
    }
    if (commandArgs.empty()) {
        throw OperationException("executeCommandInContainer", "Command to execute cannot be empty.");
    }
    std::stringstream cmd;
    cmd << "exec ";
    if (!user.empty()) {
        cmd << "-u " << std::quoted(user) << " ";
    }
    // `docker exec` genellikle interaktif olmayan komutlar için `-i` veya `-t` gerektirmez.
    cmd << std::quoted(containerIdOrName) << " " << buildCommandString(commandArgs);
    
    try {
        auto resultPair = executeCliCommand(cmd.str(), false); // Hata kodunu kendimiz yorumlayalım.
        if (resultPair.second != 0) {
             if (resultPair.first.find("No such container") != std::string::npos) {
                throw ContainerNotFoundException(containerIdOrName);
            }
            // If the container is not running, it may return an error like "is not running".
            if (resultPair.first.find("is not running") != std::string::npos) {
                throw OperationException("executeCommandInContainer", "Container '" + containerIdOrName + "' is not running. Output: " + resultPair.first);
            }
            // If `docker exec` itself fails (e.g., container not found), we throw an exception.
            // The exit code of the internal command will be in `resultPair.second`.
            throw CommandFailureException(cmd.str(), resultPair.second, resultPair.first, "Execution of command in container '" + containerIdOrName + "' failed.");
        }
        return resultPair.first; // Komutun çıktısı
    } catch (const ContainerNotFoundException& ) {
        throw;
    } catch (const OperationException& ) {
        throw;
    } catch (const CommandFailureException& e) { // General error from executeCliCommand
         throw OperationException("executeCommandInContainer", "Base command execution failed for exec in container '" + containerIdOrName + "': " + e.what());
    }
}

void DockerManager::enterInteractiveShell(const std::string& containerIdOrName) {
    if (containerIdOrName.empty()) {
        throw OperationException("enterInteractiveShell", "Container ID or name cannot be empty.");
    }
    
    // First verify container exists and is running before attempting to open shell
    try {
        ContainerInfo info = getContainerDetails(containerIdOrName);
        
        // Check if container is running
        if (info.containerStatus.find("Up") == std::string::npos && 
            info.containerStatus.find("running") == std::string::npos) {
            throw OperationException("enterInteractiveShell", 
                "Container '" + containerIdOrName + "' is not running. Current status: " + info.containerStatus);
        }
        
        // Try /bin/bash first, fall back to /bin/sh if bash is not available
        std::vector<std::string> shellCommand = {"/bin/bash"};
        std::string shellOutput;
        bool bashAvailable = false;
        
        try {
            // Check if bash exists in the container
            shellOutput = executeCommandInContainer(containerIdOrName, {"test", "-e", "/bin/bash", "&&", "echo", "available"});
            if (shellOutput.find("available") != std::string::npos) {
                bashAvailable = true;
            }
        } catch (const std::exception&) {
            // If command fails, assume bash is not available
            bashAvailable = false;
        }
        
        if (!bashAvailable) {
            shellCommand[0] = "/bin/sh";
        }
        
        // Prepare the interactive shell command
        std::stringstream ss;
        ss << "docker exec -it " << std::quoted(containerIdOrName) << " " << shellCommand[0];
        std::string command = ss.str();
        
        // Inform the user about what's happening
        std::cout << "Opening interactive " << (bashAvailable ? "bash" : "sh") << " shell to container '" 
                  << info.containerName << "' (" << info.containerId.substr(0, 12) << ")" << std::endl;
        std::cout << "Type 'exit' or press Ctrl+D to exit the shell." << std::endl;
        
        // Execute the shell with proper security measures
        // Note: system() is still used here because we need an interactive terminal.
        // A proper library implementation might use execvp or similar functions.
        int exitCode = system(command.c_str());
        
        if (exitCode != 0) {
            std::cerr << "Interactive shell exited with code: " << exitCode << std::endl;
            throw OperationException("enterInteractiveShell", 
                "Failed to open interactive shell for container '" + containerIdOrName + 
                "', exit code: " + std::to_string(exitCode));
        }
        
    } catch (const ContainerNotFoundException& e) {
        throw OperationException("enterInteractiveShell", 
            "Cannot open shell: " + std::string(e.what()));
    } catch (const CommandFailureException& e) {
        throw OperationException("enterInteractiveShell", 
            "Command execution failed: " + std::string(e.what()));
    }
}

// --- Image Management ---
bool DockerManager::imageExists(const std::string& imageNameWithTag) const {
    if (imageNameWithTag.empty()) {
        throw OperationException("imageExists", "Image name cannot be empty.");
    }
    // `docker image inspect` or `docker images --filter` can be used.
    // `docker image inspect <image>` command returns 0 if the image exists, 1 otherwise.
    std::stringstream ss;
    ss << "image inspect " << std::quoted(imageNameWithTag);
    std::string command = ss.str();
    try {
        auto resultPair = executeCliCommand(command, false); // We will handle the error state ourselves.
        return resultPair.second == 0; // Exit code 0 means the image exists.
    } catch (const CommandFailureException& e) {
        // executeCliCommand can throw in case of a popen error, etc.
        // This does not mean the image does not exist, it means the command could not be executed.
        throw OperationException("imageExists", "Failed to check image existence for '" + imageNameWithTag + "': " + e.what());
    }
}

void DockerManager::pullImage(const std::string& imageNameWithTag) {
    if (imageNameWithTag.empty()) {
        throw OperationException("pullImage", "Image name cannot be empty.");
    }
    std::stringstream ss;
    ss << "pull " << std::quoted(imageNameWithTag);
    std::string command = ss.str();
    try {
        // `docker pull` uzun sürebilir. executeCliCommand blocking olduğu için bekleyecektir.
        auto resultPair = executeCliCommand(command); // Başarısız olursa exception fırlatır.
        // Başarılı pull işlemi genellikle logları stdout'a basar.
        // resultPair.first içinde bu loglar olacaktır.
        // Ekstra bir kontrol (örn: loglarda "Status: Downloaded newer image" gibi) yapılabilir.
        if (resultPair.first.find("Pulling from") == std::string::npos && resultPair.first.find("Image is up to date") == std::string::npos && resultPair.first.find("Downloaded newer image") == std::string::npos) {
            // If expected phrases are not in the output, there might be an issue.
            // However, since `executeCliCommand` already throws an exception for non-zero exit codes,
            // this check might be redundant.
        }
    } catch (const CommandFailureException& e) {
        // `docker pull` failed (e.g., image not found, network error).
        // Accessing e.what() is standard for std::exception derived classes.
        // If CommandFailureException has a more specific message member, use that, otherwise e.what() is fine.
        // Assuming CommandFailureException is derived from std::exception and e.what() provides a good message.
        if (e.errorOutput.find("manifest for " + imageNameWithTag + " not found") != std::string::npos ||
            e.errorOutput.find("image not found") != std::string::npos) {
            throw ImageNotFoundException(imageNameWithTag, "Image not found or manifest error during pull: " + std::string(e.what()));
        } else {
            throw OperationException("pullImage", "Failed to pull image '" + imageNameWithTag + "': " + std::string(e.what()));
        }
    }
}

std::vector<ImageInfo> DockerManager::listImages() const {
    // Get the image list in JSON format
    std::string command = "images --format \"{{json .}}\"";

    auto resultPair = executeCliCommand(command);
    std::vector<ImageInfo> images;

    std::stringstream ss(resultPair.first);
    std::string line;

    while (std::getline(ss, line)) {
        line = trim(line); // Trim leading/trailing whitespace from each line
        if (line.empty()) continue;

        try {
            // Each line is a JSON object
            QByteArray jsonData = QByteArray::fromStdString(line);
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);

            if (jsonDoc.isNull() || !jsonDoc.isObject()) {
                std::cerr << "Warning: Invalid JSON data in image listing line: \n---\n" << line << "\n---\nSkipping." << std::endl;
                continue;
            }
            
            QJsonObject imageObj = jsonDoc.object();
            std::string id = imageObj.value(QStringLiteral("ID")).toString().toStdString();
            std::string repository = imageObj.value(QStringLiteral("Repository")).toString().toStdString();
            std::string tag = imageObj.value(QStringLiteral("Tag")).toString().toStdString();
            std::string size = imageObj.value(QStringLiteral("Size")).toString().toStdString();
            std::string created = imageObj.value(QStringLiteral("CreatedAt")).toString().toStdString(); 

            if (id.rfind("sha256:", 0) == 0) { 
                id = id.substr(7);
            }
            
            ImageInfo imgInfo;
            imgInfo.id = id;
            imgInfo.repository = repository;
            imgInfo.tag = tag;
            imgInfo.size = size;
            imgInfo.created = created;
            
            images.push_back(imgInfo);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Failed to parse image JSON line: \\n---\\n" << line << "\\n---\\nError: " << e.what() << std::endl;
        }
    }
    
    return images;
}

void DockerManager::removeImage(const std::string& imageNameOrId, bool force) {
    if (imageNameOrId.empty()) {
        throw OperationException("removeImage", "Image name or ID cannot be empty.");
    }
    std::stringstream cmd;
    cmd << "rmi ";
    if (force) {
        cmd << "-f ";
    }
    cmd << std::quoted(imageNameOrId);
    try {
        auto resultPair = executeCliCommand(cmd.str(), false); // Hata durumunu kendimiz yöneteceğiz.
        if (resultPair.second != 0) {
            if (resultPair.first.find("No such image") != std::string::npos) {
                throw ImageNotFoundException(imageNameOrId);
            }
            // İmaj kullanımdaysa ve force=false ise hata verir.
            if (resultPair.first.find("image is referenced in one or more repositories") != std::string::npos ||
                resultPair.first.find("image is being used by running container") != std::string::npos ||
                resultPair.first.find("conflict: unable to remove repository reference") != std::string::npos && !force) {
                 throw OperationException("removeImage", "Cannot remove image '" + imageNameOrId + "' as it is in use or referenced. Use force option. Output: " + resultPair.first);
            }
            throw OperationException("removeImage", "Failed to remove image '" + imageNameOrId + "'. Output: " + resultPair.first);
        }
    } catch (const CommandFailureException& e) {
         throw OperationException("removeImage", "Command execution failed for removing image '" + imageNameOrId + "': " + e.what());
    }
}

// Add file copying methods to DockerManager

void DockerManager::copyFileToContainer(const std::string& containerIdOrName, // Changed from containerId for consistency
                                       const std::string& hostPath,
                                       const std::string& containerPath) const {
    if (containerIdOrName.empty()) {
        throw OperationException("copyFileToContainer", "Container ID cannot be empty.");
    }
    if (hostPath.empty()) {
        throw OperationException("copyFileToContainer", "Host path cannot be empty.");
    }
    if (containerPath.empty()) {
        throw OperationException("copyFileToContainer", "Container path cannot be empty.");
    }
    
    std::stringstream cmd;
    cmd << "cp " << std::quoted(hostPath) << " " << std::quoted(containerIdOrName + ":" + containerPath);
    
    try {
        auto resultPair = executeCliCommand(cmd.str(), false);
        if (resultPair.second != 0) {
            if (resultPair.first.find("No such container") != std::string::npos) {
                throw ContainerNotFoundException(containerIdOrName);
            }
            throw OperationException("copyFileToContainer", 
                "Failed to copy file to container '" + containerIdOrName + "'. Output: " + resultPair.first);
        }
    } catch (const CommandFailureException& e) {
        throw OperationException("copyFileToContainer", 
            "Command execution failed for copying file to container '" + containerIdOrName + "': " + e.what());
    }
}

void DockerManager::copyFileFromContainer(const std::string& containerIdOrName, // Changed from containerId for consistency
                                         const std::string& containerPath,
                                         const std::string& hostPath) const {
    if (containerIdOrName.empty()) {
        throw OperationException("copyFileFromContainer", "Container ID cannot be empty.");
    }
    if (containerPath.empty()) {
        throw OperationException("copyFileFromContainer", "Container path cannot be empty.");
    }
    if (hostPath.empty()) {
        throw OperationException("copyFileFromContainer", "Host path cannot be empty.");
    }
    
    std::stringstream cmd;
    cmd << "cp " << std::quoted(containerIdOrName + ":" + containerPath) << " " << std::quoted(hostPath);
    
    try {
        auto resultPair = executeCliCommand(cmd.str(), false);
        if (resultPair.second != 0) {
            if (resultPair.first.find("No such container") != std::string::npos) {
                throw ContainerNotFoundException(containerIdOrName);
            }
            throw OperationException("copyFileFromContainer", 
                "Failed to copy file from container '" + containerIdOrName + "'. Output: " + resultPair.first);
        }
    } catch (const CommandFailureException& e) {
        throw OperationException("copyFileFromContainer", 
            "Command execution failed for copying file from container '" + containerIdOrName + "': " + e.what());
    }
}

} // namespace Docker
