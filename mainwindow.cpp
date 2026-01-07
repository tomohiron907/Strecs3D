#include "mainwindow.h"
#include "core/application/MainWindowUIAdapter.h"
#include "UI/visualization/VisualizationManager.h"
#include "UI/widgets/ObjectListWidget.h"
#include "core/commands/file/OpenVtkFileCommand.h"
#include "core/commands/file/OpenStepFileCommand.h"
#include "core/commands/processing/ProcessFilesCommand.h"
#include "core/commands/processing/Export3mfCommand.h"
#include "core/commands/processing/RunFEMPipelineCommand.h"
#include "core/commands/state/SetStressRangeCommand.h"
#include "core/commands/state/SetProcessingModeCommand.h"
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
#include <iostream>
#include <functional>
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
    if (ui && ui->getMessageConsole()) {
        ui->getMessageConsole()->appendMessage(message);
    } else {
        qWarning() << "Unable to log message - UI console not available:" << message;
    }
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
    connect(ui->getModeComboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onModeComboBoxChanged);

    // UIStateのシグナル接続
    connect(ui->getUIState(), &UIState::boundaryConditionChanged,
            this, &MainWindow::onBoundaryConditionChanged);

    // Connect face selection signal from VisualizationManager
    if (uiAdapter && uiAdapter->getVisualizationManager()) {
        connect(uiAdapter->getVisualizationManager(), &VisualizationManager::faceClicked,
                this, &MainWindow::onFaceClicked);
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

void MainWindow::onModeComboBoxChanged()
{
    UIState* state = getUIState();
    if (!state) return;

    // ModeComboBoxからProcessingModeを取得（コマンドパターン使用）
    auto modeComboBox = ui->getModeComboBox();
    if (!modeComboBox) return;

    QString currentText = modeComboBox->currentText().toLower();
    ProcessingMode mode;
    if (currentText == "bambu") {
        mode = ProcessingMode::BAMBU;
    } else if (currentText == "prusa") {
        mode = ProcessingMode::PRUSA;
    } else {
        mode = ProcessingMode::CURA; // デフォルト
    }

    auto command = std::make_unique<SetProcessingModeCommand>(
        state,
        mode
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

    // ModeComboBoxからProcessingModeを更新（コマンドパターン使用）
    auto modeComboBox = ui->getModeComboBox();
    if (modeComboBox) {
        QString currentText = modeComboBox->currentText().toLower();
        ProcessingMode mode;
        if (currentText == "bambu") {
            mode = ProcessingMode::BAMBU;
        } else if (currentText == "prusa") {
            mode = ProcessingMode::PRUSA;
        } else {
            mode = ProcessingMode::CURA; // デフォルト
        }

        auto modeCommand = std::make_unique<SetProcessingModeCommand>(
            state,
            mode
        );
        modeCommand->execute();
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

    // Create a new default constraint
    ConstraintCondition constraint;
    constraint.name = "New Constraint";
    constraint.surface_id = 0; // Default (empty)

    // Command pattern: Add constraint
    auto command = std::make_unique<SetConstraintConditionCommand>(
        state,
        constraint
    );
    command->execute();
    
    // Select the new item
    // It's the last one
    int index = state->getBoundaryCondition().constraints.size() - 1;
    if (ui && ui->getObjectListWidget()) {
        ui->getObjectListWidget()->selectObject(ObjectType::ITEM_BC_CONSTRAINT, "", index);
    }
    
    logMessage("Added new Constraint Condition.");
}

void MainWindow::onLoadButtonClicked()
{
    UIState* state = getUIState();
    if (!state) return;

    // Create a new default load
    LoadCondition load;
    load.name = "New Load";
    load.surface_id = 0; // Default (empty)
    load.magnitude = 100.0;
    load.direction = {0, 0, -1}; // Default Z down

    // Command pattern: Add load
    auto command = std::make_unique<SetLoadConditionCommand>(
        state,
        load
    );
    command->execute();
    
    // Select the new item
    int index = state->getBoundaryCondition().loads.size() - 1;
    if (ui && ui->getObjectListWidget()) {
        ui->getObjectListWidget()->selectObject(ObjectType::ITEM_BC_LOAD, "", index);
    }
    
    logMessage("Added new Load Condition.");
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

void MainWindow::onFaceClicked(int faceId)
{
    // Check if an object is selected in the list
    if (!ui || !ui->getObjectListWidget()) return;

    QList<QTreeWidgetItem*> selectedItems = ui->getObjectListWidget()->selectedItems();
    if (selectedItems.isEmpty()) return;

    // Use dynamic cast to check if it's our custom item
    for (auto* widgetItem : selectedItems) {
        ObjectTreeItem* item = dynamic_cast<ObjectTreeItem*>(widgetItem);
        if (item && item->type == ObjectType::ITEM_BC_CONSTRAINT) {
            // Update the constraint at this index
            UIState* state = getUIState();
            if (state && item->index >= 0) {
                // Get current to keep name
                BoundaryCondition bc = state->getBoundaryCondition();
                if (item->index < (int)bc.constraints.size()) {
                    ConstraintCondition c = bc.constraints[item->index];
                    c.surface_id = faceId;
                    
                    // Command pattern: Update constraint
                    auto command = std::make_unique<UpdateConstraintConditionCommand>(
                        state,
                        item->index,
                        c
                    );
                    command->execute();

                    logMessage(QString("Updated Constraint '%1' to Surface ID: %2").arg(QString::fromStdString(c.name)).arg(faceId));
                }
            }
            break; // Only handle single selection
        } else if (item && item->type == ObjectType::ITEM_BC_LOAD) {
             // Update the load at this index
            UIState* state = getUIState();
            if (state && item->index >= 0) {
                // Get current to keep name/values
                BoundaryCondition bc = state->getBoundaryCondition();
                if (item->index < (int)bc.loads.size()) {
                    LoadCondition l = bc.loads[item->index];
                    l.surface_id = faceId;
                    
                    // Command pattern: Update load
                    auto command = std::make_unique<UpdateLoadConditionCommand>(
                        state,
                        item->index,
                        l
                    );
                    command->execute();

                    logMessage(QString("Updated Load '%1' to Surface ID: %2").arg(QString::fromStdString(l.name)).arg(faceId));
                }
            }
            break;
        }
    }
}

