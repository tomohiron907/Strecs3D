#pragma once

#include <QObject>
#include <QString>
#include <QColor>
#include <vector>
#include <map>
#include "../types/StressDensityMapping.h"
#include "../types/BoundaryCondition.h"
#include <vector>
#include <map>
#include "../types/StressDensityMapping.h"
#include "../types/BoundaryCondition.h"
#include "../types/ObjectType.h"

struct SelectedObjectInfo {
    ObjectType type = ObjectType::NONE;
    QString id = "";
    int index = -1;
};
enum class ProcessingMode {
    BAMBU,
    CURA,
    PRUSA
};

// オブジェクトファイル情報
struct ObjectFileInfo {
    QString filename;
    QString filePath;
    bool isVisible = true;
    double transparency = 1.0;
};

// インフィル領域情報
struct InfillRegionInfo {
    QString name;
    QString filename;
    QString filePath;
    bool isVisible = true;
    double transparency = 1.0;
};

// オブジェクトリストデータ（階層構造）
// オブジェクトリストデータ（階層構造）
struct ObjectListData {
    ObjectFileInfo step;
    BoundaryCondition boundaryCondition;
    ObjectFileInfo simulationResult;
    std::map<QString, InfillRegionInfo> infillRegions;
    std::vector<QString> displayOrder; // 表示順序を保持するリスト

    // 表示順序管理用のキー定数
    static inline const QString KEY_STEP = "step";
    static inline const QString KEY_SIMULATION = "simulation";
    static inline const QString KEY_INFILL = "infill";
    static inline const QString KEY_BC = "bc";
};

class UIState : public QObject {
    Q_OBJECT

public:
    explicit UIState(QObject* parent = nullptr);
    ~UIState() = default;

    // オブジェクトリスト全体の取得・設定
    ObjectListData getObjectList() const { return m_objectList; }
    void setObjectList(const ObjectListData& objectList);

    // STEP ファイル関連
    void setStepFileInfo(const ObjectFileInfo& info);
    ObjectFileInfo getStepFileInfo() const { return m_objectList.step; }
    void setStepFilePath(const QString& path);
    QString getStepFilePath() const { return m_objectList.step.filePath; }
    void setStepVisibility(bool visible);
    bool getStepVisibility() const { return m_objectList.step.isVisible; }
    void setStepTransparency(double transparency);
    double getStepTransparency() const { return m_objectList.step.transparency; }
    void clearStepFile();  // Clear STEP file data

    // Boundary Condition 関連
    BoundaryCondition getBoundaryCondition() const { return m_objectList.boundaryCondition; }
    void setBoundaryCondition(const BoundaryCondition& bc);
    void addConstraintCondition(const ConstraintCondition& constraint);
    void addLoadCondition(const LoadCondition& load);
    void clearConstraintConditions();
    void updateConstraintCondition(int index, const ConstraintCondition& constraint);
    void removeConstraintCondition(int index);
    void clearLoadConditions();
    void updateLoadCondition(int index, const LoadCondition& load);
    void removeLoadCondition(int index);

    // Simulation Result 関連
    void setSimulationResultFileInfo(const ObjectFileInfo& info);
    ObjectFileInfo getSimulationResultFileInfo() const { return m_objectList.simulationResult; }
    void setSimulationResultFilePath(const QString& path);
    QString getSimulationResultFilePath() const { return m_objectList.simulationResult.filePath; }
    void setSimulationResultVisibility(bool visible);
    bool getSimulationResultVisibility() const { return m_objectList.simulationResult.isVisible; }
    void setSimulationResultTransparency(double transparency);
    double getSimulationResultTransparency() const { return m_objectList.simulationResult.transparency; }
    void clearSimulationResult();  // Clear simulation result data

    // Infill Regions 関連
    void addInfillRegion(const QString& key, const InfillRegionInfo& info);
    void removeInfillRegion(const QString& key);
    InfillRegionInfo getInfillRegion(const QString& key) const;
    std::map<QString, InfillRegionInfo> getAllInfillRegions() const { return m_objectList.infillRegions; }
    void setInfillRegionVisibility(const QString& key, bool visible);
    void setInfillRegionTransparency(const QString& key, double transparency);
    void clearAllInfillRegions();  // Clear all infill regions

    // Stress Range（階層外のデータ）
    void setStressRange(double minStress, double maxStress);
    double getMinStress() const { return m_minStress; }
    double getMaxStress() const { return m_maxStress; }

    // Stress Density Mapping（階層外のデータ）
    void setStressDensityMappings(const std::vector<StressDensityMapping>& mappings);
    std::vector<StressDensityMapping> getStressDensityMappings() const { return m_stressDensityMappings; }

    // Density Slider colors（階層外のデータ）
    void setDensitySliderColors(const std::vector<QColor>& colors);
    std::vector<QColor> getDensitySliderColors() const { return m_densitySliderColors; }

    // Processing Mode（階層外のデータ）
    void setProcessingMode(ProcessingMode mode);
    ProcessingMode getProcessingMode() const { return m_processingMode; }

    // 選択状態管理
    void setSelectedObject(ObjectType type, const QString& id = "", int index = -1);
    SelectedObjectInfo getSelectedObject() const { return m_selectedObject; }

    // デバッグ用メソッド
    void printDebugInfo() const;
    QString getDebugString() const;

signals:
    // オブジェクトリスト全体の変更
    void objectListChanged(const ObjectListData& objectList);

    // STEP関連のシグナル
    void stepFileInfoChanged(const ObjectFileInfo& info);
    void stepFilePathChanged(const QString& path);
    void stepVisibilityChanged(bool visible);
    void stepTransparencyChanged(double transparency);

    // Boundary Condition関連のシグナル
    void boundaryConditionChanged(const BoundaryCondition& condition);

    // Simulation Result関連のシグナル
    void simulationResultFileInfoChanged(const ObjectFileInfo& info);
    void simulationResultFilePathChanged(const QString& path);
    void simulationResultVisibilityChanged(bool visible);
    void simulationResultTransparencyChanged(double transparency);

    // Infill Regions関連のシグナル
    void infillRegionAdded(const QString& key, const InfillRegionInfo& info);
    void infillRegionRemoved(const QString& key);
    void infillRegionVisibilityChanged(const QString& key, bool visible);
    void infillRegionTransparencyChanged(const QString& key, double transparency);

    // 階層外のデータのシグナル
    void stressRangeChanged(double minStress, double maxStress);
    void stressDensityMappingsChanged(const std::vector<StressDensityMapping>& mappings);
    void densitySliderColorsChanged(const std::vector<QColor>& colors);
    void processingModeChanged(ProcessingMode mode);
    
    // 選択状態変更シグナル
    void selectedObjectChanged(const SelectedObjectInfo& selection);

private:
    // 階層構造のメインデータ
    ObjectListData m_objectList;

    // 階層外のデータ
    double m_minStress = 0.0;
    double m_maxStress = 1.0;
    std::vector<StressDensityMapping> m_stressDensityMappings;
    std::vector<QColor> m_densitySliderColors;
    ProcessingMode m_processingMode = ProcessingMode::BAMBU;
    
    // 選択状態
    SelectedObjectInfo m_selectedObject;
};