#include "ProcessManagerWidget.h"
#include <QVBoxLayout>
#include <QFrame>
#include "steps/ImportStepWidget.h"
#include "steps/BoundaryConditionStepWidget.h"
#include "steps/SimulationStepWidget.h"
#include "steps/InfillStepWidget.h"

ProcessManagerWidget::ProcessManagerWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
}

void ProcessManagerWidget::setUIState(UIState* uiState)
{
    // Pass UIState to widgets that need it
    if (m_infillWidget) {
        m_infillWidget->setUIState(uiState);
    }
    if (m_bcWidget) {
        m_bcWidget->setUIState(uiState);
    }
}

void ProcessManagerWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10); // Gap between top and bottom frames

    // Common Style for frames
    QString frameStyle = R"(
        QFrame {
            background-color: rgba(26, 26, 26, 180);
            border: 1px solid #444;
            border-radius: 4px;
        }
        QLabel {
            border: none;
            background-color: transparent;
        }
    )";

    // --- Top Frame (Flow) ---
    QFrame* topFrame = new QFrame(this);
    topFrame->setStyleSheet(frameStyle);
    QVBoxLayout* topLayout = new QVBoxLayout(topFrame);
    topLayout->setContentsMargins(5, 5, 5, 5); // Small padding for the flow widget

    m_flowWidget = new ProcessFlowWidget(topFrame);
    topLayout->addWidget(m_flowWidget);
    
    mainLayout->addWidget(topFrame);
    
    // --- Bottom Frame (Step Content) ---
    QFrame* bottomFrame = new QFrame(this);
    bottomFrame->setStyleSheet(frameStyle);
    QVBoxLayout* bottomLayout = new QVBoxLayout(bottomFrame);
    bottomLayout->setContentsMargins(10, 10, 10, 10); // Padding for content

    m_stepContainer = new QStackedWidget(bottomFrame);
    m_stepContainer->setStyleSheet("background-color: transparent; border: none;"); // Transparent to show frame bg, no border
    bottomLayout->addWidget(m_stepContainer);

    mainLayout->addWidget(bottomFrame);
    
    // Create Step Widgets
    m_importWidget = new ImportStepWidget(this);
    m_bcWidget = new BoundaryConditionStepWidget(this);
    m_simWidget = new SimulationStepWidget(this);
    m_infillWidget = new InfillStepWidget(this);
    
    // Add to Stack (Order must match ProcessStep enum)
    m_stepContainer->addWidget(m_importWidget);      // 0: Import
    m_stepContainer->addWidget(m_bcWidget);          // 1: BoundaryCondition
    m_stepContainer->addWidget(m_simWidget);         // 2: Simulate
    m_stepContainer->addWidget(m_infillWidget);      // 3: InfillMap
    
    // Set size policy: Top fixed, bottom expanding
    topFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    bottomFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
}

void ProcessManagerWidget::connectSignals()
{
    // Flow Widget -> Stacked Widget (if we allowed clicking, but for now it's guided)
    
    // Import Step
    connect(m_importWidget, &ImportStepWidget::importClicked, this, &ProcessManagerWidget::importStepClicked);
    connect(m_importWidget, &ImportStepWidget::fileDropped, this, &ProcessManagerWidget::importFile);
    
    // BC Step
    connect(m_bcWidget, &BoundaryConditionStepWidget::addLoadClicked, this, &ProcessManagerWidget::addLoadClicked);
    connect(m_bcWidget, &BoundaryConditionStepWidget::addConstraintClicked, this, &ProcessManagerWidget::addConstraintClicked);
    connect(m_bcWidget, &BoundaryConditionStepWidget::doneClicked, this, &ProcessManagerWidget::nextStep);
    
    // Sim Step
    connect(m_simWidget, &SimulationStepWidget::simulateClicked, this, &ProcessManagerWidget::simulateClicked);
    
    // Infill Step
    connect(m_infillWidget, &InfillStepWidget::processClicked, this, &ProcessManagerWidget::processInfillClicked);
}

void ProcessManagerWidget::onImportCompleted()
{
    // Auto advance to step 2
    if (m_flowWidget->currentStep() == ProcessStep::ImportStep) {
        nextStep();
    }
}

void ProcessManagerWidget::onSimulationCompleted()
{
    // Auto advance to step 4
     if (m_flowWidget->currentStep() == ProcessStep::Simulate) {
        nextStep();
    }
}

void ProcessManagerWidget::nextStep()
{
    ProcessStep current = m_flowWidget->currentStep();
    int nextIdx = static_cast<int>(current) + 1;
    
    if (nextIdx <= static_cast<int>(ProcessStep::InfillMap)) {
        ProcessStep next = static_cast<ProcessStep>(nextIdx);
        m_flowWidget->setCurrentStep(next);
        m_stepContainer->setCurrentIndex(nextIdx);
        emit stepChanged(next);
    }
}
