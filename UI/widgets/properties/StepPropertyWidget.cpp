#include "StepPropertyWidget.h"
#include "../../../utils/StyleManager.h"
#include <QVBoxLayout>

StepPropertyWidget::StepPropertyWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void StepPropertyWidget::setUIState(UIState* uiState)
{
    m_uiState = uiState;
    if (m_uiState) {
        // Init with current values
        updateData();
        
        // Connect signals
        connect(m_uiState, &UIState::stepFileInfoChanged, this, [this](const ObjectFileInfo& info) {
            updateData();
        });
    }
}

void StepPropertyWidget::setupUI()
{
    QFormLayout* layout = new QFormLayout(this);
    layout->setLabelAlignment(Qt::AlignLeft);
    layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    layout->setSpacing(StyleManager::FORM_SPACING);
    layout->setContentsMargins(StyleManager::FORM_SPACING, StyleManager::FORM_SPACING,
                               StyleManager::FORM_SPACING, StyleManager::FORM_SPACING);
    
    // Labels style
    QString labelStyle = "color: #aaaaaa;";
    QString valueStyle = "color: #ffffff;";

    // Filename
    m_filenameLabel = new QLabel("-");
    m_filenameLabel->setStyleSheet(valueStyle);
    layout->addRow(new QLabel("Filename:"), m_filenameLabel);
    
    // Path
    m_filePathLabel = new QLabel("-");
    m_filePathLabel->setStyleSheet(valueStyle);
    m_filePathLabel->setWordWrap(true);
    layout->addRow(new QLabel("Path:"), m_filePathLabel);
    
    // Apply label style
    for(int i = 0; i < layout->rowCount(); ++i) {
        QWidget* label = layout->itemAt(i, QFormLayout::LabelRole)->widget();
        if(label) label->setStyleSheet(labelStyle);
    }
}

void StepPropertyWidget::updateData()
{
    if (!m_uiState) return;
    
    ObjectFileInfo info = m_uiState->getStepFileInfo();
    m_filenameLabel->setText(info.filename.isEmpty() ? "-" : info.filename);
    m_filePathLabel->setText(info.filePath.isEmpty() ? "-" : info.filePath);
}
