#include "AddLoadDialog.h"
#include "../../ColorManager.h"
#include "../../visualization/VisualizationManager.h"
#include "../../../core/processing/StepReader.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QIntValidator>
#include <QDoubleValidator>

AddLoadDialog::AddLoadDialog(const QString& defaultName, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Add Load");
    setModal(false);
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    
    // Initialize default load
    m_currentLoad.name = defaultName.toStdString();
    m_currentLoad.surface_id = 0;
    m_currentLoad.magnitude = 10.0;
    m_currentLoad.direction = {0, 0, -1};
    m_currentLoad.reference_edge_id = 0;

    setupUI();
    m_nameEdit->setText(defaultName);
}

AddLoadDialog::~AddLoadDialog()
{
    if (m_isSelectingEdge) {
        cancelEdgeSelection();
    }
    // Clear preview when dialog is closed
    if (m_vizManager) {
        m_vizManager->clearPreview();
    }
    enableFaceSelectionMode(false);
}

void AddLoadDialog::setVisualizationManager(VisualizationManager* vizManager)
{
    // Disconnect previous connections if any
    if (m_vizManager) {
        disconnect(m_vizManager, &VisualizationManager::faceDoubleClicked, this, nullptr);
        disconnect(m_vizManager, &VisualizationManager::edgeClicked, this, nullptr);
    }

    m_vizManager = vizManager;

    // Connect signals
    if (m_vizManager) {
        connect(m_vizManager, &VisualizationManager::faceDoubleClicked,
                this, &AddLoadDialog::onFaceDoubleClicked);
        connect(m_vizManager, &VisualizationManager::edgeClicked,
                this, &AddLoadDialog::onEdgeSelected);
        enableFaceSelectionMode(true);
    }
}

void AddLoadDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(12);

    // Labels style - transparent background
    QString labelStyle = "color: #aaaaaa; background-color: transparent;";
    QString inputStyle = QString("QLineEdit { color: %1; background-color: %2; border: 1px solid %3; padding: 4px; border-radius: 4px; min-height: 20px; }")
        .arg(ColorManager::INPUT_TEXT_COLOR.name())
        .arg(ColorManager::INPUT_BACKGROUND_COLOR.name())
        .arg(ColorManager::INPUT_BORDER_COLOR.name());
    QString buttonStyle = "color: white; background-color: #444; border: 1px solid #666; padding: 6px; border-radius: 3px;";

    // Form layout for fields
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    // Name field
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setStyleSheet(inputStyle);
    m_nameEdit->setFixedWidth(150);
    QLabel* nameLabel = new QLabel("Name:", this);
    nameLabel->setStyleSheet(labelStyle);
    formLayout->addRow(nameLabel, m_nameEdit);

    // Surface ID field
    m_surfaceIdEdit = new QLineEdit(this);
    m_surfaceIdEdit->setValidator(new QIntValidator(0, 99999, this));
    m_surfaceIdEdit->setStyleSheet(inputStyle);
    m_surfaceIdEdit->setFixedWidth(150);
    m_surfaceIdEdit->setText("0");
    QLabel* surfaceLabel = new QLabel("Surface ID:", this);
    surfaceLabel->setStyleSheet(labelStyle);
    formLayout->addRow(surfaceLabel, m_surfaceIdEdit);

    // Magnitude field with unit
    QWidget* valueContainer = new QWidget(this);
    QString containerStyle = QString(".QWidget { color: %1; background-color: %2; border: 1px solid %3; padding: 4px; border-radius: 3px; min-height: 20px; }")
        .arg(ColorManager::INPUT_TEXT_COLOR.name())
        .arg(ColorManager::INPUT_BACKGROUND_COLOR.name())
        .arg(ColorManager::INPUT_BORDER_COLOR.name());
    valueContainer->setStyleSheet(containerStyle);
    valueContainer->setFixedWidth(150);
    QHBoxLayout* valueLayout = new QHBoxLayout(valueContainer);
    valueLayout->setContentsMargins(0, 0, 10, 0);
    valueLayout->setSpacing(2);

    m_magnitudeEdit = new QLineEdit(this);
    m_magnitudeEdit->setValidator(new QDoubleValidator(-1e9, 1e9, 3, this));
    m_magnitudeEdit->setStyleSheet("QLineEdit { background: transparent; border: none; color: white; padding: 0px; }");
    m_magnitudeEdit->setText("10.0");

    QLabel* unitLabel = new QLabel("N", this);
    unitLabel->setStyleSheet("color: #aaaaaa; border: none; background: transparent; font-size: 12px;");

    valueLayout->addWidget(m_magnitudeEdit);
    valueLayout->addWidget(unitLabel);

    QLabel* valueLabel = new QLabel("Value:", this);
    valueLabel->setStyleSheet(labelStyle);
    formLayout->addRow(valueLabel, valueContainer);

    // Reference Edge Selection
    QWidget* edgeWidget = new QWidget(this);
    QHBoxLayout* edgeLayout = new QHBoxLayout(edgeWidget);
    edgeLayout->setContentsMargins(0, 0, 0, 0);
    edgeLayout->setSpacing(5);

    m_referenceEdgeButton = new QPushButton("Select Edge", this);
    m_referenceEdgeButton->setStyleSheet(buttonStyle);
    m_referenceEdgeButton->setFixedWidth(100);
    connect(m_referenceEdgeButton, &QPushButton::clicked,
            this, &AddLoadDialog::onReferenceEdgeButtonClicked);
    edgeLayout->addWidget(m_referenceEdgeButton);

    m_selectedEdgeLabel = new QLabel("-", this);
    m_selectedEdgeLabel->setStyleSheet(labelStyle);
    edgeLayout->addWidget(m_selectedEdgeLabel);
    edgeLayout->addStretch();

    QLabel* edgeLabel = new QLabel("Reference Edge:", this);
    edgeLabel->setStyleSheet(labelStyle);
    formLayout->addRow(edgeLabel, edgeWidget);

    // Direction Display
    m_directionDisplay = new QLabel("(0.000, 0.000, -1.000)", this);
    m_directionDisplay->setStyleSheet(labelStyle);
    QLabel* dirLabel = new QLabel("Direction:", this);
    dirLabel->setStyleSheet(labelStyle);
    formLayout->addRow(dirLabel, m_directionDisplay);

    mainLayout->addLayout(formLayout);

    // Hint label
    QLabel* hintLabel = new QLabel("ヒント: 面をダブルクリックするとSurface IDと方向が設定されます。\nエッジを選択すると方向ベクトルを設定できます。", this);
    hintLabel->setWordWrap(true);
    hintLabel->setStyleSheet("color: #888888; font-size: 11px; background-color: transparent;");
    mainLayout->addWidget(hintLabel);

    mainLayout->addStretch();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setFixedWidth(80);
    m_cancelButton->setStyleSheet(
        "QPushButton { background-color: #444; color: white; border: 1px solid #666; "
        "padding: 8px 16px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #555; }"
        "QPushButton:pressed { background-color: #333; }"
    );
    connect(m_cancelButton, &QPushButton::clicked, this, [this]() {
        if (m_vizManager) {
            m_vizManager->clearPreview();
        }
        reject();
    });
    buttonLayout->addWidget(m_cancelButton);

    m_okButton = new QPushButton("OK", this);
    m_okButton->setFixedWidth(80);
    m_okButton->setStyleSheet(
        QString("QPushButton { background-color: %1; color: %2; border: none; "
                "padding: 8px 16px; border-radius: 4px; font-weight: bold; }"
                "QPushButton:hover { background-color: %3; }"
                "QPushButton:pressed { background-color: %4; }")
        .arg(ColorManager::ACCENT_COLOR.name())
        .arg(ColorManager::BUTTON_TEXT_COLOR.name())
        .arg(ColorManager::BUTTON_HOVER_COLOR.name())
        .arg(ColorManager::BUTTON_PRESSED_COLOR.name())
    );
    connect(m_okButton, &QPushButton::clicked, this, [this]() {
        if (m_vizManager) {
            m_vizManager->clearPreview();
        }
        accept();
    });
    buttonLayout->addWidget(m_okButton);

    mainLayout->addLayout(buttonLayout);

    setMinimumWidth(350);
    setStyleSheet("QDialog { background-color: #2d2d2d; }");
}

