#include "ShowcaseWidget.h"
#include "../../utils/ColorManager.h"
#include "../../utils/StyleManager.h"

#include <QVBoxLayout>
#include <QNetworkReply>
#include <QResizeEvent>
#include <QShowEvent>
#include <QPixmap>
#include <QFont>
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
    m_gridLayout->setContentsMargins(CARD_SPACING * 2, CARD_SPACING * 2,
                                      CARD_SPACING * 2, CARD_SPACING * 2);

    for (const auto& filename : IMAGE_FILES) {
        createCard(filename);
    }

    m_scrollArea->setWidget(m_containerWidget);
    mainLayout->addWidget(m_scrollArea);
}

void ShowcaseWidget::createCard(const QString& filename)
{
    CardInfo info;
    info.filename = filename;
    info.title = filenameToTitle(filename);
    info.loaded = false;

    info.card = new QFrame(m_containerWidget);
    info.card->setFrameShape(QFrame::NoFrame);
    info.card->setStyleSheet(
        QString("QFrame { background-color: %1; border-radius: %2px; }")
        .arg(ColorManager::BACKGROUND_COLOR.lighter(130).name())
        .arg(StyleManager::CONTAINER_RADIUS));

    QVBoxLayout* cardLayout = new QVBoxLayout(info.card);
    cardLayout->setContentsMargins(0, 0, 0, StyleManager::CONTAINER_PADDING);
    cardLayout->setSpacing(StyleManager::CONTAINER_PADDING);

    // Image label with placeholder
    info.imageLabel = new QLabel(info.card);
    info.imageLabel->setFixedHeight(CARD_IMAGE_HEIGHT);
    info.imageLabel->setAlignment(Qt::AlignCenter);
    info.imageLabel->setStyleSheet(
        QString("QLabel { background-color: %1; border-top-left-radius: %2px; "
                "border-top-right-radius: %2px; color: #888; }")
        .arg(ColorManager::BACKGROUND_COLOR.lighter(110).name())
        .arg(StyleManager::CONTAINER_RADIUS));
    info.imageLabel->setText("Loading...");
    cardLayout->addWidget(info.imageLabel);

    // Title label
    info.titleLabel = new QLabel(info.title, info.card);
    info.titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setPixelSize(StyleManager::FONT_SIZE_NORMAL);
    info.titleLabel->setFont(titleFont);
    info.titleLabel->setStyleSheet("QLabel { color: #ddd; background: transparent; }");
    cardLayout->addWidget(info.titleLabel);

    m_cards.append(info);
}

void ShowcaseWidget::rearrangeCards()
{
    if (m_cards.isEmpty()) return;

    // Remove all items from grid layout without deleting widgets
    while (m_gridLayout->count() > 0) {
        m_gridLayout->takeAt(0);
    }

    int availableWidth = m_scrollArea->viewport()->width() - CARD_SPACING * 4;
    int columns = qMax(1, availableWidth / (CARD_WIDTH + CARD_SPACING));

    for (int i = 0; i < m_cards.size(); ++i) {
        int row = i / columns;
        int col = i % columns;
        m_gridLayout->addWidget(m_cards[i].card, row, col);
        m_cards[i].card->setFixedWidth(CARD_WIDTH);
    }
}

void ShowcaseWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    rearrangeCards();
}

void ShowcaseWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
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
                QPixmap scaled = pixmap.scaled(
                    CARD_WIDTH, CARD_IMAGE_HEIGHT,
                    Qt::KeepAspectRatio, Qt::SmoothTransformation);
                m_cards[index].imageLabel->setPixmap(scaled);
                m_cards[index].imageLabel->setText("");
                m_cards[index].loaded = true;
            }
        } else {
            m_cards[index].imageLabel->setText("No Image");
        }

        // Fetch the next image after this one completes
        QTimer::singleShot(0, this, &ShowcaseWidget::fetchNextImage);
    });
}

QString ShowcaseWidget::filenameToTitle(const QString& filename)
{
    // Remove extension, replace underscores with spaces, capitalize each word
    QString name = filename;
    name = name.left(name.lastIndexOf('.'));
    QStringList words = name.split('_', Qt::SkipEmptyParts);
    for (auto& word : words) {
        word[0] = word[0].toUpper();
    }
    return words.join(' ');
}
