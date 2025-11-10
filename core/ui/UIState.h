#pragma once

#include <QObject>
#include <QString>
#include <QColor>
#include <vector>
#include "../types/StressDensityMapping.h"
#include "../types/BoundaryCondition.h"

enum class ProcessingMode {
    BAMBU,
    CURA,
    PRUSA
};

struct DisplaySettings {
    bool isVisible = true;
    double opacity = 1.0;
};

class UIState : public QObject {
    Q_OBJECT

public:
    explicit UIState(QObject* parent = nullptr);
    ~UIState() = default;

    // ファイルパス
    void setStlFilePath(const QString& path);
    QString getStlFilePath() const { return m_stlFilePath; }

    void setVtkFilePath(const QString& path);
    QString getVtkFilePath() const { return m_vtkFilePath; }

    void setStepFilePath(const QString& path);
    QString getStepFilePath() const { return m_stepFilePath; }

    // Stress Range
    void setStressRange(double minStress, double maxStress);
    double getMinStress() const { return m_minStress; }
    double getMaxStress() const { return m_maxStress; }

    // Stress Density Mapping
    void setStressDensityMappings(const std::vector<StressDensityMapping>& mappings);
    std::vector<StressDensityMapping> getStressDensityMappings() const { return m_stressDensityMappings; }

    // Boundary Condition
    BoundaryCondition getBoundaryCondition() const { return m_boundaryCondition; }
    void addConstrainCondition(const ConstrainCondition& constrain);
    void addLoadCondition(const LoadCondition& load);

    // Density Slider colors
    void setDensitySliderColors(const std::vector<QColor>& colors);
    std::vector<QColor> getDensitySliderColors() const { return m_densitySliderColors; }

    // Processing Mode
    void setProcessingMode(ProcessingMode mode);
    ProcessingMode getProcessingMode() const { return m_processingMode; }

    // Display Settings for different objects
    void setMeshDisplaySettings(const DisplaySettings& settings);
    DisplaySettings getMeshDisplaySettings() const { return m_meshDisplaySettings; }

    void setVtuDisplaySettings(const DisplaySettings& settings);
    DisplaySettings getVtuDisplaySettings() const { return m_vtuDisplaySettings; }

    void setDividedMesh1Settings(const DisplaySettings& settings);
    DisplaySettings getDividedMesh1Settings() const { return m_dividedMesh1Settings; }

    void setDividedMesh2Settings(const DisplaySettings& settings);
    DisplaySettings getDividedMesh2Settings() const { return m_dividedMesh2Settings; }

    void setDividedMesh3Settings(const DisplaySettings& settings);
    DisplaySettings getDividedMesh3Settings() const { return m_dividedMesh3Settings; }

    void setDividedMesh4Settings(const DisplaySettings& settings);
    DisplaySettings getDividedMesh4Settings() const { return m_dividedMesh4Settings; }

    // デバッグ用メソッド
    void printDebugInfo() const;
    QString getDebugString() const;

signals:
    void stlFilePathChanged(const QString& path);
    void vtkFilePathChanged(const QString& path);
    void stepFilePathChanged(const QString& path);
    void stressRangeChanged(double minStress, double maxStress);
    void stressDensityMappingsChanged(const std::vector<StressDensityMapping>& mappings);
    void boundaryConditionChanged(const BoundaryCondition& condition);
    void densitySliderColorsChanged(const std::vector<QColor>& colors);
    void processingModeChanged(ProcessingMode mode);
    void meshDisplaySettingsChanged(const DisplaySettings& settings);
    void vtuDisplaySettingsChanged(const DisplaySettings& settings);
    void dividedMesh1SettingsChanged(const DisplaySettings& settings);
    void dividedMesh2SettingsChanged(const DisplaySettings& settings);
    void dividedMesh3SettingsChanged(const DisplaySettings& settings);
    void dividedMesh4SettingsChanged(const DisplaySettings& settings);

private:
    // File paths
    QString m_stlFilePath;
    QString m_vtkFilePath;
    QString m_stepFilePath;

    // Stress range
    double m_minStress = 0.0;
    double m_maxStress = 1.0;

    // Stress density mappings
    std::vector<StressDensityMapping> m_stressDensityMappings;

    // Boundary condition
    BoundaryCondition m_boundaryCondition;

    // Density slider colors
    std::vector<QColor> m_densitySliderColors;

    // Processing mode
    ProcessingMode m_processingMode = ProcessingMode::CURA;

    // Display settings
    DisplaySettings m_meshDisplaySettings;
    DisplaySettings m_vtuDisplaySettings;
    DisplaySettings m_dividedMesh1Settings;
    DisplaySettings m_dividedMesh2Settings;
    DisplaySettings m_dividedMesh3Settings;
    DisplaySettings m_dividedMesh4Settings;
};