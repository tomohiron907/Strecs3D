#ifndef INFILLSTEPWIDGET_H
#define INFILLSTEPWIDGET_H

#include <QWidget>

class AdaptiveDensitySlider;
class Button;
class UIState;
class StressRangeWidget;

class InfillStepWidget : public QWidget {
    Q_OBJECT
public:
    explicit InfillStepWidget(QWidget* parent = nullptr);
    void setUIState(UIState* uiState);

    AdaptiveDensitySlider* getDensitySlider() const { return m_densitySlider; }
    StressRangeWidget* getStressRangeWidget() const { return m_stressRangeWidget; }
    Button* getProcessButton() const { return m_processButton; }

signals:
    void processClicked();

private:
    AdaptiveDensitySlider* m_densitySlider;
    StressRangeWidget* m_stressRangeWidget;
    Button* m_processButton;
};

#endif // INFILLSTEPWIDGET_H
