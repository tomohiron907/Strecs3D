#include "mainwindow.h"
#include "core/application/MainWindowUIAdapter.h"
#include "core/commands/file/OpenStlFileCommand.h"
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
#include "core/commands/visualization/SetMeshVisibilityCommand.h"
#include "core/commands/visualization/SetMeshOpacityCommand.h"
#include "UI/dialogs/ConstraintDialog.h"
#include "UI/dialogs/LoadDialog.h"
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
    connect(ui->getOpenStlButton(), &QPushButton::clicked, this, &MainWindow::openSTLFile);
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

    // Display Widgetのシグナル接続
    auto objectDisplayWidget = ui->getObjectDisplayOptionsWidget();
    if (objectDisplayWidget) {
        connect(objectDisplayWidget, &ObjectDisplayOptionsWidget::visibilityToggled,
                this, &MainWindow::onObjectVisibilityChanged);
        connect(objectDisplayWidget, &ObjectDisplayOptionsWidget::opacityChanged,
                this, &MainWindow::onObjectOpacityChanged);
    }

    auto vtkDisplayWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkDisplayWidget) {
        connect(vtkDisplayWidget, &ObjectDisplayOptionsWidget::visibilityToggled,
                this, &MainWindow::onVtkObjectVisibilityChanged);
        connect(vtkDisplayWidget, &ObjectDisplayOptionsWidget::opacityChanged,
                this, &MainWindow::onVtkObjectOpacityChanged);
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
        state->setVtkFilePath(fileName);
    }

    updateProcessButtonState();
}

