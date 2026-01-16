#pragma once

#include <QCheckBox>
#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include "../../../core/ui/UIState.h"

class VisualizationManager;

class LoadPropertyWidget : public QWidget {
    Q_OBJECT

public:
    explicit LoadPropertyWidget(QWidget* parent = nullptr);
    void setUIState(UIState* uiState);
    void setTarget(int index); // Set which load to edit

    // Set VisualizationManager reference for edge selection
    void setVisualizationManager(VisualizationManager* vizManager);

signals:
    void closeClicked();

private:
    void setupUI();
    void updateData();
    void pushData();
    void onCloseClicked();

    // Edge selection handlers
    void onReferenceEdgeButtonClicked();
    void onEdgeSelected(int edgeId);
    void updateDirectionFromEdge(int edgeId);
    void cancelEdgeSelection();
    void onReverseDirectionToggled(bool checked);

    UIState* m_uiState = nullptr;
    VisualizationManager* m_vizManager = nullptr;
    int m_currentIndex = -1;
    bool m_isSelectingEdge = false;

    QLineEdit* m_nameEdit;
    QLineEdit* m_surfaceIdEdit;
    QLineEdit* m_magnitudeEdit;

    // Edge selection UI
    QPushButton* m_referenceEdgeButton;
    QLabel* m_selectedEdgeLabel;
    QLabel* m_directionDisplay;  // Simple numeric display
    QCheckBox* m_reverseCheckBox;

    QPushButton* m_closeButton;
};
