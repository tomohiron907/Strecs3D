#ifndef ROLLBACKCONFIRMATIONDIALOG_H
#define ROLLBACKCONFIRMATIONDIALOG_H

#include <QDialog>
#include "ProcessFlowWidget.h"

class QLabel;
class QPushButton;

class RollbackConfirmationDialog : public QDialog {
    Q_OBJECT
public:
    explicit RollbackConfirmationDialog(
        ProcessStep targetStep,
        ProcessStep currentStep,
        QWidget* parent = nullptr
    );

private:
    void setupUI();
    QString generateWarningMessage(ProcessStep target, ProcessStep current);

    ProcessStep m_targetStep;
    ProcessStep m_currentStep;
    QLabel* m_messageLabel;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
};

#endif // ROLLBACKCONFIRMATIONDIALOG_H
