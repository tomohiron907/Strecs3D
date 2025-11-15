#include "ApplicationController.h"
#include "MainWindowUIAdapter.h"
#include "../../UI/mainwindowui.h"
#include "../../utils/fileUtility.h"
#include "../../utils/tempPathUtility.h"
#include "../processing/VtkProcessor.h"
#include "../processing/StepToStlConverter.h"
#include "../ui/UIState.h"
#include "../../FEM/SimulationConditionExporter.h"
#include "../../FEM/fem_pipeline.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>

ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent)
    , fileProcessor(std::make_unique<ProcessPipeline>())
    , exportManager(std::make_unique<ExportManager>())
{
}

UIState* ApplicationController::getUIState(IUserInterface* ui)
{
    if (!ui) return nullptr;

    auto* adapter = dynamic_cast<MainWindowUIAdapter*>(ui);
    if (!adapter) return nullptr;

    auto* mainWindowUI = adapter->getMainWindowUI();
    if (!mainWindowUI) return nullptr;

    return mainWindowUI->getUIState();
}


bool ApplicationController::openVtkFile(const std::string& vtkFile, IUserInterface* ui)
{
    if (!ui) return false;

    // VTK用ObjectDisplayOptionsWidgetのファイル名と状態を更新
    ui->setVtkFileName(QString::fromStdString(vtkFile));
    ui->setVtkOpacity(1.0);

    // STLを非表示にし、チェックボックスもオフ
    ui->setStlVisibilityState(false);
    ui->setStlOpacity(1.0);
    ui->hideAllStlObjects();

    try {
        ui->displayVtkFile(vtkFile, fileProcessor->getVtkProcessor().get());

        // ストレス範囲をスライダーに設定
        if (fileProcessor->getVtkProcessor()) {
            ui->initializeStressConfiguration(
                fileProcessor->getVtkProcessor()->getMinStress(),
                fileProcessor->getVtkProcessor()->getMaxStress()
            );
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error opening VTK file: " << e.what() << std::endl;
        return false;
    }
}

bool ApplicationController::openStlFile(const std::string& stlFile, IUserInterface* ui)
{
    if (!ui) return false;

    setCurrentStlFilename(QString::fromStdString(stlFile));

    // ObjectDisplayOptionsWidgetのファイル名を更新
    ui->setStlFileName(QString::fromStdString(stlFile));

    try {
        ui->displayStlFile(stlFile, fileProcessor->getVtkProcessor().get());
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error opening STL file: " << e.what() << std::endl;
        return false;
    }
}

bool ApplicationController::openStepFile(const std::string& stepFile, IUserInterface* ui)
{
    if (!ui) return false;

    try {
        // STEPファイルを表示
        ui->displayStepFile(stepFile);
        std::cout << "Successfully loaded STEP file: " << stepFile << std::endl;

        // STEPファイルをSTLに変換して保存
        StepToStlConverter converter;
        QString stlPath = converter.convertAndSave(QString::fromStdString(stepFile));

        if (stlPath.isEmpty()) {
            std::cerr << "Warning: Failed to convert STEP to STL" << std::endl;
            convertedStlPath_.clear();
            // STEPファイルの表示は成功しているので、変換失敗でもtrueを返す
        } else {
            std::cout << "STEP file converted to STL: " << stlPath.toStdString() << std::endl;
            // 変換されたSTLファイルパスを保存
            convertedStlPath_ = stlPath;
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error opening STEP file: " << e.what() << std::endl;
        return false;
    }
}

bool ApplicationController::processFiles(IUserInterface* ui)
{
    try {
        // Step 1: Validate input files
        if (!validateFiles(ui)) {
            return false;
        }
        
        // Step 2: Initialize VTK processor with stress thresholds
        if (!initializeVtkProcessor(ui)) {
            return false;
        }
        
        // Step 3: Process mesh division
        if (!processMeshDivision(ui)) {
            return false;
        }
        
        // Step 4: Process 3MF file generation
        if (!process3mfGeneration(ui)) {
            return false;
        }
        
        // Step 5: Load and display temporary STL files
        loadAndDisplayTempStlFiles(ui);
        
        // Step 6: Cleanup temporary files
        cleanupTempFiles();
        
        // Step 7: Show success message
        showSuccessMessage(ui);
        
        return true;
    }
    catch (const std::exception& e) {
        handleProcessingError(e, ui);
        return false;
    }
}

bool ApplicationController::validateFiles(IUserInterface* ui)
{
    if (!ui) return false;

    auto* uiState = getUIState(ui);
    if (!uiState) return false;

    if (uiState->getVtkFilePath().isEmpty()) {
        ui->showWarningMessage("Warning", "No VTK file selected");
        return false;
    }
    if (uiState->getStlFilePath().isEmpty()) {
        ui->showWarningMessage("Warning", "No STL file selected");
        return false;
    }
    return true;
}

bool ApplicationController::initializeVtkProcessor(IUserInterface* ui)
{
    if (!ui) return false;

    auto* uiState = getUIState(ui);
    if (!uiState) return false;

    auto thresholds = getStressThresholds(uiState);
    std::string vtkFile = uiState->getVtkFilePath().toStdString();
    std::string stlFile = uiState->getStlFilePath().toStdString();

    if (!fileProcessor->initializeVtkProcessor(vtkFile, stlFile, thresholds, nullptr)) {
        ui->showCriticalMessage("Error", "Failed to initialize VTK processor");
        return false;
    }
    return true;
}

bool ApplicationController::processMeshDivision(IUserInterface* ui)
{
    if (!ui) return false;

    auto dividedMeshes = fileProcessor->processMeshDivision();
    if (dividedMeshes.empty()) {
        ui->showCriticalMessage("Error", "No meshes generated during division");
        return false;
    }

    fileProcessor->getVtkProcessor()->saveDividedMeshes(dividedMeshes);
    return true;
}

bool ApplicationController::process3mfGeneration(IUserInterface* ui)
{
    if (!ui) return false;

    // MainWindowUIAdapterからUIStateを取得
    auto* adapter = dynamic_cast<MainWindowUIAdapter*>(ui);
    if (!adapter) return false;

    auto* mainWindowUI = adapter->getMainWindowUI();
    if (!mainWindowUI) return false;

    auto* uiState = mainWindowUI->getUIState();
    if (!uiState) return false;

    auto mappings = getStressDensityMappings(uiState);
    auto currentMode = getCurrentMode(uiState);
    double maxStress = fileProcessor->getMaxStress();

    if (!fileProcessor->process3mfFile(currentMode.toStdString(), mappings, maxStress, nullptr)) {
        ui->showCriticalMessage("Error", "Failed to process 3MF file");
        return false;
    }
    return true;
}

void ApplicationController::loadAndDisplayTempStlFiles(IUserInterface* ui)
{
    if (!ui || !fileProcessor->getVtkProcessor()) return;

    // 分割STL Actorを削除
    ui->removeDividedStlActors();
    // 分割されたメッシュウィジェットをリセット
    resetDividedMeshWidgets(ui);

    // VTKを非表示にし、チェックボックスもオフ
    ui->setVtkVisibilityState(false);
    ui->setVtkOpacity(1.0);
    ui->hideVtkObject();

    // 分割STLウィジェットのチェックボックスをオン
    for (int i = 0; i < DIVIDED_MESH_COUNT; ++i) {
        ui->setDividedMeshVisibility(i, true);
        ui->setDividedMeshOpacity(i, 1.0);
    }

    ui->showTempDividedStl(fileProcessor->getVtkProcessor().get());
}

void ApplicationController::cleanupTempFiles()
{
    std::filesystem::path tempFiledir = TempPathUtility::getTempSubDirPath("div");
    FileUtility::clearDirectoryContents(tempFiledir);
}

void ApplicationController::showSuccessMessage(IUserInterface* ui)
{
    if (!ui) return;
    ui->showInfoMessage("Success", "Files processed successfully");
}

void ApplicationController::handleProcessingError(const std::exception& e, IUserInterface* ui)
{
    if (!ui) return;
    std::cerr << "Error processing files: " << e.what() << std::endl;
    ui->showCriticalMessage("Error", QString("Failed to process files: ") + e.what());
}

bool ApplicationController::export3mfFile(IUserInterface* ui)
{
    if (!ui) return false;

    auto* uiState = getUIState(ui);
    if (!uiState) return false;

    std::string stlFile = uiState->getStlFilePath().toStdString();
    return exportManager->export3mfFile(stlFile, nullptr);
}

std::vector<int> ApplicationController::getStressThresholds(UIState* uiState)
{
    if (!uiState) return {};

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

std::vector<StressDensityMapping> ApplicationController::getStressDensityMappings(UIState* uiState)
{
    if (!uiState) return {};

    return uiState->getStressDensityMappings();
}

QString ApplicationController::getCurrentMode(UIState* uiState)
{
    if (!uiState) return "cura";

    ProcessingMode mode = uiState->getProcessingMode();
    switch(mode) {
        case ProcessingMode::BAMBU: return "bambu";
        case ProcessingMode::CURA: return "cura";
        case ProcessingMode::PRUSA: return "prusa";
        default: return "cura";
    }
}

void ApplicationController::resetDividedMeshWidgets(IUserInterface* ui)
{
    if (!ui) return;

    // 分割されたメッシュウィジェットをリセット
    for (int i = 0; i < DIVIDED_MESH_COUNT; ++i) {
        ui->setDividedMeshFileName(i, QString("Divided Mesh %1").arg(i + 1));
        ui->setDividedMeshOpacity(i, 1.0);
    }
}

void ApplicationController::setMeshVisibility(const std::string& fileName, bool visible, IUserInterface* ui)
{
    if (!ui || fileName.empty()) return;

    // IUserInterface経由で実際の可視化を更新
    ui->setVisualizationObjectVisible(fileName, visible);
}

void ApplicationController::setMeshOpacity(const std::string& fileName, double opacity, IUserInterface* ui)
{
    if (!ui || fileName.empty()) return;

    // IUserInterface経由で実際の可視化を更新
    ui->setVisualizationObjectOpacity(fileName, opacity);
}

bool ApplicationController::exportSimulationCondition(IUserInterface* ui, UIState* uiState, const QString& outputPath)
{
    if (!ui || !uiState) {
        return false;
    }

    // STEPファイルが読み込まれているか確認
    QString stepFilePath = uiState->getStepFilePath();
    if (stepFilePath.isEmpty()) {
        std::cerr << "Error: STEP file is not loaded" << std::endl;
        ui->showWarningMessage("警告", "STEPファイルが読み込まれていません");
        return false;
    }

    // SimulationConditionExporterを使用してJSONを出力
    SimulationConditionExporter exporter;
    bool success = exporter.exportToJson(uiState, outputPath);

    if (!success) {
        std::cerr << "Error: Failed to export simulation condition" << std::endl;
        ui->showCriticalMessage("エラー", "シミュレーション条件の出力に失敗しました");
    }

    return success;
}

QString ApplicationController::runSimulation(IUserInterface* ui, const QString& configFilePath)
{
    if (!ui) {
        return QString();
    }

    // 設定ファイルのパスを確認
    if (configFilePath.isEmpty()) {
        std::cerr << "Error: Configuration file path is empty" << std::endl;
        ui->showWarningMessage("警告", "設定ファイルのパスが指定されていません");
        return QString();
    }

    try {
        // FEM解析パイプラインを実行
        std::string configFilePathStd = configFilePath.toStdString();
        std::string vtuFilePath = runFEMAnalysis(configFilePathStd);

        if (!vtuFilePath.empty()) {
            ui->showInfoMessage("成功", "FEMシミュレーションが正常に完了しました");
            return QString::fromStdString(vtuFilePath);
        } else {
            std::cerr << "Error: FEM simulation failed - VTU file not generated" << std::endl;
            ui->showCriticalMessage("エラー", "FEMシミュレーションが失敗しました");
            return QString();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error running FEM simulation: " << e.what() << std::endl;
        ui->showCriticalMessage("エラー", QString("FEMシミュレーションの実行中にエラーが発生しました: ") + e.what());
        return QString();
    }
}

bool ApplicationController::runFEMPipeline(IUserInterface* ui, UIState* uiState, const QString& outputPath)
{
    if (!ui || !uiState) {
        return false;
    }

    // Step 1: FEM設定ファイルをJSONにエクスポート
    bool exportSuccess = exportSimulationCondition(ui, uiState, outputPath);

    if (!exportSuccess) {
        // エクスポートが失敗した場合、FEM解析は実行しない
        ui->showCriticalMessage("エラー", "FEM設定ファイルのエクスポートに失敗したため、解析を実行できません");
        return false;
    }

    // Step 2: エクスポートした設定ファイルを使用してFEM解析を実行
    QString vtuFilePath = runSimulation(ui, outputPath);

    // Step 3: VTUファイルが正常に生成された場合、自動的に開く
    if (!vtuFilePath.isEmpty()) {
        openVtkFile(vtuFilePath.toStdString(), ui);

        // UIStateにもVTUファイルパスを保存
        uiState->setVtkFilePath(vtuFilePath);

        return true;
    }

    return false;
}

bool ApplicationController::isStlFileLoaded(UIState* uiState) const
{
    if (!uiState) return false;
    return !uiState->getStlFilePath().isEmpty();
}

bool ApplicationController::isVtkFileLoaded(UIState* uiState) const
{
    if (!uiState) return false;
    return !uiState->getVtkFilePath().isEmpty();
}

bool ApplicationController::areBothFilesLoaded(UIState* uiState) const
{
    return isStlFileLoaded(uiState) && isVtkFileLoaded(uiState);
} 