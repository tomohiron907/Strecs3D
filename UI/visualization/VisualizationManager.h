#pragma once

#include <QObject>
#include <memory>
#include <string>
#include <vector>
#include <vtkSmartPointer.h>
#include <vtkActor.h>

struct ObjectInfo {
    vtkSmartPointer<vtkActor> actor;
    std::string filename;
    bool visible;
    double opacity;
};

class ActorFactory;
class SceneRenderer;
class VtkProcessor;
class UIState;
class MainWindowUI;
class QWidget;
class BoundaryConditionVisualizer;
class StepReader;
struct BoundaryCondition;

class VisualizationManager : public QObject {
    Q_OBJECT

public:
    explicit VisualizationManager(MainWindowUI* ui);
    ~VisualizationManager();

    // --- Public Interface for ApplicationController ---

    // File Display Operations
    void displayVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor);
    void displayStepFile(const std::string& stepFile);
    void showTempDividedStl(VtkProcessor* vtkProcessor, QWidget* parent = nullptr, UIState* uiState = nullptr);

    // Object Control
    void setObjectVisible(const std::string& filename, bool visible);
    void setObjectOpacity(const std::string& filename, double opacity);
    void setStepFileVisible(const std::string& stepFile, bool visible);
    void setStepFileOpacity(const std::string& stepFile, double opacity);
    void removeDividedStlActors();
    void hideAllStlObjects();
    void hideVtkObject();

    // Query Operations
    std::vector<std::string> getAllStlFilenames() const;
    std::string getVtkFilename() const;
    const std::vector<ObjectInfo>& getObjectList() const;

    // Rendering Control
    void render();
    void resetCamera();

    // Boundary Condition Display
    void displayBoundaryConditions(const BoundaryCondition& condition);
    void clearBoundaryConditions();

    // Process Rollback Support
    void clearStepFileActors();      // Remove all STEP-related actors (faces + edges)
    void clearSimulationActors();    // Remove VTK result actors
    void clearInfillActors();        // Remove infill mesh actors (alias for removeDividedStlActors)
    void resetStepReader();          // Reset StepReader

    // Edge Selection Mode
    void setEdgeSelectionMode(bool enabled);

    // Face Selection Mode
    void setFaceSelectionMode(bool enabled);

    // StepReader Access (for edge geometry calculation)
    std::shared_ptr<StepReader> getCurrentStepReader() const { return currentStepReader_; }

signals:
    void faceClicked(int faceId, double nx, double ny, double nz);
    void faceDoubleClicked(int faceId, double nx, double ny, double nz);
    void edgeClicked(int edgeId);

private:
    // Components
    std::unique_ptr<ActorFactory> actorFactory_;
    std::unique_ptr<SceneRenderer> sceneRenderer_;
    std::unique_ptr<BoundaryConditionVisualizer> bcVisualizer_;

    // Data - ObjectInfo list is the single source of truth
    std::vector<ObjectInfo> objectList_;

    // StepReader reference for boundary condition visualization
    std::shared_ptr<StepReader> currentStepReader_;

    // Boundary condition actors
    std::vector<vtkSmartPointer<vtkActor>> boundaryConditionActors_;

    // Helper methods
    void registerObject(const ObjectInfo& objInfo);
    ObjectInfo* findObject(const std::string& filename);
    void updateRenderingState();

    // Connection to SceneRenderer signals
    void connectSignals();
};
