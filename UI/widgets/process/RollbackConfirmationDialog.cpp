#include "RollbackConfirmationDialog.h"
#include "../../../utils/StyleManager.h"
#include "../../../utils/ColorManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

RollbackConfirmationDialog::RollbackConfirmationDialog(
    ProcessStep targetStep,
    ProcessStep currentStep,
    QWidget* parent
) : QDialog(parent), m_targetStep(targetStep), m_currentStep(currentStep)
{
    setWindowTitle("Confirm");
    setModal(false); // モーダルレスに設定
    // ダイアログを常に最前面に表示することで、マウスイベントが3Dビューに渡されるのを防ぐ
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    setupUI();
}

void RollbackConfirmationDialog::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(StyleManager::PADDING_LARGE + StyleManager::PADDING_SMALL,
                               StyleManager::PADDING_LARGE + StyleManager::PADDING_SMALL,
                               StyleManager::PADDING_LARGE + StyleManager::PADDING_SMALL,
                               StyleManager::PADDING_LARGE + StyleManager::PADDING_SMALL);
    layout->setSpacing(StyleManager::PADDING_LARGE - 1);

    // Message
    m_messageLabel = new QLabel(generateWarningMessage(m_targetStep, m_currentStep));
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setTextFormat(Qt::RichText);
    layout->addWidget(m_messageLabel);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_cancelButton = new QPushButton("Cancel", this);
    m_okButton = new QPushButton("OK", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_okButton);

    layout->addLayout(buttonLayout);

    // Connect buttons
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // Dark theme styling to match other dialogs
    setStyleSheet("QDialog { background-color: #2d2d2d; }");
    m_messageLabel->setStyleSheet("color: #aaaaaa; background-color: transparent;");
    m_cancelButton->setStyleSheet(
        QString("QPushButton { background-color: #444; color: white; border: 1px solid #666; "
        "padding: %1px %2px; border-radius: %3px; }"
        "QPushButton:hover { background-color: #555; }"
        "QPushButton:pressed { background-color: #333; }")
        .arg(StyleManager::BUTTON_PADDING_V)
        .arg(StyleManager::BUTTON_PADDING_H)
        .arg(StyleManager::BUTTON_RADIUS)
    );
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
    setMinimumWidth(400);
}

QString RollbackConfirmationDialog::generateWarningMessage(
    ProcessStep target, ProcessStep current
) {
    QString message = "The following processes will be reset:<br><br>";

    int targetIdx = static_cast<int>(target);
    int currentIdx = static_cast<int>(current);

    // List processes that will be reset (in ascending order: 1, 2, 3, 4)
    QStringList resetSteps;

    if (targetIdx == 0) {
        resetSteps << "• Import STEP File";
    }
    if (currentIdx >= static_cast<int>(ProcessStep::BoundaryCondition) && targetIdx < 1) {
        resetSteps << "• Boundary Conditions";
    }
    if (currentIdx >= static_cast<int>(ProcessStep::Simulate) && targetIdx < 2) {
        resetSteps << "• Simulate";
    }
    if (currentIdx >= static_cast<int>(ProcessStep::InfillMap) && targetIdx < 3) {
        resetSteps << "• Build Infill Map";
    }

    if (!resetSteps.isEmpty()) {
        message += resetSteps.join("<br>");
    }

    message += "<br><br>Continue?";

    return message;
}
