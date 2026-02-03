#ifndef VOLUMEFRACTIONCHARTWIDGET_H
#define VOLUMEFRACTIONCHARTWIDGET_H

#include <QWidget>
#include <vector>

class VolumeFractionChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit VolumeFractionChartWidget(QWidget* parent = nullptr);

    void setVolumeFractions(const std::vector<double>& fractions);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<double> m_volumeFractions;
};

#endif // VOLUMEFRACTIONCHARTWIDGET_H
