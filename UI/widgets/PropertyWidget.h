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

class VisualizationManager;

class PropertyWidget : public QWidget {
    Q_OBJECT

public:
    explicit PropertyWidget(QWidget* parent = nullptr);
    ~PropertyWidget() = default;

    void setUIState(UIState* uiState);

    // Set VisualizationManager for LoadPropertyWidget
    void setVisualizationManager(VisualizationManager* vizManager);

public slots:
    void onObjectSelected(ObjectType type, const QString& id, int index);

private:
    void setupUI();

    UIState* m_uiState = nullptr;
    
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
};

#endif // PROPERTYWIDGET_H
