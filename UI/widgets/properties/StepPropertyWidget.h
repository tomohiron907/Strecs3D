#pragma once

#include <QWidget>
#include <QFormLayout>
#include <QLabel>
#include "../../../core/ui/UIState.h"

class StepPropertyWidget : public QWidget {
    Q_OBJECT

public:
    explicit StepPropertyWidget(QWidget* parent = nullptr);
    void setUIState(UIState* uiState);
    void updateData();

private:
    void setupUI();
    
    UIState* m_uiState = nullptr;
    
    QLabel* m_filenameLabel;
    QLabel* m_filePathLabel;
};
