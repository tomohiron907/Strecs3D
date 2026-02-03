#include "PropertyWidget.h"
#include "../../utils/StyleManager.h"
#include "properties/StressDensityCurveWidget.h"
#include "properties/VolumeFractionChartWidget.h"
#include <QFrame>
#include <QDebug>

PropertyWidget::PropertyWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void PropertyWidget::setupUI()
{
    // Main layout for the widget itself
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    // Container Frame to provide the unified rounded border look
    QFrame* containerFrame = new QFrame(this);
    containerFrame->setObjectName("propertyContainerFrame");
    containerFrame->setStyleSheet(QString(R"(
        #propertyContainerFrame {
            background-color: rgba(26, 26, 26, 180);
            border: 1px solid #444;
            border-radius: %1px;
        }
        QLabel {
            border: none;
            background-color: transparent;
        }
    )").arg(StyleManager::CONTAINER_RADIUS));

    QVBoxLayout* containerLayout = new QVBoxLayout(containerFrame);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Title - match ObjectListWidget header style
    m_titleLabel = new QLabel("Properties", containerFrame);
    m_titleLabel->setStyleSheet(QString("font-weight: bold; font-size: %1px; padding: %2px %3px %2px %5px; color: #ffffff; border-bottom: 1px solid #444; min-height: %4px;")
        .arg(StyleManager::FONT_SIZE_LARGE)
        .arg(StyleManager::PADDING_SMALL)
        .arg(StyleManager::PADDING_MEDIUM)
        .arg(StyleManager::FONT_SIZE_LARGE + StyleManager::PADDING_SMALL * 2)
        .arg(StyleManager::PADDING_MEDIUM * 1.5));
    containerLayout->addWidget(m_titleLabel);

    // Stacked Widget for content
    m_stackedWidget = new QStackedWidget(containerFrame);
    m_stackedWidget->setStyleSheet("background-color: transparent; border: none;");

    // 0: Empty/Info
    m_emptyWidget = new QWidget();
    QVBoxLayout* emptyLayout = new QVBoxLayout(m_emptyWidget);
    QLabel* infoLabel = new QLabel("Select an item to view properties.");
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("color: #888;");
    emptyLayout->addWidget(infoLabel);
    m_stackedWidget->addWidget(m_emptyWidget);

    // 1: Step Properties
    m_stepWidget = new StepPropertyWidget();
    m_stackedWidget->addWidget(m_stepWidget);

    // 2: Constraint Properties
    m_constraintWidget = new ConstraintPropertyWidget();
    m_stackedWidget->addWidget(m_constraintWidget);

    // 3: Load Properties
    m_loadWidget = new LoadPropertyWidget();
    m_stackedWidget->addWidget(m_loadWidget);

    // 4: Simulation (Placeholders for now)
    m_simulationWidget = new QWidget(); // TODO: Implement specific widget
    QVBoxLayout* simLayout = new QVBoxLayout(m_simulationWidget);
    simLayout->addWidget(new QLabel("Simulation Result Properties (Read-only for now)"));
    m_stackedWidget->addWidget(m_simulationWidget);

    // 5: Infill (Placeholders for now)
    m_infillWidget = new QWidget(); // TODO: Implement specific widget
    QVBoxLayout* infillLayout = new QVBoxLayout(m_infillWidget);
    infillLayout->addWidget(new QLabel("Infill Region Properties"));
    m_stackedWidget->addWidget(m_infillWidget);

    // 6: Infill Default View (graphs)
    m_infillDefaultWidget = new QWidget();
    QVBoxLayout* infillDefaultLayout = new QVBoxLayout(m_infillDefaultWidget);
    infillDefaultLayout->setContentsMargins(5, 5, 5, 5);
    infillDefaultLayout->setSpacing(5);

    m_stressDensityCurveWidget = new StressDensityCurveWidget();
    m_volumeFractionChartWidget = new VolumeFractionChartWidget();

    QLabel* curveLabel = new QLabel("Stress - Density Curve");
    curveLabel->setAlignment(Qt::AlignCenter);
    curveLabel->setStyleSheet("color: #aaa; font-size: 11px;");
    infillDefaultLayout->addWidget(curveLabel);
    infillDefaultLayout->addWidget(m_stressDensityCurveWidget, 1);

    QLabel* chartLabel = new QLabel("Volume Fraction Distribution");
    chartLabel->setAlignment(Qt::AlignCenter);
    chartLabel->setStyleSheet("color: #aaa; font-size: 11px;");
    infillDefaultLayout->addWidget(chartLabel);
    infillDefaultLayout->addWidget(m_volumeFractionChartWidget, 1);

    m_stackedWidget->addWidget(m_infillDefaultWidget);

    containerLayout->addWidget(m_stackedWidget);

    // Add the container to the main layout
    m_mainLayout->addWidget(containerFrame);
}

