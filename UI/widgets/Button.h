#ifndef BUTTON_H
#define BUTTON_H

#include <QPushButton>
#include <QString>
#include <QColor>
#include <QEnterEvent>
#include <QIcon>
#include <QPixmap>

class Button : public QPushButton
{
    Q_OBJECT

public:
    explicit Button(const QString& text = "", QWidget* parent = nullptr);
    explicit Button(const QString& text, const QColor& backgroundColor, 
                        const QColor& textColor = Qt::white, QWidget* parent = nullptr);
    
    void setCustomStyle(const QColor& backgroundColor, const QColor& textColor = Qt::white);
    void setHoverColor(const QColor& hoverColor);
    void setPressedColor(const QColor& pressedColor);
    void setBorderRadius(int radius);
    void setPadding(int horizontal, int vertical);
    void setAnimationEnabled(bool enabled);
    void setHoverAnimation(bool enabled);
    void setClickAnimation(bool enabled);
    void setEmphasized(bool emphasized);
    void setIcon(const QString& iconPath);
    void setIconDark(const QString& iconPath);
    void setIconSize(const QSize& size);
    
    // 静的メソッド：全ボタンのアイコンサイズを一括設定
    static void setGlobalIconSize(const QSize& size);
    static QSize getGlobalIconSize();
    
    QColor currentColor() const;
    void setCurrentColor(const QColor& color);
    Q_PROPERTY(QColor currentColor READ currentColor WRITE setCurrentColor)

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void updateStyle();
    void setupDefaultStyle();
    void initializeDefaultValues();
    QString getCurrentIconPath() const;
    void paintBackground(QPainter& painter, const QRect& rect) const;
    void paintDisabledState(QPainter& painter, const QRect& rect);
    void paintEnabledState(QPainter& painter, const QRect& rect);
    void paintIconAndText(QPainter& painter, const QRect& rect, bool isDisabled = false);
    QRect calculateIconRect(const QPixmap& icon, const QRect& rect, int totalWidth) const;
    QRect calculateTextRect(const QRect& rect, int startX, int iconWidth, int spacing, int textWidth) const;
    QColor getTextColor(bool isDisabled = false) const;
    void createColorAnimation(const QColor& startColor, const QColor& endColor, int duration);
    
    // 定数
    static constexpr int DEFAULT_BORDER_RADIUS = 3;
    static constexpr int DEFAULT_PADDING_HORIZONTAL = 20;
    static constexpr int DEFAULT_PADDING_VERTICAL = 12;
    static constexpr int DEFAULT_MINIMUM_HEIGHT = 50;
    static constexpr int HOVER_ANIMATION_DURATION = 150;
    static constexpr int CLICK_ANIMATION_DURATION = 100;
    static constexpr int ICON_TEXT_SPACING = 8;
    static constexpr int BORDER_EDGE_OFFSET = 1;
    static constexpr double DISABLED_OPACITY = 0.5;
    
    QColor m_backgroundColor;
    QColor m_textColor;
    QColor m_hoverColor;
    QColor m_pressedColor;
    QColor m_currentColor;
    int m_borderRadius;
    int m_paddingHorizontal;
    int m_paddingVertical;
    bool m_animationEnabled;
    bool m_hoverAnimation;
    bool m_clickAnimation;
    bool m_isHovered;
    bool m_isPressed;
    bool m_isEmphasized;
    QColor m_disabledColor;
    QColor m_disabledTextColor;
    QColor m_emphasizedColor;
    QString m_iconPath;
    QString m_iconDarkPath;
    QSize m_iconSize;
    static QSize s_globalIconSize;
};

#endif // BUTTON_H 