#include "MainWindowUIAdapter.h"
#include "ApplicationController.h"
#include "../../UI/widgets/AdaptiveDensitySlider.h"
#include "../../UI/visualization/VisualizationManager.h"
#include "../processing/VtkProcessor.h"
#include "../ui/UIState.h"
#include <algorithm>
#include <vector>
#include <filesystem>

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
    // DisplayOptionsContainerが削除されたため、空の実装
    // 表示機能自体は保持されるが、UIウィジェットとの接続は行わない
}

void MainWindowUIAdapter::setStepFileName(const QString& fileName)
{
    // DisplayOptionsContainerが削除されたため、空の実装
    // 表示機能自体は保持されるが、UIウィジェットとの接続は行わない
}

void MainWindowUIAdapter::setVtkVisibilityState(bool visible)
{
    // DisplayOptionsContainerが削除されたため、空の実装
    // 表示機能自体は保持されるが、UIウィジェットとの接続は行わない
}

void MainWindowUIAdapter::setStepVisibilityState(bool visible)
{
    // DisplayOptionsContainerが削除されたため、空の実装
    // 表示機能自体は保持されるが、UIウィジェットとの接続は行わない
}

void MainWindowUIAdapter::setVtkOpacity(double opacity)
{
    // DisplayOptionsContainerが削除されたため、空の実装
    // 表示機能自体は保持されるが、UIウィジェットとの接続は行わない
}

void MainWindowUIAdapter::setStepOpacity(double opacity)
{
    // DisplayOptionsContainerが削除されたため、空の実装
    // 表示機能自体は保持されるが、UIウィジェットとの接続は行わない
}

void MainWindowUIAdapter::setDividedMeshVisibility(int meshIndex, bool visible)
{
    // DisplayOptionsContainerが削除されたため、空の実装
    // 表示機能自体は保持されるが、UIウィジェットとの接続は行わない
}

void MainWindowUIAdapter::setDividedMeshOpacity(int meshIndex, double opacity)
{
    // DisplayOptionsContainerが削除されたため、空の実装
    // 表示機能自体は保持されるが、UIウィジェットとの接続は行わない
}

void MainWindowUIAdapter::setDividedMeshFileName(int meshIndex, const QString& fileName)
{
    // DisplayOptionsContainerが削除されたため、空の実装
    // 表示機能自体は保持されるが、UIウィジェットとの接続は行わない
}

void MainWindowUIAdapter::initializeStressConfiguration(double minStress, double maxStress)
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
    
    // UIStateにストレス範囲とStressDensityMappingを登録
    UIState* uiState = ui->getUIState();
    if (uiState) {
        uiState->setStressRange(minStress, maxStress);

        // DensitySliderからStressDensityMappingを取得してUIStateに登録
        if (slider) {
            auto mappings = slider->stressDensityMappings();
            uiState->setStressDensityMappings(mappings);
        }
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



// 3D可視化制御の実装
void MainWindowUIAdapter::displayVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor)
{
    if (visualizationManager) {
        visualizationManager->displayVtkFile(vtkFile, vtkProcessor);
    }
}

void MainWindowUIAdapter::displayStepFile(const std::string& stepFile)
{
    if (visualizationManager) {
        visualizationManager->displayStepFile(stepFile);
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

void MainWindowUIAdapter::setStepFileVisible(const std::string& stepFile, bool visible)
{
    if (visualizationManager) {
        visualizationManager->setStepFileVisible(stepFile, visible);
    }
}

void MainWindowUIAdapter::setStepFileOpacity(const std::string& stepFile, double opacity)
{
    if (visualizationManager) {
        visualizationManager->setStepFileOpacity(stepFile, opacity);
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

void MainWindowUIAdapter::registerDividedMeshes(const std::vector<MeshInfo>& meshInfos)
{
    if (!ui) return;

    auto* uiState = ui->getUIState();
    if (!uiState) return;

    // 各分割メッシュをInfillRegionとして登録
    constexpr int DIVIDED_MESH_COUNT = 4;
    for (size_t i = 0; i < meshInfos.size() && i < DIVIDED_MESH_COUNT; ++i) {
        const auto& meshInfo = meshInfos[i];

        // InfillRegionInfo を作成
        InfillRegionInfo regionInfo;

        // ファイル名を抽出
        std::filesystem::path filePath(meshInfo.filePath);
        regionInfo.filename = QString::fromStdString(filePath.filename().string());

        // 名前をファイル名と同じにする
        regionInfo.name = regionInfo.filename;

        regionInfo.filePath = QString::fromStdString(meshInfo.filePath);
        regionInfo.isVisible = true;
        regionInfo.transparency = 1.0;

        // UIStateに登録（キー: ファイル名ベース、拡張子なし "modifierMesh00", "modifierMesh01", ...）
        QString key = QString::fromStdString(filePath.stem().string());
        uiState->addInfillRegion(key, regionInfo);

        qDebug() << "Registered divided mesh to UIState:" << key
                 << "Name:" << regionInfo.name
                 << "Path:" << regionInfo.filePath;
    }
}

void MainWindowUIAdapter::setSimulationProgress(int progress, const QString& message)
{
    if (!ui) return;

    auto* processManager = ui->getProcessManagerWidget();
    if (!processManager) return;

    auto* simWidget = processManager->getSimulationStep();
    if (simWidget) {
        simWidget->setProgress(progress, message);
    }
}

void MainWindowUIAdapter::setSimulationRunning(bool running) {
    if (ui && ui->getProcessManagerWidget() && ui->getProcessManagerWidget()->getSimulationStep()) {
        ui->getProcessManagerWidget()->getSimulationStep()->setSimulationRunning(running);
        if (running) {
             ui->getProcessManagerWidget()->getSimulationStep()->clearLog();
        }
    }
}

void MainWindowUIAdapter::appendSimulationLog(const QString& message) {
    if (ui && ui->getProcessManagerWidget() && ui->getProcessManagerWidget()->getSimulationStep()) {
        ui->getProcessManagerWidget()->getSimulationStep()->appendLog(message);
    }
}

void MainWindowUIAdapter::checkHighDensityWarning() {
    if (!ui) return;

    auto slider = ui->getRangeSlider();
    if (!slider) return;

    int maxDensityCount = slider->countMaxDensityRegions();

    // Show warning if 2 or more regions have reached maximum density
    if (maxDensityCount >= 2) {
        QString message = QString(
            "%1 of 4 regions are at maximum density (90%).\n\n"
            "Consider reviewing your design, load, or constraints."
        ).arg(maxDensityCount);

        QMessageBox msgBox(qobject_cast<QWidget*>(ui));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("High Density Warning");
        msgBox.setText(message);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
}

void MainWindowUIAdapter::setVolumeFractions(const std::vector<double>& fractions) {
    if (!ui) return;

    AdaptiveDensitySlider* slider = ui->getRangeSlider();
    if (slider) {
        slider->setVolumeFractions(fractions);
    }
}