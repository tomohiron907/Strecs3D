#include "DensitySlider.h"
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

DensitySlider::DensitySlider(QWidget* parent)
    : QWidget(parent)
{
    m_handles.resize(HANDLE_COUNT, 0);
    m_regionPercents.resize(REGION_COUNT, 20.0);
    setMinimumWidth(120);
    setMinimumHeight(220);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    for (int i = 0; i < REGION_COUNT; ++i) {
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
        connect(edit, &QLineEdit::editingFinished, this, &DensitySlider::onPercentEditChanged);
        m_percentEdits.push_back(edit);
    }
    updateInitialHandles();
    updateStressDensityMappings();
}

QSize DensitySlider::minimumSizeHint() const {
    return QSize(60, 220);
}

QSize DensitySlider::sizeHint() const {
    return QSize(60, 300);
}

// ====================
// Primary Public API
// ====================

void DensitySlider::setStressRange(double minStress, double maxStress) {
    m_minStress = minStress;
    m_maxStress = maxStress;
    updateInitialHandles();
    updateStressDensityMappings();
    update();
}

void DensitySlider::setOriginalStressRange(double minStress, double maxStress) {
    m_originalMinStress = minStress;
    m_originalMaxStress = maxStress;
    // 初期表示用にスライダーの範囲も設定
    setStressRange(minStress, maxStress);
}

std::vector<StressDensityMapping> DensitySlider::stressDensityMappings() const {
    return m_stressDensityMappings;
}

void DensitySlider::setRegionPercents(const std::vector<double>& percents) {
    if (percents.size() == REGION_COUNT) {
        m_regionPercents = percents;
        for (int i = 0; i < REGION_COUNT; ++i) {
            m_percentEdits[i]->setText(QString::number(m_regionPercents[i], 'g', 2));
        }
        updateStressDensityMappings();
        update();
        emit regionPercentsChanged(m_regionPercents);
    }
}

std::vector<double> DensitySlider::regionPercents() const {
    return m_regionPercents;
}

int DensitySlider::countMaxDensityRegions() const {
    int count = 0;
    for (double percent : m_regionPercents) {
        if (static_cast<int>(percent) >= SettingsManager::instance().maxDensity()) {
            count++;
        }
    }
    return count;
}

// ====================
// Other Public API
// ====================

std::vector<int> DensitySlider::handlePositions() const {
    return m_handles;
}

std::vector<int> DensitySlider::stressThresholds() const {
    std::vector<int> thresholds;
    thresholds.push_back(m_minStress);
    for (int y : m_handles) {
        thresholds.push_back(yToStress(y));
    }
    thresholds.push_back(m_maxStress);
    std::sort(thresholds.begin(), thresholds.end()); // 昇順にソート
    return thresholds;
}

std::vector<QColor> DensitySlider::getRegionColors() const {
    std::vector<QColor> colors;
    SliderBounds bounds = getSliderBounds();
    std::vector<int> positions = getRegionPositions();

    // 各領域の色を計算
    for (int i = 0; i < REGION_COUNT; ++i) {
        // 領域の中心Y座標
        int yCenter = (positions[i] + positions[i+1]) / 2;
        // グラデーション範囲で正規化
        double t = (double)(yCenter - bounds.top) / (bounds.bottom - bounds.top);
        QColor regionColor = ColorManager::getGradientColor(t);
        colors.push_back(regionColor);
    }

    return colors;
}

// ====================
// Event Handlers
// ====================

void DensitySlider::paintEvent(QPaintEvent*) {
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

void DensitySlider::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updatePercentEditPositions();
    updateInitialHandles();
    updateStressDensityMappings();
}

void DensitySlider::mousePressEvent(QMouseEvent* event) {
    m_draggedHandle = handleAtPosition(event->pos());
}

