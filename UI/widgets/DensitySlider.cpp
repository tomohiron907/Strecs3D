#include "DensitySlider.h"
#include "../ColorManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <algorithm>
#include <QLineEdit>
#include <QResizeEvent>
#include <QDoubleValidator>
#include <cassert>

// 指定した位置（0.0〜1.0）でグラデーション色を線形補間で取得
QColor getGradientColor(double t) {
    // グラデーションストップの定義（staticで一度だけ初期化）
    static const struct GradStop {
        double pos;
        QColor color;
    } gradStops[] = {
        {0.0, ColorManager::HIGH_COLOR},   // 赤
        {0.5, ColorManager::MIDDLE_COLOR}, // 白
        {1.0, ColorManager::LOW_COLOR}     // 青
    };
    
    if (t <= gradStops[0].pos) return gradStops[0].color;
    if (t >= gradStops[2].pos) return gradStops[2].color;
    for (int i = 0; i < 2; ++i) {
        if (t >= gradStops[i].pos && t <= gradStops[i+1].pos) {
            double localT = (t - gradStops[i].pos) / (gradStops[i+1].pos - gradStops[i].pos);
            QColor c1 = gradStops[i].color;
            QColor c2 = gradStops[i+1].color;
            int r = c1.red()   + (c2.red()   - c1.red())   * localT;
            int g = c1.green() + (c2.green() - c1.green()) * localT;
            int b = c1.blue()  + (c2.blue()  - c1.blue())  * localT;
            return QColor(r, g, b);
        }
    }
    return QColor(); // fallback
}

