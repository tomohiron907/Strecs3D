#include "VisualizationManager.h"
#include "ActorFactory.h"
#include "SceneRenderer.h"
#include "../../core/processing/VtkProcessor.h"
#include "../../core/processing/StepReader.h"
#include "../../core/ui/UIState.h"
#include "../../core/types/BoundaryCondition.h"
#include "../../utils/tempPathUtility.h"
#include "../mainwindowui.h"
#include <regex>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vtkPlaneSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

VisualizationManager::VisualizationManager(MainWindowUI* ui) : QObject() {
    actorFactory_ = std::make_unique<ActorFactory>();
    sceneRenderer_ = std::make_unique<SceneRenderer>(ui);
    currentStepReader_ = nullptr;

    // Create and register grid
    auto gridActor = actorFactory_->createGridActor();
    registerObject({gridActor, "__grid__", true, 0.3});

    // Create and register axes
    auto axisActors = actorFactory_->createAxisActors();
    registerObject({axisActors.xAxis, "__x_axis__", true, 1.0});
    registerObject({axisActors.yAxis, "__y_axis__", true, 1.0});
    registerObject({axisActors.zAxis, "__z_axis__", true, 1.0});
    registerObject({axisActors.origin, "__origin__", true, 1.0});

    connectSignals();
    sceneRenderer_->resetCamera();

    // Render initial scene with grid, axes, and origin
    sceneRenderer_->renderObjects(objectList_);
}

VisualizationManager::~VisualizationManager() = default;

// --- File Display Operations ---

void VisualizationManager::displayVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor) {
    if (!vtkProcessor) return;

    auto actor = actorFactory_->createVtkActor(vtkFile, vtkProcessor);
    if (!actor) return;

    registerObject({actor, vtkFile, true, 1.0});
    sceneRenderer_->addActorToRenderer(actor);
    sceneRenderer_->setupScalarBar(vtkProcessor);
    sceneRenderer_->renderObjects(objectList_);
}

void VisualizationManager::displayStepFile(const std::string& stepFile) {
    // Clear any existing STEP file actors first to prevent ghost objects
    clearStepFileActors();

    // Create and hold StepReader for boundary condition visualization
    currentStepReader_ = std::make_shared<StepReader>();
    if (!currentStepReader_->readStepFile(stepFile)) {
        std::cerr << "Failed to load STEP file: " << stepFile << std::endl;
        currentStepReader_.reset();
        return;
    }

    auto stepActors = actorFactory_->createStepActors(stepFile);

    if (stepActors.faceActors.empty()) {
        std::cerr << "Failed to create STEP actors: " << stepFile << std::endl;
        currentStepReader_.reset();
        return;
    }

    // Register each face actor
    for (size_t i = 0; i < stepActors.faceActors.size(); ++i) {
        std::string faceName = stepFile + "_face_" + std::to_string(i);
        registerObject({stepActors.faceActors[i], faceName, true, 1.0});
        sceneRenderer_->addActorToRenderer(stepActors.faceActors[i]);
    }

    // Register edges actor - DISABLED in favor of individual edge actors for picking
    /*
    if (stepActors.edgesActor) {
        registerObject({stepActors.edgesActor, stepFile + "_edges", true, 1.0});
        sceneRenderer_->addActorToRenderer(stepActors.edgesActor);
    }
    */

    // Setup face picker for hover detection
    // Edge actors are retrieved from currentStepReader_ as we need individual actors for picking
    std::vector<vtkSmartPointer<vtkActor>> edgeActors = currentStepReader_->getEdgeActors();
    
    // Register edge actors for rendering and raycasting
    for (size_t i = 0; i < edgeActors.size(); ++i) {
        // Skip null actors (degenerated edges)
        if (!edgeActors[i]) continue;
        
        std::string edgeName = stepFile + "_edge_" + std::to_string(i);
        registerObject({edgeActors[i], edgeName, true, 1.0});
        sceneRenderer_->addActorToRenderer(edgeActors[i]);
    }

    sceneRenderer_->setupStepPicker(stepActors.faceActors, edgeActors);
    
    // Remove the single combined edge actor if it was added (StepReader::getEdgesActor)
    // Actually, StepReader::getEdgesActor creates a single actor for all edges. 
    // If we use individual actors, we might want to suppress the single actor 
    // OR keep it but disable picking on it. 
    // StepActors struct from ActorFactory likely uses getEdgesActor. 
    // To avoid duplication, we should disable the single edge actor if we are using individual ones.
    // However, for performance, a single actor is better for static view. 
    // Let's keep individual actors for picking but maybe only enable them for picking?
    // Actually if we add them to renderer they will be visible. 
    // The user wants edge highlighting, so individual actors are needed.
    // The single actor "stepActors.edgesActor" is added in lines 80-84. 
    // We should probably REMOVE that part or make it invisible if we add individual edges.
    // Or, keep the single actor for non-hovered state and use individual actors only for hover?
    // No, individual actors are needed for picking. So we simply render individual actors instead of the single one.
    
    // Previously added single edge actor:
    // if (stepActors.edgesActor) { ... }
    // We should remove that block or comment it out if we use individual actors.
    // But modifying that block is separate edit. 
    // For now, let's just ADD the individual actors. 
    // Wait, having both will cause Z-fighting or double rendering.
    // I should remove the single edge actor registration from lines 80-84.

    sceneRenderer_->renderObjects(objectList_);
    sceneRenderer_->resetCamera();
}

