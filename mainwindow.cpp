#include "mainwindow.h"
#include "core/application/MainWindowUIAdapter.h"
#include "core/commands/ui/SetStressRangeCommand.h"
#include "core/commands/ui/SetProcessingModeCommand.h"
#include "core/commands/ui/SetMeshVisibilityCommand.h"
#include "core/commands/ui/SetMeshOpacityCommand.h"
#include "core/commands/business/OpenStlFileCommand.h"
#include "core/commands/business/OpenVtkFileCommand.h"
#include "core/commands/business/ProcessFilesCommand.h"
#include "core/commands/business/Export3mfCommand.h"
#include <QPushButton>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <iostream>
#include <functional>

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
    connectMessageSignals();
    connectUISignals();

    // 初期状態でUIStateを更新
    updateUIStateFromWidgets();
}

void MainWindow::connectUISignals()
{
    // File operation buttons - using command pattern
    connect(ui->getOpenStlButton(), &QPushButton::clicked, this, &MainWindow::onOpenStlButtonClicked);
    connect(ui->getOpenVtkButton(), &QPushButton::clicked, this, &MainWindow::onOpenVtkButtonClicked);
    connect(ui->getProcessButton(), &QPushButton::clicked, this, &MainWindow::onProcessButtonClicked);
    connect(ui->getExport3mfButton(), &QPushButton::clicked, this, &MainWindow::onExport3mfButtonClicked);

    // Parameter changes - using command pattern
    connect(ui->getRangeSlider(), &DensitySlider::handlePositionsChanged, this, &MainWindow::onParametersChanged);
    connect(ui->getRangeSlider(), &DensitySlider::regionPercentsChanged, this, &MainWindow::onParametersChanged);
    connect(ui->getStressRangeWidget(), &StressRangeWidget::stressRangeChanged, this, &MainWindow::onStressRangeChanged);
    connect(ui->getModeComboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onModeChanged);

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

void MainWindow::updateButtonsAfterProcessing(bool success)
{
    if (success) {
        ui->getExport3mfButton()->setEnabled(true);
        ui->getExport3mfButton()->setEmphasized(true);
        ui->getProcessButton()->setEnabled(false);
        ui->getProcessButton()->setEmphasized(false);
    }
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
    if (objectDisplayWidget) {
        QString fileName = objectDisplayWidget->getFileName();
        uiAdapter->setVisualizationObjectVisible(fileName.toStdString(), visible);
    }
}

void MainWindow::onObjectOpacityChanged(double opacity)
{
    auto objectDisplayWidget = ui->getObjectDisplayOptionsWidget();
    if (objectDisplayWidget) {
        QString fileName = objectDisplayWidget->getFileName();
        uiAdapter->setVisualizationObjectOpacity(fileName.toStdString(), opacity);
    }
}

void MainWindow::onVtkObjectVisibilityChanged(bool visible)
{
    auto vtkDisplayWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkDisplayWidget) {
        QString fileName = vtkDisplayWidget->getFileName();
        uiAdapter->setVisualizationObjectVisible(fileName.toStdString(), visible);
    }
}

void MainWindow::onVtkObjectOpacityChanged(double opacity)
{
    auto vtkDisplayWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkDisplayWidget) {
        QString fileName = vtkDisplayWidget->getFileName();
        uiAdapter->setVisualizationObjectOpacity(fileName.toStdString(), opacity);
    }
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
    
    // DensitySliderからStressDensityMappingsを更新
    auto densitySlider = ui->getRangeSlider();
    if (densitySlider) {
        state->setStressDensityMappings(densitySlider->stressDensityMappings());
    }
    
    // ModeComboBoxからProcessingModeを更新
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
        state->setProcessingMode(mode);
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

// ==================== Command Pattern Implementation ====================

void MainWindow::executeCommand(std::unique_ptr<Command> command)
{
    if (command) {
        command->execute();
    }
}

// ==================== File Operation Commands ====================

void MainWindow::onOpenStlButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open STL File",
        QDir::homePath(),
        "STL Files (*.stl)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    logMessage(QString("Opening STL file: %1").arg(fileName));

    // Create and execute command
    auto command = std::make_unique<OpenStlFileCommand>(
        appController.get(),
        uiAdapter.get(),
        fileName
    );

    executeCommand(std::move(command));

    // Update UI state
    updateProcessButtonState();
    logMessage("STL file loaded successfully");
}

void MainWindow::onOpenVtkButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open VTK File",
        "",
        "VTK Files (*.vtu)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    logMessage(QString("Opening VTK file: %1").arg(fileName));

    // Create and execute command
    auto command = std::make_unique<OpenVtkFileCommand>(
        appController.get(),
        uiAdapter.get(),
        fileName
    );

    executeCommand(std::move(command));

    // Update UI state
    updateProcessButtonState();
    logMessage("VTK file loaded successfully");
}

void MainWindow::onProcessButtonClicked()
{
    logMessage("Starting file processing...");

    // Create and execute command
    auto command = std::make_unique<ProcessFilesCommand>(
        appController.get(),
        uiAdapter.get()
    );

    executeCommand(std::move(command));

    // Update UI after processing
    updateButtonsAfterProcessing(true);
    logMessage("File processing completed successfully");
}

void MainWindow::onExport3mfButtonClicked()
{
    logMessage("Starting 3MF export...");

    // Create and execute command
    auto command = std::make_unique<Export3mfCommand>(
        appController.get(),
        uiAdapter.get()
    );

    executeCommand(std::move(command));

    logMessage("3MF export completed successfully");
}

// ==================== Parameter Change Commands ====================

void MainWindow::onStressRangeChanged(double minStress, double maxStress)
{
    // Create and execute command for UIState update
    auto command = std::make_unique<SetStressRangeCommand>(
        getUIState(),
        minStress,
        maxStress
    );

    executeCommand(std::move(command));

    // Update DensitySlider's stress range
    ui->getRangeSlider()->setStressRange(minStress, maxStress);

    // Trigger parameter change handling
    onParametersChanged();
}

void MainWindow::onModeChanged(int index)
{
    ProcessingMode mode;

    // Convert index to ProcessingMode
    switch (index) {
        case 0: mode = ProcessingMode::BAMBU; break;
        case 1: mode = ProcessingMode::CURA; break;
        case 2: mode = ProcessingMode::PRUSA; break;
        default: mode = ProcessingMode::CURA; break;
    }

    // Create and execute command
    auto command = std::make_unique<SetProcessingModeCommand>(
        getUIState(),
        mode
    );

    executeCommand(std::move(command));

    // Trigger parameter change handling
    onParametersChanged();
}
