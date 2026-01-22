#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class VisualizationManager;

class SelectBedSurfaceDialog : public QDialog {
    Q_OBJECT

public:
    explicit SelectBedSurfaceDialog(QWidget* parent = nullptr);
    ~SelectBedSurfaceDialog();

    void setVisualizationManager(VisualizationManager* vizManager);
    int getSelectedFaceId() const;

private slots:
    void onFaceDoubleClicked(int faceId, double nx, double ny, double nz);

private:
    void setupUI();
    void enableFaceSelectionMode(bool enable);

    QLineEdit* m_surfaceIdEdit;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;

    VisualizationManager* m_vizManager = nullptr;
    int m_selectedFaceId = 0;
};
