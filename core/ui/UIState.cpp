#include "UIState.h"
#include <QDebug>
#include <QFileInfo>

UIState::UIState(QObject* parent)
    : QObject(parent)
{
    // Initialize default colors for density slider (4 sections)
    m_densitySliderColors = {
        QColor(255, 0, 0),    // Red
        QColor(255, 165, 0),  // Orange
        QColor(255, 255, 0),  // Yellow
        QColor(0, 255, 0)     // Green
    };

    // Initialize default stress density mappings
    m_stressDensityMappings = {
        {0.0, 0.25, 0.2},
        {0.25, 0.5, 0.4},
        {0.5, 0.75, 0.6},
        {0.75, 1.0, 0.8}
    };
}

// オブジェクトリスト全体の設定
void UIState::setObjectList(const ObjectListData& objectList)
{
    m_objectList = objectList;
    emit objectListChanged(objectList);
    qDebug() << "UIState: Object list updated";
}

// ========== STEP ファイル関連 ==========
void UIState::setStepFileInfo(const ObjectFileInfo& info)
{
    m_objectList.step = info;
    emit stepFileInfoChanged(info);
    qDebug() << "UIState: STEP file info changed:" << info.filePath;
}

void UIState::setStepFilePath(const QString& path)
{
    if (m_objectList.step.filePath != path) {
        m_objectList.step.filePath = path;
        QFileInfo fileInfo(path);
        m_objectList.step.filename = fileInfo.fileName();
        emit stepFilePathChanged(path);
        qDebug() << "UIState: STEP file path changed to:" << path;
    }
}

void UIState::setStepVisibility(bool visible)
{
    if (m_objectList.step.isVisible != visible) {
        m_objectList.step.isVisible = visible;
        emit stepVisibilityChanged(visible);
        qDebug() << "UIState: STEP visibility changed to:" << visible;
    }
}

void UIState::setStepTransparency(double transparency)
{
    if (m_objectList.step.transparency != transparency) {
        m_objectList.step.transparency = transparency;
        emit stepTransparencyChanged(transparency);
        qDebug() << "UIState: STEP transparency changed to:" << transparency;
    }
}

// ========== Boundary Condition 関連 ==========
void UIState::setBoundaryCondition(const BoundaryCondition& bc)
{
    m_objectList.boundaryCondition = bc;
    emit boundaryConditionChanged(bc);
    qDebug() << "UIState: Boundary condition set - Constraints:" << bc.constraints.size()
             << "Loads:" << bc.loads.size();
}

void UIState::addConstraintCondition(const ConstraintCondition& constraint)
{
    m_objectList.boundaryCondition.constraints.push_back(constraint);
    emit boundaryConditionChanged(m_objectList.boundaryCondition);
    qDebug() << "UIState: Constraint condition added - Surface ID:" << constraint.surface_id
             << "Name:" << QString::fromStdString(constraint.name)
             << "Total constraints:" << m_objectList.boundaryCondition.constraints.size();
}

void UIState::addLoadCondition(const LoadCondition& load)
{
    m_objectList.boundaryCondition.loads.push_back(load);
    emit boundaryConditionChanged(m_objectList.boundaryCondition);
    qDebug() << "UIState: Load condition added - Surface ID:" << load.surface_id
             << "Name:" << QString::fromStdString(load.name)
             << "Magnitude:" << load.magnitude
             << "Total loads:" << m_objectList.boundaryCondition.loads.size();
}

void UIState::clearConstraintConditions()
{
    m_objectList.boundaryCondition.constraints.clear();
    emit boundaryConditionChanged(m_objectList.boundaryCondition);
    qDebug() << "UIState: All constraint conditions cleared";
}

void UIState::clearLoadConditions()
{
    m_objectList.boundaryCondition.loads.clear();
    emit boundaryConditionChanged(m_objectList.boundaryCondition);
    qDebug() << "UIState: All load conditions cleared";
}

