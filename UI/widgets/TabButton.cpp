#include "TabButton.h"
#include <QPainter>
#include <QFontMetrics>

TabButton::TabButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
    , m_active(false)
    , m_activeTextColor(Qt::white)
    , m_inactiveTextColor(QColor(0x88, 0x88, 0x88))
    , m_underlineColor(Qt::white)
{
    setCursor(Qt::PointingHandCursor);
    setFlat(true);
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
                 textHeight + VERTICAL_PADDING * 2 + UNDERLINE_HEIGHT);

    setStyleSheet("background: transparent; border: none;");
}

void TabButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw text
    QColor textColor = m_active ? m_activeTextColor : m_inactiveTextColor;
    painter.setPen(textColor);

    QRect textRect = rect();
    textRect.setBottom(textRect.bottom() - UNDERLINE_HEIGHT);
    painter.drawText(textRect, Qt::AlignCenter, text());

    // Draw underline if active
    if (m_active) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_underlineColor);

        QRect underlineRect(0, height() - UNDERLINE_HEIGHT, width(), UNDERLINE_HEIGHT);
        painter.drawRect(underlineRect);
    }
}
