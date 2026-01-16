#include "mainwindow.h"
#include "core/application/MainWindowUIAdapter.h"
#include "UI/visualization/VisualizationManager.h"

#include "core/commands/file/OpenVtkFileCommand.h"
#include "core/commands/file/OpenStepFileCommand.h"
#include "core/commands/processing/ProcessFilesCommand.h"
#include "core/commands/processing/Export3mfCommand.h"
#include "core/commands/processing/RunFEMPipelineCommand.h"
#include "core/commands/state/SetStressRangeCommand.h"

#include "core/commands/state/SetStressDensityMappingCommand.h"
#include "core/commands/state/SetConstraintConditionCommand.h"
#include "core/commands/state/SetLoadConditionCommand.h"
#include "core/commands/state/UpdateConstraintConditionCommand.h"
#include "core/commands/state/UpdateLoadConditionCommand.h"
#include "core/commands/visualization/SetMeshVisibilityCommand.h"
#include "core/commands/visualization/SetMeshOpacityCommand.h"
#include "core/commands/visualization/SetMeshOpacityCommand.h"
#include "utils/tempPathUtility.h"
#include <QPushButton>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>

#include <memory>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    initializeComponents();
    setupWindow();
    connectSignals();
}

void MainWindow::logMessage(const QString& message)
{
    qDebug() << message;
}

void MainWindow::initializeComponents()
{
    ui = std::make_unique<MainWindowUI>(this);
    uiAdapter = std::make_unique<MainWindowUIAdapter>(ui.get(), this);
    appController = std::make_unique<ApplicationController>(this);
}

void MainWindow::setupWindow()
{
    setWindowTitle("Strecs3D");
    setCentralWidget(ui->getCentralWidget());
    resize(1600, 900);
}

void MainWindow::connectSignals()
{
    // UI要素のシグナル接続
    connect(ui->getOpenStlButton(), &QPushButton::clicked, this, &MainWindow::openSTEPFile);
    connect(ui->getOpenVtkButton(), &QPushButton::clicked, this, &MainWindow::openVTKFile);
    connect(ui->getOpenStepButton(), &QPushButton::clicked, this, &MainWindow::openSTEPFile);
    connect(ui->getConstrainButton(), &QPushButton::clicked, this, &MainWindow::onConstrainButtonClicked);
    connect(ui->getLoadButton(), &QPushButton::clicked, this, &MainWindow::onLoadButtonClicked);
    connect(ui->getSimulateButton(), &QPushButton::clicked, this, &MainWindow::onSimulateButtonClicked);
    connect(ui->getProcessButton(), &QPushButton::clicked, this, &MainWindow::processFiles);
    connect(ui->getExport3mfButton(), &QPushButton::clicked, this, &MainWindow::export3mfFile);

    connect(ui->getRangeSlider(), &DensitySlider::handlePositionsChanged, this, &MainWindow::onDensitySliderChanged);
    connect(ui->getRangeSlider(), &DensitySlider::regionPercentsChanged, this, &MainWindow::onDensitySliderChanged);
    connect(ui->getStressRangeWidget(), &StressRangeWidget::stressRangeChanged, this, &MainWindow::onStressRangeChanged);


    // UIStateのシグナル接続
    connect(ui->getUIState(), &UIState::boundaryConditionChanged,
            this, &MainWindow::onBoundaryConditionChanged);
    connect(ui->getUIState(), &UIState::selectedObjectChanged,
            this, &MainWindow::onSelectedObjectChanged);

    // ProcessManagerWidget signals
    if (ui->getProcessManagerWidget()) {
        connect(ui->getProcessManagerWidget(), &ProcessManagerWidget::importFile,
                this, &MainWindow::loadSTEPFile);
        connect(ui->getProcessManagerWidget(), &ProcessManagerWidget::rollbackRequested,
                this, &MainWindow::handleProcessRollback);
    }

    // Connect face selection signal from VisualizationManager
    if (uiAdapter && uiAdapter->getVisualizationManager()) {
        connect(uiAdapter->getVisualizationManager(), &VisualizationManager::faceClicked,
                this, &MainWindow::onFaceClicked);
        connect(uiAdapter->getVisualizationManager(), &VisualizationManager::faceDoubleClicked,
                this, &MainWindow::onFaceDoubleClicked);
        connect(uiAdapter->getVisualizationManager(), &VisualizationManager::backgroundClicked,
                this, &MainWindow::onBackgroundClicked);

        // Connect VisualizationManager to PropertyWidget for edge selection
        if (ui && ui->getPropertyWidget()) {
            ui->getPropertyWidget()->setVisualizationManager(
                uiAdapter->getVisualizationManager());
        }
    }

    // 初期状態でUIStateを更新
    updateUIStateFromWidgets();
}

