#include "Button.h"
#include <QPainter>
#include <QEvent>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "../../utils/ColorManager.h"
#include "../../utils/StyleManager.h"

// 静的メンバ変数の定義
QSize Button::s_globalIconSize(20, 20);

Button::Button(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    initializeDefaultValues();
    m_backgroundColor = ColorManager::BUTTON_COLOR;
    m_textColor = ColorManager::BUTTON_TEXT_COLOR;
    m_currentColor = m_backgroundColor;
    setupDefaultStyle();
}

Button::Button(const QString& text, const QColor& backgroundColor, 
                         const QColor& textColor, QWidget* parent)
    : QPushButton(text, parent)
{
    initializeDefaultValues();
    m_backgroundColor = backgroundColor;
    m_textColor = textColor;
    m_currentColor = m_backgroundColor;
    setupDefaultStyle();
}

void Button::initializeDefaultValues()
{
    m_hoverColor = ColorManager::BUTTON_HOVER_COLOR.lighter(120);
    m_pressedColor = ColorManager::BUTTON_PRESSED_COLOR.darker(120);
    m_borderRadius = StyleManager::BUTTON_RADIUS;
    m_paddingHorizontal = DEFAULT_PADDING_HORIZONTAL;
    m_paddingVertical = DEFAULT_PADDING_VERTICAL;
    m_animationEnabled = true;
    m_hoverAnimation = true;
    m_clickAnimation = true;
    m_isHovered = false;
    m_isPressed = false;
    m_isEmphasized = false;
    m_disabledColor = ColorManager::BUTTON_DISABLED_COLOR;
    m_disabledTextColor = ColorManager::BUTTON_DISABLED_TEXT_COLOR;
    m_emphasizedColor = ColorManager::BUTTON_EMPHASIZED_COLOR;
    m_iconSize = s_globalIconSize;
}

void Button::setupDefaultStyle()
{
    setMinimumHeight(DEFAULT_MINIMUM_HEIGHT);
    setCursor(Qt::PointingHandCursor);
    updateStyle();
}

void Button::setCustomStyle(const QColor& backgroundColor, const QColor& textColor)
{
    m_backgroundColor = backgroundColor;
    m_textColor = textColor;
    m_hoverColor = backgroundColor.lighter(120);
    m_pressedColor = backgroundColor.darker(120);
    m_currentColor = m_backgroundColor;
    updateStyle();
}

void Button::setHoverColor(const QColor& hoverColor)
{
    m_hoverColor = hoverColor;
}

void Button::setPressedColor(const QColor& pressedColor)
{
    m_pressedColor = pressedColor;
}

void Button::setBorderRadius(int radius)
{
    m_borderRadius = radius;
    updateStyle();
}

void Button::setPadding(int horizontal, int vertical)
{
    m_paddingHorizontal = horizontal;
    m_paddingVertical = vertical;
    updateStyle();
}

void Button::setAnimationEnabled(bool enabled)
{
    m_animationEnabled = enabled;
}

void Button::setHoverAnimation(bool enabled)
{
    m_hoverAnimation = enabled;
}

void Button::setClickAnimation(bool enabled)
{
    m_clickAnimation = enabled;
}

void Button::setEmphasized(bool emphasized)
{
    m_isEmphasized = emphasized;
    if (m_isEmphasized) {
        m_backgroundColor = m_emphasizedColor;
        m_textColor = Qt::black; // 強調表示時は黒文字
    } else {
        m_backgroundColor = ColorManager::BUTTON_COLOR;
        m_textColor = ColorManager::BUTTON_TEXT_COLOR;
    }
    m_currentColor = m_backgroundColor;
    updateStyle();
}

void Button::enterEvent(QEnterEvent* event)
{
    QPushButton::enterEvent(event);
    if (!isEnabled()) return;
    
    m_isHovered = true;
    
    if (m_animationEnabled && m_hoverAnimation) {
        createColorAnimation(m_currentColor, m_hoverColor, HOVER_ANIMATION_DURATION);
    } else {
        m_currentColor = m_hoverColor;
        update();
    }
}

void Button::leaveEvent(QEvent* event)
{
    QPushButton::leaveEvent(event);
    if (!isEnabled()) return;
    
    m_isHovered = false;
    
    if (m_animationEnabled && m_hoverAnimation) {
        createColorAnimation(m_currentColor, m_backgroundColor, HOVER_ANIMATION_DURATION);
    } else {
        m_currentColor = m_backgroundColor;
        update();
    }
}

void Button::mousePressEvent(QMouseEvent* event)
{
    QPushButton::mousePressEvent(event);
    if (!isEnabled()) return;
    
    m_isPressed = true;
    
    if (m_animationEnabled && m_clickAnimation) {
        createColorAnimation(m_currentColor, m_pressedColor, CLICK_ANIMATION_DURATION);
    } else {
        m_currentColor = m_pressedColor;
        update();
    }
}

void Button::mouseReleaseEvent(QMouseEvent* event)
{
    QPushButton::mouseReleaseEvent(event);
    if (!isEnabled()) return;
    
    m_isPressed = false;
    
    QColor targetColor = m_isHovered ? m_hoverColor : m_backgroundColor;
    if (m_animationEnabled && m_clickAnimation) {
        createColorAnimation(m_currentColor, targetColor, CLICK_ANIMATION_DURATION);
    } else {
        m_currentColor = targetColor;
        update();
    }
}

void Button::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QRect rect = this->rect();
    
    if (!isEnabled()) {
        paintDisabledState(painter, rect);
    } else {
        paintEnabledState(painter, rect);
    }
}

void Button::updateStyle()
{
    update();
}

