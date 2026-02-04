#include "AdaptiveDensitySlider.h"
#include "../../utils/ColorManager.h"
#include "../../utils/SettingsManager.h"
#include "../../utils/StyleManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <algorithm>
#include <QLineEdit>
#include <QResizeEvent>
#include <QDoubleValidator>
#include <cassert>
#include <cmath>

// ====================
// Constructor and Size Hints
// ====================

AdaptiveDensitySlider::AdaptiveDensitySlider(QWidget* parent)
    : QWidget(parent)
{
    m_regionCount = SettingsManager::instance().regionCount();
    m_handles.resize(handleCount(), 0);
    m_regionPercents.resize(m_regionCount, 20.0);
    setMinimumWidth(120);
    setMinimumHeight(220);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    for (int i = 0; i < m_regionCount; ++i) {
        QLineEdit* edit = new QLineEdit(this);
        edit->setFixedWidth(40);
        edit->setAlignment(Qt::AlignCenter);
        edit->setStyleSheet(QString("QLineEdit { color: %1; background-color: %2; border: 1px solid %3; border-radius: %4px; selection-background-color: #555555; }")
            .arg(ColorManager::INPUT_TEXT_COLOR.name())
            .arg(ColorManager::INPUT_BACKGROUND_COLOR.name())
            .arg(ColorManager::INPUT_BORDER_COLOR.name())
            .arg(StyleManager::RADIUS_SMALL));
        edit->setText(QString::number(m_regionPercents[i], 'g', 2));
        edit->setValidator(new QDoubleValidator(0, 100, 2, edit));
        connect(edit, &QLineEdit::editingFinished, this, &AdaptiveDensitySlider::onPercentEditChanged);
        m_percentEdits.push_back(edit);
    }
    updateInitialHandles();
    updateRegionBoundaries();
    updateStressDensityMappings();
}

QSize AdaptiveDensitySlider::minimumSizeHint() const {
    return QSize(60, 220);
}

QSize AdaptiveDensitySlider::sizeHint() const {
    return QSize(60, 300);
}

// ====================
// Primary Public API
// ====================

void AdaptiveDensitySlider::setStressRange(double minStress, double maxStress) {
    m_minStress = minStress;
    m_maxStress = maxStress;
    updateInitialHandles();
    updateRegionBoundaries();
    updateStressDensityMappings();
    update();

    // Emit signal to update UIState with new mappings
    emit regionPercentsChanged(m_regionPercents);
}

void AdaptiveDensitySlider::setOriginalStressRange(double minStress, double maxStress) {
    m_originalMinStress = minStress;
    m_originalMaxStress = maxStress;
    setStressRange(minStress, maxStress);
}

std::vector<StressDensityMapping> AdaptiveDensitySlider::stressDensityMappings() const {
    return m_stressDensityMappings;
}

void AdaptiveDensitySlider::setRegionPercents(const std::vector<double>& percents) {
    if (percents.size() == static_cast<size_t>(m_regionCount)) {
        m_regionPercents = percents;
        for (int i = 0; i < m_regionCount; ++i) {
            m_percentEdits[i]->setText(QString::number(m_regionPercents[i], 'g', 2));
        }
        updateStressDensityMappings();
        update();
        emit regionPercentsChanged(m_regionPercents);
    }
}

std::vector<double> AdaptiveDensitySlider::regionPercents() const {
    return m_regionPercents;
}

int AdaptiveDensitySlider::countMaxDensityRegions() const {
    int count = 0;
    for (double percent : m_regionPercents) {
        if (static_cast<int>(percent) >= SettingsManager::instance().maxDensity()) {
            count++;
        }
    }
    return count;
}

void AdaptiveDensitySlider::setVolumeFractions(const std::vector<double>& fractions) {
    if (fractions.size() == VOLUME_DIVISIONS) {
        m_volumeFractions = fractions;
        updateRegionBoundaries();
        updateStressDensityMappings();
        update();

        // Emit signal to update UIState with new mappings
        emit regionPercentsChanged(m_regionPercents);
    }
}

