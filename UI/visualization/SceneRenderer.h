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

class TurntableInteractorStyle;
class StepPickerStyle;

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
    void setupStepPicker(const std::vector<vtkSmartPointer<vtkActor>>& faceActors,
                         const std::vector<vtkSmartPointer<vtkActor>>& edgeActors);

    // --- Edge Selection Mode ---
    void setEdgeSelectionMode(bool enabled);
    void setFaceSelectionMode(bool enabled);

    // --- Scalar Bar ---
    void setupScalarBar(VtkProcessor* vtkProcessor);
    void removeScalarBar();

    // --- Widget Management ---


    // --- Error Handling ---
    void handleStlFileLoadError(const std::exception& e, QWidget* parent);

signals:
    void objectVisibilityChanged(const std::string& filename, bool visible);
    void objectOpacityChanged(const std::string& filename, double opacity);
    void faceClicked(int faceId, double nx, double ny, double nz);
    void faceDoubleClicked(int faceId, double nx, double ny, double nz);
    void edgeClicked(int edgeId);
    void backgroundClicked();

private:
    MainWindowUI* ui_;

    // Scalar bar for VTK visualization
    vtkSmartPointer<vtkScalarBarActor> scalarBarActor_;

    // Interactor styles (kept for lifecycle management)
    vtkSmartPointer<TurntableInteractorStyle> turntableStyle_;
    vtkSmartPointer<StepPickerStyle> stepPickerStyle_;

    // Helper methods

};
