#include "GuideWidget.h"
#include "../../utils/ColorManager.h"
#include "../../utils/StyleManager.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QResizeEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QMouseEvent>

// Clickable QFrame that opens a URL
class ClickableCard : public QFrame {
public:
    explicit ClickableCard(const QString& url, QWidget* parent = nullptr)
        : QFrame(parent), m_url(url) {
        setCursor(Qt::PointingHandCursor);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            QDesktopServices::openUrl(QUrl(m_url));
        }
        QFrame::mousePressEvent(event);
    }

private:
    QString m_url;
};

static const QVector<GuideWidget::LinkCard> LINK_CARDS = {
    {"GitHub Wiki",
     "Official documentation and usage guide for Strecs3D",
     "https://github.com/tomohiron907/Strecs3D/wiki",
     "GitHub", QColor("#238636")},
    {"Qiita Article",
     "Technical article about Strecs3D development",
     "https://qiita.com/tags/strecs3d",
     "Qiita", QColor("#55C500")},
    {"X (Twitter) #Strecs3D",
     "Community posts and updates on X",
     "https://x.com/hashtag/Strecs3D?src=hashtag_click",
     "X", QColor("#1DA1F2")},
    {"Reddit Post",
     "Discussion on r/3Dprinting about Strecs3D",
     "https://www.reddit.com/r/3Dprinting/comments/1mls4lq/i_created_strecs3d_a_free_infill_optimizer_that/",
     "Reddit", QColor("#FF4500")},
    {"YouTube Video",
     "Video demonstration of Strecs3D",
     "https://youtu.be/GLfKM9WXlbM?si=9hY99iBu64vkSdFh",
     "YouTube", QColor("#FF0000")},
};

GuideWidget::GuideWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void GuideWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(
        QString("QScrollArea { border: none; background-color: %1; }"
                "QScrollBar:vertical { background: %1; width: 8px; }"
                "QScrollBar::handle:vertical { background: #555; border-radius: 4px; }"
                "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }")
        .arg(ColorManager::BACKGROUND_COLOR.name()));

    m_containerWidget = new QWidget(m_scrollArea);
    m_containerWidget->setStyleSheet(
        QString("background-color: %1;").arg(ColorManager::BACKGROUND_COLOR.name()));

    m_gridLayout = new QGridLayout(m_containerWidget);
    m_gridLayout->setSpacing(CARD_SPACING);
    m_gridLayout->setContentsMargins(SIDE_MARGIN, CARD_SPACING * 2,
                                      SIDE_MARGIN, CARD_SPACING * 2);

    for (int c = 0; c < COLUMNS; ++c) {
        m_gridLayout->setColumnStretch(c, 1);
    }

    for (const auto& card : LINK_CARDS) {
        QFrame* cardWidget = createCard(card);
        int index = m_cardWidgets.size();
        int row = index / COLUMNS;
        int col = index % COLUMNS;
        m_gridLayout->addWidget(cardWidget, row, col);
        m_cardWidgets.append(cardWidget);
    }

    m_scrollArea->setWidget(m_containerWidget);
    mainLayout->addWidget(m_scrollArea);
}

QFrame* GuideWidget::createCard(const LinkCard& card)
{
    ClickableCard* frame = new ClickableCard(card.url, m_containerWidget);
    frame->setObjectName("guideCard");
    frame->setStyleSheet(
        QString("#guideCard {"
                "  background-color: #2a2a2a;"
                "  border-radius: %1px;"
                "}"
                "#guideCard:hover {"
                "  background-color: #353535;"
                "}")
        .arg(StyleManager::CONTAINER_RADIUS));

    QVBoxLayout* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(8);

    // Platform label
    QLabel* platformLabel = new QLabel(card.platform, frame);
    platformLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; font-weight: bold; background: transparent;")
        .arg(card.platformColor.name()));
    layout->addWidget(platformLabel);

    // Title
    QLabel* titleLabel = new QLabel(card.title, frame);
    titleLabel->setStyleSheet(
        "color: #ffffff; font-size: 16px; font-weight: bold; background: transparent;");
    titleLabel->setWordWrap(true);
    layout->addWidget(titleLabel);

    // Description
    QLabel* descLabel = new QLabel(card.description, frame);
    descLabel->setStyleSheet(
        "color: #888888; font-size: 12px; background: transparent;");
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    layout->addStretch();

    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    frame->setFixedHeight(CARD_HEIGHT);

    return frame;
}

void GuideWidget::updateCardSizes()
{
    if (m_cardWidgets.isEmpty()) return;

    int viewportWidth = m_scrollArea->viewport()->width();
    if (viewportWidth <= 0) return;

    int totalSpacing = CARD_SPACING * (COLUMNS - 1);
    int cardWidth = (viewportWidth - SIDE_MARGIN * 2 - totalSpacing) / COLUMNS;
    if (cardWidth < 150) cardWidth = 150;

    for (auto* card : m_cardWidgets) {
        card->setMinimumWidth(cardWidth);
    }
}

void GuideWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateCardSizes();
}
