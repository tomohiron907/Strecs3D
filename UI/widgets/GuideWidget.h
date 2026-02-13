#ifndef GUIDEWIDGET_H
#define GUIDEWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QScrollArea>
#include <QVector>

class GuideWidget : public QWidget {
    Q_OBJECT
public:
    explicit GuideWidget(QWidget* parent = nullptr);

    struct LinkCard {
        QString title;
        QString description;
        QString url;
        QString platform;
        QColor platformColor;
    };

protected:
    void resizeEvent(QResizeEvent* event) override;

private:

    void setupUI();
    QFrame* createCard(const LinkCard& card);
    void updateCardSizes();

    QScrollArea* m_scrollArea;
    QWidget* m_containerWidget;
    QGridLayout* m_gridLayout;
    QVector<QFrame*> m_cardWidgets;

    static constexpr int COLUMNS = 2;
    static constexpr int CARD_SPACING = 16;
    static constexpr int SIDE_MARGIN = 400;
    static constexpr int CARD_HEIGHT = 160;
};

#endif // GUIDEWIDGET_H
