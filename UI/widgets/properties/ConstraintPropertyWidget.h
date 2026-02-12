#pragma once

#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include "../../../core/ui/UIState.h"

class ConstraintPropertyWidget : public QWidget {
    Q_OBJECT

public:
    explicit ConstraintPropertyWidget(QWidget* parent = nullptr);
    void setUIState(UIState* uiState);
    void setTarget(int index); // Set which constraint to edit
    void setReadOnly(bool readOnly);

signals:
    void closeClicked();

private:
    void setupUI();
    void updateData();
    void pushData();
    void onCloseClicked();

    UIState* m_uiState = nullptr;
    int m_currentIndex = -1;

    QLineEdit* m_nameEdit;
    QLineEdit* m_surfaceIdEdit;
    QPushButton* m_closeButton;
    QLabel* m_readOnlyHintLabel;
};
