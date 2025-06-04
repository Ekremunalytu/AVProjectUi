#include "docker/DockerManager.h"
#include "MainWindow.h"
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <QApplication>

// Function prototypes
void printContainerList(const std::vector<Docker::ContainerInfo>& containers);
void handleContainerOperations(Docker::DockerManager& manager, const std::string& containerId);
void checkDaemonStatus(Docker::DockerManager& manager);
void testFileScanning();

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Command-line arguments
    bool useGui = true;
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "--cli" || arg == "-c") {
            useGui = false;
            break;
        }
    }
    
    if (useGui) {
        // Create and show the main window
        MainWindow mainWindow;
        mainWindow.show();
        
        return app.exec();
    }
    
    // CLI Version (legacy mode)
    std::cout << "Docker Management CLI" << std::endl;
    std::cout << "====================" << std::endl;
    
    // Create Docker manager instance
    Docker::DockerManager dockerManager;
    
    try {
        // Check Docker daemon status first
        checkDaemonStatus(dockerManager);
        
        // Show test file scanning functionality
        testFileScanning();
        
        // List containers
        std::cout << "\nListing all containers..." << std::endl;
        auto containers = dockerManager.listContainers(true); // Include all containers, not just running ones
        
        if (containers.empty()) {
            std::cout << "No containers found." << std::endl;
            return 0;
        }
        
        // Print container list with indices
        printContainerList(containers);
        
        // Ask user which container to interact with
        std::cout << "\nEnter the NUMBER of the container to interact with (or 0 to exit): ";
        int choice;
        std::cin >> choice;
        
        if (choice == 0 || choice > static_cast<int>(containers.size())) {
            std::cout << "Exiting program." << std::endl;
            return 0;
        }

        // Get selected container info
        std::string selectedContainerId = containers[choice - 1].containerId;
        
        // Handle operations on selected container
        handleContainerOperations(dockerManager, selectedContainerId);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

// Check if Docker daemon is running
void checkDaemonStatus(Docker::DockerManager& manager) {
    std::cout << "Checking Docker daemon status..." << std::endl;
    try {
        if (manager.isDaemonRunning()) {
            std::cout << "Docker daemon is running." << std::endl;
        }
    } catch (const Docker::DaemonException& e) {
        std::cerr << "Docker daemon error: " << e.what() << std::endl;
        throw;
    }
}

// Display formatted list of containers
void printContainerList(const std::vector<Docker::ContainerInfo>& containers) {
    std::cout << "\nAvailable containers:" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "NUM  CONTAINER ID\tNAME\t\tSTATUS\t\tIMAGE" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    
    for (size_t i = 0; i < containers.size(); ++i) {
        const auto& container = containers[i];
        std::string shortId = container.containerId.substr(0, 12); // Use first 12 chars of ID
        
        std::cout << i + 1 << "    " 
                  << shortId << "\t" 
                  << container.containerName << "\t\t" 
                  << container.containerStatus << "\t\t"
                  << container.containerImage << std::endl;
    }
}

// Handle various operations on a selected container
void handleContainerOperations(Docker::DockerManager& manager, const std::string& containerId) {
    try {
        // Get detailed container information
        Docker::ContainerInfo containerInfo = manager.getContainerDetails(containerId);
        
        std::cout << "\nSelected container: " << containerInfo.containerName << std::endl;
        std::cout << "Status: " << containerInfo.containerStatus << std::endl;
        
        // If container is not running, ask to start it
        if (containerInfo.containerStatus.find("Up") == std::string::npos && 
            containerInfo.containerStatus != "running") {
            
            std::cout << "\nContainer is not running. Would you like to start it? (y/n): ";
            char startChoice;
            std::cin >> startChoice;
            
            if (startChoice == 'y' || startChoice == 'Y') {
                std::cout << "Starting container..." << std::endl;
                manager.startContainer(containerId);
                std::cout << "Container started successfully." << std::endl;
                
                // Refresh container info
                containerInfo = manager.getContainerDetails(containerId);
            } else {
                std::cout << "Container will not be started. Exiting." << std::endl;
                return;
            }
        }
        
        // Container should be running now - offer options
        std::cout << "\nContainer Operations:" << std::endl;
        std::cout << "1. View container logs" << std::endl;
        std::cout << "2. Execute command in container" << std::endl;
        std::cout << "3. Enter interactive shell" << std::endl;
        std::cout << "4. Stop container" << std::endl;
        std::cout << "0. Exit" << std::endl;
        
        std::cout << "\nSelect an operation: ";
        int opChoice;
        std::cin >> opChoice;
        std::cin.ignore(); // Clear input buffer
        
        switch (opChoice) {
            case 0:
                std::cout << "Exiting..." << std::endl;
                break;
            
            case 1: {
                std::cout << "Getting container logs..." << std::endl;
                std::cout << "How many lines to display? (0 for all): ";
                int lines;
                std::cin >> lines;
                std::string logs = manager.getContainerLogs(containerId, lines);
                std::cout << "\n--- Container Logs ---\n" << logs << std::endl;
                break;
            }
            
            case 2: {
                std::cout << "Enter command to execute: ";
                std::string cmd;
                std::getline(std::cin, cmd);
                
                // Simple tokenization (in a real app you might want a more robust solution)
                std::vector<std::string> cmdArgs;
                size_t pos = 0;
                std::string token;
                std::string delimiter = " ";
                
                while ((pos = cmd.find(delimiter)) != std::string::npos) {
                    token = cmd.substr(0, pos);
                    if (!token.empty()) {
                        cmdArgs.push_back(token);
                    }
                    cmd.erase(0, pos + delimiter.length());
                }
                if (!cmd.empty()) {
                    cmdArgs.push_back(cmd);
                }
                
                if (!cmdArgs.empty()) {
                    std::cout << "Executing command in container..." << std::endl;
                    std::string output = manager.executeCommandInContainer(containerId, cmdArgs);
                    std::cout << "\n--- Command Output ---\n" << output << std::endl;
                }
                break;
            }
            
            case 3:
                std::cout << "Opening interactive shell..." << std::endl;
                manager.enterInteractiveShell(containerId);
                break;
            
            case 4:
                std::cout << "Stopping container..." << std::endl;
                std::cout << "Timeout in seconds (10 is default): ";
                int timeout;
                std::cin >> timeout;
                manager.stopContainer(containerId, timeout);
                std::cout << "Container stopped successfully." << std::endl;
                break;
            
            default:
                std::cout << "Invalid choice." << std::endl;
                break;
        }
        
    } catch (const Docker::ContainerNotFoundException& e) {
        std::cerr << "Container not found: " << e.what() << std::endl;
    } catch (const Docker::OperationException& e) {
        std::cerr << "Operation failed: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

// Test file scanning functionality (placeholder for CDR and Sandbox integration)
void testFileScanning() {
    std::cout << "\n=== File Scanning Test ===" << std::endl;
    std::cout << "This function simulates selecting a file and sending it to CDR and Sandbox containers." << std::endl;
    
    // In a real application, this would be a file dialog or other selection mechanism
    std::string selectedFilePath = "/path/to/test/file.pdf"; // Placeholder path
    std::cout << "Selected file: " << selectedFilePath << std::endl;
    
    // Simulate sending to CDR container
    std::cout << "\nSending file to CDR container..." << std::endl;
    try {
        Docker::DockerManager dockerManager;
        Docker::ContainerRunConfiguration cdrConfig;
        
        // Configure CDR container
        cdrConfig.imageName = "cdr-module:latest"; // Placeholder image name
        cdrConfig.command = {"/app/scan.sh", selectedFilePath};
        
        // Mount the selected file in the container
        Docker::MountPoint fileMount;
        fileMount.mountType = "bind";
        fileMount.source = selectedFilePath;
        fileMount.destination = "/scan/file.pdf";
        fileMount.readOnly = true;
        cdrConfig.hostConfig.mounts.push_back(fileMount);
        
        std::cout << "CDR container configuration ready (simulation only)." << std::endl;
        // In actual implementation: dockerManager.runNewContainer(cdrConfig);
        
    } catch (const std::exception& e) {
        std::cerr << "CDR container simulation error: " << e.what() << std::endl;
    }
    
    // Simulate sending to Sandbox container
    std::cout << "\nSending file to Sandbox container..." << std::endl;
    try {
        Docker::DockerManager dockerManager;
        Docker::ContainerRunConfiguration sandboxConfig;
        
        // Configure Sandbox container
        sandboxConfig.imageName = "sandbox-module:latest"; // Placeholder image name
        sandboxConfig.command = {"/app/analyze.sh", selectedFilePath};
        sandboxConfig.hostConfig.privileged = true; // Sandbox might need more privileges
        
        // Mount the selected file in the container
        Docker::MountPoint fileMount;
        fileMount.mountType = "bind";
        fileMount.source = selectedFilePath;
        fileMount.destination = "/analyze/target.bin";
        fileMount.readOnly = true;
        sandboxConfig.hostConfig.mounts.push_back(fileMount);
        
        std::cout << "Sandbox container configuration ready (simulation only)." << std::endl;
        // In actual implementation: dockerManager.runNewContainer(sandboxConfig);
        
    } catch (const std::exception& e) {
        std::cerr << "Sandbox container simulation error: " << e.what() << std::endl;
    }
    
    std::cout << "\nFile scanning test completed." << std::endl;
    std::cout << "Note: This is a simulation. The actual implementation will be developed later." << std::endl;
}