// ====================
// Other Public API
// ====================

std::vector<int> AdaptiveDensitySlider::handlePositions() const {
    return m_handles;
}

std::vector<int> AdaptiveDensitySlider::stressThresholds() const {
    std::vector<int> thresholds;
    thresholds.push_back(m_minStress);
    for (int y : m_handles) {
        thresholds.push_back(yToStress(y));
    }
    thresholds.push_back(m_maxStress);
    std::sort(thresholds.begin(), thresholds.end());
    return thresholds;
}

std::vector<QColor> AdaptiveDensitySlider::getRegionColors() const {
    std::vector<QColor> colors;
    std::vector<int> positions = getRegionPositions();

    for (int i = 0; i < m_regionCount; ++i) {
        // positions[i] = bottom edge (higher Y, lower stress)
        // positions[i+1] = top edge (lower Y, higher stress)
        int yBottom = positions[i];
        int yTop = positions[i+1];

        // Get stress values using non-linear conversion
        double stressBottom = yToStress(yBottom);
        double stressTop = yToStress(yTop);

        // Calculate average stress for this region
        double avgStress = (stressTop + stressBottom) / 2.0;

        // Normalize: t=0 for max stress (red), t=1 for min stress (blue)
        double t = (m_maxStress - avgStress) / (m_maxStress - m_minStress);
        t = std::clamp(t, 0.0, 1.0);

        QColor regionColor = ColorManager::getGradientColor(t);
        colors.push_back(regionColor);
    }

    return colors;
}

// ====================
// Event Handlers
// ====================

void AdaptiveDensitySlider::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    SliderBounds bounds = getSliderBounds();

    drawGradientBar(painter, bounds);
    drawStressLabels(painter, bounds);
    drawSliderBody(painter, bounds);
    drawRegions(painter, bounds);
    drawHandles(painter, bounds);
    updatePercentEditPositions();
    drawAxisLabels(painter, bounds);
}

void AdaptiveDensitySlider::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updatePercentEditPositions();
    updateInitialHandles();
    updateRegionBoundaries();
    updateStressDensityMappings();
}

void AdaptiveDensitySlider::mousePressEvent(QMouseEvent* event) {
    m_draggedHandle = handleAtPosition(event->pos());
}

void AdaptiveDensitySlider::mouseMoveEvent(QMouseEvent* event) {
    if (m_draggedHandle >= 0) {
        int y = std::clamp(event->pos().y(), m_margin, height() - m_margin);
        if (m_draggedHandle > 0)
            y = std::max(y, m_handles[m_draggedHandle-1] + m_minDistance);
        if (m_draggedHandle < (int)m_handles.size()-1)
            y = std::min(y, m_handles[m_draggedHandle+1] - m_minDistance);
        m_handles[m_draggedHandle] = y;
        clampHandles();
        updateStressDensityMappings();
        update();
        emit handlePositionsChanged(m_handles);
    }
}

void AdaptiveDensitySlider::mouseReleaseEvent(QMouseEvent*) {
    m_draggedHandle = -1;
}

// ====================
// Slots
// ====================

void AdaptiveDensitySlider::onPercentEditChanged() {
    bool changed = false;
    for (int i = 0; i < m_regionCount; ++i) {
        bool ok = false;
        double val = m_percentEdits[i]->text().toDouble(&ok);
        if (ok && m_regionPercents[i] != val) {
            m_regionPercents[i] = val;
            changed = true;
        }
    }
    if (changed) {
        updateStressDensityMappings();
        emit regionPercentsChanged(m_regionPercents);
        update();
    }
}

// ====================
// Update and Calculation Helpers
// ====================

