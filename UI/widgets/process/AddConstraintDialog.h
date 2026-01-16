#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../../../core/types/BoundaryCondition.h"

class VisualizationManager;

class AddConstraintDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddConstraintDialog(const QString& defaultName, QWidget* parent = nullptr);
    ~AddConstraintDialog();

    void setVisualizationManager(VisualizationManager* vizManager);
    ConstraintCondition getConstraintCondition() const;

private slots:
    void onFaceDoubleClicked(int faceId, double nx, double ny, double nz);

private:
    void setupUI();
    void enableFaceSelectionMode(bool enable);

    QLineEdit* m_nameEdit;
    QLineEdit* m_surfaceIdEdit;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;

    VisualizationManager* m_vizManager = nullptr;
};