void AddLoadDialog::enableFaceSelectionMode(bool enable)
{
    if (m_vizManager) {
        m_vizManager->setFaceSelectionMode(enable);
    }
}

void AddLoadDialog::onFaceDoubleClicked(int faceId, double nx, double ny, double nz)
{
    m_surfaceIdEdit->setText(QString::number(faceId));

    // Set direction as inverse of face normal
    m_currentLoad.direction = {-nx, -ny, -nz};
    m_currentLoad.reference_edge_id = 0;
    m_selectedEdgeLabel->setText("-");
    m_directionDisplay->setText(QString("(%1, %2, %3)")
        .arg(-nx, 0, 'f', 3)
        .arg(-ny, 0, 'f', 3)
        .arg(-nz, 0, 'f', 3));

    // Show preview
    if (m_vizManager) {
        m_vizManager->showLoadPreview(faceId, -nx, -ny, -nz);
    }
}

void AddLoadDialog::onReferenceEdgeButtonClicked()
{
    if (m_isSelectingEdge) {
        // Cancel edge selection
        cancelEdgeSelection();
    } else {
        // Start edge selection
        m_isSelectingEdge = true;
        m_referenceEdgeButton->setText("Cancel");

        if (m_vizManager) {
            m_vizManager->setFaceSelectionMode(false);
            m_vizManager->setEdgeSelectionMode(true);
        }
    }
}

void AddLoadDialog::onEdgeSelected(int edgeId)
{
    if (!m_isSelectingEdge) return;

    updateDirectionFromEdge(edgeId);
    cancelEdgeSelection();
}

void AddLoadDialog::updateDirectionFromEdge(int edgeId)
{
    if (!m_vizManager) return;

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

    // Update stored load data
    m_currentLoad.reference_edge_id = edgeId;
    m_currentLoad.direction.x = edgeGeom.dirX;
    m_currentLoad.direction.y = edgeGeom.dirY;
    m_currentLoad.direction.z = edgeGeom.dirZ;

    // Update preview with new direction
    int surfaceId = m_surfaceIdEdit->text().toInt();
    if (m_vizManager && surfaceId > 0) {
        m_vizManager->showLoadPreview(surfaceId, edgeGeom.dirX, edgeGeom.dirY, edgeGeom.dirZ);
    }
}

void AddLoadDialog::cancelEdgeSelection()
{
    m_isSelectingEdge = false;
    m_referenceEdgeButton->setText("Select Edge");
    m_referenceEdgeButton->setStyleSheet(
        "color: white; background-color: #444; border: 1px solid #666; "
        "padding: 6px; border-radius: 3px;");

    if (m_vizManager) {
        m_vizManager->setEdgeSelectionMode(false);
        m_vizManager->setFaceSelectionMode(true);
    }
}

LoadCondition AddLoadDialog::getLoadCondition() const
{
    LoadCondition load = m_currentLoad;
    load.name = m_nameEdit->text().toStdString();
    load.surface_id = m_surfaceIdEdit->text().toInt();
    load.magnitude = m_magnitudeEdit->text().toDouble();
    return load;
}
