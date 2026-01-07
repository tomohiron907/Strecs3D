#pragma once

#include <QObject>
#include <QWidget>
#include <vector>
#include <string>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkScalarBarActor.h>

struct ObjectInfo;
class MainWindowUI;
class VtkProcessor;
class ObjectDisplayOptionsWidget;
class TurntableInteractorStyle;
class StepFacePickerStyle;

class SceneRenderer : public QObject {
    Q_OBJECT

public:
    explicit SceneRenderer(MainWindowUI* ui);
    ~SceneRenderer();

    // --- Rendering Operations ---
    void renderObjects(const std::vector<ObjectInfo>& objectList);
    void render();
    void clearRenderer();

    // --- Actor Management (add/remove from renderer, NOT creation) ---
    void addActorToRenderer(vtkSmartPointer<vtkActor> actor);
    void removeActorFromRenderer(vtkSmartPointer<vtkActor> actor);

    // --- Camera Control ---
    void resetCamera();
    void setCameraPosition(double x, double y, double z);
    void setCameraFocalPoint(double x, double y, double z);

    // --- Interactor Styles ---
    void enableTurntableMode(bool enable = true);
    void setTurntableRotationSpeed(double speed);
    void setupStepFacePicker(const std::vector<vtkSmartPointer<vtkActor>>& faceActors);

    // --- Scalar Bar ---
    void setupScalarBar(VtkProcessor* vtkProcessor);
    void removeScalarBar();

    // --- Widget Management ---
    std::vector<ObjectDisplayOptionsWidget*> fetchMeshDisplayWidgets();
    void updateWidgetAndConnectSignals(
        const std::vector<ObjectDisplayOptionsWidget*>& widgets,
        int& widgetIndex,
        const std::string& filename,
        const std::string& filePath);

    // --- Error Handling ---
    void handleStlFileLoadError(const std::exception& e, QWidget* parent);

signals:
    void objectVisibilityChanged(const std::string& filename, bool visible);
    void objectOpacityChanged(const std::string& filename, double opacity);
    void faceClicked(int faceId);

private:
    MainWindowUI* ui_;

    // Scalar bar for VTK visualization
    vtkSmartPointer<vtkScalarBarActor> scalarBarActor_;

    // Interactor styles (kept for lifecycle management)
    vtkSmartPointer<TurntableInteractorStyle> turntableStyle_;
    vtkSmartPointer<StepFacePickerStyle> stepFacePickerStyle_;

    // Helper methods
    void connectWidgetSignals(ObjectDisplayOptionsWidget* widget, const std::string& filePath);
};
