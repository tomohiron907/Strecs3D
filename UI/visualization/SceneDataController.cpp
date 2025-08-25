#include "SceneDataController.h"
#include "../../core/processing/VtkProcessor.h"
#include "../../utils/tempPathUtility.h"
#include <regex>
#include <algorithm>
#include <filesystem>
#include <stdexcept>

SceneDataController::SceneDataController() {}

SceneDataController::~SceneDataController() = default;

void SceneDataController::registerObject(const ObjectInfo& objInfo) {
    objectList_.push_back(objInfo);
}

void SceneDataController::setObjectVisible(const std::string& filename, bool visible) {
    for (auto& obj : objectList_) {
        if (obj.filename == filename || 
            obj.filename.find(filename) != std::string::npos ||
            filename.find(obj.filename) != std::string::npos) {
            obj.visible = visible;
            if (obj.actor) {
                obj.actor->SetVisibility(visible ? 1 : 0);
            }
            return;
        }
    }
}

void SceneDataController::setObjectOpacity(const std::string& filename, double opacity) {
    for (auto& obj : objectList_) {
        if (obj.filename == filename || 
            obj.filename.find(filename) != std::string::npos ||
            filename.find(obj.filename) != std::string::npos) {
            obj.opacity = opacity;
            if (obj.actor) {
                obj.actor->GetProperty()->SetOpacity(opacity);
            }
            return;
        }
    }
}

void SceneDataController::removeDividedStlActors() {
    std::regex dividedStlPattern(R"(modifierMesh\d+\.stl$)");
    
    objectList_.erase(
        std::remove_if(
            objectList_.begin(), objectList_.end(),
            [&](const ObjectInfo& obj) {
                return std::regex_search(obj.filename, dividedStlPattern);
            }
        ),
        objectList_.end()
    );
}

void SceneDataController::hideAllStlObjects() {
    for (const auto& obj : objectList_) {
        if (obj.filename.size() >= 4 && obj.filename.substr(obj.filename.size() - 4) == ".stl") {
            const_cast<SceneDataController*>(this)->setObjectVisible(obj.filename, false);
        }
    }
}

void SceneDataController::hideVtkObject() {
    for (const auto& obj : objectList_) {
        if ((obj.filename.size() >= 4 && obj.filename.substr(obj.filename.size() - 4) == ".vtu") ||
            (obj.filename.size() >= 4 && obj.filename.substr(obj.filename.size() - 4) == ".vtk")) {
            const_cast<SceneDataController*>(this)->setObjectVisible(obj.filename, false);
        }
    }
}

std::vector<std::string> SceneDataController::getAllStlFilenames() const {
    std::vector<std::string> result;
    for (const auto& obj : objectList_) {
        if (obj.filename.size() >= 4 && obj.filename.substr(obj.filename.size() - 4) == ".stl") {
            result.push_back(obj.filename);
        }
    }
    return result;
}

std::string SceneDataController::getVtkFilename() const {
    for (const auto& obj : objectList_) {
        if ((obj.filename.size() >= 4 && obj.filename.substr(obj.filename.size() - 4) == ".vtu") ||
            (obj.filename.size() >= 4 && obj.filename.substr(obj.filename.size() - 4) == ".vtk")) {
            return obj.filename;
        }
    }
    return "";
}

const std::vector<ObjectInfo>& SceneDataController::getObjectList() const {
    return objectList_;
}

std::vector<std::pair<std::filesystem::path, int>> SceneDataController::fetchDividedStlFiles() {
    std::filesystem::path tempDir = TempPathUtility::getTempSubDirPath("div");
    if (!std::filesystem::exists(tempDir)) {
        throw std::runtime_error(".temp directory does not exist");
    }
    return sortStlFiles(tempDir);
}