void AdaptiveDensitySlider::updateRegionBoundaries() {
    SliderBounds bounds = getSliderBounds();
    int totalHeight = bounds.bottom - bounds.top;

    m_regionBoundaries.clear();
    m_regionBoundaries.push_back(bounds.top);

    if (m_volumeFractions.size() != VOLUME_DIVISIONS) {
        // Fallback: linear division
        for (int i = 1; i < VOLUME_DIVISIONS; ++i) {
            m_regionBoundaries.push_back(bounds.top + (totalHeight * i) / VOLUME_DIVISIONS);
        }
    } else {
        // Calculate effective min/max based on total height
        // If totalHeight is too small, we need to reduce minHeight
        // If totalHeight is too large, we need to increase maxHeight
        int effectiveMinHeight = MIN_REGION_HEIGHT;
        int effectiveMaxHeight = MAX_REGION_HEIGHT;

        int minPossibleTotal = MIN_REGION_HEIGHT * VOLUME_DIVISIONS;
        int maxPossibleTotal = MAX_REGION_HEIGHT * VOLUME_DIVISIONS;

        if (totalHeight < minPossibleTotal) {
            // Total height is too small, reduce min height
            effectiveMinHeight = std::max(1, totalHeight / VOLUME_DIVISIONS);
        } else if (totalHeight > maxPossibleTotal) {
            // Total height is too large, increase max height
            effectiveMaxHeight = (totalHeight + VOLUME_DIVISIONS - 1) / VOLUME_DIVISIONS;
        }

        // First pass: calculate ideal heights based on volume fractions
        std::vector<int> regionHeights(VOLUME_DIVISIONS);
        int minTotalHeight = effectiveMinHeight * VOLUME_DIVISIONS;
        int availableHeight = std::max(0, totalHeight - minTotalHeight);

        for (int i = 0; i < VOLUME_DIVISIONS; ++i) {
            // Reverse order: region 0 (top/high stress) = volumeFractions[19]
            double fraction = m_volumeFractions[VOLUME_DIVISIONS - 1 - i];
            int idealHeight = effectiveMinHeight + static_cast<int>(availableHeight * fraction);
            // Clamp to effective min/max constraints
            regionHeights[i] = std::clamp(idealHeight, effectiveMinHeight, effectiveMaxHeight);
        }

        // Second pass: adjust to fit total height exactly
        int currentTotal = 0;
        for (int h : regionHeights) {
            currentTotal += h;
        }

        int difference = totalHeight - currentTotal;

        // Distribute difference - prioritize regions that haven't hit limits
        while (difference != 0) {
            bool adjusted = false;
            int adjustPerRegion = (difference > 0) ? 1 : -1;

            for (int i = 0; i < VOLUME_DIVISIONS && difference != 0; ++i) {
                int newHeight = regionHeights[i] + adjustPerRegion;
                // For final adjustment, allow going beyond soft limits if necessary
                if (newHeight >= 1) {  // Only enforce absolute minimum of 1px
                    regionHeights[i] = newHeight;
                    difference -= adjustPerRegion;
                    adjusted = true;
                }
            }

            // Safety: if no adjustment was possible, break to avoid infinite loop
            if (!adjusted) {
                break;
            }
        }

        // Build boundaries from heights
        int cumulativeY = bounds.top;
        for (int i = 0; i < VOLUME_DIVISIONS - 1; ++i) {
            cumulativeY += regionHeights[i];
            m_regionBoundaries.push_back(cumulativeY);
        }
    }
    m_regionBoundaries.push_back(bounds.bottom);
}

void AdaptiveDensitySlider::updateStressDensityMappings() {
    assert(m_handles.size() == static_cast<size_t>(handleCount()));

    m_stressDensityMappings.clear();

    std::vector<double> handleStresses;
    for (int i = 0; i < handleCount(); ++i) {
        handleStresses.push_back(yToStress(m_handles[i]));
    }

    struct RegionStress {
        double minStress;
        double maxStress;
    };

    // Build regions dynamically: handles[0]=top/high stress, handles[last]=bottom/low stress
    std::vector<RegionStress> regions;
    regions.push_back({m_originalMinStress, handleStresses[handleCount() - 1]});
    for (int i = handleCount() - 1; i > 0; --i) {
        regions.push_back({handleStresses[i], handleStresses[i - 1]});
    }
    regions.push_back({handleStresses[0], m_originalMaxStress});

    for (int i = 0; i < m_regionCount; ++i) {
        int calculatedDensity = calculateDensityFromStress(regions[i].maxStress);
        m_regionPercents[i] = calculatedDensity;

        m_stressDensityMappings.push_back({
            regions[i].minStress,
            regions[i].maxStress,
            static_cast<double>(calculatedDensity)
        });
    }

    for (int i = 0; i < m_regionCount; ++i) {
        m_percentEdits[i]->setText(QString::number(m_regionPercents[i], 'g', 2));
    }
}

