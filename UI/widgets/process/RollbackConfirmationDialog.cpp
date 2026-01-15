#include "RollbackConfirmationDialog.h"
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
    setupUI();
}

void RollbackConfirmationDialog::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

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

    // Simple styling
    m_okButton->setStyleSheet("QPushButton { padding: 5px 15px; }");
    m_cancelButton->setStyleSheet("QPushButton { padding: 5px 15px; }");
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
