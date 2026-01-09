#ifndef INFILLSTEPWIDGET_H
#define INFILLSTEPWIDGET_H

#include <QWidget>

class DensitySlider;
class Button;
class UIState;
class StressRangeWidget; // Explicitly adding this if needed, or if it should be shown here

// Note: The user request says "DensitySlider and Process button".
// The StressRangeWidget was also in the left pane. Should it be here?
// The user prompt: "4のステップの時には、下には、DensitySliderと、Processボタンを配置してください。"
// It doesn't mention StressRangeWidget explicitly, but it makes sense to have it if we have DensitySlider.
// I will include DensitySlider and ProcessButton as requested. 

class InfillStepWidget : public QWidget {
    Q_OBJECT
public:
    explicit InfillStepWidget(QWidget* parent = nullptr);
    void setUIState(UIState* uiState); // To setup DensitySlider connections
    
    DensitySlider* getDensitySlider() const { return m_densitySlider; }
    StressRangeWidget* getStressRangeWidget() const { return m_stressRangeWidget; }
    Button* getProcessButton() const { return m_processButton; }

signals:
    void processClicked();

private:
    DensitySlider* m_densitySlider;
    StressRangeWidget* m_stressRangeWidget;
    Button* m_processButton;
};

#endif // INFILLSTEPWIDGET_H