// ========== Simulation Result 関連 ==========
void UIState::setSimulationResultFileInfo(const ObjectFileInfo& info)
{
    m_objectList.simulationResult = info;
    emit simulationResultFileInfoChanged(info);
    qDebug() << "UIState: Simulation result file info changed:" << info.filePath;
}

void UIState::setSimulationResultFilePath(const QString& path)
{
    if (m_objectList.simulationResult.filePath != path) {
        m_objectList.simulationResult.filePath = path;
        QFileInfo fileInfo(path);
        m_objectList.simulationResult.filename = fileInfo.fileName();
        emit simulationResultFilePathChanged(path);
        qDebug() << "UIState: Simulation result file path changed to:" << path;
    }
}

void UIState::setSimulationResultVisibility(bool visible)
{
    if (m_objectList.simulationResult.isVisible != visible) {
        m_objectList.simulationResult.isVisible = visible;
        emit simulationResultVisibilityChanged(visible);
        qDebug() << "UIState: Simulation result visibility changed to:" << visible;
    }
}

void UIState::setSimulationResultTransparency(double transparency)
{
    if (m_objectList.simulationResult.transparency != transparency) {
        m_objectList.simulationResult.transparency = transparency;
        emit simulationResultTransparencyChanged(transparency);
        qDebug() << "UIState: Simulation result transparency changed to:" << transparency;
    }
}

// ========== Infill Regions 関連 ==========
void UIState::addInfillRegion(const QString& key, const InfillRegionInfo& info)
{
    m_objectList.infillRegions[key] = info;
    emit infillRegionAdded(key, info);
    qDebug() << "UIState: Infill region added - Key:" << key << "Name:" << info.name;
}

void UIState::removeInfillRegion(const QString& key)
{
    if (m_objectList.infillRegions.erase(key) > 0) {
        emit infillRegionRemoved(key);
        qDebug() << "UIState: Infill region removed - Key:" << key;
    }
}

InfillRegionInfo UIState::getInfillRegion(const QString& key) const
{
    auto it = m_objectList.infillRegions.find(key);
    if (it != m_objectList.infillRegions.end()) {
        return it->second;
    }
    qDebug() << "Warning: Infill region not found for key:" << key;
    return InfillRegionInfo();
}

void UIState::setInfillRegionVisibility(const QString& key, bool visible)
{
    auto it = m_objectList.infillRegions.find(key);
    if (it != m_objectList.infillRegions.end() && it->second.isVisible != visible) {
        it->second.isVisible = visible;
        emit infillRegionVisibilityChanged(key, visible);
        qDebug() << "UIState: Infill region visibility changed - Key:" << key << "Visible:" << visible;
    }
}

void UIState::setInfillRegionTransparency(const QString& key, double transparency)
{
    auto it = m_objectList.infillRegions.find(key);
    if (it != m_objectList.infillRegions.end() && it->second.transparency != transparency) {
        it->second.transparency = transparency;
        emit infillRegionTransparencyChanged(key, transparency);
        qDebug() << "UIState: Infill region transparency changed - Key:" << key << "Transparency:" << transparency;
    }
}

// ========== 階層外のデータ ==========
void UIState::setStressRange(double minStress, double maxStress)
{
    if (m_minStress != minStress || m_maxStress != maxStress) {
        m_minStress = minStress;
        m_maxStress = maxStress;
        emit stressRangeChanged(minStress, maxStress);
        qDebug() << "UIState: Stress range changed to:" << minStress << "-" << maxStress;
    }
}

void UIState::setStressDensityMappings(const std::vector<StressDensityMapping>& mappings)
{
    m_stressDensityMappings = mappings;
    emit stressDensityMappingsChanged(mappings);
    qDebug() << "UIState: Stress density mappings updated, count:" << mappings.size();
}

