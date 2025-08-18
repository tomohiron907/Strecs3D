#pragma once

#include <string>
#include <vector>
#include <memory>
#include <QString>
#include <QWidget>
#include <QMessageBox>
#include <vtkSmartPointer.h>
#include "../../UI/widgets/DensitySlider.h"

class VtkProcessor;
class BaseLib3mfProcessor;
class vtkPolyData;
class StressDensityMapping;

class ProcessPipeline {
public:
    ProcessPipeline();
    ~ProcessPipeline();

    // VTKファイル処理
    bool initializeVtkProcessor(const std::string& vtkFile, const std::string& stlFile, 
                               const std::vector<double>& thresholds, QWidget* parent = nullptr);
    
    // メッシュ分割処理
    std::vector<vtkSmartPointer<vtkPolyData>> processMeshDivision();
    
    // 3MFファイル処理
    bool process3mfFile(const std::string& mode, const std::vector<StressDensityMapping>& mappings, 
                       double maxStress, QWidget* parent = nullptr);
    
    // ファイル読み込み
    bool loadInputFiles(BaseLib3mfProcessor& processor, const std::string& stlFile);
    
    // モード別処理
    std::unique_ptr<BaseLib3mfProcessor> createProcessor(const QString& mode);
    bool processCuraMode(BaseLib3mfProcessor& processor, const std::vector<StressDensityMapping>& mappings, 
                        double maxStress);
    bool processBambuMode(BaseLib3mfProcessor& processor, double maxStress, const std::vector<StressDensityMapping>& mappings);
    bool processBambuZipFiles();
    
    // エラーハンドリング
    void handle3mfError(const std::exception& e, QWidget* parent = nullptr);
    
    // ゲッター
    std::unique_ptr<VtkProcessor>& getVtkProcessor() { return vtkProcessor; }
    double getMaxStress() const;

private:
    std::unique_ptr<VtkProcessor> vtkProcessor;
    std::string vtkFile;
    std::string stlFile;
}; 