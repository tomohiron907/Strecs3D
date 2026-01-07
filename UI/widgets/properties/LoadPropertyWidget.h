#pragma once

#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include "../../../core/ui/UIState.h"

class LoadPropertyWidget : public QWidget {
    Q_OBJECT

public:
    explicit LoadPropertyWidget(QWidget* parent = nullptr);
    void setUIState(UIState* uiState);
    void setTarget(int index); // Set which load to edit

private:
    void setupUI();
    void updateData();
    void pushData();
    
    UIState* m_uiState = nullptr;
    int m_currentIndex = -1;
    
    QLineEdit* m_nameEdit;
    QSpinBox* m_surfaceIdSpinBox;
    QDoubleSpinBox* m_magnitudeSpinBox;
    QDoubleSpinBox* m_dirXSpinBox;
    QDoubleSpinBox* m_dirYSpinBox;
    QDoubleSpinBox* m_dirZSpinBox;
};
