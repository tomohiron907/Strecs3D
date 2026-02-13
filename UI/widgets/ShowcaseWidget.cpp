#include "ShowcaseWidget.h"
#include "../../utils/ColorManager.h"
#include "../../utils/StyleManager.h"

#include <QVBoxLayout>
#include <QNetworkReply>
#include <QResizeEvent>
#include <QShowEvent>
#include <QPixmap>
#include <QTimer>

static const QStringList IMAGE_FILES = {
    "bearing_holder.png", "canti.png", "drone.png",
    "frame_connector.png", "wall_hook.png", "tablet_stand.png",
    "rod_connector.png", "motor_mount.png", "iphone_stand.png",
    "hook.png", "handle.png"
};

static const QString BASE_URL = "https://strecs3d.xyz/examples/";

ShowcaseWidget::ShowcaseWidget(QWidget* parent)
    : QWidget(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    setupUI();
}

void ShowcaseWidget::setupUI()
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

    for (const auto& filename : IMAGE_FILES) {
        createCard(filename);
    }

    rearrangeCards();

    m_scrollArea->setWidget(m_containerWidget);
    mainLayout->addWidget(m_scrollArea);
}

void ShowcaseWidget::createCard(const QString& filename)
{
    CardInfo info;
    info.filename = filename;
    info.loaded = false;

    info.imageLabel = new RoundedImageLabel(StyleManager::CONTAINER_RADIUS, m_containerWidget);
    info.imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    info.imageLabel->setPlaceholderText("Loading...");

    m_cards.append(info);
}

void ShowcaseWidget::rearrangeCards()
{
    if (m_cards.isEmpty()) return;

    while (m_gridLayout->count() > 0) {
        m_gridLayout->takeAt(0);
    }

    for (int i = 0; i < m_cards.size(); ++i) {
        int row = i / COLUMNS;
        int col = i % COLUMNS;
        m_gridLayout->addWidget(m_cards[i].imageLabel, row, col);
    }
}

void ShowcaseWidget::updateCardSizes()
{
    if (m_cards.isEmpty()) return;

    int viewportWidth = m_scrollArea->viewport()->width();
    if (viewportWidth <= 0) return;

    // cardWidth = (viewportWidth - left/right margins - spacing between columns) / columns
    int totalSpacing = CARD_SPACING * (COLUMNS - 1);
    int cardWidth = (viewportWidth - SIDE_MARGIN * 2 - totalSpacing) / COLUMNS;
    if (cardWidth < 100) cardWidth = 100;

    // 16:9 aspect ratio
    int cardHeight = cardWidth * 9 / 16;

    for (auto& card : m_cards) {
        card.imageLabel->setFixedHeight(cardHeight);
    }
}

void ShowcaseWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateCardSizes();
    rearrangeCards();
}

void ShowcaseWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    updateCardSizes();
    startFetching();
}

void ShowcaseWidget::startFetching()
{
    if (m_fetchStarted) return;
    m_fetchStarted = true;
    m_nextFetchIndex = 0;
    fetchNextImage();
}

void ShowcaseWidget::fetchNextImage()
{
    if (m_nextFetchIndex >= m_cards.size()) return;

    int index = m_nextFetchIndex++;
    QUrl url(BASE_URL + m_cards[index].filename);
    QNetworkReply* reply = m_networkManager->get(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, this, [this, index, reply]() {
        reply->deleteLater();
        if (index >= m_cards.size()) return;

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QPixmap pixmap;
            if (pixmap.loadFromData(data)) {
                m_cards[index].imageLabel->setPixmap(pixmap);
                m_cards[index].imageLabel->setPlaceholderText("");
                m_cards[index].loaded = true;
            }
        } else {
            m_cards[index].imageLabel->setPlaceholderText("No Image");
        }

        QTimer::singleShot(0, this, &ShowcaseWidget::fetchNextImage);
    });
}