void UIState::setDensitySliderColors(const std::vector<QColor>& colors)
{
    m_densitySliderColors = colors;
    emit densitySliderColorsChanged(colors);
    qDebug() << "UIState: Density slider colors updated, count:" << colors.size();
}

void UIState::setProcessingMode(ProcessingMode mode)
{
    if (m_processingMode != mode) {
        m_processingMode = mode;
        emit processingModeChanged(mode);
        QString modeStr;
        switch(mode) {
            case ProcessingMode::BAMBU: modeStr = "BAMBU"; break;
            case ProcessingMode::CURA: modeStr = "CURA"; break;
            case ProcessingMode::PRUSA: modeStr = "PRUSA"; break;
        }
        qDebug() << "UIState: Processing mode changed to:" << modeStr;
    }
}

// ========== デバッグ用メソッド ==========
void UIState::printDebugInfo() const
{
    qDebug() << "=== UIState Debug Information ===";
    qDebug() << "STEP File Path:" << m_objectList.step.filePath;
    qDebug() << "STEP Filename:" << m_objectList.step.filename;
    qDebug() << "STEP Visible:" << m_objectList.step.isVisible;
    qDebug() << "STEP Transparency:" << m_objectList.step.transparency;

    qDebug() << "Simulation Result File Path:" << m_objectList.simulationResult.filePath;
    qDebug() << "Simulation Result Filename:" << m_objectList.simulationResult.filename;
    qDebug() << "Simulation Result Visible:" << m_objectList.simulationResult.isVisible;
    qDebug() << "Simulation Result Transparency:" << m_objectList.simulationResult.transparency;

    qDebug() << "Infill Regions (" << m_objectList.infillRegions.size() << "entries):";
    for (const auto& [key, info] : m_objectList.infillRegions) {
        qDebug() << "  Key:" << key << "Name:" << info.name << "Path:" << info.filePath
                 << "Visible:" << info.isVisible << "Transparency:" << info.transparency;
    }

    qDebug() << "Stress Range:" << m_minStress << "to" << m_maxStress;

    QString modeStr;
    switch(m_processingMode) {
        case ProcessingMode::BAMBU: modeStr = "BAMBU"; break;
        case ProcessingMode::CURA: modeStr = "CURA"; break;
        case ProcessingMode::PRUSA: modeStr = "PRUSA"; break;
    }
    qDebug() << "Processing Mode:" << modeStr;

    qDebug() << "Stress Density Mappings (" << m_stressDensityMappings.size() << "entries):";
    for (size_t i = 0; i < m_stressDensityMappings.size(); ++i) {
        const auto& mapping = m_stressDensityMappings[i];
        qDebug() << "  [" << i << "] Stress:" << mapping.stressMin << "-" << mapping.stressMax << "Density:" << mapping.density;
    }

    qDebug() << "Density Slider Colors (" << m_densitySliderColors.size() << "colors):";
    for (size_t i = 0; i < m_densitySliderColors.size(); ++i) {
        const auto& color = m_densitySliderColors[i];
        qDebug() << "  [" << i << "] Color:" << color.name();
    }

    qDebug() << "Boundary Conditions:";
    qDebug() << "  Constraints (" << m_objectList.boundaryCondition.constraints.size() << "entries):";
    for (size_t i = 0; i < m_objectList.boundaryCondition.constraints.size(); ++i) {
        const auto& constraint = m_objectList.boundaryCondition.constraints[i];
        qDebug() << "    [" << i << "] Surface ID:" << constraint.surface_id << "Name:" << QString::fromStdString(constraint.name);
    }
    qDebug() << "  Loads (" << m_objectList.boundaryCondition.loads.size() << "entries):";
    for (size_t i = 0; i < m_objectList.boundaryCondition.loads.size(); ++i) {
        const auto& load = m_objectList.boundaryCondition.loads[i];
        qDebug() << "    [" << i << "] Surface ID:" << load.surface_id << "Name:" << QString::fromStdString(load.name)
                 << "Magnitude:" << load.magnitude
                 << "Direction:(" << load.direction.x << "," << load.direction.y << "," << load.direction.z << ")";
    }

    qDebug() << "=================================";
}

