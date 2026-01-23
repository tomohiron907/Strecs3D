#ifndef TABBUTTON_H
#define TABBUTTON_H

#include <QPushButton>

class TabButton : public QPushButton
{
    Q_OBJECT

public:
    explicit TabButton(const QString& text, QWidget* parent = nullptr);

    void setActive(bool active);
    bool isActive() const { return m_active; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void updateStyle();

    static constexpr int UNDERLINE_HEIGHT = 3;
    static constexpr int HORIZONTAL_PADDING = 16;
    static constexpr int VERTICAL_PADDING = 8;

    bool m_active = false;
    QColor m_activeTextColor;
    QColor m_inactiveTextColor;
    QColor m_underlineColor;
};

#endif // TABBUTTON_H
