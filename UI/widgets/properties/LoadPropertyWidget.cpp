#include "LoadPropertyWidget.h"
#include "../../../core/commands/state/UpdateLoadConditionCommand.h"
#include "../../ColorManager.h"
#include "../../visualization/VisualizationManager.h"
#include "../../../core/processing/StepReader.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QDebug>

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
    // Updated input style: unified width, rounded corners, min-height to prevent collapse
    QString inputStyle = "color: white; background-color: #333; border: 1px solid #555; padding: 4px; border-radius: 4px; min-height: 20px;";
    
    // Name
    m_nameEdit = new QLineEdit();
    m_nameEdit->setStyleSheet(inputStyle);
    m_nameEdit->setFixedWidth(100);
    connect(m_nameEdit, &QLineEdit::editingFinished, this, &LoadPropertyWidget::pushData);
    layout->addRow(new QLabel("Name:"), m_nameEdit);
    
    // Surface ID
    m_surfaceIdEdit = new QLineEdit();
    m_surfaceIdEdit->setValidator(new QIntValidator(0, 99999, this));
    m_surfaceIdEdit->setStyleSheet(inputStyle);
    m_surfaceIdEdit->setFixedWidth(100);
    connect(m_surfaceIdEdit, &QLineEdit::editingFinished, this, &LoadPropertyWidget::pushData);
    connect(m_surfaceIdEdit, &QLineEdit::textChanged, this, &LoadPropertyWidget::updateOkButtonState); // Update OK button immediately on type
    layout->addRow(new QLabel("Surface ID:"), m_surfaceIdEdit);
    
    // Magnitude (Value) with Unit "N"
    QWidget* valueContainer = new QWidget();
    valueContainer->setStyleSheet(".QWidget { " + inputStyle + " }"); // Apply input style to container
    valueContainer->setFixedWidth(100);
    QHBoxLayout* valueLayout = new QHBoxLayout(valueContainer);
    valueLayout->setContentsMargins(0, 0, 10, 0); // Right margin for 'N'
    valueLayout->setSpacing(2);

    m_magnitudeEdit = new QLineEdit();
    m_magnitudeEdit->setValidator(new QDoubleValidator(-1e9, 1e9, 3, this));
    // Transparent background/border for inner edit, text color white
    m_magnitudeEdit->setStyleSheet("QLineEdit { background: transparent; border: none; color: white; padding: 0px; }");
    connect(m_magnitudeEdit, &QLineEdit::editingFinished, this, &LoadPropertyWidget::pushData);
    
    QLabel* unitLabel = new QLabel("N");
    unitLabel->setStyleSheet("color: #aaaaaa; border: none; background: transparent; font-size: 12px;");
    
    valueLayout->addWidget(m_magnitudeEdit);
    valueLayout->addWidget(unitLabel);

    layout->addRow(new QLabel("Value:"), valueContainer);

    // Reference Edge Selection
    QString buttonStyle = "color: white; background-color: #444; border: 1px solid #666; padding: 6px; border-radius: 3px;";

    QWidget* edgeWidget = new QWidget();
    QHBoxLayout* edgeLayout = new QHBoxLayout(edgeWidget);
    edgeLayout->setContentsMargins(0, 0, 0, 0);
    edgeLayout->setSpacing(5);

    m_referenceEdgeButton = new QPushButton("Select Edge");
    m_referenceEdgeButton->setStyleSheet(buttonStyle);
    m_referenceEdgeButton->setFixedWidth(100);
    connect(m_referenceEdgeButton, &QPushButton::clicked,
            this, &LoadPropertyWidget::onReferenceEdgeButtonClicked);
    edgeLayout->addWidget(m_referenceEdgeButton);

    m_selectedEdgeLabel = new QLabel("-");
    m_selectedEdgeLabel->setStyleSheet(labelStyle);  // Green
    edgeLayout->addWidget(m_selectedEdgeLabel);
    edgeLayout->addStretch();

    layout->addRow(new QLabel("Reference Edge:"), edgeWidget);

    // Direction Display (read-only)
    // Direction Display (simple numeric display)
    m_directionDisplay = new QLabel("(0.000, 0.000, 0.000)");
    m_directionDisplay->setStyleSheet(labelStyle); // Just text style, no border
    layout->addRow(new QLabel("Direction:"), m_directionDisplay);
    
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
    // surface_idが変更されたらOKボタンの状態も更新
    // connected in m_surfaceIdEdit connection above
    // connect(m_surfaceIdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &LoadPropertyWidget::updateOkButtonState);
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
    
    m_surfaceIdEdit->blockSignals(true);
    m_surfaceIdEdit->setText(QString::number(l.surface_id));
    m_surfaceIdEdit->blockSignals(false);
    
    m_magnitudeEdit->blockSignals(true);
    m_magnitudeEdit->setText(QString::number(l.magnitude));
    m_magnitudeEdit->blockSignals(false);

    // Update edge selection display
    if (l.reference_edge_id > 0) {
        m_selectedEdgeLabel->setText(QString("Edge %1").arg(l.reference_edge_id));
    } else {
        m_selectedEdgeLabel->setText("-");
    }

    // Update direction display
    m_directionDisplay->setText(QString("(%1, %2, %3)")
        .arg(l.direction.x, 0, 'f', 3)
        .arg(l.direction.y, 0, 'f', 3)
        .arg(l.direction.z, 0, 'f', 3));

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
    l.surface_id = m_surfaceIdEdit->text().toInt();
    l.magnitude = m_magnitudeEdit->text().toDouble();

    // Note: direction and reference_edge_id are set by updateDirectionFromEdge()
    // or by face click in MainWindow::onFaceClicked()
    // We don't modify them here

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
    // Cancel edge selection if active
    if (m_isSelectingEdge) {
        cancelEdgeSelection();
    }

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
                    "QPushButton:hover { background-color: %4; color: %6; }"
                    "QPushButton:pressed { background-color: %5; }")
            .arg(ColorManager::BUTTON_EMPHASIZED_COLOR.name())
            .arg(ColorManager::BUTTON_COLOR.name())
            .arg(ColorManager::BUTTON_EDGE_COLOR.name())
            .arg(ColorManager::BUTTON_HOVER_COLOR.name())
            .arg(ColorManager::BUTTON_PRESSED_COLOR.name())
            .arg(ColorManager::BUTTON_TEXT_COLOR.name())
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
    if (!m_okButton || !m_surfaceIdEdit) return;

    // surface_idが0以外の場合にOKボタンを有効化
    bool hasValidSurface = m_surfaceIdEdit->text().toInt() > 0;
    m_okButton->setEnabled(hasValidSurface);
    updateOkButtonStyle();
}

