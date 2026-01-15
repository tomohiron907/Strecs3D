#ifndef PROCESSFLOWWIDGET_H
#define PROCESSFLOWWIDGET_H

#include <QWidget>
#include <vector>

class QVBoxLayout;
class QLabel;
class QProgressBar;
class QEnterEvent;

enum class ProcessStep {
    ImportStep = 0,
    BoundaryCondition = 1,
    Simulate = 2,
    InfillMap = 3
};

class ProcessCard : public QWidget {
    Q_OBJECT
public:
    explicit ProcessCard(int stepNumber, const QString& title, QWidget* parent = nullptr);

    void setActive(bool active);
    void setCompleted(bool completed);
    void setLocked(bool locked);
    void setClickable(bool clickable);
    bool isClickable() const { return m_isClickable; }

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    int m_stepNumber;
    QString m_title;
    bool m_isActive = false;
    bool m_isCompleted = false;
    bool m_isLocked = true;
    bool m_isClickable = false;
    bool m_isHovered = false;

    QLabel* m_textLabel = nullptr;

    void updateStyle();
};

class ProcessFlowWidget : public QWidget {
    Q_OBJECT
public:
    explicit ProcessFlowWidget(QWidget* parent = nullptr);
    
    void setCurrentStep(ProcessStep step);
    ProcessStep currentStep() const;
    void setStepCompleted(ProcessStep step, bool completed);

signals:
    void stepClicked(ProcessStep step); // Optional: if we want manual navigation

private:
    std::vector<ProcessCard*> m_cards;
    ProcessStep m_currentStep = ProcessStep::ImportStep;
    
    void setupUI();
};

#endif // PROCESSFLOWWIDGET_H