DensitySlider::DensitySlider(QWidget* parent)
    : QWidget(parent)
{
    // コンストラクタで固定値ではなく、後で計算する
    m_handles = {0, 0, 0}; // 仮の値
    setMinimumWidth(120);
    setMinimumHeight(220);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    // パーセント入力欄を4つ作成
    for (int i = 0; i < 4; ++i) {
        QLineEdit* edit = new QLineEdit(this);
        edit->setFixedWidth(40);
        edit->setAlignment(Qt::AlignCenter);
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

std::vector<int> DensitySlider::handlePositions() const {
    return m_handles;
}

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

void DensitySlider::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int w = width();
    int h = height();
    int x = w / 2;
    int sliderWidth = 30;
    int left = x - sliderWidth / 2;
    int right = x + sliderWidth / 2;
    int top = m_margin;
    int bottom = h - m_margin;

    // グラデーション長方形（スライダの左側に表示）
    int gradWidth = 30;
    int gradGap = 20; // スライダとグラデーションバーの間隔
    int gradLeft = left - gradGap - gradWidth; // スライダの中心から左右対称な距離
    int gradRight = gradLeft + gradWidth;
    QLinearGradient gradient(gradLeft, top, gradLeft, bottom);
    gradient.setColorAt(0.0, ColorManager::HIGH_COLOR);   // 赤
    gradient.setColorAt(0.5, ColorManager::MIDDLE_COLOR); // 白
    gradient.setColorAt(1.0, ColorManager::LOW_COLOR);    // 青
    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    painter.drawRect(gradLeft, top, gradWidth, bottom - top);

    // Stress値のラベルを描画
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    // ラベルのX座標: グラデーションバーの左側に余白を設けて表示
    int labelWidth = gradWidth + 28; // ラベルの幅を広げる
    int labelX = gradLeft - labelWidth - 8; // さらに左に余白を追加
    // 上側（最大値）
    painter.drawText(labelX, top - 5, labelWidth, 20, Qt::AlignRight | Qt::AlignVCenter,
                    QString::number(m_maxStress, 'g', 2));
    // 下側（最小値）
    painter.drawText(labelX, bottom - 15, labelWidth, 20, Qt::AlignRight | Qt::AlignVCenter,
                    QString::number(m_minStress, 'g', 2));

    for (int y : m_handles) {
        // y座標を0.0〜1.0に正規化（bottomが0、topが1になるように）
        double t = (double)(y - bottom) / (top - bottom); // top=1, bottom=0
        double stress = m_minStress + t * (m_maxStress - m_minStress);
        painter.drawText(labelX, y - 10, labelWidth, 20, Qt::AlignRight | Qt::AlignVCenter,
                        QString::number(stress, 'g', 2));
    }

    // 長方形（スライダ本体）
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(60, 60, 60));
    painter.drawRect(left, top, sliderWidth, bottom - top);

    // 領域の色
    std::vector<int> positions = {bottom};
    for (auto it = m_handles.rbegin(); it != m_handles.rend(); ++it) positions.push_back(*it);
    positions.push_back(top);
    for (int i = 0; i < 4; ++i) {
        // 領域の中心Y座標
        int yCenter = (positions[i] + positions[i+1]) / 2;
        // グラデーション範囲で正規化
        double t = (double)(yCenter - top) / (bottom - top);
        QColor regionColor = getGradientColor(t);
        regionColor.setAlpha(190); // 完全不透明
        painter.setBrush(regionColor);
        painter.drawRect(left, positions[i], sliderWidth, positions[i+1] - positions[i]);
    }

    // ハンドル
    for (int y : m_handles) {
        painter.setPen(QPen(ColorManager::HANDLE_COLOR, 2));
        painter.drawLine(gradLeft, y, right, y);
        
        // 右端に左向きの三角形を描画
        int triangleSize = 13;
        QPolygon triangle;
        triangle << QPoint(right, y)
                 << QPoint(right + triangleSize, y - triangleSize/2)
                 << QPoint(right + triangleSize, y + triangleSize/2);
        painter.setBrush(ColorManager::HANDLE_COLOR);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(triangle);
    }

    // パーセント入力欄の位置を更新
    updatePercentEditPositions();

    // スライダー右側に-90度回転した「von Mises Stress[Pa]」ラベルを描画
    painter.save();
    QFont labelFont = painter.font();
    labelFont.setPointSize(11); // お好みで調整
    labelFont.setBold(true);
    painter.setFont(labelFont);
    painter.setPen(Qt::white);
    // 描画位置を決める
    int labelY = top + (bottom - top) / 2; // スライダー中央
    int verticalLabelX = gradLeft - 60; // グラデーションバーより左に50px余白（必要に応じて調整）
    painter.translate(verticalLabelX, labelY);
    painter.rotate(-90);
    QString verticalLabel = "von Mises Stress[Pa]";
    QRect textRect(- (bottom - top) / 2, -40, (bottom - top), 80); // 幅・高さは調整
    painter.drawText(textRect, Qt::AlignCenter, verticalLabel);
    painter.restore();

    // スライダー右側に-90度回転した「Infill Density [%]」ラベルを描画
    painter.save();
    QFont rightLabelFont = painter.font();
    rightLabelFont.setPointSize(11);
    rightLabelFont.setBold(true);
    painter.setFont(rightLabelFont);
    painter.setPen(Qt::white);
    int rightLabelY = top + (bottom - top) / 2;
    int rightLabelX = right + 70; // パーセント欄よりさらに右
    painter.translate(rightLabelX, rightLabelY);
    painter.rotate(-90);
    QString rightVerticalLabel = "Infill Density [%]";
    QRect rightTextRect(- (bottom - top) / 2, -40, (bottom - top), 80);
    painter.drawText(rightTextRect, Qt::AlignCenter, rightVerticalLabel);
    painter.restore();
}

void DensitySlider::updatePercentEditPositions() {
    int w = width();
    int x = w / 2;
    int sliderWidth = 30;
    int right = x + sliderWidth / 2;
    int top = m_margin;
    int bottom = height() - m_margin;
    std::vector<int> positions = {bottom};
    for (auto it = m_handles.rbegin(); it != m_handles.rend(); ++it) positions.push_back(*it);
    positions.push_back(top);
    for (int i = 0; i < 4; ++i) {
        int yCenter = (positions[i] + positions[i+1]) / 2;
        int editX = right + 20; // スライダの右側に20px余白
        int editY = yCenter - m_percentEdits[i]->height() / 2;
        m_percentEdits[i]->move(editX, editY);
        m_percentEdits[i]->show();
    }
}

void DensitySlider::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updatePercentEditPositions();
    updateInitialHandles();
    updateStressDensityMappings();
}