void VisualizationManager::showTempDividedStl(VtkProcessor* vtkProcessor, QWidget* parent, UIState* uiState) {
    try {
        if (!vtkProcessor) {
            throw std::runtime_error("VtkProcessor is null");
        }

        // Fetch divided STL files from temp/div directory
        std::filesystem::path tempPath = TempPathUtility::getTempSubDirPath("div");

        std::vector<std::pair<std::filesystem::path, int>> stlFiles;
        std::regex pattern(R"(^modifierMesh(\d+)\.stl$)");

        if (std::filesystem::exists(tempPath) && std::filesystem::is_directory(tempPath)) {
            for (const auto& entry : std::filesystem::directory_iterator(tempPath)) {
                if (entry.is_regular_file() && entry.path().extension() == ".stl") {
                    std::string filename = entry.path().filename().string();
                    std::smatch match;

                    if (std::regex_search(filename, match, pattern)) {
                        int meshNumber = std::stoi(match[1].str());
                        stlFiles.push_back({entry.path(), meshNumber});
                    }
                }
            }
        }

        // Sort by mesh number
        std::sort(stlFiles.begin(), stlFiles.end(),
                  [](const auto& a, const auto& b) { return a.second < b.second; });

        if (stlFiles.empty()) {
            throw std::runtime_error("No valid STL files found in temp directory");
        }

        // Get stress range and mesh infos
        double minStress = vtkProcessor->getMinStress();
        double maxStress = vtkProcessor->getMaxStress();
        const auto& meshInfos = vtkProcessor->getMeshInfos();

        // Create divided mesh actors
        auto actors = actorFactory_->createDividedMeshActors(
            stlFiles, vtkProcessor, minStress, maxStress, meshInfos, uiState);

        // Get widgets for divided mesh display options
        auto widgets = sceneRenderer_->fetchMeshDisplayWidgets();

        // Register actors and setup widgets
        int widgetIndex = 0;
        for (size_t i = 0; i < stlFiles.size() && i < actors.size(); ++i) {
            const auto& [path, number] = stlFiles[i];
            std::string filename = path.filename().string();

            registerObject({actors[i], path.string(), true, 1.0});
            sceneRenderer_->addActorToRenderer(actors[i]);
            sceneRenderer_->updateWidgetAndConnectSignals(widgets, widgetIndex, filename, path.string());
        }

        sceneRenderer_->renderObjects(objectList_);
    }
    catch (const std::exception& e) {
        sceneRenderer_->handleStlFileLoadError(e, parent);
    }
}

// --- Object Control ---

void VisualizationManager::setObjectVisible(const std::string& filename, bool visible) {
    auto* obj = findObject(filename);
    if (!obj) return;

    obj->visible = visible;
    obj->actor->SetVisibility(visible ? 1 : 0);
    sceneRenderer_->renderObjects(objectList_);
}

