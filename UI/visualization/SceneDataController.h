#pragma once

#include <vector>
#include <string>
#include <optional>
#include <filesystem>
#include <vtkSmartPointer.h>
#include <vtkActor.h>

struct ObjectInfo {
    vtkSmartPointer<vtkActor> actor;
    std::string filename;
    bool visible;
    double opacity;
};

class VtkProcessor;

class SceneDataController {
public:
    SceneDataController();
    ~SceneDataController();

    // オブジェクト管理
    void registerObject(const ObjectInfo& objInfo);
    void setObjectVisible(const std::string& filename, bool visible);
    void setObjectOpacity(const std::string& filename, double opacity);
    void removeDividedStlActors();
    
    // 一括制御
    void hideAllStlObjects();
    void hideVtkObject();
    
    // ファイル情報取得
    std::vector<std::string> getAllStlFilenames() const;
    std::string getVtkFilename() const;
    const std::vector<ObjectInfo>& getObjectList() const;
    
    // STL分割ファイル処理
    std::vector<std::pair<std::filesystem::path, int>> fetchDividedStlFiles();
    std::optional<std::pair<double, double>> parseStressRange(const std::string& filename);
    
    // VTKファイル処理
    vtkSmartPointer<vtkActor> loadVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor);
    vtkSmartPointer<vtkActor> loadStlFile(const std::string& stlFile, VtkProcessor* vtkProcessor);
    
    // 分割STLファイル処理
    std::vector<vtkSmartPointer<vtkActor>> loadDividedStlFiles(
        const std::vector<std::pair<std::filesystem::path, int>>& stlFiles,
        VtkProcessor* vtkProcessor,
        double minStress,
        double maxStress);

private:
    std::vector<ObjectInfo> objectList_;
    
    std::vector<std::pair<std::filesystem::path, int>> sortStlFiles(const std::filesystem::path& tempDir);
    void calculateColor(double normalizedPos, double& r, double& g, double& b);
    vtkSmartPointer<vtkActor> createStlActorWithStress(
        const std::filesystem::path& path,
        const std::pair<double, double>& stressValues,
        double minStress,
        double maxStress,
        VtkProcessor* vtkProcessor);
    vtkSmartPointer<vtkActor> createStlActorWithColor(
        const std::filesystem::path& path,
        int number,
        size_t totalFiles,
        VtkProcessor* vtkProcessor);
};