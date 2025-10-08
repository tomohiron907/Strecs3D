#include "ModeComboBox.h"
#include "../ColorManager.h"
#include <QPainter>
#include <QStyleOptionComboBox>
#include <QEnterEvent>
#include <QMouseEvent>

ModeComboBox::ModeComboBox(QWidget* parent)
    : QComboBox(parent)
{
    addItem("cura");
    addItem("bambu");
    addItem("prusa");
    setMinimumHeight(40); // Buttonと同じ高さ
    setEditable(false); // どこをクリックしてもドロップダウン
    setStyleSheet(
        "QComboBox QAbstractItemView { text-align: center; } "
        "QComboBox::drop-down { subcontrol-position: right center; right: 12px; }"
    );
    m_currentColor = ColorManager::BUTTON_COLOR;
}

void ModeComboBox::enterEvent(QEnterEvent* event)
{
    QComboBox::enterEvent(event);
    m_isHovered = true;
    m_currentColor = ColorManager::BUTTON_HOVER_COLOR;
    update();
}

void ModeComboBox::leaveEvent(QEvent* event)
{
    QComboBox::leaveEvent(event);
    m_isHovered = false;
    m_currentColor = ColorManager::BUTTON_COLOR;
    update();
}

void ModeComboBox::mousePressEvent(QMouseEvent* event)
{
    QComboBox::mousePressEvent(event);
    m_isPressed = true;
    m_currentColor = ColorManager::BUTTON_PRESSED_COLOR;
    update();
}

void ModeComboBox::mouseReleaseEvent(QMouseEvent* event)
{
    QComboBox::mouseReleaseEvent(event);
    m_isPressed = false;
    m_currentColor = m_isHovered ? ColorManager::BUTTON_HOVER_COLOR : ColorManager::BUTTON_COLOR;
    update();
}

void ModeComboBox::paintEvent(QPaintEvent* event)
{
    QStyleOptionComboBox opt;
    opt.initFrom(this);
    opt.editable = false;
    opt.currentText = currentText();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    // 背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_currentColor);
    int borderRadius = 3;
    QRect rect = this->rect();
    painter.drawRoundedRect(rect, borderRadius, borderRadius);
    // 標準のコンボボックス部品（ドロップダウン矢印など）
    style()->drawComplexControl(QStyle::CC_ComboBox, &opt, &painter, this);
    // 枠線
    QPen edgePen(ColorManager::BUTTON_EDGE_COLOR);
    edgePen.setWidth(0);
    painter.setPen(edgePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect.adjusted(1, 1, -1, -1), borderRadius, borderRadius);
    // テキスト中央描画
    painter.setPen(QColor(255,255,255));
    painter.setFont(font());
    QRect textRect = rect.adjusted(12, 0, -32, 0); // 左右余白調整
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignHCenter, currentText());
} 