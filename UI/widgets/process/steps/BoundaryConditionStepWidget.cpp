#include "BoundaryConditionStepWidget.h"
#include "../../Button.h"
#include <QVBoxLayout>

BoundaryConditionStepWidget::BoundaryConditionStepWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Add Buttons
    
    m_addConstraintButton = new Button("Add Constraint", this);
    connect(m_addConstraintButton, &Button::clicked, this, &BoundaryConditionStepWidget::addConstraintClicked);
    
    m_addLoadButton = new Button("Add Load", this);
    connect(m_addLoadButton, &Button::clicked, this, &BoundaryConditionStepWidget::addLoadClicked);

    layout->addWidget(m_addConstraintButton);
    layout->addWidget(m_addLoadButton);
    layout->addStretch();
    
    // Done Button
    m_doneButton = new Button("Set Boundary Conditions", this);
    m_doneButton->setEmphasized(true);
    connect(m_doneButton, &Button::clicked, this, &BoundaryConditionStepWidget::doneClicked);
    
    layout->addWidget(m_doneButton);
}
