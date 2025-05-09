#ifndef IDOCKER_SCANNER_H
#define IDOCKER_SCANNER_H

#include "IScanner.h"

/**
 * @brief The IDockerScanner interface extends IScanner for scanners that utilize
 *        Docker containers for their operations, such as Content Disarm and
 *        Reconstruction (CDR) or sandboxing.
 */
class IDockerScanner : public IScanner {
public:
    virtual ~IDockerScanner() = default;
    
    /**
     * @brief Submits the selected file to a specified Docker container for processing.
     * 
     * @param containerName The name or ID of the Docker container to which the file
     *                      should be submitted.
     * @return True if the file was successfully submitted to the container, false otherwise.
     */
    virtual bool submitToContainer(const QString& containerName) = 0;
    
    /**
     * @brief Checks if the associated Docker container is ready to process files.
     *
     * This can involve checking if the container is running, not busy with
     * another task, or has all necessary dependencies initialized.
     * @return True if the container is available and ready for processing, false otherwise.
     */
    virtual bool isContainerReady() const = 0;
    
    /**
     * @brief Retrieves the current status of the Docker container.
     *
     * This could include information like 'running', 'stopped', 'starting',
     * 'processing', 'error', etc.
     * @return A QString describing the current status of the container.
     */
    virtual QString getContainerStatus() const = 0;
};

#endif // IDOCKER_SCANNER_H 