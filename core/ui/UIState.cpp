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

void UIState::setStlFilePath(const QString& path)
{
    if (m_stlFilePath != path) {
        m_stlFilePath = path;
        emit stlFilePathChanged(path);
        qDebug() << "UIState: STL file path changed to:" << path;
    }
}

void UIState::setVtkFilePath(const QString& path)
{
    if (m_vtkFilePath != path) {
        m_vtkFilePath = path;
        emit vtkFilePathChanged(path);
        qDebug() << "UIState: VTK file path changed to:" << path;
    }
}

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

void UIState::setMeshDisplaySettings(const DisplaySettings& settings)
{
    if (m_meshDisplaySettings.isVisible != settings.isVisible || 
        m_meshDisplaySettings.opacity != settings.opacity) {
        m_meshDisplaySettings = settings;
        emit meshDisplaySettingsChanged(settings);
        qDebug() << "UIState: Mesh display settings changed - Visible:" << settings.isVisible << "Opacity:" << settings.opacity;
    }
}

void UIState::setVtuDisplaySettings(const DisplaySettings& settings)
{
    if (m_vtuDisplaySettings.isVisible != settings.isVisible || 
        m_vtuDisplaySettings.opacity != settings.opacity) {
        m_vtuDisplaySettings = settings;
        emit vtuDisplaySettingsChanged(settings);
        qDebug() << "UIState: VTU display settings changed - Visible:" << settings.isVisible << "Opacity:" << settings.opacity;
    }
}

void UIState::setDividedMesh1Settings(const DisplaySettings& settings)
{
    if (m_dividedMesh1Settings.isVisible != settings.isVisible || 
        m_dividedMesh1Settings.opacity != settings.opacity) {
        m_dividedMesh1Settings = settings;
        emit dividedMesh1SettingsChanged(settings);
        qDebug() << "UIState: Divided mesh 1 settings changed - Visible:" << settings.isVisible << "Opacity:" << settings.opacity;
    }
}

void UIState::setDividedMesh2Settings(const DisplaySettings& settings)
{
    if (m_dividedMesh2Settings.isVisible != settings.isVisible || 
        m_dividedMesh2Settings.opacity != settings.opacity) {
        m_dividedMesh2Settings = settings;
        emit dividedMesh2SettingsChanged(settings);
        qDebug() << "UIState: Divided mesh 2 settings changed - Visible:" << settings.isVisible << "Opacity:" << settings.opacity;
    }
}

void UIState::setDividedMesh3Settings(const DisplaySettings& settings)
{
    if (m_dividedMesh3Settings.isVisible != settings.isVisible || 
        m_dividedMesh3Settings.opacity != settings.opacity) {
        m_dividedMesh3Settings = settings;
        emit dividedMesh3SettingsChanged(settings);
        qDebug() << "UIState: Divided mesh 3 settings changed - Visible:" << settings.isVisible << "Opacity:" << settings.opacity;
    }
}

void UIState::setDividedMesh4Settings(const DisplaySettings& settings)
{
    if (m_dividedMesh4Settings.isVisible != settings.isVisible || 
        m_dividedMesh4Settings.opacity != settings.opacity) {
        m_dividedMesh4Settings = settings;
        emit dividedMesh4SettingsChanged(settings);
        qDebug() << "UIState: Divided mesh 4 settings changed - Visible:" << settings.isVisible << "Opacity:" << settings.opacity;
    }
}

void UIState::printDebugInfo() const
{
    qDebug() << "=== UIState Debug Information ===";
    qDebug() << "STL File Path:" << m_stlFilePath;
    qDebug() << "VTK File Path:" << m_vtkFilePath;
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
    
    qDebug() << "Display Settings:";
    qDebug() << "  Mesh: Visible=" << m_meshDisplaySettings.isVisible << "Opacity=" << m_meshDisplaySettings.opacity;
    qDebug() << "  VTU: Visible=" << m_vtuDisplaySettings.isVisible << "Opacity=" << m_vtuDisplaySettings.opacity;
    qDebug() << "  Divided Mesh 1: Visible=" << m_dividedMesh1Settings.isVisible << "Opacity=" << m_dividedMesh1Settings.opacity;
    qDebug() << "  Divided Mesh 2: Visible=" << m_dividedMesh2Settings.isVisible << "Opacity=" << m_dividedMesh2Settings.opacity;
    qDebug() << "  Divided Mesh 3: Visible=" << m_dividedMesh3Settings.isVisible << "Opacity=" << m_dividedMesh3Settings.opacity;
    qDebug() << "  Divided Mesh 4: Visible=" << m_dividedMesh4Settings.isVisible << "Opacity=" << m_dividedMesh4Settings.opacity;
    qDebug() << "=================================";
}

QString UIState::getDebugString() const
{
    QString debug;
    debug += "=== UIState Debug Information ===\n";
    debug += QString("STL File Path: %1\n").arg(m_stlFilePath);
    debug += QString("VTK File Path: %1\n").arg(m_vtkFilePath);
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
    
    debug += "Display Settings:\n";
    debug += QString("  Mesh: Visible=%1, Opacity=%2\n").arg(m_meshDisplaySettings.isVisible).arg(m_meshDisplaySettings.opacity);
    debug += QString("  VTU: Visible=%1, Opacity=%2\n").arg(m_vtuDisplaySettings.isVisible).arg(m_vtuDisplaySettings.opacity);
    debug += QString("  Divided Mesh 1: Visible=%1, Opacity=%2\n").arg(m_dividedMesh1Settings.isVisible).arg(m_dividedMesh1Settings.opacity);
    debug += QString("  Divided Mesh 2: Visible=%1, Opacity=%2\n").arg(m_dividedMesh2Settings.isVisible).arg(m_dividedMesh2Settings.opacity);
    debug += QString("  Divided Mesh 3: Visible=%1, Opacity=%2\n").arg(m_dividedMesh3Settings.isVisible).arg(m_dividedMesh3Settings.opacity);
    debug += QString("  Divided Mesh 4: Visible=%1, Opacity=%2\n").arg(m_dividedMesh4Settings.isVisible).arg(m_dividedMesh4Settings.opacity);
    debug += "=================================";
    
    return debug;
}