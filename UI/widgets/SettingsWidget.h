#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>

class QLineEdit;
class QIntValidator;
class QMouseEvent;
class QComboBox;
class QHBoxLayout;
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

    // UI setup helpers
    QWidget* createSlicerSelectionGroup();
    QWidget* createDensitySliderGroup();
    QLineEdit* createDensityInput();
    QHBoxLayout* createDensityRow(const QString& labelText, QLineEdit* edit);
    void connectSignals();

    // Style helpers
    QString getTitleLabelStyle() const;
    QString getContainerFrameStyle() const;
    QString getInputLabelStyle() const;
    QString getLineEditStyle() const;
    QString getComboBoxStyle() const;

    QLineEdit* m_minDensityEdit;
    QLineEdit* m_maxDensityEdit;
    QIntValidator* m_densityValidator;
    QComboBox* m_slicerComboBox;
    UIState* m_uiState = nullptr;

protected:
    void mousePressEvent(QMouseEvent* event) override;
};

#endif // SETTINGSWIDGET_H
