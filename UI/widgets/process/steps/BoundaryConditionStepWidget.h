#ifndef BOUNDARYCONDITIONSTEPWIDGET_H
#define BOUNDARYCONDITIONSTEPWIDGET_H

#include <QWidget>

class Button;

class BoundaryConditionStepWidget : public QWidget {
    Q_OBJECT
public:
    explicit BoundaryConditionStepWidget(QWidget* parent = nullptr);
    Button* getAddLoadButton() const { return m_addLoadButton; }
    Button* getAddConstraintButton() const { return m_addConstraintButton; }
    Button* getDoneButton() const { return m_doneButton; }
signals:
    void addLoadClicked();
    void addConstraintClicked();
    void doneClicked();
private:
    Button* m_addLoadButton;
    Button* m_addConstraintButton;
    Button* m_doneButton;
};

#endif // BOUNDARYCONDITIONSTEPWIDGET_H
