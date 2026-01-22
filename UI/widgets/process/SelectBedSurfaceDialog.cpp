#include "SelectBedSurfaceDialog.h"
#include "../../ColorManager.h"
#include "../../visualization/VisualizationManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QIntValidator>

SelectBedSurfaceDialog::SelectBedSurfaceDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Select Bed Surface");
    setModal(false);
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    setupUI();
}

SelectBedSurfaceDialog::~SelectBedSurfaceDialog()
{
    // Clear preview when dialog is closed
    if (m_vizManager) {
        m_vizManager->clearPreview();
    }
    enableFaceSelectionMode(false);
}

void SelectBedSurfaceDialog::setVisualizationManager(VisualizationManager* vizManager)
{
    // Disconnect previous connections if any
    if (m_vizManager) {
        disconnect(m_vizManager, &VisualizationManager::faceDoubleClicked, this, nullptr);
    }

    m_vizManager = vizManager;

    // Connect face selection signal
    if (m_vizManager) {
        connect(m_vizManager, &VisualizationManager::faceDoubleClicked,
                this, &SelectBedSurfaceDialog::onFaceDoubleClicked);
        enableFaceSelectionMode(true);
    }
}

void SelectBedSurfaceDialog::setupUI()
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

    // Form layout for fields
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    // Surface ID field
    m_surfaceIdEdit = new QLineEdit(this);
    m_surfaceIdEdit->setValidator(new QIntValidator(0, 99999, this));
    m_surfaceIdEdit->setStyleSheet(inputStyle);
    m_surfaceIdEdit->setFixedWidth(150);
    // m_surfaceIdEdit->setText("0"); // Initially empty or 0 is fine, but maybe empty to prompt user? Let's stick with 0 or empty. "0" is safe.
    m_surfaceIdEdit->setPlaceholderText("Double-click a face");
    m_surfaceIdEdit->setReadOnly(true); // User should double click, not type manually usually, but readonly makes sense here to enforce interaction? Or allow typing if they know ID? AddConstraint allows typing. Let's allow typing but make it clear.
    m_surfaceIdEdit->setReadOnly(false); 

    QLabel* surfaceLabel = new QLabel("Selected Bed Face ID:", this);
    surfaceLabel->setStyleSheet(labelStyle);
    formLayout->addRow(surfaceLabel, m_surfaceIdEdit);

    mainLayout->addLayout(formLayout);

    // Hint label
    QLabel* hintLabel = new QLabel("Hint: Double-click a face to select it as the bed surface (bottom).", this);
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

    m_okButton = new QPushButton("Apply", this); // "Apply" might be better than OK for an action
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

    setMinimumWidth(300);
    setStyleSheet("QDialog { background-color: #2d2d2d; }");
}

void SelectBedSurfaceDialog::enableFaceSelectionMode(bool enable)
{
    if (m_vizManager) {
        m_vizManager->setFaceSelectionMode(enable);
    }
}

void SelectBedSurfaceDialog::onFaceDoubleClicked(int faceId, double nx, double ny, double nz)
{
    Q_UNUSED(nx);
    Q_UNUSED(ny);
    Q_UNUSED(nz);

    m_selectedFaceId = faceId;
    m_surfaceIdEdit->setText(QString::number(faceId));

    // Show preview - reusing constraint preview for highlighting the selected face?
    // Or maybe just show it as selected.
    if (m_vizManager) {
        // Highlighting the face as "active" or "preview" would be good.
        m_vizManager->showBedPreview(faceId); 
    }
}

int SelectBedSurfaceDialog::getSelectedFaceId() const
{
    bool ok;
    int id = m_surfaceIdEdit->text().toInt(&ok);
    if (ok) return id;
    return m_selectedFaceId; 
}
