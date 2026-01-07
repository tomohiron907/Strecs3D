#ifndef PROPERTYWIDGET_H
#define PROPERTYWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include "ObjectListWidget.h"
#include "../../core/ui/UIState.h"

class PropertyWidget : public QWidget {
    Q_OBJECT

public:
    explicit PropertyWidget(QWidget* parent = nullptr);
    ~PropertyWidget() = default;

    void setUIState(UIState* uiState);

public slots:
    void onObjectSelected(ObjectType type, const QString& id, int index);

private:
    void setupUI();
    void clearProperties();
    void addProperty(const QString& label, const QString& value);
    void addSectionHeader(const QString& title);

    UIState* m_uiState = nullptr;
    
    QVBoxLayout* m_mainLayout;
    QFormLayout* m_formLayout;
    QLabel* m_titleLabel;
    QWidget* m_contentWidget;
};

#endif // PROPERTYWIDGET_H
