#ifndef PROCESSMANAGERWIDGET_H
#define PROCESSMANAGERWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include "ProcessFlowWidget.h"

// Forward declarations for Step Widgets
class ImportStepWidget;
class BoundaryConditionStepWidget;
class SimulationStepWidget;
class InfillStepWidget;
class UIState;

class ProcessManagerWidget : public QWidget {
    Q_OBJECT
public:
    explicit ProcessManagerWidget(QWidget* parent = nullptr);
    void setUIState(UIState* uiState); // Dependency injection for step widgets

    // Getters for integration with MainWindow if needed
    ProcessFlowWidget* getFlowWidget() const { return m_flowWidget; }
    
    // Step Getters
    ImportStepWidget* getImportStep() const { return m_importWidget; }
    BoundaryConditionStepWidget* getBoundaryConditionStep() const { return m_bcWidget; }
    SimulationStepWidget* getSimulationStep() const { return m_simWidget; }
    InfillStepWidget* getInfillStep() const { return m_infillWidget; }
    
    // Signals to communicate with MainWindow actions
    // (We will connect these to MainWindow slots)
signals:
    void importStepClicked();
    void importFile(const QString& filePath);
    void addLoadClicked();
    void addConstraintClicked();
    void simulateClicked();
    void processInfillClicked();

    // Signal when step changes (for any broader UI updates)
    void stepChanged(ProcessStep newStep);

public slots:
    void onImportCompleted(); // Call this when file is loaded
    void onSimulationCompleted(); // Call this when sim is done
    void nextStep(); // Manually advance

private:
    ProcessFlowWidget* m_flowWidget;
    QStackedWidget* m_stepContainer;
    
    // Step Widgets
    ImportStepWidget* m_importWidget;
    BoundaryConditionStepWidget* m_bcWidget;
    SimulationStepWidget* m_simWidget;
    InfillStepWidget* m_infillWidget;
    
    void setupUI();
    void connectSignals();
};

#endif // PROCESSMANAGERWIDGET_H