QString UIState::getDebugString() const
{
    QString debug;
    debug += "=== UIState Debug Information ===\n";
    debug += QString("STEP File Path: %1\n").arg(m_objectList.step.filePath);
    debug += QString("STEP Filename: %1\n").arg(m_objectList.step.filename);
    debug += QString("STEP Visible: %1\n").arg(m_objectList.step.isVisible);
    debug += QString("STEP Transparency: %1\n").arg(m_objectList.step.transparency);

    debug += QString("Simulation Result File Path: %1\n").arg(m_objectList.simulationResult.filePath);
    debug += QString("Simulation Result Filename: %1\n").arg(m_objectList.simulationResult.filename);
    debug += QString("Simulation Result Visible: %1\n").arg(m_objectList.simulationResult.isVisible);
    debug += QString("Simulation Result Transparency: %1\n").arg(m_objectList.simulationResult.transparency);

    debug += QString("Infill Regions (%1 entries):\n").arg(m_objectList.infillRegions.size());
    for (const auto& [key, info] : m_objectList.infillRegions) {
        debug += QString("  Key: %1, Name: %2, Path: %3, Visible: %4, Transparency: %5\n")
                    .arg(key).arg(info.name).arg(info.filePath).arg(info.isVisible).arg(info.transparency);
    }

    debug += QString("Stress Range: %1 to %2\n").arg(m_minStress).arg(m_maxStress);

    QString modeStr;
    switch(m_processingMode) {
        case ProcessingMode::BAMBU: modeStr = "BAMBU"; break;
        case ProcessingMode::CURA: modeStr = "CURA"; break;
        case ProcessingMode::PRUSA: modeStr = "PRUSA"; break;
    }
    debug += QString("Processing Mode: %1\n").arg(modeStr);

    debug += QString("Stress Density Mappings (%1 entries):\n").arg(m_stressDensityMappings.size());
    for (size_t i = 0; i < m_stressDensityMappings.size(); ++i) {
        const auto& mapping = m_stressDensityMappings[i];
        debug += QString("  [%1] Stress: %2-%3, Density: %4\n")
                    .arg(i).arg(mapping.stressMin).arg(mapping.stressMax).arg(mapping.density);
    }

    debug += QString("Density Slider Colors (%1 colors):\n").arg(m_densitySliderColors.size());
    for (size_t i = 0; i < m_densitySliderColors.size(); ++i) {
        const auto& color = m_densitySliderColors[i];
        debug += QString("  [%1] Color: %2\n").arg(i).arg(color.name());
    }

    debug += "Boundary Conditions:\n";
    debug += QString("  Constraints (%1 entries):\n").arg(m_objectList.boundaryCondition.constraints.size());
    for (size_t i = 0; i < m_objectList.boundaryCondition.constraints.size(); ++i) {
        const auto& constraint = m_objectList.boundaryCondition.constraints[i];
        debug += QString("    [%1] Surface ID: %2, Name: %3\n")
                    .arg(i).arg(constraint.surface_id).arg(QString::fromStdString(constraint.name));
    }
    debug += QString("  Loads (%1 entries):\n").arg(m_objectList.boundaryCondition.loads.size());
    for (size_t i = 0; i < m_objectList.boundaryCondition.loads.size(); ++i) {
        const auto& load = m_objectList.boundaryCondition.loads[i];
        debug += QString("    [%1] Surface ID: %2, Name: %3, Magnitude: %4, Direction: (%5, %6, %7)\n")
                    .arg(i).arg(load.surface_id).arg(QString::fromStdString(load.name))
                    .arg(load.magnitude).arg(load.direction.x).arg(load.direction.y).arg(load.direction.z);
    }

    debug += "=================================";

    return debug;
}
