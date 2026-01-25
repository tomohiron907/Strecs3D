#include "ConstraintPropertyWidget.h"

#include "../../../core/commands/state/UpdateConstraintConditionCommand.h"
#include "../../../utils/ColorManager.h"
#include "../../../utils/StyleManager.h"
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QIntValidator>

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
    layout->setSpacing(StyleManager::FORM_SPACING);
    layout->setContentsMargins(StyleManager::FORM_SPACING, StyleManager::FORM_SPACING,
                               StyleManager::FORM_SPACING, StyleManager::FORM_SPACING);

    // Labels style
    QString labelStyle = "color: #aaaaaa;";

    // Updated input style: unified width, rounded corners, min-height to prevent collapse
    QString inputStyle = QString("QLineEdit { color: %1; background-color: %2; border: 1px solid %3; padding: %4px; border-radius: %5px; min-height: %6px; selection-background-color: #555555; }")
        .arg(ColorManager::INPUT_TEXT_COLOR.name())
        .arg(ColorManager::INPUT_BACKGROUND_COLOR.name())
        .arg(ColorManager::INPUT_BORDER_COLOR.name())
        .arg(StyleManager::PADDING_SMALL)
        .arg(StyleManager::RADIUS_SMALL)
        .arg(StyleManager::INPUT_HEIGHT_SMALL);
    
    // Name
    m_nameEdit = new QLineEdit();
    m_nameEdit->setStyleSheet(inputStyle);
    m_nameEdit->setFixedWidth(100);
    connect(m_nameEdit, &QLineEdit::editingFinished, this, &ConstraintPropertyWidget::pushData);
    layout->addRow(new QLabel("Name:"), m_nameEdit);
    
    // Surface ID
    m_surfaceIdEdit = new QLineEdit();
    m_surfaceIdEdit->setValidator(new QIntValidator(0, 99999, this));
    m_surfaceIdEdit->setStyleSheet(inputStyle);
    m_surfaceIdEdit->setFixedWidth(100);
    // Connect to editingFinished for data push (consistent with LoadPropertyWidget)
    connect(m_surfaceIdEdit, &QLineEdit::editingFinished, this, &ConstraintPropertyWidget::pushData);
    layout->addRow(new QLabel("Surface ID:"), m_surfaceIdEdit);
    
    // Apply label style
    for(int i = 0; i < layout->rowCount(); ++i) {
        QWidget* label = layout->itemAt(i, QFormLayout::LabelRole)->widget();
        if(label) label->setStyleSheet(labelStyle);
    }

    // Spacer to push Close button to the bottom
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // Close Button container for right alignment
    QWidget* closeButtonContainer = new QWidget();
    QHBoxLayout* closeButtonLayout = new QHBoxLayout(closeButtonContainer);
    closeButtonLayout->setContentsMargins(0, 0, 0, 0);
    closeButtonLayout->addStretch();

    m_closeButton = new QPushButton("Close");
    m_closeButton->setFixedWidth(80);
    m_closeButton->setStyleSheet(
        QString("QPushButton { background-color: %1; color: %2; border: none; "
                "padding: %5px %6px; border-radius: %7px; font-weight: bold; }"
                "QPushButton:hover { background-color: %3; }"
                "QPushButton:pressed { background-color: %4; }")
        .arg(ColorManager::ACCENT_COLOR.name())
        .arg(ColorManager::BUTTON_TEXT_COLOR.name())
        .arg(ColorManager::BUTTON_HOVER_COLOR.name())
        .arg(ColorManager::BUTTON_PRESSED_COLOR.name())
        .arg(StyleManager::BUTTON_PADDING_V)
        .arg(StyleManager::BUTTON_PADDING_H)
        .arg(StyleManager::BUTTON_RADIUS)
    );
    connect(m_closeButton, &QPushButton::clicked, this, &ConstraintPropertyWidget::onCloseClicked);
    closeButtonLayout->addWidget(m_closeButton);

    layout->addRow("", closeButtonContainer);
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
    
    bool oldBlockedId = m_surfaceIdEdit->blockSignals(true);
    m_surfaceIdEdit->setText(QString::number(c.surface_id));
    m_surfaceIdEdit->blockSignals(oldBlockedId);
}

void ConstraintPropertyWidget::pushData()
{
    if (!m_uiState || m_currentIndex < 0) return;
    
    auto bc = m_uiState->getBoundaryCondition();
    if (m_currentIndex >= (int)bc.constraints.size()) return;
    
    ConstraintCondition c = bc.constraints[m_currentIndex];
    c.name = m_nameEdit->text().toStdString();
    c.surface_id = m_surfaceIdEdit->text().toInt();
    
    // Update via UIState
    // Command pattern: Update constraint
    auto command = std::make_unique<UpdateConstraintConditionCommand>(
        m_uiState,
        m_currentIndex,
        c
    );
    command->execute();
}

void ConstraintPropertyWidget::onCloseClicked()
{
    if (m_uiState) {
        m_uiState->setSelectedObject(ObjectType::NONE);
    }
    emit closeClicked();
}
