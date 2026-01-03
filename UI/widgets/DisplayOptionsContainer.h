#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include "ObjectDisplayOptionsWidget.h"

class DisplayOptionsContainer : public QWidget {
    Q_OBJECT
public:
    explicit DisplayOptionsContainer(QWidget* parent = nullptr);
    ~DisplayOptionsContainer() = default;

    // 各ウィジェットへのアクセサー
    ObjectDisplayOptionsWidget* getStepDisplayWidget() const { return stepDisplayWidget; }
    ObjectDisplayOptionsWidget* getVtkDisplayWidget() const { return vtkDisplayWidget; }
    ObjectDisplayOptionsWidget* getDividedMeshWidget1() const { return dividedMeshWidget1; }
    ObjectDisplayOptionsWidget* getDividedMeshWidget2() const { return dividedMeshWidget2; }
    ObjectDisplayOptionsWidget* getDividedMeshWidget3() const { return dividedMeshWidget3; }
    ObjectDisplayOptionsWidget* getDividedMeshWidget4() const { return dividedMeshWidget4; }

    // 全ウィジェットのリストを取得
    QList<ObjectDisplayOptionsWidget*> getAllDisplayWidgets() const;

private:
    void setupUI();
    void setupStyle();

    ObjectDisplayOptionsWidget* stepDisplayWidget;
    ObjectDisplayOptionsWidget* vtkDisplayWidget;
    ObjectDisplayOptionsWidget* dividedMeshWidget1;
    ObjectDisplayOptionsWidget* dividedMeshWidget2;
    ObjectDisplayOptionsWidget* dividedMeshWidget3;
    ObjectDisplayOptionsWidget* dividedMeshWidget4;
    
    QScrollArea* scrollArea;
    QWidget* scrollContent;
    QVBoxLayout* mainLayout;
}; 