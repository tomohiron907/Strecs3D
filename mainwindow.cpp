#include "mainwindow.h"
#include "core/application/MainWindowUIAdapter.h"
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
    setupSignalSlotConnections();
    connectUISignals();
}

void MainWindow::connectUISignals()
{
    connect(ui->getOpenStlButton(), &QPushButton::clicked, this, &MainWindow::openSTLFile);
    connect(ui->getOpenVtkButton(), &QPushButton::clicked, this, &MainWindow::openVTKFile);
    connect(ui->getProcessButton(), &QPushButton::clicked, this, &MainWindow::processFiles);
    connect(ui->getExport3mfButton(), &QPushButton::clicked, this, &MainWindow::export3mfFile);
    
    connect(ui->getRangeSlider(), &DensitySlider::handlePositionsChanged, this, &MainWindow::onParametersChanged);
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
    
    handleFileLoad(fileName, [this](const std::string& file) {
        return appController->openVtkFile(file, uiAdapter.get());
    }, "VTK");
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
    
    handleFileLoad(fileName, [this](const std::string& file) {
        return appController->openStlFile(file, uiAdapter.get());
    }, "STL");
}

QString MainWindow::selectSTLFile()
{
    return QFileDialog::getOpenFileName(this,
                                      "Open STL File",
                                      QDir::homePath(),
                                      "STL Files (*.stl)");
}

void MainWindow::handleFileLoad(const QString& fileName, std::function<bool(const std::string&)> loadFunction, const QString& fileType)
{
    if (fileName.isEmpty()) {
        logMessage(QString("No %1 file selected").arg(fileType));
        return;
    }
    
    std::string file = fileName.toStdString();
    logMessage(QString("Loading %1 file: %2").arg(fileType, fileName));
    
    try {
        if (loadFunction(file)) {
            logMessage(QString("%1 file loaded successfully").arg(fileType));
            updateProcessButtonState();
        } else {
            logMessage(QString("Failed to load %1 file: Unknown error").arg(fileType));
        }
    } catch (const std::exception& e) {
        logMessage(QString("Error loading %1 file: %2").arg(fileType, QString::fromStdString(e.what())));
    }
}

void MainWindow::processFiles()
{
    logMessage("Starting file processing...");
    
    if (executeProcessing()) {
        logMessage("File processing completed successfully");
        updateButtonsAfterProcessing(true);
    } else {
        logMessage("File processing failed");
        updateButtonsAfterProcessing(false);
    }
}

bool MainWindow::executeProcessing()
{
    return appController->processFiles(uiAdapter.get());
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
    
    if (executeExport()) {
        logMessage("3MF export completed successfully");
    } else {
        logMessage("3MF export failed");
    }
}

bool MainWindow::executeExport()
{
    return appController->export3mfFile(uiAdapter.get());
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