int DensitySlider::handleAtPosition(const QPoint& pos) const {
    int w = width();
    int x = w / 2;
    int left = x - 15;
    int gradWidth = 30;
    int gradLeft = left - gradWidth - 30;
    int right = x + 15;
    int triangleSize = 13;
    int tolerance = 6; // ピクセル単位の許容範囲
    
    for (int i = 0; i < (int)m_handles.size(); ++i) {
        int y = m_handles[i];
        
        // 線の範囲内かどうか
        if (pos.x() >= gradLeft && pos.x() <= right && std::abs(pos.y() - y) <= tolerance) {
            return i;
        }
        
        // 三角形の範囲内かどうか
        if (pos.x() >= right && pos.x() <= right + triangleSize && 
            pos.y() >= y - triangleSize/2 && pos.y() <= y + triangleSize/2) {
            return i;
        }
    }
    return -1;
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

std::vector<double> DensitySlider::regionPercents() const {
    return m_regionPercents;
}

void DensitySlider::setRegionPercents(const std::vector<double>& percents) {
    if (percents.size() == 4) {
        m_regionPercents = percents;
        for (int i = 0; i < 4; ++i) {
            m_percentEdits[i]->setText(QString::number(m_regionPercents[i], 'g', 2));
        }
        updateStressDensityMappings();
        update();
        emit regionPercentsChanged(m_regionPercents);
    }
}

void DensitySlider::onPercentEditChanged() {
    bool changed = false;
    for (int i = 0; i < 4; ++i) {
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

std::vector<StressDensityMapping> DensitySlider::stressDensityMappings() const {
    return m_stressDensityMappings;
}

void DensitySlider::updateStressDensityMappings() {
    // 領域数は4固定
    assert(m_handles.size() == 3);
    int top = m_margin;
    int bottom = height() - m_margin;
    
    m_stressDensityMappings.clear();
    
    // スライダーのハンドル位置から stress 値を計算
    std::vector<double> handleStressValues;
    for (int y : m_handles) {
        double t = (double)(y - bottom) / (top - bottom);  // top=1, bottom=0に正規化
        double stress = m_minStress + t * (m_maxStress - m_minStress);
        handleStressValues.push_back(stress);
    }
    
    // 4つの領域を作成
    for (int i = 0; i < 4; ++i) {
        double stressMin, stressMax;
        
        if (i == 0) {
            // 最初の領域: vtuのmin_stress ～ スライダーの最初のハンドル値
            stressMin = m_originalMinStress;
            stressMax = handleStressValues[2];  // handles[2]は一番上のハンドル（最大stress）
        } else if (i == 3) {
            // 最後の領域: スライダーの最後のハンドル値 ～ vtuのmax_stress
            stressMin = handleStressValues[0];  // handles[0]は一番下のハンドル（最小stress）
            stressMax = m_originalMaxStress;
        } else {
            // 中間の領域: スライダーのハンドル値間
            stressMax = handleStressValues[2 - i];      // 2番目の領域: handles[1] ～ handles[2]
            stressMin = handleStressValues[2 - i + 1];  // 3番目の領域: handles[0] ～ handles[1]
        }
        
        double density = m_regionPercents[i];
        m_stressDensityMappings.push_back({stressMin, stressMax, density});
    }
}

std::vector<int> DensitySlider::stressThresholds() const {
    int top = m_margin;
    int bottom = height() - m_margin;
    std::vector<int> thresholds;
    thresholds.push_back(m_minStress);
    for (int y : m_handles) {
        double t = (double)(y - bottom) / (top - bottom);
        double stress = m_minStress + t * (m_maxStress - m_minStress);
        thresholds.push_back(stress);
    }
    thresholds.push_back(m_maxStress);
    std::sort(thresholds.begin(), thresholds.end()); // 昇順にソート
    return thresholds;
}

void DensitySlider::updateInitialHandles() {
    int top = m_margin;
    int bottom = height() - m_margin;
    int availableHeight = bottom - top;
    int segmentHeight = availableHeight / 4; // 4つの領域に分割
    
    m_handles[0] = top + segmentHeight;     // 1/4位置
    m_handles[1] = top + 2 * segmentHeight; // 2/4位置  
    m_handles[2] = top + 3 * segmentHeight; // 3/4位置
}

std::vector<QColor> DensitySlider::getRegionColors() const {
    std::vector<QColor> colors;
    int top = m_margin;
    int bottom = height() - m_margin;
    
    // 4つの領域の位置を計算
    std::vector<int> positions = {bottom};
    for (auto it = m_handles.rbegin(); it != m_handles.rend(); ++it) {
        positions.push_back(*it);
    }
    positions.push_back(top);
    
    // 各領域の色を計算
    for (int i = 0; i < 4; ++i) {
        // 領域の中心Y座標
        int yCenter = (positions[i] + positions[i+1]) / 2;
        // グラデーション範囲で正規化
        double t = (double)(yCenter - top) / (bottom - top);
        QColor regionColor = getGradientColor(t);
        colors.push_back(regionColor);
    }
    
    return colors;
} 