void AdaptiveDensitySlider::updateInitialHandles() {
    SliderBounds bounds = getSliderBounds();
    int availableHeight = bounds.bottom - bounds.top;
    int segmentHeight = availableHeight / m_regionCount;

    for (int i = 0; i < handleCount(); ++i) {
        m_handles[i] = bounds.top + (i + 1) * segmentHeight;
    }
}

void AdaptiveDensitySlider::updatePercentEditPositions() {
    SliderBounds bounds = getSliderBounds();
    std::vector<int> positions = getRegionPositions();
    for (int i = 0; i < m_regionCount; ++i) {
        int yCenter = (positions[i] + positions[i+1]) / 2;
        int editX = bounds.right + PERCENT_EDIT_GAP;
        int editY = yCenter - m_percentEdits[i]->height() / 2;
        m_percentEdits[i]->move(editX, editY);
        m_percentEdits[i]->show();
    }
}

// ... (inside mouseMoveEvent or standard helper methods)

void AdaptiveDensitySlider::clampHandles() {
    int top = m_margin;
    int bottom = height() - m_margin;
    
    // Special case for single handle
    if (m_handles.size() == 1) {
        m_handles[0] = std::clamp(m_handles[0], top, bottom);
        return;
    }

    for (int i = 0; i < (int)m_handles.size(); ++i) {
        if (i == 0)
            m_handles[i] = std::clamp(m_handles[i], top, m_handles[i+1] - m_minDistance);
        else if (i == (int)m_handles.size()-1)
            m_handles[i] = std::clamp(m_handles[i], m_handles[i-1] + m_minDistance, bottom);
        else
            m_handles[i] = std::clamp(m_handles[i], m_handles[i-1] + m_minDistance, m_handles[i+1] - m_minDistance);
    }
}



int AdaptiveDensitySlider::handleAtPosition(const QPoint& pos) const {
    SliderBounds bounds = getSliderBounds();
    int tolerance = 6;

    for (int i = 0; i < (int)m_handles.size(); ++i) {
        int y = m_handles[i];

        if (pos.x() >= bounds.gradLeft && pos.x() <= bounds.right && std::abs(pos.y() - y) <= tolerance) {
            return i;
        }

        if (pos.x() >= bounds.right && pos.x() <= bounds.right + TRIANGLE_SIZE &&
            pos.y() >= y - TRIANGLE_SIZE/2 && pos.y() <= y + TRIANGLE_SIZE/2) {
            return i;
        }
    }
    return -1;
}

double AdaptiveDensitySlider::yToStress(int y) const {
    if (m_regionBoundaries.size() != VOLUME_DIVISIONS + 1) {
        // Fallback: linear
        SliderBounds bounds = getSliderBounds();
        double t = static_cast<double>(y - bounds.bottom) / (bounds.top - bounds.bottom);
        return m_minStress + t * (m_maxStress - m_minStress);
    }

    // Find which region the y belongs to
    int regionIndex = 0;
    for (int i = 0; i < VOLUME_DIVISIONS; ++i) {
        if (y >= m_regionBoundaries[i] && y <= m_regionBoundaries[i + 1]) {
            regionIndex = i;
            break;
        }
    }

    // Relative position within the region
    int regionTop = m_regionBoundaries[regionIndex];
    int regionBottom = m_regionBoundaries[regionIndex + 1];
    double localT = (regionBottom > regionTop)
        ? static_cast<double>(y - regionTop) / (regionBottom - regionTop)
        : 0.0;

    // Stress range
    double stressStep = (m_maxStress - m_minStress) / VOLUME_DIVISIONS;
    double regionStressMax = m_maxStress - regionIndex * stressStep;
    double regionStressMin = regionStressMax - stressStep;

    return regionStressMax - localT * (regionStressMax - regionStressMin);
}

