#include "SimulationStepWidget.h"
#include "../../Button.h"
#include "../../../../utils/StyleManager.h"
#include "../../../../utils/ColorManager.h"
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
    m_progressBar->setStyleSheet(QString(R"(
        QProgressBar {
            border: 1px solid #444;
            border-radius: %1px;
            background-color: #333;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: %3;
            border-radius: %2px;
        }
    )").arg(StyleManager::RADIUS_SMALL).arg(StyleManager::RADIUS_SMALL - 1).arg(ColorManager::ACCENT_COLOR.name()));

    layout->addWidget(m_progressBar);

    // Optional: Status label
    m_statusLabel = new QLabel("", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(QString("color: #aaa; font-size: %1px;")
        .arg(StyleManager::FONT_SIZE_SMALL));
    m_statusLabel->setVisible(false);
    layout->addWidget(m_statusLabel);

    // Log Text Edit
    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setStyleSheet(QString(R"(
        QTextEdit {
            background-color: #222;
            color: #ddd;
            border: 1px solid #444;
            border-radius: %1px;
            font-family: Consolas, Monaco, monospace;
            font-size: %2px;
        }
    )").arg(StyleManager::RADIUS_SMALL).arg(StyleManager::FONT_SIZE_SMALL));
    m_logTextEdit->setMinimumHeight(120);
    layout->addWidget(m_logTextEdit);

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
    // m_progressBar->setVisible(false); // Deleted to keep visible
    if (m_statusLabel) {
        m_statusLabel->setText("");
        m_statusLabel->setVisible(false);
    }

    // Reset simulation button state
    m_simulateButton->setText("Simulate");
    m_simulateButton->setEnabled(true);
    m_simulateButton->setEmphasized(true);
}

void SimulationStepWidget::setSimulationRunning(bool running) {
    m_simulateButton->setEnabled(!running);
    if (m_statusLabel) {
        m_statusLabel->setVisible(running);
    }

    if (running) {
        m_simulateButton->setText("Simulating...");
        m_progressBar->setValue(0);
    } else {
        m_simulateButton->setText("Simulate");
        // Set progress bar to 100% when simulation completes
        m_progressBar->setValue(100);
    }
}

void SimulationStepWidget::appendLog(const QString& message) {
    if (m_logTextEdit) {
        m_logTextEdit->append(message);
        // Ensure the cursor is at the end to auto-scroll
        QTextCursor cursor = m_logTextEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_logTextEdit->setTextCursor(cursor);
    }
}

void SimulationStepWidget::clearLog() {
    if (m_logTextEdit) {
        m_logTextEdit->clear();
    }
}
