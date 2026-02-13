#ifndef SHOWCASEWIDGET_H
#define SHOWCASEWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QNetworkAccessManager>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QVector>

class ShowcaseWidget : public QWidget {
    Q_OBJECT
public:
    explicit ShowcaseWidget(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    struct CardInfo {
        QString filename;
        QString title;
        QFrame* card;
        QLabel* imageLabel;
        QLabel* titleLabel;
        bool loaded;
    };

    void setupUI();
    void createCard(const QString& filename);
    void rearrangeCards();
    void startFetching();
    void fetchNextImage();
    static QString filenameToTitle(const QString& filename);

    QScrollArea* m_scrollArea;
    QWidget* m_containerWidget;
    QGridLayout* m_gridLayout;
    QNetworkAccessManager* m_networkManager;
    QVector<CardInfo> m_cards;
    int m_nextFetchIndex = 0;
    bool m_fetchStarted = false;

    static constexpr int CARD_WIDTH = 280;
    static constexpr int CARD_SPACING = 16;
    static constexpr int CARD_IMAGE_HEIGHT = 200;
};

#endif // SHOWCASEWIDGET_H
