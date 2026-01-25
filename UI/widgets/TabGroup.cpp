#include "TabGroup.h"
#include <QPainter>
#include <QTimer>
#include "../../utils/ColorManager.h"
#include "../../utils/StyleManager.h"

TabGroup::TabGroup(QWidget *parent)
    : QWidget(parent)
    , m_animation(new QPropertyAnimation(this, "indicatorGeometry"))
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(6, 4, 6, 4);
    m_layout->setSpacing(2);
    
    // Matches StyleManager::TAB_CONTAINER_RADIUS approximately, can be passed in constructor or set
    m_borderRadius = StyleManager::TAB_CONTAINER_RADIUS;
    
    // Background style
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("TabGroup { background-color: #272727; border-radius: %1px; }")
        .arg(m_borderRadius));
        
    m_animation->setDuration(250);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

void TabGroup::addTab(TabButton* tab)
{
    m_tabs.append(tab);
    m_layout->addWidget(tab);
    
    connect(tab, &QPushButton::clicked, this, [this, tab]() {
        int index = m_tabs.indexOf(tab);
        if (index != -1 && index != m_currentIndex) {
            setActiveIndex(index);
            emit tabSelected(index);
        }
    });

    // If this is the first tab, select it by default (no animation for init)
    if (m_tabs.size() == 1) {
        m_currentIndex = 0;
        tab->setActive(true);
        // Defer initial position update until layout is done
        QTimer::singleShot(0, this, [this](){ 
            updateIndicatorPosition(false); 
        });
    }
}

void TabGroup::setActiveIndex(int index)
{
    if (index < 0 || index >= m_tabs.size()) return;
    
    if (m_currentIndex == index) return;
    
    // Update text colors
    if (m_currentIndex >= 0 && m_currentIndex < m_tabs.size()) {
        m_tabs[m_currentIndex]->setActive(false);
    }
    
    m_currentIndex = index;
    m_tabs[m_currentIndex]->setActive(true);
    
    updateIndicatorPosition(true);
}

void TabGroup::setIndicatorGeometry(const QRect &rect)
{
    m_indicatorRect = rect;
    update(); // Redraw
}

void TabGroup::updateIndicatorPosition(bool animate)
{
    if (m_currentIndex < 0 || m_currentIndex >= m_tabs.size()) return;
    
    TabButton* currentTab = m_tabs[m_currentIndex];
    QRect targetRect = currentTab->geometry();
    
    // Adjust rect slightly to act as a border/pill inside the button area if needed
    // The previous implementation used: rect().adjusted(1, 1, -1, -1) inside the button
    // Since we are painting on the parent, we use the button's geometry.
    // If we want exact match to previous look, we might need to inset it slightly.
    // Previous TabButton::paintEvent used rect().adjusted(1, 1, -1, -1)
    
    // targetRect is in TabGroup coordinates.
    // Apply 1px indent
    targetRect.adjust(1, 1, -1, -1);
    
    if (animate) {
        m_animation->stop();
        m_animation->setStartValue(m_indicatorRect);
        m_animation->setEndValue(targetRect);
        m_animation->start();
    } else {
        m_animation->stop();
        m_indicatorRect = targetRect;
        update();
    }
}

void TabGroup::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // Recalculate indicator position without animation when resizing
    updateIndicatorPosition(false);
}

void TabGroup::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw the indicator if valid
    if (m_currentIndex >= 0 && !m_indicatorRect.isNull()) {
        painter.setPen(QPen(ColorManager::ACCENT_COLOR, 1));
        painter.setBrush(Qt::NoBrush);
        
        // Use half height for full pill roundness
        qreal radius = m_indicatorRect.height() / 2.0;
        painter.drawRoundedRect(m_indicatorRect, radius, radius);
    }
}
