#include "ProcessPipeline.h"
#include "VtkProcessor.h"
#include "3mf/BaseLib3mfProcessor.h"
#include "3mf/slicers/cura/CuraLib3mfProcessor.h"
#include "3mf/slicers/bambu/BambuLib3mfProcessor.h"
#include "3mf/slicers/prusa/PrusaLib3mfProcessor.h"
#include "3mf/slicers/prusa/ModelConverter.h"
#include "../../utils/fileUtility.h"
#include "../../utils/tempPathUtility.h"
#include <QMessageBox>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <vtkPolyData.h>

ProcessPipeline::ProcessPipeline() {
    vtkProcessor = std::make_unique<VtkProcessor>("");
}

ProcessPipeline::~ProcessPipeline() = default;

bool ProcessPipeline::initializeVtkProcessor(const std::string& vtkFile, const std::string& stlFile, 
                                          const std::vector<int>& thresholds, QWidget* parent) {
    this->vtkFile = vtkFile;
    this->stlFile = stlFile;
    
    vtkProcessor->clearPreviousData();
    if (vtkFile.empty()) {
        if (parent) {
            QMessageBox::warning(parent, "Warning", "No VTK file selected");
        }
        return false;
    }
    if (stlFile.empty()) {
        if (parent) {
            QMessageBox::warning(parent, "Warning", "No STL file selected");
        }
        return false;
    }
    
    // VtkProcessorにファイル名を設定し、データを読み込む
    vtkProcessor->setVtuFileName(vtkFile);
    if (!vtkProcessor->LoadAndPrepareData()) {
        if (parent) {
            QMessageBox::critical(parent, "Error", "Failed to load VTK file: " + QString::fromStdString(vtkFile));
        }
        return false;
    }
    
    vtkProcessor->showInfo();
    vtkProcessor->prepareStressValues(thresholds);
    return true;
}

std::vector<vtkSmartPointer<vtkPolyData>> ProcessPipeline::processMeshDivision() {
    if (!vtkProcessor) {
        throw std::runtime_error("VtkProcessor not initialized");
    }
    auto dividedMeshes = vtkProcessor->divideMesh();
    if (dividedMeshes.empty()) {
        throw std::runtime_error("No meshes generated");
    }
    return dividedMeshes;
}

bool ProcessPipeline::process3mfFile(const std::string& mode, const std::vector<StressDensityMapping>& mappings, 
                                  double maxStress, QWidget* parent) {
    try {
        QString currentMode = QString::fromStdString(mode);
        auto processor = createProcessor(currentMode);
        if (!processor) {
            throw std::runtime_error("Failed to create processor for mode: " + mode);
        }
        
        if (!loadInputFiles(*processor, stlFile)) {
            throw std::runtime_error("Failed to load input files");
        }
        
        if (currentMode == "cura") {
            if (!processCuraMode(*processor, mappings, maxStress)) {
                throw std::runtime_error("Failed to process in cura mode");
            }
        } else if (currentMode == "bambu") {
            if (!processBambuMode(*processor, maxStress, mappings)) {
                throw std::runtime_error("Failed to process in bambu mode");
            }
        } else if (currentMode == "prusa") {
            if (!processPrusaMode(*processor, maxStress, mappings)) {
                throw std::runtime_error("Failed to process in prusa mode");
            }
        } else {
            throw std::runtime_error("Unknown mode: " + mode);
        }
        
        return true;
    }
    catch (const std::exception& e) {
        handle3mfError(e, parent);
        return false;
    }
}

bool ProcessPipeline::loadInputFiles(BaseLib3mfProcessor& processor, const std::string& stlFile) {
    if (!processor.getMeshes()) {
        throw std::runtime_error("Failed to load divided meshes");
    }
    if (!processor.setStl(stlFile)) {
        throw std::runtime_error("Failed to load STL file: " + stlFile);
    }
    return true;
}

std::unique_ptr<BaseLib3mfProcessor> ProcessPipeline::createProcessor(const QString& mode) {
    if (mode == "cura") {
        return std::make_unique<CuraLib3mfProcessor>();
    } else if (mode == "bambu") {
        return std::make_unique<BambuLib3mfProcessor>();
    } else if (mode == "prusa") {
        return std::make_unique<PrusaLib3mfProcessor>();
    }
    return nullptr;
}

