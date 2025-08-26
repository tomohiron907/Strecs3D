#ifndef BUTTON_H
#define BUTTON_H

#include <QPushButton>
#include <QString>
#include <QColor>
#include <QEnterEvent>

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
};

#endif // BUTTON_H 