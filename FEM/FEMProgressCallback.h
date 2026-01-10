#ifndef FEMPROGRESSCALLBACK_H
#define FEMPROGRESSCALLBACK_H

#include <string>
#include <functional>

/**
 * Callback interface for FEM analysis progress reporting
 * This allows the FEM layer to report progress without Qt dependencies
 */
class FEMProgressCallback {
public:
    virtual ~FEMProgressCallback() = default;

    /**
     * Report progress
     * @param progress Progress value 0-100
     * @param message Optional status message
     */
    virtual void reportProgress(int progress, const std::string& message = "") = 0;

    /**
     * Check if cancellation was requested
     * @return true if user requested cancellation
     */
    virtual bool isCancelled() const = 0;
};

// Convenience typedef for lambda-based callbacks
using ProgressCallbackFn = std::function<void(int, const std::string&)>;
using CancelCheckFn = std::function<bool()>;

/**
 * Simple implementation using function objects
 */
class SimpleFEMProgressCallback : public FEMProgressCallback {
public:
    SimpleFEMProgressCallback(ProgressCallbackFn progressFn, CancelCheckFn cancelFn = nullptr)
        : m_progressFn(progressFn), m_cancelFn(cancelFn) {}

    void reportProgress(int progress, const std::string& message = "") override {
        if (m_progressFn) m_progressFn(progress, message);
    }

    bool isCancelled() const override {
        return m_cancelFn ? m_cancelFn() : false;
    }

private:
    ProgressCallbackFn m_progressFn;
    CancelCheckFn m_cancelFn;
};

#endif // FEMPROGRESSCALLBACK_H
