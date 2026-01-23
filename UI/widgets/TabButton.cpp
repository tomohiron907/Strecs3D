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
    setFont(f);

    updateStyle();
}

void TabButton::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        update();
    }
}

void TabButton::updateStyle()
{
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(text());
    int textHeight = fm.height();

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
    } else if (m_hovered) {
        textColor = m_hoverTextColor;
    } else {
        textColor = m_inactiveTextColor;
    }

    painter.setFont(font());
    painter.setPen(textColor);
    painter.drawText(rect(), Qt::AlignCenter, text());
}
