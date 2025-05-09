#ifndef IREMOTE_SCANNER_H
#define IREMOTE_SCANNER_H

#include "IScanner.h"

/**
 * @brief The IRemoteScanner interface extends IScanner for scanners that interact
 *        with remote analysis services (e.g., VirusTotal, Hybrid Analysis).
 */
class IRemoteScanner : public IScanner {
public:
    virtual ~IRemoteScanner() = default;
    
    /**
     * @brief Submits the selected file to a remote analysis service.
     *
     * @param apiKey The API key required for authenticating with the remote service.
     *               Can be optional if the service allows anonymous submissions or
     *               uses other authentication methods.
     * @return True if the file was successfully submitted to the remote service, false otherwise.
     */
    virtual bool submitToRemoteService(const QString& apiKey = QString()) = 0;
    
    /**
     * @brief Retrieves the status of the file submission to the remote service.
     *
     * This could include information like 'pending', 'in_progress', 'completed',
     * 'error', or a more detailed analysis report ID.
     * @return A QString describing the status of the remote submission.
     */
    virtual QString getSubmissionStatus() const = 0;
};

#endif // IREMOTE_SCANNER_H 