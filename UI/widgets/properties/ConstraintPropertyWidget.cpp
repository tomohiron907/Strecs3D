#include "ConstraintPropertyWidget.h"
#include <QVBoxLayout>

ConstraintPropertyWidget::ConstraintPropertyWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void ConstraintPropertyWidget::setUIState(UIState* uiState)
{
    m_uiState = uiState;
    // We don't connect signals here for updates because this widget is transient/dependant on selection.
    // However, if external change happens, we might want to refresh.
    // But usually PropertyWidget is refreshed on selection or explicit signal.
    // For now, simple binding.
}

void ConstraintPropertyWidget::setTarget(int index)
{
    m_currentIndex = index;
    updateData();
}

void ConstraintPropertyWidget::setupUI()
{
    QFormLayout* layout = new QFormLayout(this);
    layout->setLabelAlignment(Qt::AlignLeft);
    layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    
    // Labels style
    QString labelStyle = "color: #aaaaaa;";
    
    // Name
    m_nameEdit = new QLineEdit();
    m_nameEdit->setStyleSheet("QLineEdit { color: white; background-color: #333; border: 1px solid #555; padding: 4px; }");
    connect(m_nameEdit, &QLineEdit::editingFinished, this, &ConstraintPropertyWidget::pushData);
    layout->addRow(new QLabel("Name:"), m_nameEdit);
    
    // Surface ID
    m_surfaceIdSpinBox = new QSpinBox();
    m_surfaceIdSpinBox->setRange(0, 99999);
    // 0 is represented as empty or "-"
    m_surfaceIdSpinBox->setSpecialValueText("-"); 
    m_surfaceIdSpinBox->setStyleSheet("QSpinBox { color: white; background-color: #333; border: 1px solid #555; padding: 4px; }");
    connect(m_surfaceIdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int){ pushData(); });
    layout->addRow(new QLabel("Surface ID:"), m_surfaceIdSpinBox);
    
    // Apply label style
    for(int i = 0; i < layout->rowCount(); ++i) {
        QWidget* label = layout->itemAt(i, QFormLayout::LabelRole)->widget();
        if(label) label->setStyleSheet(labelStyle);
    }
}

void ConstraintPropertyWidget::updateData()
{
    if (!m_uiState || m_currentIndex < 0) return;
    
    auto bc = m_uiState->getBoundaryCondition();
    if (m_currentIndex >= (int)bc.constraints.size()) return;
    
    const auto& c = bc.constraints[m_currentIndex];
    
    bool oldBlockedName = m_nameEdit->blockSignals(true);
    m_nameEdit->setText(QString::fromStdString(c.name));
    m_nameEdit->blockSignals(oldBlockedName);
    
    bool oldBlockedId = m_surfaceIdSpinBox->blockSignals(true);
    m_surfaceIdSpinBox->setValue(c.surface_id);
    m_surfaceIdSpinBox->blockSignals(oldBlockedId);
}

void ConstraintPropertyWidget::pushData()
{
    if (!m_uiState || m_currentIndex < 0) return;
    
    auto bc = m_uiState->getBoundaryCondition();
    if (m_currentIndex >= (int)bc.constraints.size()) return;
    
    ConstraintCondition c = bc.constraints[m_currentIndex];
    c.name = m_nameEdit->text().toStdString();
    c.surface_id = m_surfaceIdSpinBox->value();
    
    // Update via UIState
    m_uiState->updateConstraintCondition(m_currentIndex, c);
}
