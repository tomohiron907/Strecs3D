#ifndef VOLUMEFRACTIONCHARTWIDGET_H
#define VOLUMEFRACTIONCHARTWIDGET_H

#include <QWidget>
#include <vector>

class VolumeFractionChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit VolumeFractionChartWidget(QWidget* parent = nullptr);

    void setVolumeFractions(const std::vector<double>& fractions);
    void setStressRange(double minStress, double maxStress);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<double> m_volumeFractions;
    double m_stressMin = 0.0;
    double m_stressMax = 0.0;
};

#endif // VOLUMEFRACTIONCHARTWIDGET_H
