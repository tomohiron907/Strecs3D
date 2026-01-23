#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>

class QLineEdit;
class QIntValidator;

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget* parent = nullptr);

private slots:
    void onMinDensityEditingFinished();
    void onMaxDensityEditingFinished();

private:
    void setupUI();
    void loadSettings();

    QLineEdit* m_minDensityEdit;
    QLineEdit* m_maxDensityEdit;
    QIntValidator* m_densityValidator;
};

#endif // SETTINGSWIDGET_H