std::optional<std::pair<double, double>> SceneDataController::parseStressRange(const std::string& filename, const std::vector<MeshInfo>& meshInfos) {
    std::regex modifierPattern(R"(^modifierMesh(\d+)\.stl$)");
    std::smatch match;
    if (std::regex_search(filename, match, modifierPattern)) {
        int meshID = std::stoi(match[1].str());
        // Look up stress values from meshInfos
        for (const auto& meshInfo : meshInfos) {
            if (meshInfo.meshID == meshID) {
                return std::make_pair(static_cast<double>(meshInfo.stressMin), static_cast<double>(meshInfo.stressMax));
            }
        }
    }
    return std::nullopt;
}

vtkSmartPointer<vtkActor> SceneDataController::loadVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor) {
    if (!vtkProcessor) return nullptr;
    return vtkProcessor->getVtuActor(vtkFile);
}

vtkSmartPointer<vtkActor> SceneDataController::loadStlFile(const std::string& stlFile, VtkProcessor* vtkProcessor) {
    if (!vtkProcessor) return nullptr;
    return vtkProcessor->getStlActor(stlFile);
}

std::vector<vtkSmartPointer<vtkActor>> SceneDataController::loadDividedStlFiles(
    const std::vector<std::pair<std::filesystem::path, int>>& stlFiles,
    VtkProcessor* vtkProcessor,
    double minStress,
    double maxStress,
    const std::vector<MeshInfo>& meshInfos) {
    
    std::vector<vtkSmartPointer<vtkActor>> actors;
    
    for (const auto& [path, number] : stlFiles) {
        std::string filename = path.filename().string();
        vtkSmartPointer<vtkActor> actor = nullptr;
        
        if (auto stressValues = parseStressRange(filename, meshInfos)) {
            actor = createStlActorWithStress(path, *stressValues, minStress, maxStress, vtkProcessor);
        } else {
            actor = createStlActorWithColor(path, number, stlFiles.size(), vtkProcessor);
        }
        
        if (actor) {
            actors.push_back(actor);
            ObjectInfo objInfo{actor, path.string(), true, 1.0};
            registerObject(objInfo);
        }
    }
    
    return actors;
}

std::vector<std::pair<std::filesystem::path, int>> SceneDataController::sortStlFiles(const std::filesystem::path& tempDir) {
    std::vector<std::pair<std::filesystem::path, int>> stlFiles;
    std::regex filePattern(R"(^modifierMesh(\d+)\.stl$)");
    
    for (const auto& entry : std::filesystem::directory_iterator(tempDir)) {
        if (entry.path().extension() == ".stl") {
            std::string filename = entry.path().filename().string();
            std::smatch match;
            if (std::regex_search(filename, match, filePattern)) {
                int number = std::stoi(match[1].str());
                stlFiles.push_back({entry.path(), number});
            }
        }
    }
    
    std::sort(stlFiles.begin(), stlFiles.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
    
    return stlFiles;
}

void SceneDataController::calculateColor(double normalizedPos, double& r, double& g, double& b) {
    if (normalizedPos <= 0.5) {
        double t = normalizedPos * 2.0;
        r = t;
        g = t;
        b = 1.0;
    } else {
        double t = (normalizedPos - 0.5) * 2.0;
        r = 1.0;
        g = 1.0 - t;
        b = 1.0 - t;
    }
}

vtkSmartPointer<vtkActor> SceneDataController::createStlActorWithStress(
    const std::filesystem::path& path,
    const std::pair<double, double>& stressValues,
    double minStress,
    double maxStress,
    VtkProcessor* vtkProcessor) {
    
    double stressValue = (stressValues.first + stressValues.second) / 2.0;
    return vtkProcessor->getColoredStlActorByStress(path.string(), stressValue, minStress, maxStress);
}

vtkSmartPointer<vtkActor> SceneDataController::createStlActorWithColor(
    const std::filesystem::path& path,
    int number,
    size_t totalFiles,
    VtkProcessor* vtkProcessor) {
    
    double r, g, b;
    double normalizedPos = static_cast<double>(number) / totalFiles;
    calculateColor(normalizedPos, r, g, b);
    return vtkProcessor->getColoredStlActor(path.string(), r, g, b);
}