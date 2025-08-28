#include "Button.h"
#include <QPainter>
#include <QEvent>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "../ColorManager.h"

// 静的メンバ変数の定義
QSize Button::s_globalIconSize(20, 20);

Button::Button(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
    , m_backgroundColor(ColorManager::BUTTON_COLOR)
    , m_textColor(ColorManager::BUTTON_TEXT_COLOR)
    , m_hoverColor(ColorManager::BUTTON_HOVER_COLOR.lighter(120))
    , m_pressedColor(ColorManager::BUTTON_PRESSED_COLOR.darker(120))
    , m_currentColor(m_backgroundColor)
    , m_borderRadius(3)
    , m_paddingHorizontal(20)
    , m_paddingVertical(12)
    , m_animationEnabled(true)
    , m_hoverAnimation(true)
    , m_clickAnimation(true)
    , m_isHovered(false)
    , m_isPressed(false)
    , m_isEmphasized(false)
    , m_disabledColor(ColorManager::BUTTON_DISABLED_COLOR)
    , m_disabledTextColor(ColorManager::BUTTON_DISABLED_TEXT_COLOR)
    , m_emphasizedColor(ColorManager::BUTTON_EMPHASIZED_COLOR)
    , m_iconSize(s_globalIconSize)
{
    setupDefaultStyle();
}

Button::Button(const QString& text, const QColor& backgroundColor, 
                         const QColor& textColor, QWidget* parent)
    : QPushButton(text, parent)
    , m_backgroundColor(backgroundColor)
    , m_textColor(textColor)
    , m_hoverColor(ColorManager::BUTTON_HOVER_COLOR.lighter(120))
    , m_pressedColor(ColorManager::BUTTON_PRESSED_COLOR.darker(120))
    , m_currentColor(m_backgroundColor)
    , m_borderRadius(3)
    , m_paddingHorizontal(20)
    , m_paddingVertical(12)
    , m_animationEnabled(true)
    , m_hoverAnimation(true)
    , m_clickAnimation(true)
    , m_isHovered(false)
    , m_isPressed(false)
    , m_isEmphasized(false)
    , m_disabledColor(ColorManager::BUTTON_DISABLED_COLOR)
    , m_disabledTextColor(ColorManager::BUTTON_DISABLED_TEXT_COLOR)
    , m_emphasizedColor(ColorManager::BUTTON_EMPHASIZED_COLOR)
    , m_iconSize(s_globalIconSize)
{
    setupDefaultStyle();
}

void Button::setupDefaultStyle()
{
    setMinimumHeight(50);
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
        QPropertyAnimation* animation = new QPropertyAnimation(this, "currentColor");
        animation->setDuration(150);
        animation->setStartValue(m_currentColor);
        animation->setEndValue(m_hoverColor);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
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
        QPropertyAnimation* animation = new QPropertyAnimation(this, "currentColor");
        animation->setDuration(150);
        animation->setStartValue(m_currentColor);
        animation->setEndValue(m_backgroundColor);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
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
        QPropertyAnimation* animation = new QPropertyAnimation(this, "currentColor");
        animation->setDuration(100);
        animation->setStartValue(m_currentColor);
        animation->setEndValue(m_pressedColor);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
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
    
    if (m_animationEnabled && m_clickAnimation) {
        QPropertyAnimation* animation = new QPropertyAnimation(this, "currentColor");
        animation->setDuration(100);
        animation->setStartValue(m_currentColor);
        animation->setEndValue(m_isHovered ? m_hoverColor : m_backgroundColor);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        m_currentColor = m_isHovered ? m_hoverColor : m_backgroundColor;
        update();
    }
}

void Button::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QRect rect = this->rect();
    
    // 無効状態の場合
    if (!isEnabled()) {
        // 無効状態の背景
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_disabledColor);
        painter.drawRoundedRect(rect, m_borderRadius, m_borderRadius);
        
        // 無効状態でのアイコンとテキストの描画
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
                int spacing = hasIcon && !text().isEmpty() ? 8 : 0;
                int totalWidth = iconWidth + spacing + textWidth;
                
                // 中央揃えのための開始位置
                int startX = rect.center().x() - totalWidth / 2;
                
                // アイコンを描画（無効状態の透明度を適用）
                painter.setOpacity(0.5); // 無効状態の透明度
                QRect iconRect(startX, rect.center().y() - iconPixmap.height() / 2, 
                              iconPixmap.width(), iconPixmap.height());
                painter.drawPixmap(iconRect, iconPixmap);
                painter.setOpacity(1.0); // 透明度をリセット
                
                // テキストを描画（アイコンの右側）
                if (!text().isEmpty()) {
                    painter.setPen(m_disabledTextColor);
                    painter.setFont(font());
                    
                    QRect textRect(startX + iconWidth + spacing, 
                                  rect.y() + m_paddingVertical,
                                  textWidth, 
                                  rect.height() - 2 * m_paddingVertical);
                    painter.drawText(textRect, Qt::AlignCenter, text());
                }
            } else {
                // アイコンが読み込めない場合はテキストのみ
                painter.setPen(m_disabledTextColor);
                painter.setFont(font());
                QRect textRect = rect.adjusted(m_paddingHorizontal, m_paddingVertical, 
                                              -m_paddingHorizontal, -m_paddingVertical);
                painter.drawText(textRect, Qt::AlignCenter, text());
            }
        } else {
            // アイコンがない場合はテキストのみ
            painter.setPen(m_disabledTextColor);
            painter.setFont(font());
            QRect textRect = rect.adjusted(m_paddingHorizontal, m_paddingVertical, 
                                          -m_paddingHorizontal, -m_paddingVertical);
            painter.drawText(textRect, Qt::AlignCenter, text());
        }
        return;
    }
    
    // 通常状態
    // 背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_currentColor);
    painter.drawRoundedRect(rect, m_borderRadius, m_borderRadius);
    
    // エッジ（枠線）
    QPen edgePen(ColorManager::BUTTON_EDGE_COLOR);
    edgePen.setWidth(0); // 枠線の太さ
    painter.setPen(edgePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect.adjusted(1, 1, -1, -1), m_borderRadius, m_borderRadius);

    // アイコンとテキストの描画
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
            int spacing = hasIcon && !text().isEmpty() ? 8 : 0;
            int totalWidth = iconWidth + spacing + textWidth;
            
            // 中央揃えのための開始位置
            int startX = rect.center().x() - totalWidth / 2;
            
            // アイコンを描画
            QRect iconRect(startX, rect.center().y() - iconPixmap.height() / 2, 
                          iconPixmap.width(), iconPixmap.height());
            painter.drawPixmap(iconRect, iconPixmap);
            
            // テキストを描画（アイコンの右側）
            if (!text().isEmpty()) {
                QColor textColor = m_textColor;
                // 強調表示ボタンのホバー時は白文字に変更
                if (m_isEmphasized && m_isHovered) {
                    textColor = Qt::white;
                }
                painter.setPen(textColor);
                painter.setFont(font());
                
                QRect textRect(startX + iconWidth + spacing, 
                              rect.y() + m_paddingVertical,
                              textWidth, 
                              rect.height() - 2 * m_paddingVertical);
                painter.drawText(textRect, Qt::AlignCenter, text());
            }
        } else {
            // アイコンが読み込めない場合はテキストのみ
            QColor textColor = m_textColor;
            if (m_isEmphasized && m_isHovered) {
                textColor = Qt::white;
            }
            painter.setPen(textColor);
            painter.setFont(font());
            QRect textRect = rect.adjusted(m_paddingHorizontal, m_paddingVertical, 
                                          -m_paddingHorizontal, -m_paddingVertical);
            painter.drawText(textRect, Qt::AlignCenter, text());
        }
    } else {
        // アイコンがない場合はテキストのみ
        QColor textColor = m_textColor;
        if (m_isEmphasized && m_isHovered) {
            textColor = Qt::white;
        }
        painter.setPen(textColor);
        painter.setFont(font());
        QRect textRect = rect.adjusted(m_paddingHorizontal, m_paddingVertical, 
                                      -m_paddingHorizontal, -m_paddingVertical);
        painter.drawText(textRect, Qt::AlignCenter, text());
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