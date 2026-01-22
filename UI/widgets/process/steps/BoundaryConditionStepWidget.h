#ifndef BOUNDARYCONDITIONSTEPWIDGET_H
#define BOUNDARYCONDITIONSTEPWIDGET_H

#include <QWidget>

class Button;
class UIState;

class BoundaryConditionStepWidget : public QWidget {
    Q_OBJECT
public:
    explicit BoundaryConditionStepWidget(QWidget* parent = nullptr);
    void setUIState(UIState* uiState);

    Button* getAddLoadButton() const { return m_addLoadButton; }
    Button* getAddConstraintButton() const { return m_addConstraintButton; }
    Button* getDoneButton() const { return m_doneButton; }
    Button* getSelectBedSurfaceButton() const { return m_selectBedSurfaceButton; }

signals:
    void addLoadClicked();
    void addConstraintClicked();
    void selectBedSurfaceClicked();
    void doneClicked();

private slots:
    void updateDoneButtonState();

private:
    Button* m_addLoadButton;
    Button* m_addConstraintButton;
    Button* m_selectBedSurfaceButton;
    Button* m_doneButton;
    UIState* m_uiState = nullptr;
};

#endif // BOUNDARYCONDITIONSTEPWIDGET_H