MainWindow::~MainWindow() = default;

void MainWindow::openVTKFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                     "Open VTK File",
                                                     "",
                                                     "VTK Files (*.vtu)");
    if (fileName.isEmpty()) {
        return;
    }

    logMessage(QString("Loading VTK file: %1").arg(fileName));

    // コマンドパターンを使用してファイルを開く
    auto command = std::make_unique<OpenVtkFileCommand>(
        appController.get(),
        uiAdapter.get(),
        fileName
    );
    command->execute();

    // UIStateにファイルパスを設定
    if (UIState* state = getUIState()) {
        state->setSimulationResultFilePath(fileName);
    }

    updateProcessButtonState();
}

void MainWindow::openSTEPFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                     "Open STEP File",
                                                     QDir::homePath(),
                                                     "STEP Files (*.step *.stp)");
    loadSTEPFile(fileName);
}

void MainWindow::loadSTEPFile(const QString& fileName)
{
    if (fileName.isEmpty()) {
        return;
    }

    logMessage(QString("Loading STEP file: %1").arg(fileName));

    // コマンドパターンを使用してファイルを開く
    auto command = std::make_unique<OpenStepFileCommand>(
        appController.get(),
        uiAdapter.get(),
        fileName
    );
    command->execute();

    // UIStateにファイルパスを設定
    if (UIState* state = getUIState()) {
        state->setStepFilePath(fileName);

        // 変換されたSTLファイルパスのログ（UIStateには保存しない）
        QString convertedStlPath = appController->getConvertedStlPath();
        if (!convertedStlPath.isEmpty()) {
            logMessage(QString("Converted STL file generated: %1").arg(convertedStlPath));
        }
    }
    
    // Notify ProcessManager to advance step
    if (ui->getProcessManagerWidget()) {
        ui->getProcessManagerWidget()->onImportCompleted();
    }

    updateProcessButtonState();
}


void MainWindow::processFiles()
{
    logMessage("Starting file processing...");

    // コマンドパターンを使用してファイルを処理
    auto command = std::make_unique<ProcessFilesCommand>(
        appController.get(),
        uiAdapter.get()
    );
    command->execute();

    logMessage("File processing completed successfully");
    updateButtonsAfterProcessing(true);
}

void MainWindow::updateButtonsAfterProcessing(bool success)
{
    if (success) {
        ui->getExport3mfButton()->setEnabled(true);
        ui->getExport3mfButton()->setEmphasized(true);
        ui->getProcessButton()->setEnabled(false);
        ui->getProcessButton()->setEmphasized(false);
    }
}

void MainWindow::export3mfFile()
{
    logMessage("Starting 3MF export...");

    // コマンドパターンを使用して3MFファイルをエクスポート
    auto command = std::make_unique<Export3mfCommand>(
        appController.get(),
        uiAdapter.get()
    );
    command->execute();

    logMessage("3MF export completed successfully");
}

void MainWindow::onStepObjectVisibilityChanged(bool visible)
{
    UIState* state = getUIState();
    if (!state) return;

    QString fileName = state->getStepFilePath();
    if (fileName.isEmpty()) return;

    // コマンドパターンを使用してメッシュの表示/非表示を設定
    auto command = std::make_unique<SetMeshVisibilityCommand>(
        state,
        appController.get(),
        uiAdapter.get(),
        SetMeshVisibilityCommand::MeshType::STEP_MESH,
        fileName,
        visible
    );
    command->execute();
}

void MainWindow::onStepObjectOpacityChanged(double opacity)
{
    UIState* state = getUIState();
    if (!state) return;

    QString fileName = state->getStepFilePath();
    if (fileName.isEmpty()) return;

    // コマンドパターンを使用してメッシュの不透明度を設定
    auto command = std::make_unique<SetMeshOpacityCommand>(
        state,
        appController.get(),
        uiAdapter.get(),
        SetMeshOpacityCommand::MeshType::STEP_MESH,
        fileName,
        opacity
    );
    command->execute();
}

