#include "VolumeFractionChartWidget.h"
#include "../../../utils/ColorManager.h"
#include <QPainter>
#include <algorithm>

VolumeFractionChartWidget::VolumeFractionChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void VolumeFractionChartWidget::setVolumeFractions(const std::vector<double>& fractions)
{
    m_volumeFractions = fractions;
    update();
}

void VolumeFractionChartWidget::paintEvent(QPaintEvent* /*event*/)
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

    QColor axisColor(180, 180, 180);
    QColor gridColor(60, 60, 60);
    QColor barColor = ColorManager::ACCENT_COLOR;
    QColor textColor(160, 160, 160);

    const auto& fractions = m_volumeFractions;

    int numBins = fractions.empty() ? 20 : static_cast<int>(fractions.size());
    double maxFraction = 1.0;
    if (!fractions.empty()) {
        double dataMax = *std::max_element(fractions.begin(), fractions.end());
        if (dataMax > 0.0) {
            // Round up to nearest 0.1 for better axis display
            maxFraction = std::ceil(dataMax * 10.0) / 10.0;
            if (maxFraction < 0.1) maxFraction = 0.1;
        }
    }

    // Draw horizontal grid
    painter.setPen(QPen(gridColor, 1, Qt::DotLine));
    int numGridLines = 5;
    for (int i = 0; i <= numGridLines; ++i) {
        int y = plotArea.bottom() - static_cast<int>(plotArea.height() * i / static_cast<double>(numGridLines));
        painter.drawLine(plotArea.left(), y, plotArea.right(), y);
    }

    // Draw axes
    painter.setPen(QPen(axisColor, 1));
    painter.drawLine(plotArea.bottomLeft(), plotArea.bottomRight());
    painter.drawLine(plotArea.topLeft(), plotArea.bottomLeft());

    // Y axis labels
    QFont labelFont;
    labelFont.setPixelSize(10);
    painter.setFont(labelFont);
    painter.setPen(textColor);

    for (int i = 0; i <= numGridLines; ++i) {
        double val = maxFraction * i / numGridLines;
        int y = plotArea.bottom() - static_cast<int>(plotArea.height() * i / static_cast<double>(numGridLines));
        QString label = QString::number(val, 'f', 1);
        QRect labelRect(0, y - 8, margin_left - 5, 16);
        painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, label);
    }

    // X axis labels (every 5 bins)
    for (int i = 0; i < numBins; i += 5) {
        double barWidth = static_cast<double>(plotArea.width()) / numBins;
        int x = plotArea.left() + static_cast<int>(barWidth * (i + 0.5));
        QString label = QString::number(i);
        QRect labelRect(x - 15, plotArea.bottom() + 3, 30, 15);
        painter.drawText(labelRect, Qt::AlignCenter, label);
    }

    // Axis titles
    QFont titleFont;
    titleFont.setPixelSize(11);
    painter.setFont(titleFont);
    painter.setPen(textColor);

    QRect xTitleRect(plotArea.left(), plotArea.bottom() + 18, plotArea.width(), 15);
    painter.drawText(xTitleRect, Qt::AlignCenter, "Bin");

    painter.save();
    painter.translate(12, plotArea.center().y());
    painter.rotate(-90);
    painter.drawText(QRect(-plotArea.height() / 2, -8, plotArea.height(), 16),
                     Qt::AlignCenter, "Volume Fraction");
    painter.restore();

    // Draw bars
    if (!fractions.empty()) {
        double barWidth = static_cast<double>(plotArea.width()) / numBins;
        int gap = std::max(1, static_cast<int>(barWidth * 0.1));

        for (int i = 0; i < numBins; ++i) {
            double fraction = fractions[i];
            int barHeight = static_cast<int>(plotArea.height() * fraction / maxFraction);

            int x = plotArea.left() + static_cast<int>(barWidth * i) + gap;
            int w = static_cast<int>(barWidth) - gap * 2;
            if (w < 1) w = 1;
            int y = plotArea.bottom() - barHeight;

            painter.fillRect(x, y, w, barHeight, barColor);
        }
    } else {
        // No data message
        painter.setPen(QColor(120, 120, 120));
        QFont msgFont;
        msgFont.setPixelSize(12);
        painter.setFont(msgFont);
        painter.drawText(plotArea, Qt::AlignCenter, "No data available");
    }
}
