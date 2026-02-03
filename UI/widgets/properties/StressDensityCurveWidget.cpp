#include "StressDensityCurveWidget.h"
#include "../../../utils/SettingsManager.h"
#include <QPainter>
#include <QPainterPath>
#include <cmath>

StressDensityCurveWidget::StressDensityCurveWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

double StressDensityCurveWidget::calculateDensityPercent(double stressMPa) const
{
    int minDensity = SettingsManager::instance().minDensity();
    int maxDensity = SettingsManager::instance().maxDensity();

    double numerator = SAFE_FACTOR * stressMPa;
    double denominator = YIELD_STRENGTH * C;
    double density = std::pow(numerator / denominator, M) * 100.0;

    return std::clamp(density, static_cast<double>(minDensity), static_cast<double>(maxDensity));
}

void StressDensityCurveWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int margin_left = 50;
    const int margin_right = 15;
    const int margin_top = 20;
    const int margin_bottom = 35;

    QRect plotArea(margin_left, margin_top,
                   width() - margin_left - margin_right,
                   height() - margin_top - margin_bottom);

    if (plotArea.width() <= 0 || plotArea.height() <= 0) return;

    // Background
    painter.fillRect(rect(), QColor(26, 26, 26));

    // Grid and axes colors
    QColor axisColor(180, 180, 180);
    QColor gridColor(60, 60, 60);
    QColor curveColor(100, 180, 255);
    QColor textColor(160, 160, 160);
    QColor clampColor(255, 160, 80);

    // Draw grid
    painter.setPen(QPen(gridColor, 1, Qt::DotLine));
    // Horizontal grid (density: 0, 20, 40, 60, 80, 100)
    for (int d = 0; d <= 100; d += 20) {
        int y = plotArea.bottom() - static_cast<int>(plotArea.height() * d / 100.0);
        painter.drawLine(plotArea.left(), y, plotArea.right(), y);
    }
    // Vertical grid (stress: 0, 0.5, 1.0, 1.5, 2.0)
    for (int i = 0; i <= 4; ++i) {
        int x = plotArea.left() + static_cast<int>(plotArea.width() * i / 4.0);
        painter.drawLine(x, plotArea.top(), x, plotArea.bottom());
    }

    // Draw axes
    painter.setPen(QPen(axisColor, 1));
    painter.drawLine(plotArea.bottomLeft(), plotArea.bottomRight()); // X axis
    painter.drawLine(plotArea.topLeft(), plotArea.bottomLeft());     // Y axis

    // Axis labels
    QFont labelFont;
    labelFont.setPixelSize(10);
    painter.setFont(labelFont);
    painter.setPen(textColor);

    // X axis labels
    for (int i = 0; i <= 4; ++i) {
        double stress = STRESS_MAX_MPA * i / 4.0;
        int x = plotArea.left() + static_cast<int>(plotArea.width() * i / 4.0);
        QString label = QString::number(stress, 'f', 1);
        QRect labelRect(x - 20, plotArea.bottom() + 3, 40, 15);
        painter.drawText(labelRect, Qt::AlignCenter, label);
    }

    // Y axis labels
    for (int d = 0; d <= 100; d += 20) {
        int y = plotArea.bottom() - static_cast<int>(plotArea.height() * d / 100.0);
        QString label = QString::number(d);
        QRect labelRect(0, y - 8, margin_left - 5, 16);
        painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, label);
    }

    // Axis titles
    QFont titleFont;
    titleFont.setPixelSize(11);
    painter.setFont(titleFont);
    painter.setPen(textColor);

    // X axis title
    QRect xTitleRect(plotArea.left(), plotArea.bottom() + 18, plotArea.width(), 15);
    painter.drawText(xTitleRect, Qt::AlignCenter, "Stress (MPa)");

    // Y axis title (rotated)
    painter.save();
    painter.translate(12, plotArea.center().y());
    painter.rotate(-90);
    painter.drawText(QRect(-plotArea.height() / 2, -8, plotArea.height(), 16),
                     Qt::AlignCenter, "Density (%)");
    painter.restore();

    // Draw clamp range indicators
    int minDensity = SettingsManager::instance().minDensity();
    int maxDensity = SettingsManager::instance().maxDensity();

    painter.setPen(QPen(clampColor, 1, Qt::DashLine));
    int yMin = plotArea.bottom() - static_cast<int>(plotArea.height() * minDensity / 100.0);
    int yMax = plotArea.bottom() - static_cast<int>(plotArea.height() * maxDensity / 100.0);
    painter.drawLine(plotArea.left(), yMin, plotArea.right(), yMin);
    painter.drawLine(plotArea.left(), yMax, plotArea.right(), yMax);

    // Draw curve
    QPainterPath path;
    const int numPoints = plotArea.width();
    bool first = true;

    for (int px = 0; px <= numPoints; ++px) {
        double stressMPa = STRESS_MAX_MPA * px / numPoints;
        double density = calculateDensityPercent(stressMPa);

        int x = plotArea.left() + px;
        int y = plotArea.bottom() - static_cast<int>(plotArea.height() * density / 100.0);

        if (first) {
            path.moveTo(x, y);
            first = false;
        } else {
            path.lineTo(x, y);
        }
    }

    painter.setPen(QPen(curveColor, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
}
