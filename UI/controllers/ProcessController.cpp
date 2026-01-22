#include "ProcessController.h"
#include "../../core/application/ApplicationController.h"
#include "../../core/application/MainWindowUIAdapter.h"
#include "../../core/ui/UIState.h"
#include "../visualization/VisualizationManager.h"
#include "../widgets/process/ProcessManagerWidget.h"
#include "../../core/commands/processing/RunFEMPipelineCommand.h"
#include "../../utils/tempPathUtility.h"

#include <QWidget>
#include <QDir>
#include <QMessageBox>
#include <QDebug>

ProcessController::ProcessController(
    ApplicationController* appController,
    MainWindowUIAdapter* uiAdapter,
    ProcessManagerWidget* processManager,
    UIState* uiState,
    QWidget* parent)
    : QObject(parent)
    , appController_(appController)
    , uiAdapter_(uiAdapter)
    , processManager_(processManager)
    , uiState_(uiState)
    , parentWidget_(parent)
{
}

void ProcessController::runSimulation()
{
    qDebug() << "Starting FEM analysis pipeline...";

    if (!uiState_) {
        qDebug() << "Error: UIState is null";
        QMessageBox::critical(parentWidget_, "Error", "UIState is not available");
        return;
    }

    // Create FEM temp directory
    QString femTempDir = TempPathUtility::getTempSubDir("FEM");
    QDir dir;
    if (!dir.exists(femTempDir)) {
        dir.mkpath(femTempDir);
        qDebug() << "Created FEM temp directory:" << femTempDir;
    }

    QString outputPath = femTempDir + "/simulation_condition.json";

    // Execute FEM pipeline using command pattern
    auto command = std::make_unique<RunFEMPipelineCommand>(
        appController_,
        uiAdapter_,
        uiState_,
        outputPath
    );
    command->execute();

    qDebug() << "FEM analysis pipeline completed.";

    // Notify ProcessManager to advance step
    if (processManager_) {
        processManager_->onSimulationCompleted();
    }

    emit simulationCompleted();
}

void ProcessController::rollbackToStep(ProcessStep targetStep)
{
    qDebug() << QString("Rolling back to step %1").arg(static_cast<int>(targetStep));

    if (!uiState_) {
        QMessageBox::critical(parentWidget_, "Error", "UIState not available");
        return;
    }

    VisualizationManager* visManager = uiAdapter_ ? uiAdapter_->getVisualizationManager() : nullptr;
    if (!visManager) {
        QMessageBox::critical(parentWidget_, "Error", "VisualizationManager not available");
        return;
    }

    if (!processManager_) {
        return;
    }

    ProcessStep currentStep = processManager_->getFlowWidget()->currentStep();
    clearStepData(static_cast<int>(targetStep), static_cast<int>(currentStep));

    // Update ProcessManager UI
    processManager_->rollbackToStep(targetStep);

    qDebug() << "Rollback completed";
    emit rollbackCompleted(targetStep);
}

void ProcessController::clearStepData(int targetIdx, int currentIdx)
{
    VisualizationManager* visManager = uiAdapter_ ? uiAdapter_->getVisualizationManager() : nullptr;
    if (!visManager || !uiState_) return;

    // Step 4: Infill Map (if current >= 3 && target < 3)
    if (currentIdx >= 3 && targetIdx < 3) {
        qDebug() << "Clearing infill data...";
        uiState_->clearAllInfillRegions();
        visManager->clearInfillActors();
    }

    // Step 3: Simulation (if current >= 2 && target < 2)
    if (currentIdx >= 2 && targetIdx < 2) {
        qDebug() << "Clearing simulation data...";
        uiState_->clearSimulationResult();
        visManager->clearSimulationActors();
    }

    // Step 2: Boundary Conditions (if current >= 1 && target < 1)
    if (currentIdx >= 1 && targetIdx < 1) {
        qDebug() << "Clearing boundary conditions...";
        uiState_->clearLoadConditions();
        uiState_->clearConstraintConditions();
        visManager->clearBoundaryConditions();
    }

    // Step 1: Import STEP (if target == 0)
    if (targetIdx == 0) {
        qDebug() << "Clearing STEP file...";
        uiState_->clearStepFile();
        visManager->clearStepFileActors();
        visManager->resetStepReader();
    }
}
