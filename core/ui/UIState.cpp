#include "UIState.h"
#include <QDebug>
#include <QFileInfo>

UIState::UIState(QObject* parent)
    : QObject(parent)
{
    // Colors and stress density mappings are initialized dynamically
    // from the slider when stress range is set
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
        
        // Update display order
        if (!path.isEmpty()) {
            if (std::find(m_objectList.displayOrder.begin(), m_objectList.displayOrder.end(), ObjectListData::KEY_STEP) == m_objectList.displayOrder.end()) {
                m_objectList.displayOrder.push_back(ObjectListData::KEY_STEP);
            }
        } else {
            auto it = std::remove(m_objectList.displayOrder.begin(), m_objectList.displayOrder.end(), ObjectListData::KEY_STEP);
            m_objectList.displayOrder.erase(it, m_objectList.displayOrder.end());
        }

        emit stepFilePathChanged(path);
        
        // ObjectListWidgetの更新などのために、FileInfo変更シグナルも発行する
        emit stepFileInfoChanged(m_objectList.step);
        
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

void UIState::clearStepFile()
{
    m_objectList.step = ObjectFileInfo();  // Reset to default

    // Remove from display order
    auto it = std::remove(m_objectList.displayOrder.begin(),
                         m_objectList.displayOrder.end(),
                         ObjectListData::KEY_STEP);
    m_objectList.displayOrder.erase(it, m_objectList.displayOrder.end());

    emit stepFilePathChanged("");
    emit stepFileInfoChanged(m_objectList.step);
    qDebug() << "UIState: STEP file cleared";
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
    
    // Update display order
    if (std::find(m_objectList.displayOrder.begin(), m_objectList.displayOrder.end(), ObjectListData::KEY_BC) == m_objectList.displayOrder.end()) {
        m_objectList.displayOrder.push_back(ObjectListData::KEY_BC);
    }
    
    emit boundaryConditionChanged(m_objectList.boundaryCondition);
    qDebug() << "UIState: Constraint condition added - Surface ID:" << constraint.surface_id
             << "Name:" << QString::fromStdString(constraint.name)
             << "Total constraints:" << m_objectList.boundaryCondition.constraints.size();
}

void UIState::addLoadCondition(const LoadCondition& load)
{
    m_objectList.boundaryCondition.loads.push_back(load);
    
    // Update display order
    if (std::find(m_objectList.displayOrder.begin(), m_objectList.displayOrder.end(), ObjectListData::KEY_BC) == m_objectList.displayOrder.end()) {
        m_objectList.displayOrder.push_back(ObjectListData::KEY_BC);
    }

    emit boundaryConditionChanged(m_objectList.boundaryCondition);
    qDebug() << "UIState: Load condition added - Surface ID:" << load.surface_id
             << "Name:" << QString::fromStdString(load.name)
             << "Magnitude:" << load.magnitude
             << "Total loads:" << m_objectList.boundaryCondition.loads.size();
}

void UIState::updateConstraintCondition(int index, const ConstraintCondition& constraint)
{
    if (index >= 0 && index < (int)m_objectList.boundaryCondition.constraints.size()) {
        m_objectList.boundaryCondition.constraints[index] = constraint;
        emit boundaryConditionChanged(m_objectList.boundaryCondition);
        qDebug() << "UIState: Constraint condition updated at index:" << index;
    }
}

void UIState::removeConstraintCondition(int index)
{
    if (index >= 0 && index < (int)m_objectList.boundaryCondition.constraints.size()) {
        m_objectList.boundaryCondition.constraints.erase(m_objectList.boundaryCondition.constraints.begin() + index);
        
        // Remove from display order if both loads and constraints are empty
        if (m_objectList.boundaryCondition.constraints.empty() && m_objectList.boundaryCondition.loads.empty()) {
            auto it = std::remove(m_objectList.displayOrder.begin(), m_objectList.displayOrder.end(), ObjectListData::KEY_BC);
            m_objectList.displayOrder.erase(it, m_objectList.displayOrder.end());
        }
        
        emit boundaryConditionChanged(m_objectList.boundaryCondition);
        qDebug() << "UIState: Constraint condition removed at index:" << index;
    }
}

void UIState::updateLoadCondition(int index, const LoadCondition& load)
{
    if (index >= 0 && index < (int)m_objectList.boundaryCondition.loads.size()) {
        m_objectList.boundaryCondition.loads[index] = load;
        emit boundaryConditionChanged(m_objectList.boundaryCondition);
        qDebug() << "UIState: Load condition updated at index:" << index;
    }
}

void UIState::removeLoadCondition(int index)
{
    if (index >= 0 && index < (int)m_objectList.boundaryCondition.loads.size()) {
        m_objectList.boundaryCondition.loads.erase(m_objectList.boundaryCondition.loads.begin() + index);

        // Remove from display order if both loads and constraints are empty
        if (m_objectList.boundaryCondition.constraints.empty() && m_objectList.boundaryCondition.loads.empty()) {
            auto it = std::remove(m_objectList.displayOrder.begin(), m_objectList.displayOrder.end(), ObjectListData::KEY_BC);
            m_objectList.displayOrder.erase(it, m_objectList.displayOrder.end());
        }
        
        emit boundaryConditionChanged(m_objectList.boundaryCondition);
        qDebug() << "UIState: Load condition removed at index:" << index;
    }
}

void UIState::clearConstraintConditions()
{
    m_objectList.boundaryCondition.constraints.clear();
    
    // Remove from display order if both loads and constraints are empty
    if (m_objectList.boundaryCondition.constraints.empty() && m_objectList.boundaryCondition.loads.empty()) {
        auto it = std::remove(m_objectList.displayOrder.begin(), m_objectList.displayOrder.end(), ObjectListData::KEY_BC);
        m_objectList.displayOrder.erase(it, m_objectList.displayOrder.end());
    }

    emit boundaryConditionChanged(m_objectList.boundaryCondition);
    qDebug() << "UIState: All constraint conditions cleared";
}

void UIState::clearLoadConditions()
{
    m_objectList.boundaryCondition.loads.clear();

    // Remove from display order if both loads and constraints are empty
    if (m_objectList.boundaryCondition.constraints.empty() && m_objectList.boundaryCondition.loads.empty()) {
        auto it = std::remove(m_objectList.displayOrder.begin(), m_objectList.displayOrder.end(), ObjectListData::KEY_BC);
        m_objectList.displayOrder.erase(it, m_objectList.displayOrder.end());
    }
    
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
        
        // Update display order
        if (!path.isEmpty()) {
            if (std::find(m_objectList.displayOrder.begin(), m_objectList.displayOrder.end(), ObjectListData::KEY_SIMULATION) == m_objectList.displayOrder.end()) {
                m_objectList.displayOrder.push_back(ObjectListData::KEY_SIMULATION);
            }
        } else {
            auto it = std::remove(m_objectList.displayOrder.begin(), m_objectList.displayOrder.end(), ObjectListData::KEY_SIMULATION);
            m_objectList.displayOrder.erase(it, m_objectList.displayOrder.end());
        }
        
        emit simulationResultFilePathChanged(path);
        
        // ObjectListWidgetの更新などのために、FileInfo変更シグナルも発行する
        emit simulationResultFileInfoChanged(m_objectList.simulationResult);
        
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

void UIState::clearSimulationResult()
{
    m_objectList.simulationResult = ObjectFileInfo();  // Reset to default

    // Remove from display order
    auto it = std::remove(m_objectList.displayOrder.begin(),
                         m_objectList.displayOrder.end(),
                         ObjectListData::KEY_SIMULATION);
    m_objectList.displayOrder.erase(it, m_objectList.displayOrder.end());

    emit simulationResultFilePathChanged("");
    emit simulationResultFileInfoChanged(m_objectList.simulationResult);
    qDebug() << "UIState: Simulation result cleared";
}

// ========== Infill Regions 関連 ==========
void UIState::addInfillRegion(const QString& key, const InfillRegionInfo& info)
{
    m_objectList.infillRegions[key] = info;
    
    // Update display order
    if (std::find(m_objectList.displayOrder.begin(), m_objectList.displayOrder.end(), ObjectListData::KEY_INFILL) == m_objectList.displayOrder.end()) {
        m_objectList.displayOrder.push_back(ObjectListData::KEY_INFILL);
    }
    
    emit infillRegionAdded(key, info);
    qDebug() << "UIState: Infill region added - Key:" << key << "Name:" << info.name;
}

void UIState::removeInfillRegion(const QString& key)
{
    if (m_objectList.infillRegions.erase(key) > 0) {
        // Update display order if empty
        if (m_objectList.infillRegions.empty()) {
            auto it = std::remove(m_objectList.displayOrder.begin(), m_objectList.displayOrder.end(), ObjectListData::KEY_INFILL);
            m_objectList.displayOrder.erase(it, m_objectList.displayOrder.end());
        }

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

void UIState::clearAllInfillRegions()
{
    // Clear all infill regions
    m_objectList.infillRegions.clear();

    // Remove from display order
    auto it = std::remove(m_objectList.displayOrder.begin(),
                         m_objectList.displayOrder.end(),
                         ObjectListData::KEY_INFILL);
    m_objectList.displayOrder.erase(it, m_objectList.displayOrder.end());

    // Note: ObjectListWidget will automatically rebuild based on the cleared data
    // through its connection to various UIState signals
    qDebug() << "UIState: All infill regions cleared";
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

// ========== 選択状態管理 ==========
void UIState::setSelectedObject(ObjectType type, const QString& id, int index)
{
    if (m_selectedObject.type != type || m_selectedObject.id != id || m_selectedObject.index != index) {
        m_selectedObject.type = type;
        m_selectedObject.id = id;
        m_selectedObject.index = index;
        emit selectedObjectChanged(m_selectedObject);
        qDebug() << "UIState: Selected object changed - Type:" << (int)type << "ID:" << id << "Index:" << index;
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