int AdaptiveDensitySlider::stressToY(double stress) const {
    if (m_regionBoundaries.size() != VOLUME_DIVISIONS + 1) {
        // Fallback: linear
        SliderBounds bounds = getSliderBounds();
        double t = (stress - m_minStress) / (m_maxStress - m_minStress);
        return bounds.bottom + static_cast<int>(t * (bounds.top - bounds.bottom));
    }

    // Find which region the stress belongs to
    double stressStep = (m_maxStress - m_minStress) / VOLUME_DIVISIONS;
    int regionIndex = static_cast<int>((m_maxStress - stress) / stressStep);
    regionIndex = std::clamp(regionIndex, 0, VOLUME_DIVISIONS - 1);

    double regionStressMax = m_maxStress - regionIndex * stressStep;
    double regionStressMin = regionStressMax - stressStep;

    int regionTop = m_regionBoundaries[regionIndex];
    int regionBottom = m_regionBoundaries[regionIndex + 1];

    double localT = (regionStressMax > regionStressMin)
        ? (regionStressMax - stress) / (regionStressMax - regionStressMin)
        : 0.0;

    return regionTop + static_cast<int>(localT * (regionBottom - regionTop));
}

int AdaptiveDensitySlider::calculateDensityFromStress(double stress) const {
    const double SAFE_FACTOR = SettingsManager::instance().safetyFactor();
    const double YIELD_STRENGTH = 30.0;
    const double C = 0.23;
    const double M = 2.0 / 3.0;

    const int minDensity = SettingsManager::instance().minDensity();
    const int maxDensity = SettingsManager::instance().maxDensity();

    double stressMPa = stress / 1e6;

    double numerator = SAFE_FACTOR * stressMPa;
    double denominator = YIELD_STRENGTH * C;
    double density = std::pow(numerator / denominator, M);

    double densityPercent = density * 100.0;

    int densityInt = static_cast<int>(std::round(densityPercent));
    densityInt = std::clamp(densityInt, minDensity, maxDensity);

    return densityInt;
}

AdaptiveDensitySlider::SliderBounds AdaptiveDensitySlider::getSliderBounds() const {
    int w = width();
    // Calculate total width of both bars (gradient bar + gap + slider)
    int totalBarsWidth = GRADIENT_WIDTH + GRADIENT_GAP + SLIDER_WIDTH;
    // Center both bars in the widget
    int barsLeft = w / 2 - totalBarsWidth / 2;

    SliderBounds bounds;
    bounds.gradLeft = barsLeft;
    bounds.gradRight = barsLeft + GRADIENT_WIDTH;
    bounds.left = barsLeft + GRADIENT_WIDTH + GRADIENT_GAP;
    bounds.right = bounds.left + SLIDER_WIDTH;
    bounds.top = m_margin;
    bounds.bottom = height() - m_margin;
    return bounds;
}

std::vector<int> AdaptiveDensitySlider::getRegionPositions() const {
    SliderBounds bounds = getSliderBounds();
    std::vector<int> positions = {bounds.bottom};
    for (auto it = m_handles.rbegin(); it != m_handles.rend(); ++it) {
        positions.push_back(*it);
    }
    positions.push_back(bounds.top);
    return positions;
}

// ====================
// Drawing Helpers
// ====================