void MainWindow::onVtkObjectVisibilityChanged(bool visible)
{
    UIState* state = getUIState();
    if (!state) return;

    QString fileName = state->getSimulationResultFilePath();
    if (fileName.isEmpty()) return;

    // コマンドパターンを使用してメッシュの表示/非表示を設定
    auto command = std::make_unique<SetMeshVisibilityCommand>(
        state,
        appController.get(),
        uiAdapter.get(),
        SetMeshVisibilityCommand::MeshType::VTU_MESH,
        fileName,
        visible
    );
    command->execute();
}

void MainWindow::onVtkObjectOpacityChanged(double opacity)
{
    UIState* state = getUIState();
    if (!state) return;

    QString fileName = state->getSimulationResultFilePath();
    if (fileName.isEmpty()) return;

    // コマンドパターンを使用してメッシュの不透明度を設定
    auto command = std::make_unique<SetMeshOpacityCommand>(
        state,
        appController.get(),
        uiAdapter.get(),
        SetMeshOpacityCommand::MeshType::VTU_MESH,
        fileName,
        opacity
    );
    command->execute();
}

void MainWindow::onDensitySliderChanged()
{
    // コマンドパターンを使用してDensity Mappingを設定
    auto densitySlider = ui->getRangeSlider();
    if (!densitySlider) return;


    auto command = std::make_unique<SetStressDensityMappingCommand>(
        getUIState(),
        densitySlider->stressDensityMappings()
    );
    command->execute();

    resetExportButton();
    updateProcessButtonState();
}



void MainWindow::onStressRangeChanged(double minStress, double maxStress)
{
    // コマンドパターンを使用してストレス範囲を設定
    auto command = std::make_unique<SetStressRangeCommand>(
        getUIState(),
        minStress,
        maxStress
    );
    command->execute();

    // DensitySliderのStressRangeを更新
    ui->getRangeSlider()->setStressRange(minStress, maxStress);

    // DensitySliderの更新に伴い、Density Mappingも更新
    onDensitySliderChanged();
}

void MainWindow::resetExportButton()
{
    ui->getExport3mfButton()->setEnabled(false);
    ui->getExport3mfButton()->setEmphasized(false);
}

void MainWindow::updateProcessButtonState()
{
    // UIStateを取得
    UIState* state = getUIState();
    if (!state) return;

    // 両ファイル（STLとVTK）が読み込まれている場合のみProcessボタンを有効化
    bool bothFilesLoaded = appController->areBothFilesLoaded(state);
    ui->getProcessButton()->setEnabled(bothFilesLoaded);

    if (bothFilesLoaded) {
        ui->getProcessButton()->setEmphasized(true);
    } else {
        ui->getProcessButton()->setEmphasized(false);
    }
}

UIState* MainWindow::getUIState() const
{
    return ui ? ui->getUIState() : nullptr;
}

void MainWindow::updateUIStateFromWidgets()
{
    UIState* state = getUIState();
    if (!state) return;

    // DensitySliderからStressDensityMappingsを更新（コマンドパターン使用）
    auto densitySlider = ui->getRangeSlider();
    if (densitySlider) {
        auto mappingCommand = std::make_unique<SetStressDensityMappingCommand>(
            state,
            densitySlider->stressDensityMappings()
        );
        mappingCommand->execute();
    }


}

void MainWindow::showUIStateDebugInfo()
{
    if (UIState* state = getUIState()) {
        QString debugInfo = state->getDebugString();
        logMessage("=== UIState Debug Info ===");

        // Split the debug string into lines and log each one
        QStringList lines = debugInfo.split('\n');
        for (const QString& line : lines) {
            if (!line.trimmed().isEmpty()) {
                logMessage(line);
            }
        }
    } else {
        logMessage("UIState is not available for debugging");
    }
}

void MainWindow::onConstrainButtonClicked()
{
    UIState* state = getUIState();
    if (!state) return;

    // Generate a unique name for the new constraint
    BoundaryCondition bc = state->getBoundaryCondition();
    int nextId = 1;
    std::string newName;
    while (true) {
        newName = "Constraint" + std::to_string(nextId);
        bool exists = false;
        for (const auto& c : bc.constraints) {
            if (c.name == newName) {
                exists = true;
                break;
            }
        }
        if (!exists) break;
        nextId++;
    }

    // Create a new default constraint
    ConstraintCondition constraint;
    constraint.name = newName;
    constraint.surface_id = 0; // Default (empty)

    // Command pattern: Add constraint
    auto command = std::make_unique<SetConstraintConditionCommand>(
        state,
        constraint
    );
    command->execute();
    
    // Select the new item
    // It's the last one
    // Select the new item via UIState
    int index = state->getBoundaryCondition().constraints.size() - 1;
    state->setSelectedObject(ObjectType::ITEM_BC_CONSTRAINT, "", index);
    
    logMessage("Added new Constraint Condition: " + QString::fromStdString(newName));
}

