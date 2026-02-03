#include "StressDensityCurveWidget.h"
#include "../../../utils/SettingsManager.h"
#include "../../../utils/ColorManager.h"
#include <QPainter>
#include <QPainterPath>
#include <cmath>

StressDensityCurveWidget::StressDensityCurveWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
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

    // Grid and axes colors
    QColor axisColor(180, 180, 180);
    QColor gridColor(60, 60, 60);
    QColor curveColor = ColorManager::ACCENT_COLOR;
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

    // Draw curve using cubic Bezier segments for smoothness
    const int numSamples = 64;
    struct Pt { double x, y; };
    std::vector<Pt> pts(numSamples + 1);

    for (int i = 0; i <= numSamples; ++i) {
        double stressMPa = STRESS_MAX_MPA * i / numSamples;
        double density = calculateDensityPercent(stressMPa);
        pts[i].x = plotArea.left() + plotArea.width() * (stressMPa / STRESS_MAX_MPA);
        pts[i].y = plotArea.bottom() - plotArea.height() * (density / 100.0);
    }

    QPainterPath path;
    path.moveTo(pts[0].x, pts[0].y);

    for (size_t i = 0; i < pts.size() - 1; ++i) {
        // Catmull-Rom to cubic Bezier control points
        const Pt& p0 = pts[i > 0 ? i - 1 : 0];
        const Pt& p1 = pts[i];
        const Pt& p2 = pts[i + 1];
        const Pt& p3 = pts[i + 1 < pts.size() - 1 ? i + 2 : pts.size() - 1];

        double cp1x = p1.x + (p2.x - p0.x) / 6.0;
        double cp1y = p1.y + (p2.y - p0.y) / 6.0;
        double cp2x = p2.x - (p3.x - p1.x) / 6.0;
        double cp2y = p2.y - (p3.y - p1.y) / 6.0;

        path.cubicTo(cp1x, cp1y, cp2x, cp2y, p2.x, p2.y);
    }

    painter.setPen(QPen(curveColor, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
}