void AdaptiveDensitySlider::drawGradientBar(QPainter& painter, const SliderBounds& bounds) {
    painter.setPen(Qt::NoPen);

    if (m_regionBoundaries.size() != VOLUME_DIVISIONS + 1) {
        // Fallback: conventional linear gradient
        QLinearGradient gradient(bounds.gradLeft, bounds.top, bounds.gradLeft, bounds.bottom);
        gradient.setColorAt(0.0, ColorManager::HIGH_COLOR);
        gradient.setColorAt(0.5, ColorManager::MIDDLE_COLOR);
        gradient.setColorAt(1.0, ColorManager::LOW_COLOR);
        painter.setBrush(gradient);
        painter.drawRect(bounds.gradLeft, bounds.top, GRADIENT_WIDTH, bounds.bottom - bounds.top);
        return;
    }

    // Draw each region individually
    for (int i = 0; i < VOLUME_DIVISIONS; ++i) {
        int regionTop = m_regionBoundaries[i];
        int regionBottom = m_regionBoundaries[i + 1];
        int regionHeight = regionBottom - regionTop;
        if (regionHeight <= 0) continue;

        // Stress normalized value
        // t=0.0 → HIGH_COLOR (red, high stress at top)
        // t=1.0 → LOW_COLOR (blue, low stress at bottom)
        double tTop = static_cast<double>(i) / VOLUME_DIVISIONS;
        double tBottom = static_cast<double>(i + 1) / VOLUME_DIVISIONS;

        QLinearGradient regionGradient(bounds.gradLeft, regionTop, bounds.gradLeft, regionBottom);
        regionGradient.setColorAt(0.0, ColorManager::getGradientColor(tTop));
        regionGradient.setColorAt(1.0, ColorManager::getGradientColor(tBottom));

        painter.setBrush(regionGradient);
        painter.drawRect(bounds.gradLeft, regionTop, GRADIENT_WIDTH, regionHeight);
    }

    // Draw tick marks at each boundary to show non-linear divisions
    // Ticks are drawn on the left side of gradient bar, touching the gradient bar
    static constexpr int TICK_WIDTH = 6;
    int tickRight = bounds.gradLeft;
    int tickLeft = tickRight - TICK_WIDTH;
    painter.setPen(QPen(QColor(180, 180, 180), 1));
    for (int i = 0; i <= VOLUME_DIVISIONS; ++i) {
        int y = m_regionBoundaries[i];
        painter.drawLine(tickLeft, y, tickRight, y);
    }
}

void AdaptiveDensitySlider::drawStressLabels(QPainter& painter, const SliderBounds& bounds) {
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    int labelWidth = GRADIENT_WIDTH + LABEL_EXTRA_WIDTH;
    int labelX = bounds.gradLeft - labelWidth - LABEL_GAP;

    painter.drawText(labelX, bounds.top - 5, labelWidth, 20, Qt::AlignRight | Qt::AlignVCenter,
                    QString::number(m_maxStress, 'g', 2));

    painter.drawText(labelX, bounds.bottom - 15, labelWidth, 20, Qt::AlignRight | Qt::AlignVCenter,
                    QString::number(m_minStress, 'g', 2));

    for (int y : m_handles) {
        double stress = yToStress(y);
        painter.drawText(labelX, y - 10, labelWidth, 20, Qt::AlignRight | Qt::AlignVCenter,
                        QString::number(stress, 'g', 2));
    }
}

void AdaptiveDensitySlider::drawSliderBody(QPainter& painter, const SliderBounds& bounds) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(60, 60, 60));
    painter.drawRect(bounds.left, bounds.top, SLIDER_WIDTH, bounds.bottom - bounds.top);
}

void AdaptiveDensitySlider::drawRegions(QPainter& painter, const SliderBounds& bounds) {
    std::vector<int> positions = getRegionPositions();
    for (int i = 0; i < m_regionCount; ++i) {
        // positions[i] = bottom edge of region (higher Y value, lower stress)
        // positions[i+1] = top edge of region (lower Y value, higher stress)
        int yBottom = positions[i];
        int yTop = positions[i+1];

        // Get stress values using non-linear conversion from adaptive gradient bar
        double stressBottom = yToStress(yBottom);
        double stressTop = yToStress(yTop);

        // Calculate average stress for this region
        double avgStress = (stressTop + stressBottom) / 2.0;

        // Normalize: t=0 for max stress (red), t=1 for min stress (blue)
        double t = (m_maxStress - avgStress) / (m_maxStress - m_minStress);
        t = std::clamp(t, 0.0, 1.0);

        QColor regionColor = ColorManager::getGradientColor(t);
        regionColor.setAlpha(190);
        painter.setBrush(regionColor);
        painter.drawRect(bounds.left, positions[i], SLIDER_WIDTH, positions[i+1] - positions[i]);
    }
}