void DensitySlider::mouseMoveEvent(QMouseEvent* event) {
    if (m_draggedHandle >= 0) {
        int y = std::clamp(event->pos().y(), m_margin, height() - m_margin);
        // ハンドル間の最小距離を保つ
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

void DensitySlider::mouseReleaseEvent(QMouseEvent*) {
    m_draggedHandle = -1;
}

// ====================
// Slots
// ====================

void DensitySlider::onPercentEditChanged() {
    bool changed = false;
    for (int i = 0; i < REGION_COUNT; ++i) {
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

void DensitySlider::updateStressDensityMappings() {
    assert(m_handles.size() == HANDLE_COUNT);

    m_stressDensityMappings.clear();

    std::vector<double> handleStresses;
    for (int i = 0; i < HANDLE_COUNT; ++i) {
        handleStresses.push_back(yToStress(m_handles[i]));
    }

    struct RegionStress {
        double minStress;
        double maxStress;
    };

    // Build regions dynamically: handles[0]=top/high stress, handles[last]=bottom/low stress
    std::vector<RegionStress> regions;
    regions.push_back({m_originalMinStress, handleStresses[HANDLE_COUNT - 1]});
    for (int i = HANDLE_COUNT - 1; i > 0; --i) {
        regions.push_back({handleStresses[i], handleStresses[i - 1]});
    }
    regions.push_back({handleStresses[0], m_originalMaxStress});

    for (int i = 0; i < REGION_COUNT; ++i) {
        int calculatedDensity = calculateDensityFromStress(regions[i].maxStress);
        m_regionPercents[i] = calculatedDensity;

        m_stressDensityMappings.push_back({
            regions[i].minStress,
            regions[i].maxStress,
            static_cast<double>(calculatedDensity)
        });
    }

    for (int i = 0; i < REGION_COUNT; ++i) {
        m_percentEdits[i]->setText(QString::number(m_regionPercents[i], 'g', 2));
    }
}

void DensitySlider::updateInitialHandles() {
    SliderBounds bounds = getSliderBounds();
    int availableHeight = bounds.bottom - bounds.top;
    int segmentHeight = availableHeight / REGION_COUNT;

    for (int i = 0; i < HANDLE_COUNT; ++i) {
        m_handles[i] = bounds.top + (i + 1) * segmentHeight;
    }
}

void DensitySlider::updatePercentEditPositions() {
    SliderBounds bounds = getSliderBounds();
    std::vector<int> positions = getRegionPositions();
    for (int i = 0; i < REGION_COUNT; ++i) {
        int yCenter = (positions[i] + positions[i+1]) / 2;
        int editX = bounds.right + PERCENT_EDIT_GAP;
        int editY = yCenter - m_percentEdits[i]->height() / 2;
        m_percentEdits[i]->move(editX, editY);
        m_percentEdits[i]->show();
    }
}

void DensitySlider::clampHandles() {
    int top = m_margin;
    int bottom = height() - m_margin;
    for (int i = 0; i < (int)m_handles.size(); ++i) {
        if (i == 0)
            m_handles[i] = std::clamp(m_handles[i], top, m_handles[i+1] - m_minDistance);
        else if (i == (int)m_handles.size()-1)
            m_handles[i] = std::clamp(m_handles[i], m_handles[i-1] + m_minDistance, bottom);
        else
            m_handles[i] = std::clamp(m_handles[i], m_handles[i-1] + m_minDistance, m_handles[i+1] - m_minDistance);
    }
}

int DensitySlider::handleAtPosition(const QPoint& pos) const {
    SliderBounds bounds = getSliderBounds();
    int tolerance = 6; // ピクセル単位の許容範囲

    for (int i = 0; i < (int)m_handles.size(); ++i) {
        int y = m_handles[i];

        // 線の範囲内かどうか
        if (pos.x() >= bounds.gradLeft && pos.x() <= bounds.right && std::abs(pos.y() - y) <= tolerance) {
            return i;
        }

        // 三角形の範囲内かどうか
        if (pos.x() >= bounds.right && pos.x() <= bounds.right + TRIANGLE_SIZE &&
            pos.y() >= y - TRIANGLE_SIZE/2 && pos.y() <= y + TRIANGLE_SIZE/2) {
            return i;
        }
    }
    return -1;
}

double DensitySlider::yToStress(int y) const {
    SliderBounds bounds = getSliderBounds();
    double t = (double)(y - bounds.bottom) / (bounds.top - bounds.bottom);
    return m_minStress + t * (m_maxStress - m_minStress);
}

int DensitySlider::calculateDensityFromStress(double stress) const {
    // 定数
    const double SAFE_FACTOR = 3.0;
    const double YIELD_STRENGTH = 30.0;
    const double C = 0.23;
    const double M = 2.0 / 3.0;

    // SettingsManagerから密度の最小値・最大値を取得
    const int minDensity = SettingsManager::instance().minDensity();
    const int maxDensity = SettingsManager::instance().maxDensity();

    // Pa から MPa に変換
    double stressMPa = stress / 1e6;

    // density = ((safe_factor * stress) / (yield_strength * C))^m
    double numerator = SAFE_FACTOR * stressMPa;
    double denominator = YIELD_STRENGTH * C;
    double density = std::pow(numerator / denominator, M);

    // パーセンテージに変換（0-1 → 0-100）
    double densityPercent = density * 100.0;

    // 整数に変換（設定された範囲でクランプ）
    int densityInt = static_cast<int>(std::round(densityPercent));
    densityInt = std::clamp(densityInt, minDensity, maxDensity);

    return densityInt;
}

DensitySlider::SliderBounds DensitySlider::getSliderBounds() const {
    int w = width();
    int x = w / 2;
    SliderBounds bounds;
    bounds.left = x - SLIDER_WIDTH / 2;
    bounds.right = x + SLIDER_WIDTH / 2;
    bounds.top = m_margin;
    bounds.bottom = height() - m_margin;
    bounds.gradLeft = bounds.left - GRADIENT_GAP - GRADIENT_WIDTH;
    bounds.gradRight = bounds.gradLeft + GRADIENT_WIDTH;
    return bounds;
}

std::vector<int> DensitySlider::getRegionPositions() const {
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

void DensitySlider::drawGradientBar(QPainter& painter, const SliderBounds& bounds) {
    QLinearGradient gradient(bounds.gradLeft, bounds.top, bounds.gradLeft, bounds.bottom);
    gradient.setColorAt(0.0, ColorManager::HIGH_COLOR);
    gradient.setColorAt(0.5, ColorManager::MIDDLE_COLOR);
    gradient.setColorAt(1.0, ColorManager::LOW_COLOR);
    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    painter.drawRect(bounds.gradLeft, bounds.top, GRADIENT_WIDTH, bounds.bottom - bounds.top);
}

void DensitySlider::drawStressLabels(QPainter& painter, const SliderBounds& bounds) {
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    int labelWidth = GRADIENT_WIDTH + LABEL_EXTRA_WIDTH;
    int labelX = bounds.gradLeft - labelWidth - LABEL_GAP;

    // 上側（最大値）
    painter.drawText(labelX, bounds.top - 5, labelWidth, 20, Qt::AlignRight | Qt::AlignVCenter,
                    QString::number(m_maxStress, 'g', 2));

    // 下側（最小値）
    painter.drawText(labelX, bounds.bottom - 15, labelWidth, 20, Qt::AlignRight | Qt::AlignVCenter,
                    QString::number(m_minStress, 'g', 2));

    // ハンドル位置のストレス値
    for (int y : m_handles) {
        double stress = yToStress(y);
        painter.drawText(labelX, y - 10, labelWidth, 20, Qt::AlignRight | Qt::AlignVCenter,
                        QString::number(stress, 'g', 2));
    }
}

void DensitySlider::drawSliderBody(QPainter& painter, const SliderBounds& bounds) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(60, 60, 60));
    painter.drawRect(bounds.left, bounds.top, SLIDER_WIDTH, bounds.bottom - bounds.top);
}

void DensitySlider::drawRegions(QPainter& painter, const SliderBounds& bounds) {
    std::vector<int> positions = getRegionPositions();
    for (int i = 0; i < REGION_COUNT; ++i) {
        int yCenter = (positions[i] + positions[i+1]) / 2;
        double t = (double)(yCenter - bounds.top) / (bounds.bottom - bounds.top);
        QColor regionColor = ColorManager::getGradientColor(t);
        regionColor.setAlpha(190);
        painter.setBrush(regionColor);
        painter.drawRect(bounds.left, positions[i], SLIDER_WIDTH, positions[i+1] - positions[i]);
    }
}

void DensitySlider::drawHandles(QPainter& painter, const SliderBounds& bounds) {
    for (int y : m_handles) {
        // ハンドル線を描画
        painter.setPen(QPen(ColorManager::HANDLE_COLOR, 2));
        painter.drawLine(bounds.gradLeft, y, bounds.right, y);

        // 右端に三角形を描画
        QPolygon triangle;
        triangle << QPoint(bounds.right, y)
                 << QPoint(bounds.right + TRIANGLE_SIZE, y - TRIANGLE_SIZE/2)
                 << QPoint(bounds.right + TRIANGLE_SIZE, y + TRIANGLE_SIZE/2);
        painter.setBrush(ColorManager::HANDLE_COLOR);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(triangle);
    }
}

void DensitySlider::drawAxisLabels(QPainter& painter, const SliderBounds& bounds) {
    QFont labelFont = painter.font();
    labelFont.setPointSize(11);
    labelFont.setBold(true);
    painter.setFont(labelFont);
    painter.setPen(Qt::white);

    int centerY = bounds.top + (bounds.bottom - bounds.top) / 2;
    int labelHeight = bounds.bottom - bounds.top;

    // 左側ラベル: von Mises Stress[Pa]
    painter.save();
    int leftLabelX = bounds.gradLeft - VERTICAL_LABEL_DISTANCE;
    painter.translate(leftLabelX, centerY);
    painter.rotate(-90);
    QRect leftTextRect(-labelHeight / 2, -40, labelHeight, 80);
    painter.drawText(leftTextRect, Qt::AlignCenter, "Stress[Pa]");
    painter.restore();

    // 右側ラベル: Infill Density [%]
    painter.save();
    int rightLabelX = bounds.right + RIGHT_LABEL_DISTANCE;
    painter.translate(rightLabelX, centerY);
    painter.rotate(-90);
    QRect rightTextRect(-labelHeight / 2, -40, labelHeight, 80);
    painter.drawText(rightTextRect, Qt::AlignCenter, "Infill Density [%]");
    painter.restore();
} 