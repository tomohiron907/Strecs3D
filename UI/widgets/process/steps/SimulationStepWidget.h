#ifndef SIMULATIONSTEPWIDGET_H
#define SIMULATIONSTEPWIDGET_H

#include <QWidget>

class Button;

class SimulationStepWidget : public QWidget {
    Q_OBJECT
public:
    explicit SimulationStepWidget(QWidget* parent = nullptr);
    Button* getSimulateButton() const { return m_simulateButton; }
signals:
    void simulateClicked();
private:
    Button* m_simulateButton;
};

#endif // SIMULATIONSTEPWIDGET_H
