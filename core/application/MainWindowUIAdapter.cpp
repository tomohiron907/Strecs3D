#include "MainWindowUIAdapter.h"
#include "ApplicationController.h"
#include "../../UI/widgets/DensitySlider.h"
#include "../../UI/visualization/VisualizationManager.h"
#include <algorithm>
#include <vector>

MainWindowUIAdapter::MainWindowUIAdapter(MainWindowUI* ui, QObject* parent) 
    : IUserInterface(parent), ui(ui)
{
    // Initialize VisualizationManager
    if (ui) {
        visualizationManager = std::make_unique<VisualizationManager>(ui);
    }
}

MainWindowUIAdapter::~MainWindowUIAdapter() = default;

void MainWindowUIAdapter::setVtkFileName(const QString& fileName)
{
    if (!ui) return;
    auto vtkDisplayWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkDisplayWidget) {
        vtkDisplayWidget->setFileName(fileName);
    }
}

void MainWindowUIAdapter::setStlFileName(const QString& fileName)
{
    if (!ui) return;
    auto stlDisplayWidget = ui->getObjectDisplayOptionsWidget();
    if (stlDisplayWidget) {
        stlDisplayWidget->setFileName(fileName);
    }
}

void MainWindowUIAdapter::setVtkVisibilityState(bool visible)
{
    if (!ui) return;
    auto vtkWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkWidget) {
        vtkWidget->setVisibleState(visible);
    }
}

void MainWindowUIAdapter::setStlVisibilityState(bool visible)
{
    if (!ui) return;
    auto stlWidget = ui->getObjectDisplayOptionsWidget();
    if (stlWidget) {
        stlWidget->setVisibleState(visible);
    }
}

void MainWindowUIAdapter::setVtkOpacity(double opacity)
{
    if (!ui) return;
    auto vtkWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkWidget) {
        vtkWidget->setOpacity(opacity);
    }
}

void MainWindowUIAdapter::setStlOpacity(double opacity)
{
    if (!ui) return;
    auto stlWidget = ui->getObjectDisplayOptionsWidget();
    if (stlWidget) {
        stlWidget->setOpacity(opacity);
    }
}

void MainWindowUIAdapter::setDividedMeshVisibility(int meshIndex, bool visible)
{
    auto widget = getDividedMeshWidget(meshIndex);
    if (widget) {
        widget->setVisibleState(visible);
    }
}

void MainWindowUIAdapter::setDividedMeshOpacity(int meshIndex, double opacity)
{
    auto widget = getDividedMeshWidget(meshIndex);
    if (widget) {
        widget->setOpacity(opacity);
    }
}

void MainWindowUIAdapter::setDividedMeshFileName(int meshIndex, const QString& fileName)
{
    auto widget = getDividedMeshWidget(meshIndex);
    if (widget) {
        widget->setFileName(fileName);
    }
}

std::vector<int> MainWindowUIAdapter::getStressThresholds() const
{
    if (!ui) return {};
    
    // UIStateから情報を取得
    UIState* uiState = ui->getUIState();
    if (uiState) {
        // StressDensityMappingから閾値を計算
        auto mappings = uiState->getStressDensityMappings();
        if (mappings.empty()) return {};
        
        std::vector<double> thresholdValues;
        
        // 各マッピングからstressMin, stressMaxを収集
        for (const auto& mapping : mappings) {
            thresholdValues.push_back(mapping.stressMin);
            thresholdValues.push_back(mapping.stressMax);
        }
        
        // 重複を除去して昇順ソート
        std::sort(thresholdValues.begin(), thresholdValues.end());
        thresholdValues.erase(std::unique(thresholdValues.begin(), thresholdValues.end()), thresholdValues.end());
        
        // doubleからintに変換
        std::vector<int> thresholds;
        for (double val : thresholdValues) {
            thresholds.push_back(static_cast<int>(val));
        }
        
        return thresholds;
    }
    return {};
}

std::vector<StressDensityMapping> MainWindowUIAdapter::getStressDensityMappings() const
{
    if (!ui) return {};
    
    // UIStateから情報を取得
    UIState* uiState = ui->getUIState();
    if (uiState) {
        return uiState->getStressDensityMappings();
    }
    return {};
}

QString MainWindowUIAdapter::getCurrentMode() const
{
    if (!ui) return "cura";
    
    // UIStateから情報を取得
    UIState* uiState = ui->getUIState();
    if (uiState) {
        ProcessingMode mode = uiState->getProcessingMode();
        switch(mode) {
            case ProcessingMode::BAMBU: return "bambu";
            case ProcessingMode::CURA: return "cura";
            case ProcessingMode::PRUSA: return "prusa";
            default: return "cura";
        }
    }
    return "cura";
}

