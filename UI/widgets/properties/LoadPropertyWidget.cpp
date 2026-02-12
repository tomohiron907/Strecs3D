#include "LoadPropertyWidget.h"
#include "../../../core/commands/state/UpdateLoadConditionCommand.h"
#include "../../../utils/ColorManager.h"
#include "../../../utils/StyleManager.h"
#include "../../visualization/VisualizationManager.h"
#include "../../../core/processing/StepReader.h"
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QIntValidator>
#include <QDoubleValidator>

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
    layout->setSpacing(StyleManager::FORM_SPACING);
    layout->setContentsMargins(StyleManager::FORM_SPACING, StyleManager::FORM_SPACING,
                               StyleManager::FORM_SPACING, StyleManager::FORM_SPACING);

    // Read-only hint label (initially hidden)
    m_readOnlyHintLabel = new QLabel("Go back to Step 2 to edit.");
    m_readOnlyHintLabel->setStyleSheet(QString("color: %1; font-size: %2px; padding: 2px 4px;")
        .arg(ColorManager::ACCENT_COLOR.name())
        .arg(StyleManager::FONT_SIZE_SMALL));
    m_readOnlyHintLabel->setVisible(false);
    layout->addRow(m_readOnlyHintLabel);

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
    connect(m_nameEdit, &QLineEdit::editingFinished, this, &LoadPropertyWidget::pushData);
    layout->addRow(new QLabel("Name:"), m_nameEdit);
    
    // Surface ID
    m_surfaceIdEdit = new QLineEdit();
    m_surfaceIdEdit->setValidator(new QIntValidator(0, 99999, this));
    m_surfaceIdEdit->setStyleSheet(inputStyle);
    m_surfaceIdEdit->setFixedWidth(100);
    connect(m_surfaceIdEdit, &QLineEdit::editingFinished, this, &LoadPropertyWidget::pushData);
    layout->addRow(new QLabel("Surface ID:"), m_surfaceIdEdit);
    
    // Magnitude (Value) with Unit "N"
    QWidget* valueContainer = new QWidget();
    // Build style for container using ColorManager constants to match QLineEdit look
    QString containerStyle = QString(".QWidget { color: %1; background-color: %2; border: 1px solid %3; padding: %4px; border-radius: %5px; min-height: %6px; }")
        .arg(ColorManager::INPUT_TEXT_COLOR.name())
        .arg(ColorManager::INPUT_BACKGROUND_COLOR.name())
        .arg(ColorManager::INPUT_BORDER_COLOR.name())
        .arg(StyleManager::PADDING_SMALL)
        .arg(StyleManager::RADIUS_SMALL)
        .arg(StyleManager::INPUT_HEIGHT_SMALL);
    valueContainer->setStyleSheet(containerStyle);
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
    unitLabel->setStyleSheet(QString("color: #aaaaaa; border: none; background: transparent; font-size: %1px;")
        .arg(StyleManager::FONT_SIZE_NORMAL));
    
    valueLayout->addWidget(m_magnitudeEdit);
    valueLayout->addWidget(unitLabel);

    layout->addRow(new QLabel("Value:"), valueContainer);

    // Reference Edge Selection
    QString buttonStyle = QString("color: white; background-color: #444; border: 1px solid #666; padding: %1px; border-radius: %2px;")
        .arg(StyleManager::PADDING_MEDIUM)
        .arg(StyleManager::RADIUS_SMALL);

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

    // Reverse Checkbox
    m_reverseCheckBox = new QCheckBox(this);
    m_reverseCheckBox->setStyleSheet("QCheckBox { background-color: transparent; }");
    connect(m_reverseCheckBox, &QCheckBox::toggled, this, &LoadPropertyWidget::onReverseDirectionToggled);
    
    layout->addRow(new QLabel("Reverse Direction:"), m_reverseCheckBox);

    // Direction Display (read-only)
    m_directionDisplay = new QLabel("(0.000, 0.000, 0.000)");
    m_directionDisplay->setStyleSheet(labelStyle); // Just text style, no border
    layout->addRow(new QLabel("Direction:"), m_directionDisplay);
    
    // Apply label style
    for(int i = 0; i < layout->rowCount(); ++i) {
        QLayoutItem* item = layout->itemAt(i, QFormLayout::LabelRole);
        if (item && item->widget()) {
            item->widget()->setStyleSheet(labelStyle);
        }
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
    connect(m_closeButton, &QPushButton::clicked, this, &LoadPropertyWidget::onCloseClicked);
    closeButtonLayout->addWidget(m_closeButton);

    layout->addRow("", closeButtonContainer);
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
        
    // Reset reverse checkbox to false when loading (user can toggle it to flip relative to current)
    bool oldCheckBlock = m_reverseCheckBox->blockSignals(true);
    m_reverseCheckBox->setChecked(false);
    m_reverseCheckBox->blockSignals(oldCheckBlock);

    blockSignals(oldBlock);
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

void LoadPropertyWidget::setReadOnly(bool readOnly)
{
    m_nameEdit->setReadOnly(readOnly);
    m_surfaceIdEdit->setReadOnly(readOnly);
    m_magnitudeEdit->setReadOnly(readOnly);
    m_referenceEdgeButton->setEnabled(!readOnly);
    m_reverseCheckBox->setEnabled(!readOnly);
    m_readOnlyHintLabel->setVisible(readOnly);

    QString inputStyle;
    if (readOnly) {
        inputStyle = QString("QLineEdit { color: #666666; background-color: #1a1a1a; border: 1px solid #333333; padding: %1px; border-radius: %2px; min-height: %3px; }")
            .arg(StyleManager::PADDING_SMALL)
            .arg(StyleManager::RADIUS_SMALL)
            .arg(StyleManager::INPUT_HEIGHT_SMALL);
    } else {
        inputStyle = QString("QLineEdit { color: %1; background-color: %2; border: 1px solid %3; padding: %4px; border-radius: %5px; min-height: %6px; selection-background-color: #555555; }")
            .arg(ColorManager::INPUT_TEXT_COLOR.name())
            .arg(ColorManager::INPUT_BACKGROUND_COLOR.name())
            .arg(ColorManager::INPUT_BORDER_COLOR.name())
            .arg(StyleManager::PADDING_SMALL)
            .arg(StyleManager::RADIUS_SMALL)
            .arg(StyleManager::INPUT_HEIGHT_SMALL);
    }
    m_nameEdit->setStyleSheet(inputStyle);
    m_surfaceIdEdit->setStyleSheet(inputStyle);
    // m_magnitudeEdit has transparent background inside its container, so style differently
    if (readOnly) {
        m_magnitudeEdit->setStyleSheet("QLineEdit { background: transparent; border: none; color: #666666; padding: 0px; }");
    } else {
        m_magnitudeEdit->setStyleSheet("QLineEdit { background: transparent; border: none; color: white; padding: 0px; }");
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

    l.direction.z = edgeGeom.dirZ;

    auto command = std::make_unique<UpdateLoadConditionCommand>(
        m_uiState, m_currentIndex, l);
    command->execute();

    // Reset reverse checkbox
    bool oldBlock = m_reverseCheckBox->blockSignals(true);
    m_reverseCheckBox->setChecked(false);
    m_reverseCheckBox->blockSignals(oldBlock);
}

void LoadPropertyWidget::cancelEdgeSelection()
{
    m_isSelectingEdge = false;
    m_referenceEdgeButton->setText("Select Edge");
    m_referenceEdgeButton->setStyleSheet(
        QString("color: white; background-color: #444; border: 1px solid #666; "
        "padding: %1px; border-radius: %2px;")
        .arg(StyleManager::PADDING_MEDIUM)
        .arg(StyleManager::RADIUS_SMALL));

    if (m_vizManager) {
        m_vizManager->setEdgeSelectionMode(false);
        // Load編集中なので、面選択モードに戻す
        m_vizManager->setFaceSelectionMode(true);
    }
}

void LoadPropertyWidget::onCloseClicked()
{
    if (m_isSelectingEdge) {
        cancelEdgeSelection();
    }

    if (m_uiState) {
        m_uiState->setSelectedObject(ObjectType::NONE);
    }
    emit closeClicked();
}

void LoadPropertyWidget::onReverseDirectionToggled(bool checked)
{
    if (!m_uiState || m_currentIndex < 0) return;
    
    auto bc = m_uiState->getBoundaryCondition();
    if (m_currentIndex >= (int)bc.loads.size()) return;
    
    LoadCondition l = bc.loads[m_currentIndex];
    
    // Invert direction
    l.direction.x = -l.direction.x;
    l.direction.y = -l.direction.y;
    l.direction.z = -l.direction.z;
    
    // Update model (this will trigger updates, but we also update local UI for responsiveness)
    auto command = std::make_unique<UpdateLoadConditionCommand>(
        m_uiState, m_currentIndex, l);
    command->execute();
    
    // Direction display will be updated by updateData() if signals are connected properly,
    // or we can force update text here just in case (updateData resets the checkbox which might cycle)
    // Actually, UpdateLoadConditionCommand will trigger UIState update, which might trigger MainWindow refresh,
    // which might call setTarget again.
    // If setTarget is called, updateData is called, which resets checkbox to false.
    // This is desired: we flipped it, now the new state is "Force", and if we check it again, it flips again.
}
