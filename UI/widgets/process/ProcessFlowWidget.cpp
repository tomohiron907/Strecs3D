#include "ProcessFlowWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QMouseEvent>
#include "../../ColorManager.h"

// --- ProcessCard Implementation ---

ProcessCard::ProcessCard(int stepNumber, const QString& title, QWidget* parent)
    : QWidget(parent), m_stepNumber(stepNumber), m_title(title)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(15, 12, 15, 12);
    layout->setSpacing(10);
    
    setAttribute(Qt::WA_StyledBackground, true);
    
    // Title
    m_textLabel = new QLabel(title, this);
    QFont font = m_textLabel->font();
    font.setPointSize(12);
    m_textLabel->setFont(font);
    
    // Centered text
    m_textLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(m_textLabel);
    layout->addStretch();
    
    setFixedHeight(60);
    updateStyle();
}

void ProcessCard::setActive(bool active) {
    if (m_isActive == active) return;
    m_isActive = active;
    updateStyle();
}

void ProcessCard::setCompleted(bool completed) {
    if (m_isCompleted == completed) return;
    m_isCompleted = completed;
    
    if (m_isCompleted) {
        // Completed state logic if needed (e.g. green border)
    }
    updateStyle();
}

void ProcessCard::setLocked(bool locked) {
    if (m_isLocked == locked) return;
    m_isLocked = locked;
    updateStyle();
}

void ProcessCard::setClickable(bool clickable) {
    if (m_isClickable == clickable) return;
    m_isClickable = clickable;
    // Update cursor and style when clickability changes
    if (m_isClickable) {
        setCursor(Qt::PointingHandCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void ProcessCard::mousePressEvent(QMouseEvent* event) {
    if (m_isClickable && event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QWidget::mousePressEvent(event);
}

void ProcessCard::enterEvent(QEnterEvent* event) {
    if (m_isClickable) {
        m_isHovered = true;
        updateStyle();
    }
    QWidget::enterEvent(event);
}

void ProcessCard::leaveEvent(QEvent* event) {
    if (m_isHovered) {
        m_isHovered = false;
        updateStyle();
    }
    QWidget::leaveEvent(event);
}

void ProcessCard::updateStyle() {
    QString style;
    QString textStyle;

    // Colors
    QString bgColor;
    QString borderColor;
    QString textColor;

    if (m_isActive) {
        // Active: Distinct Blue Background from ColorManager
        QColor accent = ColorManager::ACCENT_COLOR;
        bgColor = QString("rgba(%1, %2, %3, 102)").arg(accent.red()).arg(accent.green()).arg(accent.blue()); // ~40% opacity
        borderColor = QString("rgba(%1, %2, %3, 102)").arg(accent.red()).arg(accent.green()).arg(accent.blue());
        textColor = "#FFFFFF";
    } else if (m_isCompleted) {
        // Completed: Standard border
        if (m_isHovered && m_isClickable) {
            // Hover state: brighter background
            bgColor = "rgba(255, 255, 255, 0.12)";
            borderColor = "rgba(255, 255, 255, 0.4)";
            textColor = "#FFFFFF";
        } else {
            bgColor = "rgba(255, 255, 255, 0.05)";
            borderColor = "rgba(255, 255, 255, 0.3)"; // Visible border
            textColor = "#CCCCCC";
        }
    } else if (m_isLocked) {
        // Locked/Future: Dimmed but with border
        bgColor = "transparent";
        borderColor = "rgba(255, 255, 255, 0.1)"; // Visible border (fainter)
        textColor = "#666666";
    } else {
        // Pending/Accessible but not active: Normal
        bgColor = "rgba(255, 255, 255, 0.05)";
        borderColor = "rgba(255, 255, 255, 0.2)"; // Slightly clearer border
        textColor = "#AAAAAA";
    }

    style = QString(
        "ProcessCard {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 3px;"
        "}"
    ).arg(bgColor, borderColor);

    textStyle = QString("color: %1;").arg(textColor);

    this->setStyleSheet(style);
    m_textLabel->setStyleSheet(textStyle);
}

// --- ProcessFlowWidget Implementation ---

ProcessFlowWidget::ProcessFlowWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void ProcessFlowWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    // Create Steps
    m_cards.push_back(new ProcessCard(1, "1.Import STEP File", this));
    m_cards.push_back(new ProcessCard(2, "2.Set Boundary Conditions", this));
    m_cards.push_back(new ProcessCard(3, "3.Simulate", this));
    m_cards.push_back(new ProcessCard(4, "4.Build Infill Map", this));

    // Connect each card's clicked signal
    for (size_t i = 0; i < m_cards.size(); ++i) {
        ProcessCard* card = m_cards[i];
        mainLayout->addWidget(card);

        // Connect clicked signal to emit stepClicked with the corresponding ProcessStep
        connect(card, &ProcessCard::clicked, this, [this, i]() {
            emit stepClicked(static_cast<ProcessStep>(i));
        });
    }

    // Initial State
    setCurrentStep(ProcessStep::ImportStep);

    // Fixed height based on contents
    int totalHeight = 10 + 10 + (60 * 4) + (8 * 3); // margins + cards + spacing roughly
    setFixedHeight(totalHeight + 20); // Extra buffer
}

void ProcessFlowWidget::setCurrentStep(ProcessStep step)
{
    m_currentStep = step;

    for (size_t i = 0; i < m_cards.size(); ++i) {
        int stepIdx = static_cast<int>(i);
        int targetIdx = static_cast<int>(step);

        if (stepIdx < targetIdx) {
            // Past steps - completed and clickable
            m_cards[i]->setLocked(false);
            m_cards[i]->setActive(false);
            m_cards[i]->setCompleted(true);
            m_cards[i]->setClickable(true);
        } else if (stepIdx == targetIdx) {
            // Current step - active but not clickable
            m_cards[i]->setLocked(false);
            m_cards[i]->setActive(true);
            m_cards[i]->setCompleted(false);
            m_cards[i]->setClickable(false);
        } else {
            // Future steps - locked and not clickable
            m_cards[i]->setLocked(true);
            m_cards[i]->setActive(false);
            m_cards[i]->setCompleted(false);
            m_cards[i]->setClickable(false);
        }
    }
}

ProcessStep ProcessFlowWidget::currentStep() const
{
    return m_currentStep;
}

void ProcessFlowWidget::setStepCompleted(ProcessStep step, bool completed)
{
    int index = static_cast<int>(step);
    if (index >= 0 && index < m_cards.size()) {
        m_cards[index]->setCompleted(completed);
    }
}
