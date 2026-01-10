#include "SimulationStepWidget.h"
#include "../../Button.h"
#include <QVBoxLayout>
#include <QProgressBar>
#include <QLabel>

SimulationStepWidget::SimulationStepWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_simulateButton = new Button("Simulate", this);
    m_simulateButton->setMinimumHeight(50);
    m_simulateButton->setEmphasized(true);
    connect(m_simulateButton, &Button::clicked, this, &SimulationStepWidget::simulateClicked);

    layout->addWidget(m_simulateButton);

    // Progress bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat("%p%");
    m_progressBar->setMinimumHeight(16);
    m_progressBar->setMaximumHeight(16);
    m_progressBar->setVisible(true);  // Always visible

    // Style the progress bar to match app theme
    m_progressBar->setStyleSheet(R"(
        QProgressBar {
            border: 1px solid #444;
            border-radius: 4px;
            background-color: #333;
        }
        QProgressBar::chunk {
            background-color: #0078D7;
            border-radius: 3px;
        }
    )");

    layout->addWidget(m_progressBar);

    // Optional: Status label
    m_statusLabel = new QLabel("", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #aaa; font-size: 11px;");
    m_statusLabel->setVisible(false);
    layout->addWidget(m_statusLabel);

    layout->addStretch();
}

void SimulationStepWidget::setProgress(int value, const QString& message) {
    m_progressBar->setValue(value);
    if (!message.isEmpty() && m_statusLabel) {
        m_statusLabel->setText(message);
    }
}

void SimulationStepWidget::resetProgress() {
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    if (m_statusLabel) {
        m_statusLabel->setText("");
        m_statusLabel->setVisible(false);
    }
}

void SimulationStepWidget::setSimulationRunning(bool running) {
    m_simulateButton->setEnabled(!running);
    if (m_statusLabel) {
        m_statusLabel->setVisible(running);
    }

    if (running) {
        m_simulateButton->setText("Simulating...");
    } else {
        m_simulateButton->setText("Simulate");
        // Reset progress bar to 0% when simulation completes
        m_progressBar->setValue(0);
    }
}
