Visualization Layer Refactoring Plan

 Overview

 Refactor the visualization layer into three new classes with clear separation of concerns:
 - VisualizationManager: Manages ALL 3D objects using ObjectInfo, provides public API for
 ApplicationController
 - ActorFactory: Centralized actor creation/update for ALL actor types
 - SceneRenderer: Rendering only, no actor creation

 Existing classes will be renamed with "Legacy" prefix and new classes will be created from scratch.

 ---
 Architecture

 ApplicationController/IUserInterface
           ↓
   VisualizationManager (facade + ObjectInfo management)
           ↓
     ActorFactory (centralized actor creation)
           ↓
     SceneRenderer (rendering only)

 ObjectInfo Structure (unchanged)

 struct ObjectInfo {
     vtkSmartPointer<vtkActor> actor;
     std::string filename;
     bool visible;
     double opacity;
 };

 ---
 Class Designs

 1. VisualizationManager

 Location: UI/visualization/VisualizationManager.h/cpp

 Responsibilities:
 - Manage ObjectInfo list (single source of truth for all 3D objects)
 - Coordinate ActorFactory and SceneRenderer
 - Provide public API for ApplicationController
 - Handle object lifecycle (add, remove, visibility, opacity)

 Key Public Methods:
 // File Display
 void displayVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor);
 void displayStepFile(const std::string& stepFile);
 void showTempDividedStl(VtkProcessor* vtkProcessor, QWidget* parent, UIState* uiState);

 // Object Control
 void setObjectVisible(const std::string& filename, bool visible);
 void setObjectOpacity(const std::string& filename, double opacity);
 void removeDividedStlActors();
 void hideAllStlObjects();
 void hideVtkObject();

 // Query
 std::vector<std::string> getAllStlFilenames() const;
 std::string getVtkFilename() const;
 const std::vector<ObjectInfo>& getObjectList() const;

 // Rendering
 void render();
 void resetCamera();

 Key Private Members:
 std::unique_ptr<ActorFactory> actorFactory_;
 std::unique_ptr<SceneRenderer> sceneRenderer_;
 std::vector<ObjectInfo> objectList_;  // Single source of truth

 Special Objects: Grid, axes, and origin are managed as regular ObjectInfo items with special filenames:
 - "__grid__"
 - "__x_axis__", "__y_axis__", "__z_axis__"
 - "__origin__"

 ---
 2. ActorFactory

 Location: UI/visualization/ActorFactory.h/cpp

 Responsibilities:
 - Create ALL actor types (VTK, STL, STEP, Grid, Axes, Origin)
 - Update existing actors (color, opacity, visibility)
 - Delegate to VtkProcessor/StepReader when appropriate
 - Centralize color calculation and stress mapping

 Key Public Methods:
 // VTK Files
 vtkSmartPointer<vtkActor> createVtkActor(const std::string& vtkFile, VtkProcessor*);

 // STL Files (for divided meshes only - stress-based coloring)
 vtkSmartPointer<vtkActor> createStlActorWithStress(const std::string& stlFile, double stress, double
 minStress, double maxStress, VtkProcessor*);

 // STEP Files
 struct StepActors {
     std::vector<vtkSmartPointer<vtkActor>> faceActors;
     vtkSmartPointer<vtkActor> edgesActor;
 };
 StepActors createStepActors(const std::string& stepFile);

 // Divided Meshes
 std::vector<vtkSmartPointer<vtkActor>> createDividedMeshActors(
     const std::vector<std::pair<std::filesystem::path, int>>& stlFiles,
     VtkProcessor* vtkProcessor,
     double minStress, double maxStress,
     const std::vector<MeshInfo>& meshInfos,
     UIState* uiState);

 // Scene Elements
 vtkSmartPointer<vtkActor> createGridActor(int gridSize = 400, int gridInterval = 10, double opacity =
 0.3);

 struct AxisActors {
     vtkSmartPointer<vtkActor> xAxis;
     vtkSmartPointer<vtkActor> yAxis;
     vtkSmartPointer<vtkActor> zAxis;
     vtkSmartPointer<vtkActor> origin;
 };
 AxisActors createAxisActors(double axisLength = 50.0);

 // Updates
 void updateActorColor(vtkActor* actor, double r, double g, double b);
 void updateActorOpacity(vtkActor* actor, double opacity);
 void updateActorVisibility(vtkActor* actor, bool visible);

 Design Note: ActorFactory delegates to existing components:
 - VtkProcessor for VTK/STL pipeline operations
 - StepReader for STEP file parsing
 - ColorManager for color constants
 - UIState for density slider colors

 ---
 3. SceneRenderer

 Location: UI/visualization/SceneRenderer.h/cpp

 Responsibilities:
 - Rendering ONLY (no actor creation)
 - Manage vtkRenderer lifecycle
 - Camera control
 - Interactor style management (TurntableInteractorStyle, StepFacePickerStyle)
 - Scalar bar display
 - Widget signal connections

 Key Public Methods:
 // Rendering
 void renderObjects(const std::vector<ObjectInfo>& objectList);
 void render();
 void clearRenderer();

 // Actor Management (add/remove from renderer, NOT creation)
 void addActorToRenderer(vtkSmartPointer<vtkActor> actor);
 void removeActorFromRenderer(vtkSmartPointer<vtkActor> actor);

 // Camera
 void resetCamera();
 void setCameraPosition(double x, double y, double z);
 void setCameraFocalPoint(double x, double y, double z);

 // Interactor Styles
 void enableTurntableMode(bool enable = true);
 void setTurntableRotationSpeed(double speed);
 void setupStepFacePicker(const std::vector<vtkSmartPointer<vtkActor>>& faceActors);

 // Scalar Bar
 void setupScalarBar(VtkProcessor* vtkProcessor);
 void removeScalarBar();

 // Widget Management
 std::vector<ObjectDisplayOptionsWidget*> fetchMeshDisplayWidgets();
 void updateWidgetAndConnectSignals(const std::vector<ObjectDisplayOptionsWidget*>& widgets,
                                    int& widgetIndex, const std::string& filename,
                                    const std::string& filePath);

 Signals:
 signals:
     void objectVisibilityChanged(const std::string& filename, bool visible);
     void objectOpacityChanged(const std::string& filename, double opacity);

 Design Note: SceneRenderer emits signals that VisualizationManager connects to for widget interactions.

 ---
 Implementation Steps

 Step 1: Rename Legacy Classes

 Rename existing files with "Legacy" prefix:
 VisualizationManager.h/cpp       → LegacyVisualizationManager.h/cpp
 SceneDataController.h/cpp        → LegacySceneDataController.h/cpp
 SceneRenderer.h/cpp              → LegacySceneRenderer.h/cpp

 Update class names and include guards inside these files.

 Step 2: Create ActorFactory

 Create UI/visualization/ActorFactory.h and ActorFactory.cpp.

 Extract actor creation from:
 - LegacySceneDataController::createStlActorWithStress()
 - LegacySceneRenderer::createGrid()
 - LegacySceneRenderer::createAxes()

 Implement methods:
 1. VTK file actors: createVtkActor()
   - Delegate to VtkProcessor
 2. STEP actors: createStepActors()
   - Use StepReader internally
 3. Scene elements: createGridActor(), createAxisActors()
   - Use ColorManager for colors
 4. Divided meshes: createDividedMeshActors()
   - Complex logic with stress values
   - Uses createStlActorWithStress() internally for stress-based coloring
   - Note: Direct STL file display is not supported (only STEP), but divided meshes use temporary STL
 files

 Dependencies:
 - VtkProcessor (passed as pointer)
 - StepReader (created internally)
 - ColorManager (static access)
 - UIState (passed as pointer, nullable)

 Step 3: Create SceneRenderer

 Create UI/visualization/SceneRenderer.h and SceneRenderer.cpp.

 Keep from LegacySceneRenderer:
 - vtkRenderer management via MainWindowUI
 - renderObjects() - clear and re-add all actors
 - Interactor style management (TurntableInteractorStyle, StepFacePickerStyle)
 - Scalar bar handling
 - Widget signal connections
 - Camera controls

 Remove:
 - Grid/axes creation (now comes from VisualizationManager via ActorFactory)
 - Any actor creation logic

 Constructor:
 SceneRenderer::SceneRenderer(MainWindowUI* ui) : ui_(ui) {
     turntableStyle_ = vtkSmartPointer<TurntableInteractorStyle>::New();
     stepFacePickerStyle_ = vtkSmartPointer<StepFacePickerStyle>::New();
     enableTurntableMode(true);
 }

 Step 4: Create VisualizationManager

 Create UI/visualization/VisualizationManager.h and VisualizationManager.cpp.

 Constructor - Initialize scene elements:
 VisualizationManager::VisualizationManager(MainWindowUI* ui) {
     actorFactory_ = std::make_unique<ActorFactory>();
     sceneRenderer_ = std::make_unique<SceneRenderer>(ui);

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
 }

 Implement file display methods:
 Each follows the pattern:
 1. Create actor via ActorFactory
 2. Register in objectList_
 3. Add to SceneRenderer
 4. Trigger render

 Example:
 void VisualizationManager::displayVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor) {
     auto actor = actorFactory_->createVtkActor(vtkFile, vtkProcessor);
     if (!actor) return;

     registerObject({actor, vtkFile, true, 1.0});
     sceneRenderer_->addActorToRenderer(actor);
     sceneRenderer_->setupScalarBar(vtkProcessor);
     sceneRenderer_->renderObjects(objectList_);
 }

 Implement object control methods:
 void VisualizationManager::setObjectVisible(const std::string& filename, bool visible) {
     auto* obj = findObject(filename);
     if (!obj) return;

     obj->visible = visible;
     obj->actor->SetVisibility(visible);
     sceneRenderer_->renderObjects(objectList_);
 }

 Helper methods:
 void VisualizationManager::registerObject(const ObjectInfo& objInfo);
 ObjectInfo* VisualizationManager::findObject(const std::string& filename);  // Fuzzy matching
 void VisualizationManager::connectSignals();  // Connect SceneRenderer signals

 Step 5: Update MainWindowUIAdapter

 Update core/application/MainWindowUIAdapter.h/cpp:
 - Keep include of new VisualizationManager.h
 - No changes to method signatures (delegates to VisualizationManager)
 - Ensure IUserInterface contract is satisfied

 Step 6: Update CMakeLists.txt

 Add new files to the build:
 UI/visualization/ActorFactory.h
 UI/visualization/ActorFactory.cpp
 UI/visualization/SceneRenderer.h
 UI/visualization/SceneRenderer.cpp
 UI/visualization/VisualizationManager.h
 UI/visualization/VisualizationManager.cpp

 Keep legacy files temporarily (for reference during testing).

 Step 7: Integration Testing

 Test each file type and operation:
 - VTK file display
 - STEP file display (faces + edges)
 - Divided mesh display with stress coloring (temporary STL files)
 - Visibility controls
 - Opacity controls
 - Camera controls
 - Turntable mode
 - STEP face picker
 - Scalar bar display

 Step 8: Cleanup

 Once validated:
 - Remove LegacyVisualizationManager.h/cpp
 - Remove LegacySceneDataController.h/cpp
 - Remove LegacySceneRenderer.h/cpp
 - Update CMakeLists.txt to remove legacy files

 ---
 Key Implementation Details

 ObjectInfo Registration Pattern

 // 1. Create via ActorFactory
 auto actor = actorFactory_->createXXX(...);

 // 2. Register in objectList_
 ObjectInfo objInfo{actor, filename, true, 1.0};
 registerObject(objInfo);

 // 3. Add to renderer
 sceneRenderer_->addActorToRenderer(actor);

 // 4. Trigger render
 sceneRenderer_->renderObjects(objectList_);

 STEP File Handling (Multiple Actors)

 auto stepActors = actorFactory_->createStepActors(stepFile);

 // Register each face actor
 for (size_t i = 0; i < stepActors.faceActors.size(); ++i) {
     std::string faceName = stepFile + "_face_" + std::to_string(i);
     registerObject({stepActors.faceActors[i], faceName, true, 1.0});
 }

 // Register edges actor
 registerObject({stepActors.edgesActor, stepFile + "_edges", true, 1.0});

 // Setup face picker
 sceneRenderer_->setupStepFacePicker(stepActors.faceActors);

 Fuzzy Filename Matching (Legacy Compatibility)

 ObjectInfo* VisualizationManager::findObject(const std::string& filename) {
     for (auto& obj : objectList_) {
         if (obj.filename == filename) return &obj;
         if (obj.filename.find(filename) != std::string::npos ||
             filename.find(obj.filename) != std::string::npos) {
             return &obj;
         }
     }
     return nullptr;
 }

 SceneRenderer::renderObjects Pattern

 void SceneRenderer::renderObjects(const std::vector<ObjectInfo>& objectList) {
     clearRenderer();  // Remove all actors

     for (const auto& obj : objectList) {
         if (obj.visible && obj.actor) {
             obj.actor->SetVisibility(1);
             obj.actor->GetProperty()->SetOpacity(obj.opacity);
             ui_->getRenderer()->AddActor(obj.actor);
         }
     }

     render();
 }

 ---
 Critical Files

 Files to Create

 - UI/visualization/ActorFactory.h - Actor creation logic
 - UI/visualization/ActorFactory.cpp
 - UI/visualization/SceneRenderer.h - Rendering-only logic
 - UI/visualization/SceneRenderer.cpp
 - UI/visualization/VisualizationManager.h - Facade + ObjectInfo management
 - UI/visualization/VisualizationManager.cpp

 Files to Rename (temporarily)

 - UI/visualization/VisualizationManager.h → LegacyVisualizationManager.h
 - UI/visualization/VisualizationManager.cpp → LegacyVisualizationManager.cpp
 - UI/visualization/SceneDataController.h → LegacySceneDataController.h
 - UI/visualization/SceneDataController.cpp → LegacySceneDataController.cpp
 - UI/visualization/SceneRenderer.h → LegacySceneRenderer.h
 - UI/visualization/SceneRenderer.cpp → LegacySceneRenderer.cpp

 Files to Modify

 - core/application/MainWindowUIAdapter.h - Update include
 - core/application/MainWindowUIAdapter.cpp - Ensure delegates to new VisualizationManager
 - CMakeLists.txt - Add new files, remove legacy files after testing

 Files Referenced (unchanged)

 - UI/visualization/TurntableInteractorStyle.h/cpp - Used by SceneRenderer
 - UI/visualization/StepFacePickerStyle.h/cpp - Used by SceneRenderer
 - core/processing/VtkProcessor.h - Used by ActorFactory
 - core/processing/StepReader.h - Used by ActorFactory
 - UI/ColorManager.h - Used by ActorFactory
 - core/interfaces/IUserInterface.h - Contract that must be satisfied

 ---
 Potential Issues & Solutions

 Issue 1: Grid/Axes as ObjectInfo

 Problem: Grid/axes are not user-loaded files, but always present
 Solution: Use special filenames with "__" prefix (e.g., "grid", "x_axis")

 Issue 2: VtkProcessor Ownership

 Problem: VtkProcessor is owned by ProcessPipeline, passed as pointer
 Solution: Keep pointer-passing pattern, add null checks in ActorFactory

 Issue 3: Signal Connection

 Problem: Widget signals need to reach VisualizationManager from SceneRenderer
 Solution: SceneRenderer emits signals, VisualizationManager connects in constructor

 Issue 4: IUserInterface Compatibility

 Problem: Must maintain backward compatibility with ApplicationController
 Solution: MainWindowUIAdapter methods remain unchanged, just delegate to new VisualizationManager

 ---
 Success Criteria

 - All 3 new classes created from scratch
 - Legacy classes renamed with "Legacy" prefix
 - All file types display correctly (VTK, STEP, divided meshes with temporary STL files)
 - All object controls work (visibility, opacity, removal)
 - Interactor styles work (turntable, face picker)
 - No regressions in existing functionality
 - ApplicationController unchanged
 - Clean separation: VisualizationManager (data), ActorFactory (creation), SceneRenderer (rendering)