#include "StepPropertyWidget.h"
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
        connect(m_uiState, &UIState::stepVisibilityChanged, this, [this](bool visible) {
            if (m_visibleCheckBox->isChecked() != visible) {
                bool oldState = m_visibleCheckBox->blockSignals(true);
                m_visibleCheckBox->setChecked(visible);
                m_visibleCheckBox->blockSignals(oldState);
            }
        });
        connect(m_uiState, &UIState::stepTransparencyChanged, this, [this](double val) {
            if (m_transparencySpinBox->value() != val) {
                bool oldState = m_transparencySpinBox->blockSignals(true);
                m_transparencySpinBox->setValue(val);
                m_transparencySpinBox->blockSignals(oldState);
            }
        });
    }
}

void StepPropertyWidget::setupUI()
{
    QFormLayout* layout = new QFormLayout(this);
    layout->setLabelAlignment(Qt::AlignLeft);
    layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    
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
    
    // Visible
    m_visibleCheckBox = new QCheckBox();
    m_visibleCheckBox->setStyleSheet("QCheckBox { color: white; }");
    connect(m_visibleCheckBox, &QCheckBox::toggled, this, [this](bool checked){
        if (m_uiState) m_uiState->setStepVisibility(checked);
    });
    layout->addRow(new QLabel("Visible:"), m_visibleCheckBox);
    
    // Transparency
    m_transparencySpinBox = new QDoubleSpinBox();
    m_transparencySpinBox->setRange(0.0, 1.0);
    m_transparencySpinBox->setSingleStep(0.1);
    m_transparencySpinBox->setStyleSheet("color: white; background-color: #333;");
    connect(m_transparencySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val){
        if (m_uiState) m_uiState->setStepTransparency(val);
    });
    layout->addRow(new QLabel("Transparency:"), m_transparencySpinBox);
    
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
    
    bool oldStateVis = m_visibleCheckBox->blockSignals(true);
    m_visibleCheckBox->setChecked(info.isVisible);
    m_visibleCheckBox->blockSignals(oldStateVis);
    
    bool oldStateTrans = m_transparencySpinBox->blockSignals(true);
    m_transparencySpinBox->setValue(info.transparency);
    m_transparencySpinBox->blockSignals(oldStateTrans);
}
