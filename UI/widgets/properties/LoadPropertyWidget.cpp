#include "LoadPropertyWidget.h"
#include "../../../core/commands/state/UpdateLoadConditionCommand.h"
#include "../../ColorManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>

LoadPropertyWidget::LoadPropertyWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void LoadPropertyWidget::setUIState(UIState* uiState)
{
    m_uiState = uiState;
}

void LoadPropertyWidget::setTarget(int index)
{
    m_currentIndex = index;
    updateData();
}

void LoadPropertyWidget::setupUI()
{
    QFormLayout* layout = new QFormLayout(this);
    layout->setLabelAlignment(Qt::AlignLeft);
    layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    
    // Labels style
    QString labelStyle = "color: #aaaaaa;";
    QString inputStyle = "color: white; background-color: #333; border: 1px solid #555; padding: 4px;";
    
    // Name
    m_nameEdit = new QLineEdit();
    m_nameEdit->setStyleSheet(inputStyle);
    connect(m_nameEdit, &QLineEdit::editingFinished, this, &LoadPropertyWidget::pushData);
    layout->addRow(new QLabel("Name:"), m_nameEdit);
    
    // Surface ID
    m_surfaceIdSpinBox = new QSpinBox();
    m_surfaceIdSpinBox->setRange(0, 99999);
    // 0 is represented as empty or "-"
    m_surfaceIdSpinBox->setSpecialValueText("-");
    m_surfaceIdSpinBox->setStyleSheet(inputStyle);
    connect(m_surfaceIdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int){ pushData(); });
    layout->addRow(new QLabel("Surface ID:"), m_surfaceIdSpinBox);
    
    // Magnitude
    m_magnitudeSpinBox = new QDoubleSpinBox();
    m_magnitudeSpinBox->setRange(-1e9, 1e9);
    m_magnitudeSpinBox->setSingleStep(1.0);
    m_magnitudeSpinBox->setStyleSheet(inputStyle);
    connect(m_magnitudeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double){ pushData(); });
    layout->addRow(new QLabel("Magnitude:"), m_magnitudeSpinBox);

    // Direction
    QWidget* vectorWidget = new QWidget();
    QHBoxLayout* vectorLayout = new QHBoxLayout(vectorWidget);
    vectorLayout->setContentsMargins(0, 0, 0, 0);
    vectorLayout->setSpacing(5);
    
    auto createSpinBox = [&](QDoubleSpinBox*& sb) {
        sb = new QDoubleSpinBox();
        sb->setRange(-1.0, 1.0);
        sb->setSingleStep(0.1);
        sb->setDecimals(3);
        sb->setStyleSheet(inputStyle);
        connect(sb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double){ pushData(); });
        vectorLayout->addWidget(sb);
    };

    createSpinBox(m_dirXSpinBox);
    createSpinBox(m_dirYSpinBox);
    createSpinBox(m_dirZSpinBox);
    
    layout->addRow(new QLabel("Direction (x,y,z):"), vectorWidget);
    
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
    connect(m_okButton, &QPushButton::clicked, this, &LoadPropertyWidget::onOkClicked);
    okButtonLayout->addWidget(m_okButton);
    
    layout->addRow("", okButtonContainer);
    
    // surface_idが変更されたらOKボタンの状態も更新
    connect(m_surfaceIdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &LoadPropertyWidget::updateOkButtonState);
}

void LoadPropertyWidget::updateData()
{
    if (!m_uiState || m_currentIndex < 0) return;
    
    auto bc = m_uiState->getBoundaryCondition();
    if (m_currentIndex >= (int)bc.loads.size()) return;
    
    const auto& l = bc.loads[m_currentIndex];
    
    bool oldBlock = blockSignals(true);
    
    m_nameEdit->blockSignals(true);
    m_nameEdit->setText(QString::fromStdString(l.name));
    m_nameEdit->blockSignals(false);
    
    m_surfaceIdSpinBox->blockSignals(true);
    m_surfaceIdSpinBox->setValue(l.surface_id);
    m_surfaceIdSpinBox->blockSignals(false);
    
    m_magnitudeSpinBox->blockSignals(true);
    m_magnitudeSpinBox->setValue(l.magnitude);
    m_magnitudeSpinBox->blockSignals(false);
    
    // Helper to block and set
    auto setVal = [](QDoubleSpinBox* sb, double val) {
        sb->blockSignals(true);
        sb->setValue(val);
        sb->blockSignals(false);
    };
    
    setVal(m_dirXSpinBox, l.direction.x);
    setVal(m_dirYSpinBox, l.direction.y);
    setVal(m_dirZSpinBox, l.direction.z);
    
    blockSignals(oldBlock);
    
    // OKボタンの状態を更新
    updateOkButtonState();
}

void LoadPropertyWidget::pushData()
{
    if (!m_uiState || m_currentIndex < 0) return;
    
    auto bc = m_uiState->getBoundaryCondition();
    if (m_currentIndex >= (int)bc.loads.size()) return;
    
    LoadCondition l = bc.loads[m_currentIndex];
    l.name = m_nameEdit->text().toStdString();
    l.surface_id = m_surfaceIdSpinBox->value();
    l.magnitude = m_magnitudeSpinBox->value();
    l.direction.x = m_dirXSpinBox->value();
    l.direction.y = m_dirYSpinBox->value();
    l.direction.z = m_dirZSpinBox->value();
    
    // Update via UIState
    // Command pattern: Update load
    auto command = std::make_unique<UpdateLoadConditionCommand>(
        m_uiState,
        m_currentIndex,
        l
    );
    command->execute();
}

void LoadPropertyWidget::onOkClicked()
{
    if (m_uiState) {
        // 選択状態をクリアして何も選択されていない状態に戻す
        m_uiState->setSelectedObject(ObjectType::NONE);
    }
    emit okClicked();
}

void LoadPropertyWidget::updateOkButtonStyle()
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

void LoadPropertyWidget::updateOkButtonState()
{
    if (!m_okButton || !m_surfaceIdSpinBox) return;
    
    // surface_idが0以外の場合にOKボタンを有効化
    bool hasValidSurface = m_surfaceIdSpinBox->value() > 0;
    m_okButton->setEnabled(hasValidSurface);
    updateOkButtonStyle();
}