void VisualizationManager::setObjectOpacity(const std::string& filename, double opacity) {
    auto* obj = findObject(filename);
    if (!obj) return;

    obj->opacity = opacity;
    obj->actor->GetProperty()->SetOpacity(opacity);
    sceneRenderer_->renderObjects(objectList_);
}

void VisualizationManager::setStepFileVisible(const std::string& stepFile, bool visible) {
    // Find all actors related to this STEP file (faces and edges)
    for (auto& obj : objectList_) {
        // Match actors whose filename contains stepFile and has _face_ or _edges suffix
        if (obj.filename.find(stepFile) != std::string::npos &&
            (obj.filename.find("_face_") != std::string::npos ||
             obj.filename.find("_edges") != std::string::npos)) {
            obj.visible = visible;
            obj.actor->SetVisibility(visible ? 1 : 0);
        }
    }
    sceneRenderer_->renderObjects(objectList_);
}

void VisualizationManager::setStepFileOpacity(const std::string& stepFile, double opacity) {
    // Find all actors related to this STEP file (faces and edges)
    for (auto& obj : objectList_) {
        // Match actors whose filename contains stepFile and has _face_ or _edges suffix
        if (obj.filename.find(stepFile) != std::string::npos &&
            (obj.filename.find("_face_") != std::string::npos ||
             obj.filename.find("_edges") != std::string::npos)) {
            obj.opacity = opacity;
            obj.actor->GetProperty()->SetOpacity(opacity);
        }
    }
    sceneRenderer_->renderObjects(objectList_);
}

void VisualizationManager::removeDividedStlActors() {
    std::regex pattern(R"(modifierMesh\d+\.stl$)");

    auto it = std::remove_if(objectList_.begin(), objectList_.end(),
                             [&pattern](const ObjectInfo& obj) {
                                 return std::regex_search(obj.filename, pattern);
                             });

    objectList_.erase(it, objectList_.end());
    sceneRenderer_->renderObjects(objectList_);
}

void VisualizationManager::hideAllStlObjects() {
    for (auto& obj : objectList_) {
        // Check if filename ends with ".stl"
        if (obj.filename.length() >= 4 &&
            obj.filename.substr(obj.filename.length() - 4) == ".stl") {
            obj.visible = false;
            obj.actor->SetVisibility(0);
        }
    }
    sceneRenderer_->renderObjects(objectList_);
}

void VisualizationManager::hideVtkObject() {
    for (auto& obj : objectList_) {
        // Check if filename ends with ".vtu" or ".vtk"
        if (obj.filename.length() >= 4) {
            std::string extension = obj.filename.substr(obj.filename.length() - 4);
            if (extension == ".vtu" || extension == ".vtk") {
                obj.visible = false;
                obj.actor->SetVisibility(0);
            }
        }
    }
    sceneRenderer_->renderObjects(objectList_);
}

// --- Query Operations ---

std::vector<std::string> VisualizationManager::getAllStlFilenames() const {
    std::vector<std::string> stlFilenames;

    for (const auto& obj : objectList_) {
        if (obj.filename.length() >= 4 &&
            obj.filename.substr(obj.filename.length() - 4) == ".stl") {
            stlFilenames.push_back(obj.filename);
        }
    }

    return stlFilenames;
}

std::string VisualizationManager::getVtkFilename() const {
    for (const auto& obj : objectList_) {
        if (obj.filename.length() >= 4) {
            std::string extension = obj.filename.substr(obj.filename.length() - 4);
            if (extension == ".vtu" || extension == ".vtk") {
                return obj.filename;
            }
        }
    }
    return "";
}

const std::vector<ObjectInfo>& VisualizationManager::getObjectList() const {
    return objectList_;
}

// --- Rendering Control ---

void VisualizationManager::render() {
    sceneRenderer_->render();
}

void VisualizationManager::resetCamera() {
    sceneRenderer_->resetCamera();
}

// --- Private Helper Methods ---

void VisualizationManager::registerObject(const ObjectInfo& objInfo) {
    objectList_.push_back(objInfo);
}