bool ProcessPipeline::processCuraMode(BaseLib3mfProcessor& processor, const std::vector<StressDensityMapping>& mappings, 
                                   double maxStress) {
    
    const auto& meshInfos = vtkProcessor->getMeshInfos();
    if (!processor.setMetaData(maxStress, mappings, meshInfos)) {
        throw std::runtime_error("Failed to set metadata");
    }
    if (!processor.assembleObjects()) {
        throw std::runtime_error("Failed to assemble objects");
    }
    const std::string outputPath = TempPathUtility::getTempFilePath("result/result.3mf").toStdString();
    if (!processor.save3mf(outputPath)) {
        throw std::runtime_error("Failed to save 3MF file");
    }
    return true;
}

bool ProcessPipeline::processBambuMode(BaseLib3mfProcessor& processor, double maxStress, const std::vector<StressDensityMapping>& mappings) {
    const auto& meshInfos = vtkProcessor->getMeshInfos();
    processor.setMetaData(maxStress, mappings, meshInfos);
    const std::string tempFile = TempPathUtility::getTempFilePath("result/base.3mf").toStdString();
    if (!processor.save3mf(tempFile)) {
        throw std::runtime_error("Failed to save temporary 3MF file");
    }
    const std::string extractDir = TempPathUtility::getTempSubDirPath("3mf").string();
    const std::string zipFile = TempPathUtility::getTempFilePath("result/base.3mf").toStdString();
    const std::string outputFile = TempPathUtility::getTempFilePath("result/result.3mf").toStdString();
    if (!FileUtility::unzipFile(zipFile, extractDir)) {
        throw std::runtime_error("Failed to extract ZIP file");
    }
    if (!FileUtility::zipDirectory(extractDir, outputFile)) {
        throw std::runtime_error("Failed to create output ZIP file");
    }
    return true;
}

bool ProcessPipeline::processPrusaMode(BaseLib3mfProcessor& processor, double maxStress, const std::vector<StressDensityMapping>& mappings) {
    const std::string extractDir = TempPathUtility::getTempSubDirPath("3mf").string();
    const std::string zipFile = TempPathUtility::getTempFilePath("result/base.3mf").toStdString();
    const std::string outputFile = TempPathUtility::getTempFilePath("result/result.3mf").toStdString();
    if (!processor.save3mf(zipFile)) {
        throw std::runtime_error("Failed to save 3MF file");
    }
    if (!FileUtility::unzipFile(zipFile, extractDir)) {
        throw std::runtime_error("Failed to extract ZIP file");
    }

    // PrusaModelConverterを使用して3dmodel.modelを変換
    const std::string modelInputPath = extractDir + "/3D/3dmodel.model";
    const std::string modelOutputPath = extractDir + "/3D/3dmodel_converted.model";
    
    ModelConverter converter;
    if (!converter.process(modelInputPath, modelOutputPath)) {
        throw std::runtime_error("Failed to convert 3dmodel.model using PrusaModelConverter");
    }
    
    // 変換済みファイルを元のファイル名に置き換え
    try {
        std::filesystem::remove(modelInputPath);
        std::filesystem::rename(modelOutputPath, modelInputPath);
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error("Failed to replace original 3dmodel.model with converted version: " + std::string(e.what()));
    }
    // メタデータ設定
    const auto& meshInfos = vtkProcessor->getMeshInfos();
    PrusaLib3mfProcessor prusaProcessor;
    if (!prusaProcessor.setMetaData(extractDir, converter, meshInfos, mappings, maxStress)) {
        throw std::runtime_error("Failed to generate metadata for Prusa");
    }
    
    if (!FileUtility::zipDirectory(extractDir, outputFile)) {
        throw std::runtime_error("Failed to create output ZIP file");
    }
    return true;
}

void ProcessPipeline::handle3mfError(const std::exception& e, QWidget* parent) {
    std::cerr << "3MF Processing Error: " << e.what() << std::endl;
    if (parent) {
        QMessageBox::critical(parent, 
                             "3MF Processing Error", 
                             QString::fromStdString("Failed to process 3MF file:\n" + std::string(e.what())));
    }
}

double ProcessPipeline::getMaxStress() const {
    if (vtkProcessor) {
        return vtkProcessor->getMaxStress();
    }
    return 0.0;
}

const std::vector<double>& ProcessPipeline::getVolumeFractions() const {
    static const std::vector<double> empty;
    if (vtkProcessor && vtkProcessor->hasVolumeFractions()) {
        return vtkProcessor->getVolumeFractions();
    }
    return empty;
}

bool ProcessPipeline::hasVolumeFractions() const {
    return vtkProcessor && vtkProcessor->hasVolumeFractions();
} 