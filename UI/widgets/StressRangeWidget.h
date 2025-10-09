#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>

class StressRangeWidget : public QWidget {
    Q_OBJECT

public:
    explicit StressRangeWidget(QWidget* parent = nullptr);
    
    void setStressRange(double minStress, double maxStress);
    double getMinStress() const;
    double getMaxStress() const;

signals:
    void stressRangeChanged(double minStress, double maxStress);

private slots:
    void onMinStressChanged();
    void onMaxStressChanged();

private:
    QLineEdit* m_minStressEdit;
    QLineEdit* m_maxStressEdit;
    QLabel* m_label;
    
    double m_currentMinStress = 0.0;
    double m_currentMaxStress = 1.0;
    
    void validateAndEmitChanges();
};