void LoadPropertyWidget::setVisualizationManager(VisualizationManager* vizManager)
{
    // Disconnect previous connections if any
    if (m_vizManager) {
        disconnect(m_vizManager, &VisualizationManager::edgeClicked, this, nullptr);
    }

    m_vizManager = vizManager;

    // Connect edge selection signal
    if (m_vizManager) {
        connect(m_vizManager, &VisualizationManager::edgeClicked,
                this, &LoadPropertyWidget::onEdgeSelected);
    }
}

void LoadPropertyWidget::onReferenceEdgeButtonClicked()
{
    if (m_isSelectingEdge) {
        // Cancel edge selection
        cancelEdgeSelection();
    } else {
        // Start edge selection
        m_isSelectingEdge = true;
        m_referenceEdgeButton->setText("Cancel");

        if (m_vizManager) {
            m_vizManager->setEdgeSelectionMode(true);
        }
    }
}

void LoadPropertyWidget::onEdgeSelected(int edgeId)
{
    if (!m_isSelectingEdge) return;

    updateDirectionFromEdge(edgeId);
    cancelEdgeSelection();
}

void LoadPropertyWidget::updateDirectionFromEdge(int edgeId)
{
    if (!m_vizManager || !m_uiState) return;

    auto stepReader = m_vizManager->getCurrentStepReader();
    if (!stepReader) {
        m_selectedEdgeLabel->setText("Error: No model");
        return;
    }

    EdgeGeometry edgeGeom = stepReader->getEdgeGeometry(edgeId);
    if (!edgeGeom.isValid) {
        m_selectedEdgeLabel->setText("Error: Invalid edge");
        return;
    }

    // Update UI
    m_selectedEdgeLabel->setText(QString("Edge %1").arg(edgeId));
    m_directionDisplay->setText(QString("(%1, %2, %3)")
        .arg(edgeGeom.dirX, 0, 'f', 3)
        .arg(edgeGeom.dirY, 0, 'f', 3)
        .arg(edgeGeom.dirZ, 0, 'f', 3));

    // Update data model
    if (m_currentIndex < 0) return;

    auto bc = m_uiState->getBoundaryCondition();
    if (m_currentIndex >= (int)bc.loads.size()) return;

    LoadCondition l = bc.loads[m_currentIndex];
    l.reference_edge_id = edgeId;
    l.direction.x = edgeGeom.dirX;
    l.direction.y = edgeGeom.dirY;
    l.direction.z = edgeGeom.dirZ;

    auto command = std::make_unique<UpdateLoadConditionCommand>(
        m_uiState, m_currentIndex, l);
    command->execute();
}

void LoadPropertyWidget::cancelEdgeSelection()
{
    m_isSelectingEdge = false;
    m_referenceEdgeButton->setText("Select Edge");
    m_referenceEdgeButton->setStyleSheet(
        "color: white; background-color: #444; border: 1px solid #666; "
        "padding: 6px; border-radius: 3px;");

    if (m_vizManager) {
        m_vizManager->setEdgeSelectionMode(false);
    }
}
