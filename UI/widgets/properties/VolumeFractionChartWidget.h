#ifndef VOLUMEFRACTIONCHARTWIDGET_H
#define VOLUMEFRACTIONCHARTWIDGET_H

#include <QWidget>

class VolumeFractionCalculator;

class VolumeFractionChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit VolumeFractionChartWidget(QWidget* parent = nullptr);

    void setVolumeFractionCalculator(const VolumeFractionCalculator* calculator);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    const VolumeFractionCalculator* m_calculator = nullptr;
};

#endif // VOLUMEFRACTIONCHARTWIDGET_H
