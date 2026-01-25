#ifndef TABGROUP_H
#define TABGROUP_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QVector>
#include "TabButton.h"

class TabGroup : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QRect indicatorGeometry READ indicatorGeometry WRITE setIndicatorGeometry)

public:
    explicit TabGroup(QWidget *parent = nullptr);

    void addTab(TabButton* tab);
    void setActiveIndex(int index);
    int currentIndex() const { return m_currentIndex; }

    QRect indicatorGeometry() const { return m_indicatorRect; }
    void setIndicatorGeometry(const QRect &rect);

signals:
    void tabSelected(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateIndicatorPosition(bool animate);

    QVector<TabButton*> m_tabs;
    QHBoxLayout* m_layout;
    int m_currentIndex = -1;
    QRect m_indicatorRect;
    QPropertyAnimation* m_animation;
    
    // Config
    int m_borderRadius = 20; // Will be set from StyleManager
};

#endif // TABGROUP_H
