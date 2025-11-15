#pragma once

#include <memory>
#include <string>
#include <QString>
#include <QObject>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include "../processing/ProcessPipeline.h"
#include "../export/ExportManager.h"
#include "../interfaces/IUserInterface.h"

class UIState;

class ApplicationController : public QObject {
    Q_OBJECT
public:
    static constexpr int DIVIDED_MESH_COUNT = 4;
    ApplicationController(QObject* parent = nullptr);
    ~ApplicationController() = default;
    

    // ファイル操作
    bool openVtkFile(const std::string& vtkFile, IUserInterface* ui);
    bool openStlFile(const std::string& stlFile, IUserInterface* ui);
    bool openStepFile(const std::string& stepFile, IUserInterface* ui);

    // STEPファイルから変換されたSTLファイルパスを取得
    QString getConvertedStlPath() const { return convertedStlPath_; }
    
    // メイン処理
    bool processFiles(IUserInterface* ui);
    
    // エクスポート
    bool export3mfFile(IUserInterface* ui);
    bool exportSimulationCondition(IUserInterface* ui, UIState* uiState, const QString& outputPath);

    // シミュレーション実行
    QString runSimulation(IUserInterface* ui, const QString& configFilePath);
    bool runFEMPipeline(IUserInterface* ui, UIState* uiState, const QString& outputPath);

    // 可視化
    void loadAndDisplayTempStlFiles(IUserInterface* ui);
    void setMeshVisibility(const std::string& fileName, bool visible, IUserInterface* ui);
    void setMeshOpacity(const std::string& fileName, double opacity, IUserInterface* ui);

    // 状態管理
    // TODO: これらの状態管理のメソッドが使われていない可能性高．削除できないか検討
    void setVtkFile(const std::string& vtkFile) { this->vtkFile = vtkFile; }
    void setStlFile(const std::string& stlFile) { this->stlFile = stlFile; }
    std::string getVtkFile() const { return vtkFile; }
    std::string getStlFile() const { return stlFile; }
    QString getCurrentStlFilename() const { return currentStlFilename; }
    void setCurrentStlFilename(const QString& filename) { currentStlFilename = filename; }
    
    // ファイル読み込み状態確認
    bool isStlFileLoaded(UIState* uiState) const;
    bool isVtkFileLoaded(UIState* uiState) const;
    bool areBothFilesLoaded(UIState* uiState) const;
    
    // ゲッター
    ProcessPipeline* getFileProcessor() { return fileProcessor.get(); }
    ExportManager* getExportManager() { return exportManager.get(); }

private:
    std::string vtkFile;
    std::string stlFile;
    QString currentStlFilename;
    QString convertedStlPath_;  // STEPから変換されたSTLファイルパス

    std::unique_ptr<ProcessPipeline> fileProcessor;
    std::unique_ptr<ExportManager> exportManager;
    
    // ヘルパーメソッド
    bool validateFiles(IUserInterface* ui);
    std::vector<int> getStressThresholds(UIState* uiState);
    std::vector<StressDensityMapping> getStressDensityMappings(UIState* uiState);
    QString getCurrentMode(UIState* uiState);
    
    // ファイル処理のヘルパーメソッド
    bool initializeVtkProcessor(IUserInterface* ui);
    bool processMeshDivision(IUserInterface* ui);
    bool process3mfGeneration(IUserInterface* ui);
    void cleanupTempFiles();
    void showSuccessMessage(IUserInterface* ui);
    void handleProcessingError(const std::exception& e, IUserInterface* ui);
    void resetDividedMeshWidgets(IUserInterface* ui);

}; 