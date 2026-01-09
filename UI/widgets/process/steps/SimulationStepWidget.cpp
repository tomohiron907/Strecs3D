#include "SimulationStepWidget.h"
#include "../../Button.h"
#include <QVBoxLayout>

SimulationStepWidget::SimulationStepWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch();
    
    m_simulateButton = new Button("Simulate", this);
    m_simulateButton->setMinimumHeight(50);
    m_simulateButton->setEmphasized(true);
    connect(m_simulateButton, &Button::clicked, this, &SimulationStepWidget::simulateClicked);
    
    layout->addWidget(m_simulateButton);
    layout->addStretch();
}
