#pragma once

#include <QObject>
#include <QString>
#include <vector>
#include <string>

struct StressDensityMapping;
class VtkProcessor;

// MeshInfo の完全な定義が必要
#include "../processing/VtkProcessor.h"

class IUserInterface : public QObject {
    Q_OBJECT
public:
    explicit IUserInterface(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IUserInterface() = default;
    
    // ファイル名設定
    virtual void setVtkFileName(const QString& fileName) = 0;
    virtual void setStepFileName(const QString& fileName) = 0;

    // 表示状態制御
    virtual void setVtkVisibilityState(bool visible) = 0;
    virtual void setStepVisibilityState(bool visible) = 0;
    virtual void setVtkOpacity(double opacity) = 0;
    virtual void setStepOpacity(double opacity) = 0;
    
    // 分割メッシュ制御
    virtual void setDividedMeshVisibility(int meshIndex, bool visible) = 0;
    virtual void setDividedMeshOpacity(int meshIndex, double opacity) = 0;
    virtual void setDividedMeshFileName(int meshIndex, const QString& fileName) = 0;

    // ストレス範囲設定
    virtual void initializeStressConfiguration(double minStress, double maxStress) = 0;
    
    // メッセージ表示
    virtual void showWarningMessage(const QString& title, const QString& message) = 0;
    virtual void showCriticalMessage(const QString& title, const QString& message) = 0;
    virtual void showInfoMessage(const QString& title, const QString& message) = 0;
    
    // ファイル選択・保存ダイアログ
    virtual bool showFileValidationError() = 0;
    virtual bool showProcessingError(const QString& errorMessage) = 0;
    virtual void showProcessingSuccess() = 0;
    
    // 3D可視化制御
    virtual void displayVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor) = 0;
    virtual void displayStepFile(const std::string& stepFile) = 0;
    virtual void showTempDividedStl(VtkProcessor* vtkProcessor) = 0;
    virtual void setVisualizationObjectVisible(const std::string& filename, bool visible) = 0;
    virtual void setVisualizationObjectOpacity(const std::string& filename, double opacity) = 0;
    virtual void setStepFileVisible(const std::string& stepFile, bool visible) = 0;
    virtual void setStepFileOpacity(const std::string& stepFile, double opacity) = 0;
    virtual void removeDividedStlActors() = 0;
    virtual void hideAllStlObjects() = 0;
    virtual void hideVtkObject() = 0;
    virtual std::vector<std::string> getAllStlFilenames() const = 0;
    virtual std::string getVtkFilename() const = 0;

    // UIState管理（UI層の責務）
    virtual void registerDividedMeshes(const std::vector<MeshInfo>& meshInfos) = 0;

public slots:
    // ApplicationControllerからのシグナルを受信するスロット
    virtual void onVtkFileNameChanged(const QString& fileName) { setVtkFileName(fileName); }
    virtual void onStepFileNameChanged(const QString& fileName) { setStepFileName(fileName); }
    virtual void onDividedMeshFileNameChanged(int meshIndex, const QString& fileName) { setDividedMeshFileName(meshIndex, fileName); }
    virtual void onVtkVisibilityChanged(bool visible) { setVtkVisibilityState(visible); }
    virtual void onStepVisibilityChanged(bool visible) { setStepVisibilityState(visible); }
    virtual void onDividedMeshVisibilityChanged(int meshIndex, bool visible) { setDividedMeshVisibility(meshIndex, visible); }
    virtual void onVtkOpacityChanged(double opacity) { setVtkOpacity(opacity); }
    virtual void onStepOpacityChanged(double opacity) { setStepOpacity(opacity); }
    virtual void onDividedMeshOpacityChanged(int meshIndex, double opacity) { setDividedMeshOpacity(meshIndex, opacity); }
    virtual void onStressRangeChanged(double minStress, double maxStress) { initializeStressConfiguration(minStress, maxStress); }
    virtual void onShowWarningMessage(const QString& title, const QString& message) { showWarningMessage(title, message); }
    virtual void onShowCriticalMessage(const QString& title, const QString& message) { showCriticalMessage(title, message); }
    virtual void onShowInfoMessage(const QString& title, const QString& message) { showInfoMessage(title, message); }
};