QColor Button::currentColor() const
{
    return m_currentColor;
}

void Button::setCurrentColor(const QColor& color)
{
    m_currentColor = color;
    update();
}

void Button::setIcon(const QString& iconPath)
{
    m_iconPath = iconPath;
    update();
}

void Button::setIconDark(const QString& iconPath)
{
    m_iconDarkPath = iconPath;
    update();
}

void Button::setIconSize(const QSize& size)
{
    m_iconSize = size;
    update();
}

QString Button::getCurrentIconPath() const
{
    // 無効状態の場合は常に通常のアイコンを使用
    if (!isEnabled()) {
        return m_iconPath;
    }
    
    if (m_isEmphasized && !m_iconDarkPath.isEmpty()) {
        if (m_isHovered && !m_iconPath.isEmpty()) {
            return m_iconPath;
        }
        return m_iconDarkPath;
    }
    return m_iconPath;
}

void Button::setGlobalIconSize(const QSize& size)
{
    s_globalIconSize = size;
}

QSize Button::getGlobalIconSize()
{
    return s_globalIconSize;
}

void Button::createColorAnimation(const QColor& startColor, const QColor& endColor, int duration)
{
    QPropertyAnimation* animation = new QPropertyAnimation(this, "currentColor");
    animation->setDuration(duration);
    animation->setStartValue(startColor);
    animation->setEndValue(endColor);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Button::paintBackground(QPainter& painter, const QRect& rect) const
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_currentColor);
    painter.drawRoundedRect(rect, m_borderRadius, m_borderRadius);
    
    // エッジ（枠線）
    QPen edgePen(ColorManager::BUTTON_EDGE_COLOR);
    edgePen.setWidth(0);
    painter.setPen(edgePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect.adjusted(BORDER_EDGE_OFFSET, BORDER_EDGE_OFFSET, 
                                        -BORDER_EDGE_OFFSET, -BORDER_EDGE_OFFSET), 
                           m_borderRadius, m_borderRadius);
}

void Button::paintDisabledState(QPainter& painter, const QRect& rect)
{
    // 無効状態の背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_disabledColor);
    painter.drawRoundedRect(rect, m_borderRadius, m_borderRadius);
    
    paintIconAndText(painter, rect, true);
}

void Button::paintEnabledState(QPainter& painter, const QRect& rect)
{
    paintBackground(painter, rect);
    paintIconAndText(painter, rect, false);
}

void Button::paintIconAndText(QPainter& painter, const QRect& rect, bool isDisabled)
{
    QString iconPath = getCurrentIconPath();
    bool hasIcon = !iconPath.isEmpty();
    
    if (hasIcon) {
        QPixmap iconPixmap(iconPath);
        if (!iconPixmap.isNull()) {
            iconPixmap = iconPixmap.scaled(m_iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            
            // テキストとアイコンの配置計算
            QFontMetrics fm(font());
            int textWidth = fm.horizontalAdvance(text());
            int iconWidth = iconPixmap.width();
            int spacing = hasIcon && !text().isEmpty() ? ICON_TEXT_SPACING : 0;
            int totalWidth = iconWidth + spacing + textWidth;
            
            // 中央揃えのための開始位置
            int startX = rect.center().x() - totalWidth / 2;
            
            // アイコンを描画
            if (isDisabled) {
                painter.setOpacity(DISABLED_OPACITY);
            }
            
            QRect iconRect = calculateIconRect(iconPixmap, rect, totalWidth);
            iconRect.moveLeft(startX);
            painter.drawPixmap(iconRect, iconPixmap);
            
            if (isDisabled) {
                painter.setOpacity(1.0);
            }
            
            // テキストを描画（アイコンの右側）
            if (!text().isEmpty()) {
                painter.setPen(getTextColor(isDisabled));
                painter.setFont(font());
                
                QRect textRect = calculateTextRect(rect, startX, iconWidth, spacing, textWidth);
                painter.drawText(textRect, Qt::AlignCenter, text());
            }
        } else {
            // アイコンが読み込めない場合はテキストのみ
            painter.setPen(getTextColor(isDisabled));
            painter.setFont(font());
            QRect textRect = rect.adjusted(m_paddingHorizontal, m_paddingVertical, 
                                          -m_paddingHorizontal, -m_paddingVertical);
            painter.drawText(textRect, Qt::AlignCenter, text());
        }
    } else {
        // アイコンがない場合はテキストのみ
        painter.setPen(getTextColor(isDisabled));
        painter.setFont(font());
        QRect textRect = rect.adjusted(m_paddingHorizontal, m_paddingVertical, 
                                      -m_paddingHorizontal, -m_paddingVertical);
        painter.drawText(textRect, Qt::AlignCenter, text());
    }
}

QRect Button::calculateIconRect(const QPixmap& icon, const QRect& rect, int totalWidth) const
{
    int startX = rect.center().x() - totalWidth / 2;
    return QRect(startX, rect.center().y() - icon.height() / 2, 
                icon.width(), icon.height());
}

QRect Button::calculateTextRect(const QRect& rect, int startX, int iconWidth, int spacing, int textWidth) const
{
    return QRect(startX + iconWidth + spacing, 
                rect.y() + m_paddingVertical,
                textWidth, 
                rect.height() - 2 * m_paddingVertical);
}

QColor Button::getTextColor(bool isDisabled) const
{
    if (isDisabled) {
        return m_disabledTextColor;
    }
    
    QColor textColor = m_textColor;
    // 強調表示ボタンのホバー時は白文字に変更
    if (m_isEmphasized && m_isHovered) {
        textColor = Qt::white;
    }
    return textColor;
} 