ObjectInfo* VisualizationManager::findObject(const std::string& filename) {
    for (auto& obj : objectList_) {
        // Exact match
        if (obj.filename == filename) {
            return &obj;
        }

        // Fuzzy match (for legacy compatibility)
        if (obj.filename.find(filename) != std::string::npos ||
            filename.find(obj.filename) != std::string::npos) {
            return &obj;
        }
    }
    return nullptr;
}

void VisualizationManager::updateRenderingState() {
    sceneRenderer_->renderObjects(objectList_);
}

void VisualizationManager::connectSignals() {
    connect(sceneRenderer_.get(), &SceneRenderer::objectVisibilityChanged,
            [this](const std::string& filename, bool visible) {
                setObjectVisible(filename, visible);
            });

    connect(sceneRenderer_.get(), &SceneRenderer::objectOpacityChanged,
            [this](const std::string& filename, double opacity) {
                setObjectOpacity(filename, opacity);
            });

    connect(sceneRenderer_.get(), &SceneRenderer::faceClicked,
            [this](int faceId, double nx, double ny, double nz) {
                emit faceClicked(faceId, nx, ny, nz);
            });

    connect(sceneRenderer_.get(), &SceneRenderer::faceDoubleClicked,
            [this](int faceId, double nx, double ny, double nz) {
                emit faceDoubleClicked(faceId, nx, ny, nz);
            });

    connect(sceneRenderer_.get(), &SceneRenderer::edgeClicked,
            [this](int edgeId) {
                emit edgeClicked(edgeId);
            });

    connect(sceneRenderer_.get(), &SceneRenderer::backgroundClicked,
            [this]() {
                emit backgroundClicked();
            });
}

void VisualizationManager::setEdgeSelectionMode(bool enabled) {
    if (sceneRenderer_) {
        sceneRenderer_->setEdgeSelectionMode(enabled);
    }
}

void VisualizationManager::setFaceSelectionMode(bool enabled) {
    if (sceneRenderer_) {
        sceneRenderer_->setFaceSelectionMode(enabled);
    }
}

// --- Boundary Condition Display ---

void VisualizationManager::displayBoundaryConditions(const BoundaryCondition& condition) {
    // Clear existing boundary condition actors
    clearBoundaryConditions();

    // Check if STEP file is loaded
    if (!currentStepReader_ || !currentStepReader_->isValid()) {
        std::cerr << "STEP file not loaded - cannot display boundary conditions" << std::endl;
        return;
    }

    // If no boundary conditions, just return
    if (condition.constraints.empty() && condition.loads.empty()) {
        return;
    }

    // Create boundary condition actors
    // Create boundary condition actors
    auto bcActors = actorFactory_->createBoundaryConditionActors(condition, currentStepReader_.get());

    // Add constraint actors
    int constraintIndex = 0;
    for (auto& actor : bcActors.constraintActors) {
        std::string actorName = "__bc_constraint_" + std::to_string(constraintIndex++);
        registerObject({actor, actorName, true, 0.8});
        sceneRenderer_->addActorToRenderer(actor);
        boundaryConditionActors_.push_back(actor);
    }

    // Add load actors
    int loadIndex = 0;
    for (auto& actor : bcActors.loadActors) {
        std::string actorName = "__bc_load_" + std::to_string(loadIndex++);
        registerObject({actor, actorName, true, 0.8});
        sceneRenderer_->addActorToRenderer(actor);
        boundaryConditionActors_.push_back(actor);
    }

    // Render
    render();
}

void VisualizationManager::clearBoundaryConditions() {
    // Remove all boundary condition actors from objectList_
    objectList_.erase(
        std::remove_if(objectList_.begin(), objectList_.end(),
                       [](const ObjectInfo& obj) {
                           return obj.filename.find("__bc_") == 0;
                       }),
        objectList_.end());

    // Remove all boundary condition actors from renderer
    for (auto& actor : boundaryConditionActors_) {
        sceneRenderer_->removeActorFromRenderer(actor);
    }

    // Clear the list
    boundaryConditionActors_.clear();

    // Render
    render();
}

