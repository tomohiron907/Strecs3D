#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>

class QLineEdit;
class QIntValidator;
class QMouseEvent;
class QComboBox;
class UIState;

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget* parent = nullptr);
    void setUIState(UIState* uiState);

private slots:
    void onMinDensityEditingFinished();
    void onMaxDensityEditingFinished();
    void onSlicerTypeChanged(const QString& text);

private:
    void setupUI();
    void loadSettings();

    QLineEdit* m_minDensityEdit;
    QLineEdit* m_maxDensityEdit;
    QIntValidator* m_densityValidator;
    QComboBox* m_slicerComboBox;
    UIState* m_uiState = nullptr;

protected:
    void mousePressEvent(QMouseEvent* event) override;
};

#endif // SETTINGSWIDGET_H
