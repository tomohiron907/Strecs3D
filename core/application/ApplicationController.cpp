#include "ApplicationController.h"
#include "MainWindowUIAdapter.h"
#include "../../UI/mainwindowui.h"
#include "../../utils/fileUtility.h"
#include "../../utils/tempPathUtility.h"
#include "../processing/VtkProcessor.h"
#include <iostream>
#include <stdexcept>

ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent)
    , fileProcessor(std::make_unique<ProcessPipeline>())
    , exportManager(std::make_unique<ExportManager>())
{
}


bool ApplicationController::openVtkFile(const std::string& vtkFile, IUserInterface* ui)
{
    if (!ui) return false;
    
    setVtkFile(vtkFile);
    
    // VTK用ObjectDisplayOptionsWidgetのファイル名を更新
    emit vtkFileNameChanged(QString::fromStdString(vtkFile));
    emit vtkOpacityChanged(1.0);
    
    // --- 追加: STLを非表示にし、チェックボックスもオフ ---
    emit stlVisibilityChanged(false);
    emit stlOpacityChanged(1.0);
    ui->hideAllStlObjects();
    // --- ここまで追加 ---
    try {
        ui->displayVtkFile(vtkFile, fileProcessor->getVtkProcessor().get());
        
        // ストレス範囲をスライダーに設定
        if (fileProcessor->getVtkProcessor()) {
            emit stressRangeChanged(
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
    
    setStlFile(stlFile);
    setCurrentStlFilename(QString::fromStdString(stlFile));
    
    // ObjectDisplayOptionsWidgetのファイル名を更新
    emit stlFileNameChanged(QString::fromStdString(stlFile));
    
    try {
        ui->displayStlFile(stlFile, fileProcessor->getVtkProcessor().get());
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error opening STL file: " << e.what() << std::endl;
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
    
    if (vtkFile.empty()) {
        emit showWarningMessage("Warning", "No VTK file selected");
        return false;
    }
    if (stlFile.empty()) {
        emit showWarningMessage("Warning", "No STL file selected");
        return false;
    }
    return true;
}

bool ApplicationController::initializeVtkProcessor(IUserInterface* ui)
{
    if (!ui) return false;
    
    auto thresholds = getStressThresholds(ui);
    if (!fileProcessor->initializeVtkProcessor(vtkFile, stlFile, thresholds, nullptr)) {
        emit showCriticalMessage("Error", "Failed to initialize VTK processor");
        return false;
    }
    return true;
}

bool ApplicationController::processMeshDivision(IUserInterface* ui)
{
    if (!ui) return false;
    
    auto dividedMeshes = fileProcessor->processMeshDivision();
    if (dividedMeshes.empty()) {
        emit showCriticalMessage("Error", "No meshes generated during division");
        return false;
    }
    
    fileProcessor->getVtkProcessor()->saveDividedMeshes(dividedMeshes);
    return true;
}

bool ApplicationController::process3mfGeneration(IUserInterface* ui)
{
    if (!ui) return false;
    
    auto mappings = getStressDensityMappings(ui);
    auto currentMode = getCurrentMode(ui);
    double maxStress = fileProcessor->getMaxStress();
    
    if (!fileProcessor->process3mfFile(currentMode.toStdString(), mappings, maxStress, nullptr)) {
        emit showCriticalMessage("Error", "Failed to process 3MF file");
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
    
    // --- 追加: VTKを非表示にし、チェックボックスもオフ ---
    emit vtkVisibilityChanged(false);
    emit vtkOpacityChanged(1.0);
    ui->hideVtkObject();
    // 分割STLウィジェットのチェックボックスをオン
    for (int i = 0; i < DIVIDED_MESH_COUNT; ++i) {
        emit dividedMeshVisibilityChanged(i, true);
        emit dividedMeshOpacityChanged(i, 1.0);
    }
    // --- ここまで追加 ---
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
    emit showInfoMessage("Success", "Files processed successfully");
}

void ApplicationController::handleProcessingError(const std::exception& e, IUserInterface* ui)
{
    if (!ui) return;
    std::cerr << "Error processing files: " << e.what() << std::endl;
    emit showCriticalMessage("Error", QString("Failed to process files: ") + e.what());
}

bool ApplicationController::export3mfFile(IUserInterface* ui)
{
    if (!ui) return false;
    return exportManager->export3mfFile(stlFile, nullptr);
}

std::vector<int> ApplicationController::getStressThresholds(IUserInterface* ui)
{
    if (!ui) return {};
    
    return ui->getStressThresholds();
}

std::vector<StressDensityMapping> ApplicationController::getStressDensityMappings(IUserInterface* ui)
{
    if (!ui) return {};
    
    return ui->getStressDensityMappings();
}

QString ApplicationController::getCurrentMode(IUserInterface* ui)
{
    if (!ui) return "cura";
    
    return ui->getCurrentMode();
}

void ApplicationController::resetDividedMeshWidgets(IUserInterface* ui)
{
    if (!ui) return;
    
    // 分割されたメッシュウィジェットをリセット
    for (int i = 0; i < DIVIDED_MESH_COUNT; ++i) {
        emit dividedMeshFileNameChanged(i, QString("Divided Mesh %1").arg(i + 1));
        emit dividedMeshOpacityChanged(i, 1.0);
    }
} 