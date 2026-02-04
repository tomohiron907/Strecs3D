#ifndef STRESSDENSITYCURVEWIDGET_H
#define STRESSDENSITYCURVEWIDGET_H

#include <QWidget>

class StressDensityCurveWidget : public QWidget {
    Q_OBJECT

public:
    explicit StressDensityCurveWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    double calculateDensityPercent(double stressMPa) const;

    static constexpr double STRESS_MAX_MPA = 2.0;
    static constexpr double YIELD_STRENGTH = 30.0;
    static constexpr double C = 0.23;
    static constexpr double M = 2.0 / 3.0;
};

#endif // STRESSDENSITYCURVEWIDGET_H
