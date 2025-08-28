#include "VisualizationManager.h"
#include "SceneDataController.h"
#include "SceneRenderer.h"
#include "../../core/processing/VtkProcessor.h"
#include "../mainwindowui.h"

VisualizationManager::VisualizationManager(MainWindowUI* ui) : QObject() {
    initializeComponents(ui);
    // ソフトウェア起動時にカメラをリセット
    renderer_->resetCamera();
}

VisualizationManager::~VisualizationManager() = default;

void VisualizationManager::initializeComponents(MainWindowUI* ui) {
    dataController_ = std::make_unique<SceneDataController>();
    renderer_ = std::make_unique<SceneRenderer>(ui);
    
    // Connect signals
    connect(renderer_.get(), &SceneRenderer::objectVisibilityChanged,
            [this](const std::string& filename, bool visible) {
                dataController_->setObjectVisible(filename, visible);
                renderer_->renderObjects(dataController_->getObjectList());
            });
    connect(renderer_.get(), &SceneRenderer::objectOpacityChanged,
            [this](const std::string& filename, double opacity) {
                dataController_->setObjectOpacity(filename, opacity);
                renderer_->renderObjects(dataController_->getObjectList());
            });
}

void VisualizationManager::displayVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor) {
    if (!vtkProcessor) return;
    
    auto importActor = dataController_->loadVtkFile(vtkFile, vtkProcessor);
    if (importActor) {
        renderer_->addActorToRenderer(importActor);
        ObjectInfo objInfo{importActor, vtkFile, true, 1.0};
        dataController_->registerObject(objInfo);
        renderer_->setupScalarBar(vtkProcessor);
        renderer_->renderObjects(dataController_->getObjectList());
    }
}

void VisualizationManager::displayStlFile(const std::string& stlFile, VtkProcessor* vtkProcessor) {
    if (!vtkProcessor) return;
    
    auto importActor = dataController_->loadStlFile(stlFile, vtkProcessor);
    if (importActor) {
        renderer_->addActorToRenderer(importActor);
        ObjectInfo objInfo{importActor, stlFile, true, 1.0};
        dataController_->registerObject(objInfo);
        renderer_->renderObjects(dataController_->getObjectList());
    }
}

void VisualizationManager::showTempDividedStl(VtkProcessor* vtkProcessor, QWidget* parent) {
    try {
        auto stlFiles = dataController_->fetchDividedStlFiles();
        if (stlFiles.empty()) {
            throw std::runtime_error("No valid STL files found");
        }
        double minStress = vtkProcessor->getMinStress();
        double maxStress = vtkProcessor->getMaxStress();
        const auto& meshInfos = vtkProcessor->getMeshInfos();
        auto widgets = renderer_->fetchMeshDisplayWidgets();
        
        auto actors = dataController_->loadDividedStlFiles(stlFiles, vtkProcessor, minStress, maxStress, meshInfos);
        
        int widgetIndex = 0;
        for (size_t i = 0; i < stlFiles.size() && i < actors.size(); ++i) {
            const auto& [path, number] = stlFiles[i];
            std::string filename = path.filename().string();
            renderer_->addActorToRenderer(actors[i]);
            renderer_->updateWidgetAndConnectSignals(widgets, widgetIndex, filename, path.string());
        }
        
        renderer_->renderObjects(dataController_->getObjectList());
    }
    catch (const std::exception& e) {
        renderer_->handleStlFileLoadError(e, parent);
    }
}

 


void VisualizationManager::setObjectVisible(const std::string& filename, bool visible) {
    dataController_->setObjectVisible(filename, visible);
    renderer_->renderObjects(dataController_->getObjectList());
}

void VisualizationManager::setObjectOpacity(const std::string& filename, double opacity) {
    dataController_->setObjectOpacity(filename, opacity);
    renderer_->renderObjects(dataController_->getObjectList());
}

 

void VisualizationManager::removeDividedStlActors() {
    dataController_->removeDividedStlActors();
    renderer_->renderObjects(dataController_->getObjectList());
} 

void VisualizationManager::hideAllStlObjects() {
    dataController_->hideAllStlObjects();
    renderer_->renderObjects(dataController_->getObjectList());
}

void VisualizationManager::hideVtkObject() {
    dataController_->hideVtkObject();
    renderer_->renderObjects(dataController_->getObjectList());
}

std::vector<std::string> VisualizationManager::getAllStlFilenames() const {
    return dataController_->getAllStlFilenames();
}

std::string VisualizationManager::getVtkFilename() const {
    return dataController_->getVtkFilename();
}

 