void MainWindow::openSTLFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                     "Open STL File",
                                                     QDir::homePath(),
                                                     "STL Files (*.stl)");
    if (fileName.isEmpty()) {
        return;
    }

    logMessage(QString("Loading STL file: %1").arg(fileName));

    // コマンドパターンを使用してファイルを開く
    auto command = std::make_unique<OpenStlFileCommand>(
        appController.get(),
        uiAdapter.get(),
        fileName
    );
    command->execute();

    // UIStateにファイルパスを設定
    if (UIState* state = getUIState()) {
        state->setStlFilePath(fileName);
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

        // 変換されたSTLファイルパスもUIStateに登録
        QString convertedStlPath = appController->getConvertedStlPath();
        if (!convertedStlPath.isEmpty()) {
            state->setStlFilePath(convertedStlPath);
            logMessage(QString("Converted STL file registered: %1").arg(convertedStlPath));
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

QString MainWindow::getCurrentMode() const
{
    return ui->getModeComboBox()->currentText();
}

QString MainWindow::getCurrentStlFilename() const
{
    return appController->getCurrentStlFilename();
}

void MainWindow::onObjectVisibilityChanged(bool visible)
{
    auto objectDisplayWidget = ui->getObjectDisplayOptionsWidget();
    if (!objectDisplayWidget) return;

    QString fileName = objectDisplayWidget->getFileName();

    // コマンドパターンを使用してメッシュの表示/非表示を設定
    auto command = std::make_unique<SetMeshVisibilityCommand>(
        getUIState(),
        appController.get(),
        uiAdapter.get(),
        SetMeshVisibilityCommand::MeshType::STL_MESH,
        fileName,
        visible
    );
    command->execute();
}

void MainWindow::onObjectOpacityChanged(double opacity)
{
    auto objectDisplayWidget = ui->getObjectDisplayOptionsWidget();
    if (!objectDisplayWidget) return;

    QString fileName = objectDisplayWidget->getFileName();

    // コマンドパターンを使用してメッシュの不透明度を設定
    auto command = std::make_unique<SetMeshOpacityCommand>(
        getUIState(),
        appController.get(),
        uiAdapter.get(),
        SetMeshOpacityCommand::MeshType::STL_MESH,
        fileName,
        opacity
    );
    command->execute();
}

void MainWindow::onVtkObjectVisibilityChanged(bool visible)
{
    auto vtkDisplayWidget = ui->getVtkDisplayOptionsWidget();
    if (!vtkDisplayWidget) return;

    QString fileName = vtkDisplayWidget->getFileName();

    // コマンドパターンを使用してメッシュの表示/非表示を設定
    auto command = std::make_unique<SetMeshVisibilityCommand>(
        getUIState(),
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
    auto vtkDisplayWidget = ui->getVtkDisplayOptionsWidget();
    if (!vtkDisplayWidget) return;

    QString fileName = vtkDisplayWidget->getFileName();

    // コマンドパターンを使用してメッシュの不透明度を設定
    auto command = std::make_unique<SetMeshOpacityCommand>(
        getUIState(),
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
    // ConstraintDialogを開く（モーダルレス）
    ConstraintDialog* dialog = new ConstraintDialog(this);

    // UIStateから既存の拘束条件を取得してダイアログにロード
    UIState* state = getUIState();
    if (state) {
        BoundaryCondition boundaryCondition = state->getBoundaryCondition();
        std::vector<int> existingSurfaceIds;

        for (const auto& constraint : boundaryCondition.constraints) {
            existingSurfaceIds.push_back(constraint.surface_id);
        }

        dialog->loadSurfaceIds(existingSurfaceIds);
    }

    // ダイアログが閉じられたときの処理
    connect(dialog, &QDialog::accepted, this, [this, dialog]() {
        // OKが押された場合、選択されたsurface_idを取得
        std::vector<int> surfaceIds = dialog->getSelectedSurfaceIds();

        // 各surface_idに対してSetConstraintConditionCommandを実行
        UIState* state = getUIState();
        if (!state) {
            dialog->deleteLater();
            return;
        }

        // 既存の拘束条件をクリア（新しいデータで上書き）
        state->clearConstraintConditions();

        // テーブルIDは1から始まる
        int tableId = 1;
        for (int surfaceId : surfaceIds) {
            ConstraintCondition constraint;
            constraint.surface_id = surfaceId;
            constraint.name = "Constraint_" + std::to_string(tableId);

            auto command = std::make_unique<SetConstraintConditionCommand>(
                state,
                constraint
            );
            command->execute();

            tableId++;
        }

        if (!surfaceIds.empty()) {
            logMessage(QString("Total %1 constraint condition(s) set.").arg(surfaceIds.size()));
        } else {
            logMessage("All constraint conditions cleared.");
        }

        dialog->deleteLater();
    });

    // ダイアログがキャンセルされたときの処理
    connect(dialog, &QDialog::rejected, this, [dialog]() {
        dialog->deleteLater();
    });

    // モーダルレスで表示
    dialog->show();
}

void MainWindow::onLoadButtonClicked()
{
    // LoadDialogを開く（モーダルレス）
    LoadDialog* dialog = new LoadDialog(this);

    // UIStateから既存の荷重条件を取得してダイアログにロード
    UIState* state = getUIState();
    if (state) {
        BoundaryCondition boundaryCondition = state->getBoundaryCondition();
        dialog->loadLoadConditions(boundaryCondition.loads);
    }

    // ダイアログが閉じられたときの処理
    connect(dialog, &QDialog::accepted, this, [this, dialog]() {
        // OKが押された場合、選択された荷重条件を取得
        std::vector<LoadCondition> loadConditions = dialog->getSelectedLoadConditions();

        // 各荷重条件に対してSetLoadConditionCommandを実行
        UIState* state = getUIState();
        if (!state) {
            dialog->deleteLater();
            return;
        }

        // 既存の荷重条件をクリア（新しいデータで上書き）
        state->clearLoadConditions();

        // テーブルIDは1から始まる
        int tableId = 1;
        for (LoadCondition& load : loadConditions) {
            load.name = "Load_" + std::to_string(tableId);

            auto command = std::make_unique<SetLoadConditionCommand>(
                state,
                load
            );
            command->execute();

            tableId++;
        }

        if (!loadConditions.empty()) {
            logMessage(QString("Total %1 load condition(s) set.").arg(loadConditions.size()));
        } else {
            logMessage("All load conditions cleared.");
        }

        dialog->deleteLater();
    });

    // ダイアログがキャンセルされたときの処理
    connect(dialog, &QDialog::rejected, this, [dialog]() {
        dialog->deleteLater();
    });

    // モーダルレスで表示
    dialog->show();
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
}