void VisualizationManager::clearStepFileActors() {
    // Remove all STEP-related actors (faces, edges)
    auto it = std::remove_if(objectList_.begin(), objectList_.end(),
                             [](const ObjectInfo& obj) {
                                 return obj.filename.find("_face_") != std::string::npos ||
                                        obj.filename.find("_edge_") != std::string::npos;
                             });
    objectList_.erase(it, objectList_.end());

    // Reset StepReader
    currentStepReader_.reset();

    sceneRenderer_->renderObjects(objectList_);
    qDebug() << "VisualizationManager: STEP file actors cleared";
}

void VisualizationManager::clearSimulationActors() {
    // Remove VTK actors (.vtu, .vtk files)
    auto it = std::remove_if(objectList_.begin(), objectList_.end(),
                             [](const ObjectInfo& obj) {
                                 std::string ext;
                                 if (obj.filename.length() >= 4) {
                                     ext = obj.filename.substr(obj.filename.length() - 4);
                                 }
                                 return ext == ".vtu" || ext == ".vtk";
                             });
    objectList_.erase(it, objectList_.end());

    sceneRenderer_->renderObjects(objectList_);
    qDebug() << "VisualizationManager: Simulation actors cleared";
}

void VisualizationManager::clearInfillActors() {
    // Alias for existing method
    removeDividedStlActors();
    qDebug() << "VisualizationManager: Infill actors cleared";
}

void VisualizationManager::resetStepReader() {
    currentStepReader_.reset();
    qDebug() << "VisualizationManager: StepReader reset";
}

void VisualizationManager::showConstraintPreview(int surfaceId) {
    // Clear any existing preview
    clearPreview();

    if (!currentStepReader_ || !currentStepReader_->isValid()) {
        return;
    }

    FaceGeometry geom = currentStepReader_->getFaceGeometry(surfaceId);
    if (!geom.isValid) {
        return;
    }

    // Create constraint preview actor
    // Create constraint preview actor directly
    // Get geometry info
    double centerX = geom.centerX;
    double centerY = geom.centerY;
    double centerZ = geom.centerZ;
    
    // Create actor via factory
    previewActor_ = actorFactory_->createConstraintActor(centerX, centerY, centerZ);
    
    if (previewActor_) {
        registerObject({previewActor_, "__preview_bc__", true, 0.8});
        sceneRenderer_->addActorToRenderer(previewActor_);
        render();
    }
}

void VisualizationManager::showLoadPreview(int surfaceId, double dirX, double dirY, double dirZ) {
    // Clear any existing preview
    clearPreview();

    if (!currentStepReader_ || !currentStepReader_->isValid()) {
        return;
    }

    FaceGeometry geom = currentStepReader_->getFaceGeometry(surfaceId);
    if (!geom.isValid) {
        return;
    }

    // Create load preview actor
    // Create load preview actor directly
    previewActor_ = actorFactory_->createLoadArrowActor(
        geom.centerX, geom.centerY, geom.centerZ,
        dirX, dirY, dirZ,
        geom.normalX, geom.normalY, geom.normalZ
    );
    
    if (previewActor_) {
        registerObject({previewActor_, "__preview_bc__", true, 0.8});
        sceneRenderer_->addActorToRenderer(previewActor_);
        render();
    }
}

void VisualizationManager::showBedPreview(int surfaceId) {
    // Clear any existing preview
    clearPreview();

    if (!currentStepReader_ || !currentStepReader_->isValid()) {
        return;
    }

    FaceGeometry geom = currentStepReader_->getFaceGeometry(surfaceId);
    if (!geom.isValid) {
        return;
    }

    // Use ActorFactory to create bed preview actor
    previewActor_ = actorFactory_->createBedPreviewActor(geom);
    
    if (previewActor_) {
        registerObject({previewActor_, "__preview_bc__", true, 0.5});
        sceneRenderer_->addActorToRenderer(previewActor_);
        render();
    }
    render();
}

void VisualizationManager::clearPreview() {
    if (previewActor_) {
        // Remove from objectList_
        objectList_.erase(
            std::remove_if(objectList_.begin(), objectList_.end(),
                           [](const ObjectInfo& obj) {
                               return obj.filename == "__preview_bc__";
                           }),
            objectList_.end());

        // Remove from renderer
        sceneRenderer_->removeActorFromRenderer(previewActor_);
        previewActor_ = nullptr;
        render();
    }
}