void PropertyWidget::setUIState(UIState* uiState)
{
    m_uiState = uiState;
    if (m_stepWidget) m_stepWidget->setUIState(uiState);
    if (m_constraintWidget) m_constraintWidget->setUIState(uiState);
    if (m_loadWidget) m_loadWidget->setUIState(uiState);

    // UIStateの選択状態変更を監視
    if (m_uiState) {
        connect(m_uiState, &UIState::selectedObjectChanged,
                this, [this](const SelectedObjectInfo& selection) {
            onObjectSelected(selection.type, selection.id, selection.index);
        });
    }
}

void PropertyWidget::setVisualizationManager(VisualizationManager* vizManager)
{
    if (m_loadWidget) {
        m_loadWidget->setVisualizationManager(vizManager);
    }
}

void PropertyWidget::setVolumeFractions(const std::vector<double>& fractions)
{
    if (m_volumeFractionChartWidget) {
        m_volumeFractionChartWidget->setVolumeFractions(fractions);
    }
}

void PropertyWidget::setStressRange(double minStress, double maxStress)
{
    if (m_volumeFractionChartWidget) {
        m_volumeFractionChartWidget->setStressRange(minStress, maxStress);
    }
}

void PropertyWidget::setCurrentStep(ProcessStep step)
{
    m_currentStep = step;
    updateDefaultView();
}

void PropertyWidget::updateDefaultView()
{
    if (m_currentStep == ProcessStep::InfillMap) {
        // InfillMap step: show graph view as default
        if (m_stackedWidget->currentWidget() == m_emptyWidget) {
            m_titleLabel->setText("Infill Analysis");
            m_stackedWidget->setCurrentWidget(m_infillDefaultWidget);
        }
    } else {
        // Other steps: revert to empty widget if graph view is showing
        if (m_stackedWidget->currentWidget() == m_infillDefaultWidget) {
            m_titleLabel->setText("Properties");
            m_stackedWidget->setCurrentWidget(m_emptyWidget);
        }
    }
}

void PropertyWidget::onObjectSelected(ObjectType type, const QString& id, int index)
{
    if (!m_uiState) return;

    switch (type) {
        case ObjectType::ITEM_STEP:
        case ObjectType::ROOT_STEP: {
            m_titleLabel->setText("Model Properties");
            m_stackedWidget->setCurrentWidget(m_stepWidget);
            m_stepWidget->updateData(); // Force refresh
            break;
        }
        case ObjectType::ITEM_SIMULATION:
        case ObjectType::ROOT_SIMULATION: {
            m_titleLabel->setText("Simulation Result");
            m_stackedWidget->setCurrentWidget(m_simulationWidget);
            break;
        }
        case ObjectType::ITEM_INFILL_REGION: {
            m_titleLabel->setText("Infill Region");
            m_stackedWidget->setCurrentWidget(m_infillWidget);
            break;
        }
        case ObjectType::ITEM_BC_CONSTRAINT: {
            m_titleLabel->setText("Constraint Condition");
            m_constraintWidget->setTarget(index);
            m_stackedWidget->setCurrentWidget(m_constraintWidget);
            break;
        }
        case ObjectType::ITEM_BC_LOAD: {
            m_titleLabel->setText("Load Condition");
            m_loadWidget->setTarget(index);
            m_stackedWidget->setCurrentWidget(m_loadWidget);
            break;
        }
        case ObjectType::ROOT_BC:
        case ObjectType::ROOT_BC_CONSTRAINTS:
        case ObjectType::ROOT_BC_LOADS:
        case ObjectType::ROOT_INFILL:
        default:
            if (m_currentStep == ProcessStep::InfillMap) {
                m_titleLabel->setText("Infill Analysis");
                m_stackedWidget->setCurrentWidget(m_infillDefaultWidget);
            } else {
                m_titleLabel->setText("Properties");
                m_stackedWidget->setCurrentWidget(m_emptyWidget);
            }
            break;
    }
}
