#include "ProcessFlowWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QStyle>
#include "../../ColorManager.h"

// --- ProcessCard Implementation ---

ProcessCard::ProcessCard(int stepNumber, const QString& title, QWidget* parent)
    : QWidget(parent), m_stepNumber(stepNumber), m_title(title)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(15, 12, 15, 12);
    layout->setSpacing(10);
    
    // Status Icon (Number or Checkmark)
    m_iconLabel = new QLabel(QString::number(stepNumber), this);
    m_iconLabel->setFixedSize(24, 24);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet("border-radius: 12px; font-weight: bold;");
    
    // Title
    m_textLabel = new QLabel(title, this);
    QFont font = m_textLabel->font();
    font.setPointSize(12);
    m_textLabel->setFont(font);
    
    layout->addWidget(m_iconLabel);
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
        // TODO: Replace with checkmark icon if available, or just keep number but green
        m_iconLabel->setText("✓"); 
    } else {
        m_iconLabel->setText(QString::number(m_stepNumber));
    }
    updateStyle();
}

void ProcessCard::setLocked(bool locked) {
    if (m_isLocked == locked) return;
    m_isLocked = locked;
    updateStyle();
}

void ProcessCard::updateStyle() {
    QString style;
    QString iconStyle;
    QString textStyle;
    
    // Colors (Should ideally come from ColorManager, using hardcoded for now to match request quickly)
    // Dark theme assumption based on mockups
    QString bgColor;
    QString borderColor;
    QString textColor;
    QString iconBgColor;
    QString iconTextColor;
    
    if (m_isActive) {
        // Active: Blue accent
        bgColor = "rgba(0, 120, 215, 0.2)"; // Stronger blue tint
        borderColor = "#3399FF"; // Bright blue border
        textColor = "#FFFFFF";
        iconBgColor = "#3399FF";
        iconTextColor = "#FFFFFF";
    } else if (m_isCompleted) {
        // Completed: Subtle green or just standard interaction
        bgColor = "rgba(255, 255, 255, 0.05)";
        borderColor = "rgba(255, 255, 255, 0.1)";
        textColor = "#CCCCCC";
        iconBgColor = "#4CAF50"; // Green
        iconTextColor = "#FFFFFF";
    } else if (m_isLocked) {
        // Locked/Future: Dimmed
        bgColor = "transparent";
        borderColor = "transparent";
        textColor = "#666666";
        iconBgColor = "transparent";
        iconTextColor = "#666666";
        iconStyle += "border: 1px solid #444444;"; // Hollow circle
    } else {
        // Pending/Accessible but not active: Normal
        bgColor = "rgba(255, 255, 255, 0.05)";
        borderColor = "rgba(255, 255, 255, 0.1)";
        textColor = "#AAAAAA";
        iconBgColor = "#444444";
        iconTextColor = "#AAAAAA";
    }
    
    style = QString(
        "ProcessCard {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "}"
    ).arg(bgColor, borderColor);
    
    iconStyle += QString(
        "background-color: %1;"
        "color: %2;"
        "border-radius: 12px;"
    ).arg(iconBgColor, iconTextColor);
    
    textStyle = QString("color: %1;").arg(textColor);
    
    this->setStyleSheet(style);
    m_iconLabel->setStyleSheet(iconStyle);
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
    m_cards.push_back(new ProcessCard(1, "Import STEP File", this));
    m_cards.push_back(new ProcessCard(2, "Set Boundary Conditions", this));
    m_cards.push_back(new ProcessCard(3, "Simulate", this));
    m_cards.push_back(new ProcessCard(4, "Build Infill Map", this));
    
    for (auto* card : m_cards) {
        mainLayout->addWidget(card);
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
            // Past steps
            m_cards[i]->setLocked(false);
            m_cards[i]->setActive(false);
            m_cards[i]->setCompleted(true);
        } else if (stepIdx == targetIdx) {
            // Current step
            m_cards[i]->setLocked(false);
            m_cards[i]->setActive(true);
            m_cards[i]->setCompleted(false);
        } else {
            // Future steps
            m_cards[i]->setLocked(true);
            m_cards[i]->setActive(false);
            m_cards[i]->setCompleted(false);
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
