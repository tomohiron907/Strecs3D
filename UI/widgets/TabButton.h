#ifndef TABBUTTON_H
#define TABBUTTON_H

#include <QPushButton>
#include <QEnterEvent>

class TabButton : public QPushButton
{
    Q_OBJECT

public:
    explicit TabButton(const QString& text, QWidget* parent = nullptr);

    void setActive(bool active);
    bool isActive() const { return m_active; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void updateStyle();

    static constexpr int HORIZONTAL_PADDING = 20;
    static constexpr int VERTICAL_PADDING = 12;
    static constexpr int FONT_SIZE = 15;

    bool m_active = false;
    bool m_hovered = false;
    QColor m_activeTextColor;
    QColor m_inactiveTextColor;
    QColor m_hoverTextColor;
};

#endif // TABBUTTON_H
