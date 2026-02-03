#ifndef PROPERTYWIDGET_H
#define PROPERTYWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include "ObjectListWidget.h"
#include "../../core/ui/UIState.h"
#include "properties/StepPropertyWidget.h"
#include "properties/ConstraintPropertyWidget.h"
#include "properties/LoadPropertyWidget.h"
#include "../widgets/process/ProcessFlowWidget.h"

class VisualizationManager;
class StressDensityCurveWidget;
class VolumeFractionChartWidget;

class PropertyWidget : public QWidget {
    Q_OBJECT

public:
    explicit PropertyWidget(QWidget* parent = nullptr);
    ~PropertyWidget() = default;

    void setUIState(UIState* uiState);

    // Set VisualizationManager for LoadPropertyWidget
    void setVisualizationManager(VisualizationManager* vizManager);

    // Set volume fractions for the chart widget
    void setVolumeFractions(const std::vector<double>& fractions);

public slots:
    void onObjectSelected(ObjectType type, const QString& id, int index);
    void setCurrentStep(ProcessStep step);

private:
    void setupUI();
    void updateDefaultView();

    UIState* m_uiState = nullptr;
    ProcessStep m_currentStep = ProcessStep::ImportStep;

    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QStackedWidget* m_stackedWidget;

    // Property Widgets
    StepPropertyWidget* m_stepWidget;
    ConstraintPropertyWidget* m_constraintWidget;
    LoadPropertyWidget* m_loadWidget;
    QWidget* m_emptyWidget;

    // Legacy placeholders or future implementations
    QWidget* m_simulationWidget;
    QWidget* m_infillWidget;

    // Infill default view (graphs shown when nothing is selected in Step4)
    QWidget* m_infillDefaultWidget;
    StressDensityCurveWidget* m_stressDensityCurveWidget;
    VolumeFractionChartWidget* m_volumeFractionChartWidget;
};

#endif // PROPERTYWIDGET_H
