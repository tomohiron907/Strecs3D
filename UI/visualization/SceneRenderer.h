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

class SceneRenderer : public QObject {
    Q_OBJECT
    
public:
    SceneRenderer(MainWindowUI* ui);
    ~SceneRenderer();

    // レンダリング操作
    void renderObjects(const std::vector<ObjectInfo>& objectList);
    void addActorToRenderer(vtkSmartPointer<vtkActor> actor);
    void removeActorFromRenderer(vtkSmartPointer<vtkActor> actor);
    void clearRenderer();
    void render();
    
    // カメラ操作
    void resetCamera();
    
    // グリッド操作
    void showGrid(bool show = true);
    void hideGrid();
    
    // 座標軸操作
    void showAxes(bool show = true);
    void hideAxes();
    
    // スカラーバー
    void setupScalarBar(VtkProcessor* vtkProcessor);
    
    // UI Widget管理
    std::vector<ObjectDisplayOptionsWidget*> fetchMeshDisplayWidgets();
    void updateWidgetAndConnectSignals(
        const std::vector<ObjectDisplayOptionsWidget*>& widgets,
        int& widgetIndex,
        const std::string& filename,
        const std::string& filePath);
    
    // エラーハンドリング
    void handleStlFileLoadError(const std::exception& e, QWidget* parent);

signals:
    void objectVisibilityChanged(const std::string& filename, bool visible);
    void objectOpacityChanged(const std::string& filename, double opacity);

private:
    MainWindowUI* ui_;
    vtkSmartPointer<vtkActor> gridActor_;
    vtkSmartPointer<vtkActor> xAxisActor_;
    vtkSmartPointer<vtkActor> yAxisActor_;
    vtkSmartPointer<vtkActor> zAxisActor_;
    vtkSmartPointer<vtkActor> originActor_;
    
    void connectWidgetSignals(ObjectDisplayOptionsWidget* widget, const std::string& filePath);
    void createGrid();
    void createAxes();
};