#ifndef SHOWCASEWIDGET_H
#define SHOWCASEWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QNetworkAccessManager>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QVector>
#include <QPainter>
#include <QPainterPath>

// Custom label that paints a pixmap with rounded corners
class RoundedImageLabel : public QWidget {
    Q_OBJECT
public:
    explicit RoundedImageLabel(int radius, QWidget* parent = nullptr)
        : QWidget(parent), m_radius(radius) {}

    void setPixmap(const QPixmap& pixmap) {
        m_pixmap = pixmap;
        m_scaledCache = QPixmap();
        m_cachedSize = QSize();
        update();
    }
    void setPlaceholderText(const QString& text) { m_placeholder = text; update(); }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addRoundedRect(rect(), m_radius, m_radius);
        painter.setClipPath(path);

        if (!m_pixmap.isNull()) {
            if (m_cachedSize != size()) {
                m_scaledCache = m_pixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                m_cachedSize = size();
            }
            int x = (width() - m_scaledCache.width()) / 2;
            int y = (height() - m_scaledCache.height()) / 2;
            painter.fillRect(rect(), QColor("#222222"));
            painter.drawPixmap(x, y, m_scaledCache);
        } else {
            painter.fillRect(rect(), QColor("#222222"));
            painter.setPen(QColor("#888888"));
            painter.drawText(rect(), Qt::AlignCenter, m_placeholder);
        }
    }

private:
    QPixmap m_pixmap;
    QPixmap m_scaledCache;
    QSize m_cachedSize;
    QString m_placeholder = "Loading...";
    int m_radius;
};

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
        RoundedImageLabel* imageLabel;
        bool loaded;
    };

    void setupUI();
    void createCard(const QString& filename);
    void rearrangeCards();
    void updateCardSizes();
    void startFetching();
    void fetchNextImage();

    QScrollArea* m_scrollArea;
    QWidget* m_containerWidget;
    QGridLayout* m_gridLayout;
    QNetworkAccessManager* m_networkManager;
    QVector<CardInfo> m_cards;
    int m_nextFetchIndex = 0;
    bool m_fetchStarted = false;

    static constexpr int COLUMNS = 3;
    static constexpr int CARD_SPACING = 16;
    static constexpr int SIDE_MARGIN = 300;
};

#endif // SHOWCASEWIDGET_H
