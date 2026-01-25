#include "AddConstraintDialog.h"
#include "../../../utils/ColorManager.h"
#include "../../../utils/StyleManager.h"
#include "../../visualization/VisualizationManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QIntValidator>

AddConstraintDialog::AddConstraintDialog(const QString& defaultName, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Add Constraint");
    setModal(false);
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    setupUI();
    m_nameEdit->setText(defaultName);
}

AddConstraintDialog::~AddConstraintDialog()
{
    // Clear preview when dialog is closed
    if (m_vizManager) {
        m_vizManager->clearPreview();
    }
    enableFaceSelectionMode(false);
}

void AddConstraintDialog::setVisualizationManager(VisualizationManager* vizManager)
{
    // Disconnect previous connections if any
    if (m_vizManager) {
        disconnect(m_vizManager, &VisualizationManager::faceDoubleClicked, this, nullptr);
    }

    m_vizManager = vizManager;

    // Connect face selection signal
    if (m_vizManager) {
        connect(m_vizManager, &VisualizationManager::faceDoubleClicked,
                this, &AddConstraintDialog::onFaceDoubleClicked);
        enableFaceSelectionMode(true);
    }
}

void AddConstraintDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(StyleManager::PADDING_LARGE, StyleManager::PADDING_LARGE,
                                   StyleManager::PADDING_LARGE, StyleManager::PADDING_LARGE);
    mainLayout->setSpacing(StyleManager::FORM_SPACING);

    // Labels style - transparent background
    QString labelStyle = "color: #aaaaaa; background-color: transparent;";
    QString inputStyle = QString("QLineEdit { color: %1; background-color: %2; border: 1px solid %3; padding: %4px; border-radius: %5px; min-height: %6px; }")
        .arg(ColorManager::INPUT_TEXT_COLOR.name())
        .arg(ColorManager::INPUT_BACKGROUND_COLOR.name())
        .arg(ColorManager::INPUT_BORDER_COLOR.name())
        .arg(StyleManager::PADDING_SMALL)
        .arg(StyleManager::RADIUS_SMALL)
        .arg(StyleManager::INPUT_HEIGHT_SMALL);

    // Form layout for fields
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(StyleManager::FORM_SPACING);

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

    mainLayout->addLayout(formLayout);

    // Hint label
    QLabel* hintLabel = new QLabel("Hint: Double-click a face to set the Surface ID.", this);
    hintLabel->setWordWrap(true);
    hintLabel->setStyleSheet(QString("color: #888888; font-size: %1px; background-color: transparent;")
        .arg(StyleManager::FONT_SIZE_SMALL));
    mainLayout->addWidget(hintLabel);

    mainLayout->addStretch();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setFixedWidth(80);
    m_cancelButton->setStyleSheet(
        QString("QPushButton { background-color: #444; color: white; border: 1px solid #666; "
        "padding: %1px %2px; border-radius: %3px; }"
        "QPushButton:hover { background-color: #555; }"
        "QPushButton:pressed { background-color: #333; }")
        .arg(StyleManager::BUTTON_PADDING_V)
        .arg(StyleManager::BUTTON_PADDING_H)
        .arg(StyleManager::BUTTON_RADIUS)
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

void AddConstraintDialog::enableFaceSelectionMode(bool enable)
{
    if (m_vizManager) {
        m_vizManager->setFaceSelectionMode(enable);
    }
}

void AddConstraintDialog::onFaceDoubleClicked(int faceId, double nx, double ny, double nz)
{
    Q_UNUSED(nx);
    Q_UNUSED(ny);
    Q_UNUSED(nz);

    m_surfaceIdEdit->setText(QString::number(faceId));

    // Show preview
    if (m_vizManager) {
        m_vizManager->showConstraintPreview(faceId);
    }
}

ConstraintCondition AddConstraintDialog::getConstraintCondition() const
{
    ConstraintCondition constraint;
    constraint.name = m_nameEdit->text().toStdString();
    constraint.surface_id = m_surfaceIdEdit->text().toInt();
    return constraint;
}
