#include "ConstraintPropertyWidget.h"

#include "../../../core/commands/state/UpdateConstraintConditionCommand.h"
#include "../../ColorManager.h"
#include <QVBoxLayout>
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
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    
    // Labels style
    QString labelStyle = "color: #aaaaaa;";
    
    // Name
    m_nameEdit = new QLineEdit();
    // Updated input style: unified width, rounded corners, min-height to prevent collapse
    QString inputStyle = "color: white; background-color: #333; border: 1px solid #555; padding: 4px; border-radius: 4px; min-height: 20px;";
    
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
    
    // Spacer to push OK button to the bottom
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
    
    // OK Button container for right alignment
    QWidget* okButtonContainer = new QWidget();
    QHBoxLayout* okButtonLayout = new QHBoxLayout(okButtonContainer);
    okButtonLayout->setContentsMargins(0, 0, 0, 0);
    okButtonLayout->addStretch();
    
    m_okButton = new QPushButton("OK");
    m_okButton->setFixedWidth(80);
    m_okButton->setEnabled(false); // 初期状態は無効
    updateOkButtonStyle();
    connect(m_okButton, &QPushButton::clicked, this, &ConstraintPropertyWidget::onOkClicked);
    okButtonLayout->addWidget(m_okButton);
    
    layout->addRow("", okButtonContainer);
    
    // surface_idが変更されたらOKボタンの状態も更新
    // surface_idが変更されたらOKボタンの状態も更新
    connect(m_surfaceIdEdit, &QLineEdit::textChanged, this, &ConstraintPropertyWidget::updateOkButtonState);
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
    
    // OKボタンの状態を更新
    updateOkButtonState();
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

void ConstraintPropertyWidget::onOkClicked()
{
    if (m_uiState) {
        // 選択状態をクリアして何も選択されていない状態に戻す
        m_uiState->setSelectedObject(ObjectType::NONE);
    }
    emit okClicked();
}

void ConstraintPropertyWidget::updateOkButtonStyle()
{
    if (!m_okButton) return;
    
    if (m_okButton->isEnabled()) {
        // 有効時: 強調表示色
        m_okButton->setStyleSheet(
            QString("QPushButton { background-color: %1; color: %2; border: 1px solid %3; "
                    "padding: 8px 16px; border-radius: 4px; font-weight: bold; }"
                    "QPushButton:hover { background-color: %4; }"
                    "QPushButton:pressed { background-color: %5; }")
            .arg(ColorManager::BUTTON_EMPHASIZED_COLOR.name())
            .arg(ColorManager::BUTTON_COLOR.name())
            .arg(ColorManager::BUTTON_EDGE_COLOR.name())
            .arg(ColorManager::BUTTON_HOVER_COLOR.name())
            .arg(ColorManager::BUTTON_PRESSED_COLOR.name())
        );
    } else {
        // 無効時: グレーアウト
        m_okButton->setStyleSheet(
            QString("QPushButton { background-color: %1; color: %2; border: 1px solid %3; "
                    "padding: 8px 16px; border-radius: 4px; font-weight: bold; }")
            .arg(ColorManager::BUTTON_DISABLED_COLOR.name())
            .arg(ColorManager::BUTTON_DISABLED_TEXT_COLOR.name())
            .arg(ColorManager::BUTTON_EDGE_COLOR.name())
        );
    }
}

void ConstraintPropertyWidget::updateOkButtonState()
{
    if (!m_okButton || !m_surfaceIdEdit) return;
    
    // surface_idが0以外の場合にOKボタンを有効化
    bool hasValidSurface = m_surfaceIdEdit->text().toInt() > 0;
    m_okButton->setEnabled(hasValidSurface);
    updateOkButtonStyle();
}
