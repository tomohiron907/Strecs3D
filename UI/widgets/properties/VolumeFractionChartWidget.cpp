#include "VolumeFractionChartWidget.h"
#include "../../../utils/ColorManager.h"
#include <QPainter>
#include <algorithm>

VolumeFractionChartWidget::VolumeFractionChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumHeight(150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void VolumeFractionChartWidget::setVolumeFractions(const std::vector<double>& fractions)
{
    m_volumeFractions = fractions;
    update();
}

void VolumeFractionChartWidget::setStressRange(double minStress, double maxStress)
{
    m_stressMin = minStress;
    m_stressMax = maxStress;
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

    // X axis labels — min, mid, max only, inset to avoid clipping
    double stressRange = m_stressMax - m_stressMin;
    bool hasStress = stressRange > 0.0;
    const int labelW = 60;
    const int labelY = plotArea.bottom() + 3;
    struct TickInfo { double frac; int offset; Qt::Alignment align; };
    TickInfo ticks[] = {
        {0.0,  0,            Qt::AlignLeft},
        {0.5,  -labelW / 2,  Qt::AlignHCenter},
        {1.0,  -labelW,      Qt::AlignRight},
    };
    for (const auto& tick : ticks) {
        int x = plotArea.left() + static_cast<int>(plotArea.width() * tick.frac);
        QString label;
        if (hasStress) {
            double stressPa = m_stressMin + stressRange * tick.frac;
            label = QString::number(stressPa, 'g', 2);
        } else {
            label = QString::number(static_cast<int>(numBins * tick.frac));
        }
        QRect labelRect(x + tick.offset, labelY, labelW, 15);
        painter.drawText(labelRect, tick.align | Qt::AlignVCenter, label);
    }

    // Axis titles
    QFont titleFont;
    titleFont.setPixelSize(11);
    painter.setFont(titleFont);
    painter.setPen(textColor);

    QString xTitle = hasStress ? "Stress (Pa)" : "Bin";
    QRect xTitleRect(plotArea.left(), plotArea.bottom() + 18, plotArea.width(), 15);
    painter.drawText(xTitleRect, Qt::AlignCenter, xTitle);

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
