#ifndef SIMULATIONSTEPWIDGET_H
#define SIMULATIONSTEPWIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QTextEdit>

class Button;

class SimulationStepWidget : public QWidget {
    Q_OBJECT
public:
    explicit SimulationStepWidget(QWidget* parent = nullptr);
    Button* getSimulateButton() const { return m_simulateButton; }
    QProgressBar* getProgressBar() const { return m_progressBar; }

public slots:
    void setProgress(int value, const QString& message = "");
    void resetProgress();
    void setSimulationRunning(bool running);

    void appendLog(const QString& message);
    void clearLog();

signals:
    void simulateClicked();

private:
    Button* m_simulateButton;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QTextEdit* m_logTextEdit;
};

#endif // SIMULATIONSTEPWIDGET_H
