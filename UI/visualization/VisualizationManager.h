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

private:
    // Components
    std::unique_ptr<ActorFactory> actorFactory_;
    std::unique_ptr<SceneRenderer> sceneRenderer_;

    // Data - ObjectInfo list is the single source of truth
    std::vector<ObjectInfo> objectList_;

    // Helper methods
    void registerObject(const ObjectInfo& objInfo);
    ObjectInfo* findObject(const std::string& filename);
    void updateRenderingState();

    // Connection to SceneRenderer signals
    void connectSignals();
};
