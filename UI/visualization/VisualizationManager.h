#pragma once

#include <QObject>
#include <QWidget>
#include <string>
#include <memory>

class MainWindowUI;
class VtkProcessor;
class ObjectDisplayOptionsWidget;
class SceneDataController;
class SceneRenderer;
class UIState;

class VisualizationManager : public QObject {
    Q_OBJECT
public:
    VisualizationManager(MainWindowUI* ui);
    ~VisualizationManager();

    // ファイル表示
    void displayVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor);
    void displayStlFile(const std::string& stlFile, VtkProcessor* vtkProcessor);
    void showTempDividedStl(VtkProcessor* vtkProcessor, QWidget* parent = nullptr, UIState* uiState = nullptr);

    // オブジェクト制御
    void setObjectVisible(const std::string& filename, bool visible);
    void setObjectOpacity(const std::string& filename, double opacity);
    void removeDividedStlActors();
    void hideAllStlObjects();
    void hideVtkObject();
    
    // ファイル情報取得
    std::vector<std::string> getAllStlFilenames() const;
    std::string getVtkFilename() const;


private:
    std::unique_ptr<SceneDataController> dataController_;
    std::unique_ptr<SceneRenderer> renderer_;
    
    void initializeComponents(MainWindowUI* ui);
}; 