#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>

class QLineEdit;
class QIntValidator;
class QMouseEvent;
class QComboBox;
class QDoubleValidator;
class QHBoxLayout;

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget* parent = nullptr);

signals:
    void regionCountChanged(int count);
    void settingsChanged();

private slots:
    void onMinDensityEditingFinished();
    void onMaxDensityEditingFinished();
    void onSlicerTypeChanged(const QString& text);
    void onMaterialTypeChanged(const QString& text);
    void onInfillPatternChanged(const QString& text);
    void onSafetyFactorEditingFinished();
    void onZStressFactorEditingFinished();
    void onRegionCountEditingFinished();

private:
    void setupUI();
    void loadSettings();

    // UI setup helpers
    QWidget* createSlicerSelectionGroup();
    QWidget* createMaterialSelectionGroup();
    QWidget* createDensitySliderGroup();
    QWidget* createSafetyGroup();
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
    QComboBox* m_materialComboBox;
    QComboBox* m_infillPatternComboBox;
    QLineEdit* m_safetyFactorEdit;
    QDoubleValidator* m_safetyFactorValidator;
    QLineEdit* m_zStressFactorEdit;
    QDoubleValidator* m_zStressFactorValidator;
    QLineEdit* m_regionCountEdit;
    QIntValidator* m_regionCountValidator;
    bool m_initialLoadComplete = false;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;
};

#endif // SETTINGSWIDGET_H
