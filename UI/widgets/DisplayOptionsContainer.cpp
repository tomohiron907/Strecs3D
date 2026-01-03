#include "DisplayOptionsContainer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>

DisplayOptionsContainer::DisplayOptionsContainer(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupStyle();
}

void DisplayOptionsContainer::setupUI()
{
    // メインレイアウト
    QVBoxLayout* containerLayout = new QVBoxLayout(this);
    containerLayout->setContentsMargins(0, 0, 0, 0); // マージンを削除して右ペインレイアウトで統一管理
    containerLayout->setSpacing(10);

    // 各表示オプションウィジェットを作成
    stepDisplayWidget = new ObjectDisplayOptionsWidget("No STEP file selected", this);
    vtkDisplayWidget = new ObjectDisplayOptionsWidget("No VTK file selected", this);
    dividedMeshWidget1 = new ObjectDisplayOptionsWidget("Divided Mesh 1", this);
    dividedMeshWidget2 = new ObjectDisplayOptionsWidget("Divided Mesh 2", this);
    dividedMeshWidget3 = new ObjectDisplayOptionsWidget("Divided Mesh 3", this);
    dividedMeshWidget4 = new ObjectDisplayOptionsWidget("Divided Mesh 4", this);

    // 4分割メッシュウィジェットのサイズを小さくする
    int dividedWidgetHeight = 50; // 小さめの高さ
    dividedMeshWidget1->setMinimumHeight(dividedWidgetHeight);
    dividedMeshWidget2->setMinimumHeight(dividedWidgetHeight);
    dividedMeshWidget3->setMinimumHeight(dividedWidgetHeight);
    dividedMeshWidget4->setMinimumHeight(dividedWidgetHeight);

    // 4つのウィジェットをQFrameで囲む
    QFrame* dividedFrame = new QFrame(this);
    dividedFrame->setFrameShape(QFrame::StyledPanel);
    dividedFrame->setFrameShadow(QFrame::Raised);
    dividedFrame->setStyleSheet(
        "QFrame {"
        "  border: 1px solid rgba(200,200,200,0.3);"
        "  border-radius: 4px;"
        "  background: transparent;"
        "  margin-top: 4px;"
        "  margin-bottom: 4px;"
        "}"
    );
    QVBoxLayout* dividedLayout = new QVBoxLayout(dividedFrame);
    dividedLayout->setContentsMargins(8, 8, 8, 8);
    dividedLayout->setSpacing(6);
    dividedLayout->addWidget(dividedMeshWidget1);
    dividedLayout->addWidget(dividedMeshWidget2);
    dividedLayout->addWidget(dividedMeshWidget3);
    dividedLayout->addWidget(dividedMeshWidget4);

    // レイアウトに追加
    containerLayout->addWidget(stepDisplayWidget);
    containerLayout->addWidget(vtkDisplayWidget);
    containerLayout->addSpacing(100); // 上2つと4つのフレームの間に隙間を追加
    containerLayout->addWidget(dividedFrame);
    containerLayout->addStretch(); // 下部にスペースを追加
}

void DisplayOptionsContainer::setupStyle()
{
    // コンテナ全体のスタイル
    setStyleSheet(
        "DisplayOptionsContainer {"
        "    background-color: rgba(45, 45, 45, 200);"
        "    border-radius: 10px;"
        "    border: 1px solid rgba(255, 255, 255, 0.1);"
        "}"
        "QScrollArea {"
        "    background-color: transparent;"
        "    border: none;"
        "}"
        "QScrollBar:vertical {"
        "    background-color: rgba(60, 60, 60, 100);"
        "    width: 8px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background-color: rgba(120, 120, 120, 150);"
        "    border-radius: 4px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background-color: rgba(150, 150, 150, 200);"
        "}"
    );
}

QList<ObjectDisplayOptionsWidget*> DisplayOptionsContainer::getAllDisplayWidgets() const
{
    QList<ObjectDisplayOptionsWidget*> widgets;
    widgets.append(stepDisplayWidget);
    widgets.append(vtkDisplayWidget);
    widgets.append(dividedMeshWidget1);
    widgets.append(dividedMeshWidget2);
    widgets.append(dividedMeshWidget3);
    widgets.append(dividedMeshWidget4);
    return widgets;
} 