void AdaptiveDensitySlider::drawHandles(QPainter& painter, const SliderBounds& bounds) {
    for (int y : m_handles) {
        painter.setPen(QPen(ColorManager::HANDLE_COLOR, 2));
        painter.drawLine(bounds.gradLeft, y, bounds.right, y);

        QPolygon triangle;
        triangle << QPoint(bounds.right, y)
                 << QPoint(bounds.right + TRIANGLE_SIZE, y - TRIANGLE_SIZE/2)
                 << QPoint(bounds.right + TRIANGLE_SIZE, y + TRIANGLE_SIZE/2);
        painter.setBrush(ColorManager::HANDLE_COLOR);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(triangle);
    }
}

void AdaptiveDensitySlider::drawAxisLabels(QPainter& painter, const SliderBounds& bounds) {
    QFont labelFont = painter.font();
    labelFont.setPointSize(11);
    labelFont.setBold(true);
    painter.setFont(labelFont);
    painter.setPen(Qt::white);

    int centerY = bounds.top + (bounds.bottom - bounds.top) / 2;
    int labelHeight = bounds.bottom - bounds.top;

    painter.save();
    int leftLabelX = bounds.gradLeft - VERTICAL_LABEL_DISTANCE;
    painter.translate(leftLabelX, centerY);
    painter.rotate(-90);
    QRect leftTextRect(-labelHeight / 2, -40, labelHeight, 80);
    painter.drawText(leftTextRect, Qt::AlignCenter, "Stress[Pa]");
    painter.restore();

    painter.save();
    int rightLabelX = bounds.right + RIGHT_LABEL_DISTANCE;
    painter.translate(rightLabelX, centerY);
    painter.rotate(-90);
    QRect rightTextRect(-labelHeight / 2, -40, labelHeight, 80);
    painter.drawText(rightTextRect, Qt::AlignCenter, "Infill Density [%]");
    painter.restore();
}

void AdaptiveDensitySlider::setRegionCount(int count) {
    if (count >= 2 && count <= 10 && count != m_regionCount) {
        rebuildForRegionCount(count);
    }
}

void AdaptiveDensitySlider::rebuildForRegionCount(int count) {
    m_regionCount = count;

    // Clear and delete old percent edit widgets
    for (QLineEdit* edit : m_percentEdits) {
        edit->hide();
        edit->deleteLater();
    }
    m_percentEdits.clear();

    // Resize handles and region percents
    m_handles.clear();
    m_handles.resize(handleCount(), 0);
    m_regionPercents.clear();
    m_regionPercents.resize(m_regionCount, 20.0);

    // Create new percent edit widgets
    for (int i = 0; i < m_regionCount; ++i) {
        QLineEdit* edit = new QLineEdit(this);
        edit->setFixedWidth(40);
        edit->setAlignment(Qt::AlignCenter);
        edit->setStyleSheet(QString("QLineEdit { color: %1; background-color: %2; border: 1px solid %3; border-radius: %4px; selection-background-color: #555555; }")
            .arg(ColorManager::INPUT_TEXT_COLOR.name())
            .arg(ColorManager::INPUT_BACKGROUND_COLOR.name())
            .arg(ColorManager::INPUT_BORDER_COLOR.name())
            .arg(StyleManager::RADIUS_SMALL));
        edit->setText(QString::number(m_regionPercents[i], 'g', 2));
        edit->setValidator(new QDoubleValidator(0, 100, 2, edit));
        connect(edit, &QLineEdit::editingFinished, this, &AdaptiveDensitySlider::onPercentEditChanged);
        m_percentEdits.push_back(edit);
    }

    // Recalculate everything
    updateInitialHandles();
    updateRegionBoundaries();
    updateStressDensityMappings();
    updatePercentEditPositions();
    update();

    emit regionPercentsChanged(m_regionPercents);
}