void MainWindow::onLoadButtonClicked()
{
    UIState* state = getUIState();
    if (!state) return;

    // Generate a unique name for the new load
    BoundaryCondition bc = state->getBoundaryCondition();
    int nextId = 1;
    std::string newName;
    while (true) {
        newName = "Load" + std::to_string(nextId);
        bool exists = false;
        for (const auto& l : bc.loads) {
            if (l.name == newName) {
                exists = true;
                break;
            }
        }
        if (!exists) break;
        nextId++;
    }

    // Create a new default load
    LoadCondition load;
    load.name = newName;
    load.surface_id = 0; // Default (empty)
    load.magnitude = 10.0;
    load.direction = {0, 0, -1}; // Default Z down

    // Command pattern: Add load
    auto command = std::make_unique<SetLoadConditionCommand>(
        state,
        load
    );
    command->execute();
    
    // Select the new item
    // Select the new item via UIState
    int index = state->getBoundaryCondition().loads.size() - 1;
    state->setSelectedObject(ObjectType::ITEM_BC_LOAD, "", index);
    
    logMessage("Added new Load Condition: " + QString::fromStdString(newName));
}

void MainWindow::onSimulateButtonClicked()
{
    logMessage("Starting FEM analysis pipeline...");

    UIState* uiState = getUIState();
    if (!uiState) {
        logMessage("Error: UIState is null");
        QMessageBox::critical(this, "エラー", "UIStateが取得できませんでした");
        return;
    }

    // 一時ディレクトリのFEMサブディレクトリに保存
    QString femTempDir = TempPathUtility::getTempSubDir("FEM");

    // FEMディレクトリが存在しない場合は作成
    QDir dir;
    if (!dir.exists(femTempDir)) {
        dir.mkpath(femTempDir);
        logMessage("Created FEM temp directory: " + femTempDir);
    }

    // 一時ファイルパスを生成
    QString outputPath = femTempDir + "/simulation_condition.json";

    // コマンドパターンを使用してFEM解析パイプライン全体を実行
    // (設定ファイルのエクスポート → FEM解析の実行)
    auto command = std::make_unique<RunFEMPipelineCommand>(
        appController.get(),
        uiAdapter.get(),
        uiState,
        outputPath
    );
    command->execute();

    logMessage("FEM analysis pipeline completed.");

    // Notify ProcessManager to advance step
    if (ui->getProcessManagerWidget()) {
        ui->getProcessManagerWidget()->onSimulationCompleted();
    }

    updateProcessButtonState();
}

void MainWindow::onBoundaryConditionChanged()
{
    UIState* state = getUIState();
    if (!state) {
        return;
    }

    BoundaryCondition condition = state->getBoundaryCondition();

    if (uiAdapter) {
        auto* vizManager = uiAdapter->getVisualizationManager();
        if (vizManager) {
            vizManager->displayBoundaryConditions(condition);
        }
    }
}

void MainWindow::onFaceClicked(int faceId, double nx, double ny, double nz)
{
    // Check if an object is selected in the list
    UIState* state = getUIState();
    if (!state) return;

    // Use selection from UIState
    SelectedObjectInfo selection = state->getSelectedObject();

    // Constraint update moved to double-click handler
    Q_UNUSED(faceId);
    Q_UNUSED(nx);
    Q_UNUSED(ny);
    Q_UNUSED(nz);
}

void MainWindow::onBackgroundClicked()
{
    UIState* state = getUIState();
    if (!state) return;

    // 背景（何も選択されていない領域）がクリックされた場合、選択を解除
    SelectedObjectInfo selection = state->getSelectedObject();
    if (selection.type != ObjectType::NONE) {
        state->setSelectedObject(ObjectType::NONE);
    }
}

