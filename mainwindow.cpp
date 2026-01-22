#include "mainwindow.h"
#include "core/application/MainWindowUIAdapter.h"
#include "UI/visualization/VisualizationManager.h"

#include "core/commands/file/OpenVtkFileCommand.h"
#include "core/commands/file/OpenStepFileCommand.h"
#include "core/commands/processing/ProcessFilesCommand.h"
#include "core/commands/processing/Export3mfCommand.h"
#include "core/commands/state/SetStressRangeCommand.h"
#include "core/commands/state/SetStressDensityMappingCommand.h"
#include "core/commands/visualization/SetMeshVisibilityCommand.h"
#include "core/commands/visualization/SetMeshOpacityCommand.h"

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

    // Initialize controllers
    bcController_ = std::make_unique<BoundaryConditionController>(
        ui->getUIState(),
        uiAdapter->getVisualizationManager(),
        ui->getProcessManagerWidget(),
        this
    );

    processController_ = std::make_unique<ProcessController>(
        appController.get(),
        uiAdapter.get(),
        ui->getProcessManagerWidget(),
        ui->getUIState(),
        this
    );

    alignmentController_ = std::make_unique<ModelAlignmentController>(
        appController.get(),
        uiAdapter.get(),
        ui->getProcessManagerWidget(),
        this
    );
}

void MainWindow::setupWindow()
{
    setWindowTitle("Strecs3D");
    setCentralWidget(ui->getCentralWidget());
    resize(1600, 900);
}

void MainWindow::connectSignals()
{
    // UI buttons -> MainWindow slots
    connect(ui->getOpenStlButton(), &QPushButton::clicked, this, &MainWindow::openSTEPFile);
    connect(ui->getOpenVtkButton(), &QPushButton::clicked, this, &MainWindow::openVTKFile);
    connect(ui->getOpenStepButton(), &QPushButton::clicked, this, &MainWindow::openSTEPFile);
    connect(ui->getConstrainButton(), &QPushButton::clicked, this, &MainWindow::onConstrainButtonClicked);
    connect(ui->getLoadButton(), &QPushButton::clicked, this, &MainWindow::onLoadButtonClicked);
    connect(ui->getSimulateButton(), &QPushButton::clicked, this, &MainWindow::onSimulateButtonClicked);
    connect(ui->getProcessButton(), &QPushButton::clicked, this, &MainWindow::processFiles);
    connect(ui->getExport3mfButton(), &QPushButton::clicked, this, &MainWindow::export3mfFile);

    // Sliders/Widgets -> MainWindow slots
    connect(ui->getRangeSlider(), &DensitySlider::handlePositionsChanged, this, &MainWindow::onDensitySliderChanged);
    connect(ui->getRangeSlider(), &DensitySlider::regionPercentsChanged, this, &MainWindow::onDensitySliderChanged);
    connect(ui->getStressRangeWidget(), &StressRangeWidget::stressRangeChanged, this, &MainWindow::onStressRangeChanged);

    // UIState -> MainWindow slots
    connect(ui->getUIState(), &UIState::boundaryConditionChanged, this, &MainWindow::onBoundaryConditionChanged);
    connect(ui->getUIState(), &UIState::selectedObjectChanged, this, &MainWindow::onSelectedObjectChanged);

    // ProcessManager and Visualization signals
    connectProcessManagerSignals();
    connectVisualizationSignals();

    // Initialize UIState from widgets
    updateUIStateFromWidgets();
}

void MainWindow::connectProcessManagerSignals()
{
    if (!ui->getProcessManagerWidget()) return;

    auto* pm = ui->getProcessManagerWidget();

    connect(pm, &ProcessManagerWidget::importFile, this, &MainWindow::loadSTEPFile);
    connect(pm, &ProcessManagerWidget::rollbackRequested, this, &MainWindow::handleProcessRollback);
    connect(pm, &ProcessManagerWidget::bedSurfaceSelectionRequested, this, &MainWindow::onBedSurfaceSelectionRequested);

    // Auto-close property widget on left pane interactions
    auto clearSelection = [this]() {
        if (UIState* state = getUIState()) {
            state->setSelectedObject(ObjectType::NONE);
        }
    };

    connect(pm, &ProcessManagerWidget::stepChanged, this, clearSelection);
    connect(pm, &ProcessManagerWidget::importStepClicked, this, clearSelection);
    connect(pm, &ProcessManagerWidget::addLoadClicked, this, clearSelection);
    connect(pm, &ProcessManagerWidget::addConstraintClicked, this, clearSelection);
    connect(pm, &ProcessManagerWidget::simulateClicked, this, clearSelection);
    connect(pm, &ProcessManagerWidget::processInfillClicked, this, clearSelection);
}

void MainWindow::connectVisualizationSignals()
{
    if (!uiAdapter || !uiAdapter->getVisualizationManager()) return;

    auto* vm = uiAdapter->getVisualizationManager();

    connect(vm, &VisualizationManager::faceClicked, this, &MainWindow::onFaceClicked);
    connect(vm, &VisualizationManager::faceDoubleClicked, this, &MainWindow::onFaceDoubleClicked);
    connect(vm, &VisualizationManager::backgroundClicked, this, &MainWindow::onBackgroundClicked);

    // Connect VisualizationManager to PropertyWidget for edge selection
    if (ui && ui->getPropertyWidget()) {
        ui->getPropertyWidget()->setVisualizationManager(vm);
    }
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

    // Check for high density warning after division processing
    if (uiAdapter) {
        uiAdapter->checkHighDensityWarning();
    }
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
    bcController_->showAddConstraintDialog();
}

void MainWindow::onLoadButtonClicked()
{
    bcController_->showAddLoadDialog();
}

void MainWindow::onSimulateButtonClicked()
{
    processController_->runSimulation();
    updateProcessButtonState();
}

void MainWindow::onBoundaryConditionChanged()
{
    bcController_->updateVisualization();
}

void MainWindow::onFaceClicked(int faceId, double nx, double ny, double nz)
{
    bcController_->handleFaceClicked(faceId, nx, ny, nz);
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
    bcController_->handleFaceDoubleClicked(faceId, nx, ny, nz);
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
    processController_->rollbackToStep(targetStep);
}

void MainWindow::onBedSurfaceSelectionRequested()
{
    alignmentController_->showBedSurfaceSelectionDialog();
}

