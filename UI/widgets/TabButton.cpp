#include "TabButton.h"
#include <QPainter>
#include <QFontMetrics>

TabButton::TabButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
    , m_active(false)
    , m_hovered(false)
    , m_activeTextColor(Qt::white)
    , m_inactiveTextColor(QColor(0x88, 0x88, 0x88))
    , m_hoverTextColor(QColor(0xCC, 0xCC, 0xCC))
{
    setCursor(Qt::PointingHandCursor);
    setFlat(true);

    QFont f = font();
    f.setPixelSize(FONT_SIZE);
    f.setWeight(QFont::Medium);
    setFont(f);

    updateStyle();
}

void TabButton::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        // updateStyle(); // No longer need to resize based on active state if size is same
        update();
    }
}

void TabButton::updateStyle()
{
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(text());
    int textHeight = fm.height();
    
    // Fixed size based on text only, no extra space for dot
    setFixedSize(textWidth + HORIZONTAL_PADDING * 2,
                 textHeight + VERTICAL_PADDING * 2);

    setStyleSheet("background: transparent; border: none;");
}

void TabButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QPushButton::enterEvent(event);
}

void TabButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QPushButton::leaveEvent(event);
}

void TabButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Determine text color based on state
    QColor textColor;
    if (m_active) {
        textColor = m_activeTextColor;
        
        // Draw active border (Pill shape)
        painter.setPen(QPen(QColor(60, 60, 60), 1)); 
        painter.setBrush(Qt::NoBrush);
        QRectF borderRect = rect().adjusted(1, 1, -1, -1);
        painter.drawRoundedRect(borderRect, borderRect.height()/2.0, borderRect.height()/2.0);
        
        // No dot, just text
    } else {
        textColor = m_hovered ? m_hoverTextColor : m_inactiveTextColor;
    }

    painter.setFont(font());
    painter.setPen(textColor);
    painter.drawText(rect(), Qt::AlignCenter, text());
}