void MainWindow::onFaceDoubleClicked(int faceId, double nx, double ny, double nz)
{
    // Check if an object is selected in the list
    UIState* state = getUIState();
    if (!state) return;

    // Use selection from UIState
    SelectedObjectInfo selection = state->getSelectedObject();

    if (selection.type == ObjectType::ITEM_BC_CONSTRAINT) {
        // Update the constraint at this index
        if (selection.index >= 0) {
            // Get current to keep name
            BoundaryCondition bc = state->getBoundaryCondition();
            if (selection.index < (int)bc.constraints.size()) {
                ConstraintCondition c = bc.constraints[selection.index];
                c.surface_id = faceId;
                
                // Command pattern: Update constraint
                auto command = std::make_unique<UpdateConstraintConditionCommand>(
                    state,
                    selection.index,
                    c
                );
                command->execute();

                logMessage(QString("Updated Constraint '%1' to Surface ID: %2").arg(QString::fromStdString(c.name)).arg(faceId));
            }
        }
    } else if (selection.type == ObjectType::ITEM_BC_LOAD) {
        // Update the load at this index
        if (selection.index >= 0) {
            // Get current to keep name/values
            BoundaryCondition bc = state->getBoundaryCondition();
            if (selection.index < (int)bc.loads.size()) {
                LoadCondition l = bc.loads[selection.index];
                l.surface_id = faceId;
                l.direction = {-nx, -ny, -nz}; // Automatically input inverse normal vector
                l.reference_edge_id = 0;  // Clear edge reference when face is clicked

                // Command pattern: Update load
                auto command = std::make_unique<UpdateLoadConditionCommand>(
                    state,
                    selection.index,
                    l
                );
                command->execute();

                logMessage(QString("Updated Load '%1' to Surface ID: %2, Direction: (%3, %4, %5)")
                    .arg(QString::fromStdString(l.name))
                    .arg(faceId)
                    .arg(-nx, 0, 'f', 2)
                    .arg(-ny, 0, 'f', 2)
                    .arg(-nz, 0, 'f', 2));
            }
        }
    }
}

void MainWindow::onSelectedObjectChanged(const SelectedObjectInfo& selection)
{
    if (!uiAdapter) return;
    auto vizManager = uiAdapter->getVisualizationManager();
    if (!vizManager) return;

    // Check if the selected object is a Load or Constraint condition
    bool isConditionSelected = (selection.type == ObjectType::ITEM_BC_CONSTRAINT ||
                                selection.type == ObjectType::ITEM_BC_LOAD);

    // Enable face selection mode only when a condition is selected
    vizManager->setFaceSelectionMode(isConditionSelected);

    // Ensure edge selection is off when switching selections (safety)
    // ただし、LoadPropertyWidgetでのエッジ選択中に別のオブジェクトを選んだ場合などを想定
    vizManager->setEdgeSelectionMode(false);
}

void MainWindow::handleProcessRollback(ProcessStep targetStep)
{
    logMessage(QString("Rolling back to step %1").arg(static_cast<int>(targetStep)));

    UIState* uiState = ui->getUIState();
    if (!uiState) {
        QMessageBox::critical(this, "Error", "UIState not available");
        return;
    }

    VisualizationManager* visManager = uiAdapter ? uiAdapter->getVisualizationManager() : nullptr;
    if (!visManager) {
        QMessageBox::critical(this, "Error", "VisualizationManager not available");
        return;
    }

    ProcessManagerWidget* processManager = ui->getProcessManagerWidget();
    if (!processManager) {
        return;
    }

    ProcessStep currentStep = processManager->getFlowWidget()->currentStep();

    int targetIdx = static_cast<int>(targetStep);
    int currentIdx = static_cast<int>(currentStep);

    // Reset data for each step that needs to be cleared
    // Process in reverse order (highest to lowest)

    // Step 4: Infill Map (if current >= 3 && target < 3)
    if (currentIdx >= 3 && targetIdx < 3) {
        logMessage("Clearing infill data...");
        uiState->clearAllInfillRegions();
        visManager->clearInfillActors();
    }

    // Step 3: Simulation (if current >= 2 && target < 2)
    if (currentIdx >= 2 && targetIdx < 2) {
        logMessage("Clearing simulation data...");
        uiState->clearSimulationResult();
        visManager->clearSimulationActors();
    }

    // Step 2: Boundary Conditions (if current >= 1 && target < 1)
    if (currentIdx >= 1 && targetIdx < 1) {
        logMessage("Clearing boundary conditions...");
        uiState->clearLoadConditions();
        uiState->clearConstraintConditions();
        visManager->clearBoundaryConditions();
    }

    // Step 1: Import STEP (if target == 0)
    if (targetIdx == 0) {
        logMessage("Clearing STEP file...");
        uiState->clearStepFile();
        visManager->clearStepFileActors();
        visManager->resetStepReader();
    }

    // Update ProcessManager UI
    processManager->rollbackToStep(targetStep);

    logMessage("Rollback completed");
}

