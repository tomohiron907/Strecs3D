#pragma once

#include <QObject>
#include "../widgets/process/ProcessFlowWidget.h"

class ApplicationController;
class MainWindowUIAdapter;
class ProcessManagerWidget;
class UIState;
class VisualizationManager;
class QWidget;

/**
 * @brief Controller for managing simulation and process operations
 *
 * This controller handles:
 * - Running FEM simulations
 * - Rolling back to previous process steps
 */
class ProcessController : public QObject {
    Q_OBJECT
public:
    explicit ProcessController(
        ApplicationController* appController,
        MainWindowUIAdapter* uiAdapter,
        ProcessManagerWidget* processManager,
        UIState* uiState,
        QWidget* parent = nullptr);
    ~ProcessController() = default;

    /**
     * @brief Run the FEM simulation pipeline
     */
    void runSimulation();

    /**
     * @brief Rollback to a specific process step
     */
    void rollbackToStep(ProcessStep targetStep);

signals:
    void simulationCompleted();
    void rollbackCompleted(ProcessStep step);

private:
    /**
     * @brief Clear data for steps from fromIdx to toIdx (exclusive)
     */
    void clearStepData(int targetIdx, int currentIdx);

    ApplicationController* appController_;
    MainWindowUIAdapter* uiAdapter_;
    ProcessManagerWidget* processManager_;
    UIState* uiState_;
    QWidget* parentWidget_;
};