void MainWindowUIAdapter::setStressRange(double minStress, double maxStress)
{
    if (!ui) return;
    auto slider = ui->getRangeSlider();
    if (slider) {
        slider->setOriginalStressRange(minStress, maxStress);
    }
    
    auto stressRangeWidget = ui->getStressRangeWidget();
    if (stressRangeWidget) {
        stressRangeWidget->setStressRange(minStress, maxStress);
    }
}

void MainWindowUIAdapter::showWarningMessage(const QString& title, const QString& message)
{
    if (ui) {
        QMessageBox::warning(qobject_cast<QWidget*>(ui), title, message);
    }
}

void MainWindowUIAdapter::showCriticalMessage(const QString& title, const QString& message)
{
    if (ui) {
        QMessageBox::critical(qobject_cast<QWidget*>(ui), title, message);
    }
}

void MainWindowUIAdapter::showInfoMessage(const QString& title, const QString& message)
{
    if (ui) {
        QMessageBox::information(qobject_cast<QWidget*>(ui), title, message);
    }
}

bool MainWindowUIAdapter::showFileValidationError()
{
    if (ui) {
        QMessageBox::warning(qobject_cast<QWidget*>(ui), "Validation Error", "File validation failed");
        return false;
    }
    return false;
}

bool MainWindowUIAdapter::showProcessingError(const QString& errorMessage)
{
    if (ui) {
        QMessageBox::critical(qobject_cast<QWidget*>(ui), "Processing Error", errorMessage);
        return false;
    }
    return false;
}

void MainWindowUIAdapter::showProcessingSuccess()
{
    if (ui) {
        QMessageBox::information(qobject_cast<QWidget*>(ui), "Success", "Processing completed successfully");
    }
}

ObjectDisplayOptionsWidget* MainWindowUIAdapter::getDividedMeshWidget(int meshIndex) const
{
    if (!ui || meshIndex < 0 || meshIndex >= ApplicationController::DIVIDED_MESH_COUNT) return nullptr;
    
    switch (meshIndex) {
        case 0: return ui->getDividedMeshWidget1();
        case 1: return ui->getDividedMeshWidget2();
        case 2: return ui->getDividedMeshWidget3();
        case 3: return ui->getDividedMeshWidget4();
        default: return nullptr;
    }
}

// 3D可視化制御の実装
void MainWindowUIAdapter::displayVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor)
{
    if (visualizationManager) {
        visualizationManager->displayVtkFile(vtkFile, vtkProcessor);
    }
}

void MainWindowUIAdapter::displayStlFile(const std::string& stlFile, VtkProcessor* vtkProcessor)
{
    if (visualizationManager) {
        visualizationManager->displayStlFile(stlFile, vtkProcessor);
    }
}

void MainWindowUIAdapter::showTempDividedStl(VtkProcessor* vtkProcessor)
{
    if (visualizationManager) {
        // UIStateを取得してVisualizationManagerに渡す
        UIState* uiState = ui ? ui->getUIState() : nullptr;
        visualizationManager->showTempDividedStl(vtkProcessor, qobject_cast<QWidget*>(ui), uiState);
    }
}

void MainWindowUIAdapter::setVisualizationObjectVisible(const std::string& filename, bool visible)
{
    if (visualizationManager) {
        visualizationManager->setObjectVisible(filename, visible);
    }
}

void MainWindowUIAdapter::setVisualizationObjectOpacity(const std::string& filename, double opacity)
{
    if (visualizationManager) {
        visualizationManager->setObjectOpacity(filename, opacity);
    }
}

void MainWindowUIAdapter::removeDividedStlActors()
{
    if (visualizationManager) {
        visualizationManager->removeDividedStlActors();
    }
}

void MainWindowUIAdapter::hideAllStlObjects()
{
    if (visualizationManager) {
        visualizationManager->hideAllStlObjects();
    }
}

void MainWindowUIAdapter::hideVtkObject()
{
    if (visualizationManager) {
        visualizationManager->hideVtkObject();
    }
}

std::vector<std::string> MainWindowUIAdapter::getAllStlFilenames() const
{
    if (visualizationManager) {
        return visualizationManager->getAllStlFilenames();
    }
    return {};
}

std::string MainWindowUIAdapter::getVtkFilename() const
{
    if (visualizationManager) {
        return visualizationManager->getVtkFilename();
    }
    return "";
}