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
    setWindowTitle("Confirm Process Rollback");
    setupUI();
}

void RollbackConfirmationDialog::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    // Warning icon + message
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

    // Style
    m_okButton->setStyleSheet("QPushButton { background-color: #ff6666; color: white; padding: 5px 15px; }");
    m_cancelButton->setStyleSheet("QPushButton { padding: 5px 15px; }");
    setMinimumWidth(450);
}

QString RollbackConfirmationDialog::generateWarningMessage(
    ProcessStep target, ProcessStep current
) {
    QString message = "<b style='font-size: 14px;'>Warning: This action will reset the following processes:</b><br><br>";

    int targetIdx = static_cast<int>(target);
    int currentIdx = static_cast<int>(current);

    // List processes that will be reset
    QStringList resetSteps;
    if (currentIdx >= static_cast<int>(ProcessStep::InfillMap) && targetIdx < 3) {
        resetSteps << "<li><b>4. Build Infill Map</b> - All infill regions and divided meshes will be removed</li>";
    }
    if (currentIdx >= static_cast<int>(ProcessStep::Simulate) && targetIdx < 2) {
        resetSteps << "<li><b>3. Simulate</b> - Simulation results will be deleted</li>";
    }
    if (currentIdx >= static_cast<int>(ProcessStep::BoundaryCondition) && targetIdx < 1) {
        resetSteps << "<li><b>2. Boundary Conditions</b> - All loads and constraints will be removed</li>";
    }
    if (targetIdx == 0) {
        resetSteps << "<li><b>1. Import STEP File</b> - The STEP file and all associated data will be removed</li>";
    }

    if (!resetSteps.isEmpty()) {
        message += "<ul style='margin-left: 15px;'>";
        message += resetSteps.join("");
        message += "</ul>";
    }

    message += "<br><b style='color: #ff6666;'>This action cannot be undone. Do you want to continue?</b>";

    return message;
}
