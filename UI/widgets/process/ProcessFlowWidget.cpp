#include "ProcessFlowWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include "../../../utils/ColorManager.h"
#include "../../../utils/StyleManager.h"

// --- ProcessCard Implementation ---

ProcessCard::ProcessCard(int stepNumber, const QString& title, bool isFirst, bool isLast, QWidget* parent)
    : QWidget(parent), m_stepNumber(stepNumber), m_title(title), m_isFirst(isFirst), m_isLast(isLast)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    // Left padding increases to make room for the stepper graphic (approx 50px)
    layout->setContentsMargins(50, 12, 15, 12);
    layout->setSpacing(10);
    
    setAttribute(Qt::WA_StyledBackground, true);
    
    // Title
    m_textLabel = new QLabel(title, this);
    QFont font = m_textLabel->font();
    font.setPointSize(12);
    m_textLabel->setFont(font);
    
    // Left aligned text for better stepper look? Or Keep Center?
    // User image shows text aligned left relative to the content area (right of the dots)
    m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    layout->addWidget(m_textLabel);
    layout->addStretch();
    
    setFixedHeight(60);
    // Don't call updateStyle() here yet, we'll do it after setup or rely on defaults
    // But we need to set initial style.
    updateStyle();
}

void ProcessCard::setFirst(bool first) {
    if (m_isFirst == first) return;
    m_isFirst = first;
    update();
}

void ProcessCard::setLast(bool last) {
    if (m_isLast == last) return;
    m_isLast = last;
    update();
}

void ProcessCard::setActive(bool active) {
    if (m_isActive == active) return;
    m_isActive = active;
    updateStyle();
    update(); // Trigger repaint for the graphic
}

void ProcessCard::setCompleted(bool completed) {
    if (m_isCompleted == completed) return;
    m_isCompleted = completed;
    updateStyle();
    update(); // Trigger repaint
}

void ProcessCard::setLocked(bool locked) {
    if (m_isLocked == locked) return;
    m_isLocked = locked;
    updateStyle();
    update(); // Trigger repaint
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
        event->accept(); // Event accepted to prevent propagation
        return;
    }
    QWidget::mousePressEvent(event);
}

void ProcessCard::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Geometry for the stepper part
    int margin = 10; // Left margin from widget edge
    int lineX = 25;  // Center X of the vertical line/circle
    int cy = height() / 2; // Center Y

    // Colors
    QColor activeColor = ColorManager::ACCENT_COLOR;
    QColor completedColor = ColorManager::ACCENT_COLOR; // Or maybe a different 'done' color
    QColor inactiveColor = QColor(255, 255, 255, 30); // Dim grey
    
    QColor lineColor = inactiveColor;
    QColor circleFill = Qt::transparent; // Default hollow
    QColor circleBorder = inactiveColor;

    // Determine colors
    if (m_isActive) {
        lineColor = activeColor;
        circleFill = activeColor;
        circleBorder = activeColor;
    } else if (m_isCompleted) {
        lineColor = completedColor;
        circleFill = completedColor;
        circleBorder = completedColor;
    } else {
        // Future/Locked
        lineColor = inactiveColor;
        circleFill = Qt::transparent; // Hollow for future?
        circleBorder = inactiveColor;
    }

    // --- Draw Vertical Lines ---
    int lineWidth = 2;
    int radius = 8; // Defined earlier so we can use it for line clipping
    
    // Top Line (connects to previous step)
    if (!m_isFirst) {
        // Draw top half line
        // If THIS step is active or completed, the path TO it (top line) should be colored?
        // Usually: The line between Step A (Done) and Step B (Active) is colored 'Done'.
        // So, if this step is Active or Completed, the line coming from above is 'Done' colored.
        QColor topChunkColor = (m_isActive || m_isCompleted) ? completedColor : inactiveColor;
        
        QPen pen(topChunkColor);
        pen.setWidth(lineWidth);
        painter.setPen(pen);
        // Stop at circle boundary (radius) to avoid seeing line inside hollow inactive circles
        painter.drawLine(lineX, 0, lineX, cy - radius);
    }

    // Bottom Line (connects to next step)
    if (!m_isLast) {
        // Draw bottom half line
        // If THIS step is completed, the line going down proceeds.
        QColor bottomChunkColor = m_isCompleted ? completedColor : inactiveColor;
        
        QPen pen(bottomChunkColor);
        pen.setWidth(lineWidth);
        painter.setPen(pen);
        // Start from circle boundary (radius)
        painter.drawLine(lineX, cy + radius, lineX, height());
    }

    // --- Draw Circle/Dot ---
    // int radius = 8; // Moved up
    // if (m_isActive) radius = 10; // Removed as per user request
    
    painter.setPen(Qt::NoPen);
    
    if (m_isActive) {
        // Active visual: Solid circle only
        
        // Main dot
        painter.setBrush(activeColor);
        painter.drawEllipse(QPoint(lineX, cy), radius, radius);
        
    } else if (m_isCompleted) {
        // Solid circle
        painter.setBrush(completedColor);
        painter.drawEllipse(QPoint(lineX, cy), radius, radius);
    } else {
        // Inactive: Hollow ring
        QPen borderPen(circleBorder);
        borderPen.setWidth(2);
        painter.setPen(borderPen);
        painter.setBrush(Qt::transparent); // or transparent
        painter.drawEllipse(QPoint(lineX, cy), radius, radius);
        
        // Inner dot? No.
    }
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

    // Text & Background Stylings
    // We reduced the background prominence as the stepper visual handles active state heavily.
    // But we still want some feedback on hover or active step box.
    
    QString bgColor;
    QString borderColor;
    QString textColor;

    if (m_isActive) {
        // Active Text Highlight
        textColor = "#FFFFFF";
        // Subtle background for the whole row?
        bgColor = "rgba(255, 255, 255, 0.05)";
        borderColor = "transparent";
    } else if (m_isCompleted) {
        // Completed
        textColor = (m_isHovered && m_isClickable) ? "#FFFFFF" : "#CCCCCC";
        bgColor = (m_isHovered && m_isClickable) ? "rgba(255, 255, 255, 0.05)" : "transparent";
        borderColor = "transparent";
    } else if (m_isLocked) {
        // Locked
        textColor = "#666666";
        bgColor = "transparent";
        borderColor = "transparent";
    } else {
        // Default
        textColor = "#AAAAAA";
        bgColor = "transparent";
        borderColor = "transparent";
    }

    style = QString(
        "ProcessCard {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: %3px;"
        "}"
    ).arg(bgColor, borderColor).arg(StyleManager::BUTTON_RADIUS);

    textStyle = QString("color: %1; font-weight: %2;")
        .arg(textColor)
        .arg(m_isActive ? "bold" : "normal");

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
    mainLayout->setSpacing(0); // Zero spacing for continuous line connected

    // Create Steps
    // We pass isFirst/isLast flags.
    // 1. Import
    m_cards.push_back(new ProcessCard(1, "1.Import STEP File", true, false, this));
    // 2. Boundary
    m_cards.push_back(new ProcessCard(2, "2.Set Boundary Conditions", false, false, this));
    // 3. Simulate
    m_cards.push_back(new ProcessCard(3, "3.Simulate", false, false, this));
    // 4. Infill
    m_cards.push_back(new ProcessCard(4, "4.Build Infill Map", false, true, this));

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
    int totalHeight = 10 + 10 + (60 * 4) + (0 * 3); // margins + cards + spacing roughly
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

