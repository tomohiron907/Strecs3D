#pragma once

#include <QObject>
#include <QMessageBox>
#include <memory>
#include "../interfaces/IUserInterface.h"
#include "../../UI/mainwindowui.h"

class ObjectDisplayOptionsWidget;
class VisualizationManager;

class MainWindowUIAdapter : public IUserInterface {
    Q_OBJECT
public:
    explicit MainWindowUIAdapter(MainWindowUI* ui, QObject* parent = nullptr);
    ~MainWindowUIAdapter();
    
    // IUserInterface implementation
    void setVtkFileName(const QString& fileName) override;
    void setStlFileName(const QString& fileName) override;
    void setVtkVisibilityState(bool visible) override;
    void setStlVisibilityState(bool visible) override;
    void setVtkOpacity(double opacity) override;
    void setStlOpacity(double opacity) override;
    void setDividedMeshVisibility(int meshIndex, bool visible) override;
    void setDividedMeshOpacity(int meshIndex, double opacity) override;
    void setDividedMeshFileName(int meshIndex, const QString& fileName) override;
    std::vector<double> getStressThresholds() const override;
    std::vector<StressDensityMapping> getStressDensityMappings() const override;
    QString getCurrentMode() const override;
    void setStressRange(double minStress, double maxStress) override;
    
    // メッセージ表示
    void showWarningMessage(const QString& title, const QString& message) override;
    void showCriticalMessage(const QString& title, const QString& message) override;
    void showInfoMessage(const QString& title, const QString& message) override;
    
    // ファイル選択・保存ダイアログ
    bool showFileValidationError() override;
    bool showProcessingError(const QString& errorMessage) override;
    void showProcessingSuccess() override;
    
    // 3D可視化制御
    void displayVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor) override;
    void displayStlFile(const std::string& stlFile, VtkProcessor* vtkProcessor) override;
    void showTempDividedStl(VtkProcessor* vtkProcessor) override;
    void setVisualizationObjectVisible(const std::string& filename, bool visible) override;
    void setVisualizationObjectOpacity(const std::string& filename, double opacity) override;
    void removeDividedStlActors() override;
    void hideAllStlObjects() override;
    void hideVtkObject() override;
    std::vector<std::string> getAllStlFilenames() const override;
    std::string getVtkFilename() const override;
    
    // Adapter specific method
    MainWindowUI* getMainWindowUI() const { return ui; }

private:
    ObjectDisplayOptionsWidget* getDividedMeshWidget(int meshIndex) const;
    MainWindowUI* ui;
    std::unique_ptr<VisualizationManager> visualizationManager;
};