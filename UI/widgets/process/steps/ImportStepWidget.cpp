#include "ImportStepWidget.h"
#include "../../Button.h"
#include <QVBoxLayout>

ImportStepWidget::ImportStepWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch();
    
    m_importButton = new Button("Import STEP File", this);
    m_importButton->setGlobalIconSize(QSize(24, 24));
    m_importButton->setIcon(":/resources/icons/step.png");
    m_importButton->setMinimumHeight(50);
    
    connect(m_importButton, &Button::clicked, this, &ImportStepWidget::importClicked);
    
    layout->addWidget(m_importButton);
    layout->addStretch();
}
