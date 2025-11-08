#include "mainwindow.h"
#include "core/application/MainWindowUIAdapter.h"
#include "core/commands/file/OpenStlFileCommand.h"
#include "core/commands/file/OpenVtkFileCommand.h"
#include "core/commands/processing/ProcessFilesCommand.h"
#include "core/commands/processing/Export3mfCommand.h"
#include "core/commands/state/SetStressRangeCommand.h"
#include "core/commands/state/SetProcessingModeCommand.h"
#include "core/commands/state/SetStressDensityMappingCommand.h"
#include "core/commands/visualization/SetMeshVisibilityCommand.h"
#include "core/commands/visualization/SetMeshOpacityCommand.h"
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
    setupSignalSlotConnections();
    connectUISignals();
    
    // 初期状態でUIStateを更新
    updateUIStateFromWidgets();
}

void MainWindow::connectUISignals()
{
    connect(ui->getOpenStlButton(), &QPushButton::clicked, this, &MainWindow::openSTLFile);
    connect(ui->getOpenVtkButton(), &QPushButton::clicked, this, &MainWindow::openVTKFile);
    connect(ui->getProcessButton(), &QPushButton::clicked, this, &MainWindow::processFiles);
    connect(ui->getExport3mfButton(), &QPushButton::clicked, this, &MainWindow::export3mfFile);
    
    connect(ui->getRangeSlider(), &DensitySlider::handlePositionsChanged, this, &MainWindow::onParametersChanged);
    connect(ui->getRangeSlider(), &DensitySlider::regionPercentsChanged, this, &MainWindow::onParametersChanged);
    connect(ui->getStressRangeWidget(), &StressRangeWidget::stressRangeChanged, this, &MainWindow::onStressRangeChanged);
    connect(ui->getModeComboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onParametersChanged);
    
    connectDisplayWidgetSignals();
}

void MainWindow::connectDisplayWidgetSignals()
{
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
}

MainWindow::~MainWindow() = default;

void MainWindow::openVTKFile()
{
    QString fileName = selectVTKFile();
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

QString MainWindow::selectVTKFile()
{
    return QFileDialog::getOpenFileName(this,
                                      "Open VTK File",
                                      "",
                                      "VTK Files (*.vtu)");
}

void MainWindow::openSTLFile()
{
    QString fileName = selectSTLFile();
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

QString MainWindow::selectSTLFile()
{
    return QFileDialog::getOpenFileName(this,
                                      "Open STL File",
                                      QDir::homePath(),
                                      "STL Files (*.stl)");
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

void MainWindow::setupSignalSlotConnections()
{
    connectFileSignals();
    connectVisibilitySignals();
    connectOpacitySignals();
    connectMessageSignals();
}

void MainWindow::connectFileSignals()
{
    connect(appController.get(), &ApplicationController::vtkFileNameChanged,
            uiAdapter.get(), &IUserInterface::onVtkFileNameChanged);
    connect(appController.get(), &ApplicationController::stlFileNameChanged,
            uiAdapter.get(), &IUserInterface::onStlFileNameChanged);
    connect(appController.get(), &ApplicationController::dividedMeshFileNameChanged,
            uiAdapter.get(), &IUserInterface::onDividedMeshFileNameChanged);
    connect(appController.get(), &ApplicationController::stressRangeChanged,
            uiAdapter.get(), &IUserInterface::onStressRangeChanged);
}

void MainWindow::connectVisibilitySignals()
{
    connect(appController.get(), &ApplicationController::vtkVisibilityChanged,
            uiAdapter.get(), &IUserInterface::onVtkVisibilityChanged);
    connect(appController.get(), &ApplicationController::stlVisibilityChanged,
            uiAdapter.get(), &IUserInterface::onStlVisibilityChanged);
    connect(appController.get(), &ApplicationController::dividedMeshVisibilityChanged,
            uiAdapter.get(), &IUserInterface::onDividedMeshVisibilityChanged);
}

void MainWindow::connectOpacitySignals()
{
    connect(appController.get(), &ApplicationController::vtkOpacityChanged,
            uiAdapter.get(), &IUserInterface::onVtkOpacityChanged);
    connect(appController.get(), &ApplicationController::stlOpacityChanged,
            uiAdapter.get(), &IUserInterface::onStlOpacityChanged);
    connect(appController.get(), &ApplicationController::dividedMeshOpacityChanged,
            uiAdapter.get(), &IUserInterface::onDividedMeshOpacityChanged);
}

void MainWindow::connectMessageSignals()
{
    connect(appController.get(), &ApplicationController::showWarningMessage,
            uiAdapter.get(), &IUserInterface::onShowWarningMessage);
    connect(appController.get(), &ApplicationController::showCriticalMessage,
            uiAdapter.get(), &IUserInterface::onShowCriticalMessage);
    connect(appController.get(), &ApplicationController::showInfoMessage,
            uiAdapter.get(), &IUserInterface::onShowInfoMessage);
}

void MainWindow::onParametersChanged()
{
    // UIStateを更新
    updateUIStateFromWidgets();
    
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

    // パラメータ変更処理を呼び出し
    onParametersChanged();
}

void MainWindow::resetExportButton()
{
    ui->getExport3mfButton()->setEnabled(false);
    ui->getExport3mfButton()->setEmphasized(false);
}

void MainWindow::updateProcessButtonState()
{
    // 両ファイル（STLとVTK）が読み込まれている場合のみProcessボタンを有効化
    bool bothFilesLoaded = appController->areBothFilesLoaded();
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

void MainWindow::printUIStateDebugInfo() const
{
    if (UIState* state = getUIState()) {
        state->printDebugInfo();
    } else {
        qDebug() << "UIState is not available";
    }
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
