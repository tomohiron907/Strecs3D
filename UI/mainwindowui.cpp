#include "mainwindowui.h"
#include "../mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>
#include <QFrame>
#include "widgets/Button.h"
#include "ColorManager.h"
#include "widgets/ObjectDisplayOptionsWidget.h"
#include "widgets/DisplayOptionsContainer.h"

MainWindowUI::MainWindowUI(MainWindow* mainWindow)
    : mainWindow(mainWindow)
{
    setupUI();
}

void MainWindowUI::setupUI()
{
    centralWidget = new QWidget(mainWindow);
    // 新しい全体レイアウト（縦方向）
    QVBoxLayout* outerLayout = new QVBoxLayout(centralWidget);
    outerLayout->setContentsMargins(0, 0, 0, 0); 

    // ロゴ画像（全体の一番上・左寄せ）
    QWidget* headerWidget = new QWidget(centralWidget);
    headerWidget->setFixedHeight(70); 
    headerWidget->setStyleSheet(QString("background-color: %1;").arg(ColorManager::HEADER_COLOR.name()));
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0); 
    QLabel* logoLabel = new QLabel(centralWidget);
    QPixmap logoPixmap(":/resources/white_symbol.png");
    logoLabel->setPixmap(logoPixmap.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    headerLayout->addWidget(logoLabel);
    QLabel* logoTypeLabel = new QLabel(centralWidget);
    QPixmap logoTypePixmap(":/resources/logo_type.png");
    logoTypeLabel->setPixmap(logoTypePixmap.scaledToHeight(19, Qt::SmoothTransformation));
    logoTypeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    headerLayout->addWidget(logoTypeLabel);
    headerLayout->addStretch(); // 右側にスペース
    headerLayout->setSpacing(15);
    headerLayout->setContentsMargins(35, 0, 0, 3);
    outerLayout->addWidget(headerWidget);

    // メインの横並びレイアウト
    QHBoxLayout* mainLayout = new QHBoxLayout();

    // 左ペイン
    QVBoxLayout* leftPaneLayout = new QVBoxLayout();
    
    // 3Dオブジェクト表示オプションウィジェットを追加（左ペインから削除）
    // ObjectDisplayOptionsWidget* objectOptions = new ObjectDisplayOptionsWidget("sample.stl", centralWidget);
    // leftPaneLayout->addWidget(objectOptions);
    openStlButton = new Button("Open STL File", centralWidget);
    openVtkButton = new Button("Open VTK File", centralWidget);
    rangeSlider = new DensitySlider(centralWidget);
    modeComboBox = new ModeComboBox(centralWidget);
    
    processButton = new Button("Process", centralWidget);
    processButton->setEnabled(false); // 初期状態で無効
    processButton->setEmphasized(false); // 初期状態で強調表示なし
    export3mfButton = new Button("Export 3MF", centralWidget);
    export3mfButton->setEnabled(false); // 初期状態で無効
    messageConsole = new MessageConsole(centralWidget);
    messageConsole->setMinimumHeight(200);

    leftPaneLayout->addWidget(openStlButton);
    leftPaneLayout->addWidget(openVtkButton);
    leftPaneLayout->addWidget(rangeSlider);
    leftPaneLayout->addWidget(modeComboBox);
    leftPaneLayout->addWidget(processButton);
    leftPaneLayout->addWidget(messageConsole);
    leftPaneLayout->addStretch();

    QWidget* leftPaneWidget = new QWidget(centralWidget);
    leftPaneWidget->setLayout(leftPaneLayout);
    leftPaneWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    leftPaneWidget->setMaximumWidth(300);

    // 右ペイン（VTKウィジェット）
    vtkWidget = new QVTKOpenGLNativeWidget(centralWidget);
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkWidget->setRenderWindow(renderWindow);
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);
    renderer->SetBackground(0.1, 0.1, 0.1);

    mainLayout->addWidget(vtkWidget, 1); // まずvtkWidgetだけをレイアウトに追加
    // mainLayoutをouterLayoutに追加
    outerLayout->addLayout(mainLayout);
    outerLayout->setSpacing(5);

    // leftPaneWidgetをvtkWidgetの上に重ねて配置
    leftPaneWidget->setParent(vtkWidget);
    leftPaneWidget->adjustSize(); // レイアウト内容に合わせて自動でサイズを決める
    leftPaneWidget->move(20, 20); // 位置だけ指定
    leftPaneWidget->setStyleSheet("QWidget { background-color:rgba(45, 45, 45, 0); border-radius: 10px; }");
    leftPaneWidget->raise();
    leftPaneWidget->show();
    // 必要なら: leftPaneWidget->setAttribute(Qt::WA_TransparentForMouseEvents);

    // 右ペインレイアウトの作成
    QVBoxLayout* rightPaneLayout = new QVBoxLayout();
    rightPaneLayout->setContentsMargins(10, 10, 10, 10); // displayOptionsContainerと同じマージンに統一
    rightPaneLayout->setSpacing(10);
    
    // 表示オプションコンテナを右ペインに追加
    displayOptionsContainer = new DisplayOptionsContainer(vtkWidget);
    displayOptionsContainer->setFixedWidth(300);
    displayOptionsContainer->setMaximumHeight(1000);
    rightPaneLayout->addWidget(displayOptionsContainer);
    
    // Export 3MFボタンを右ペインに追加（幅をdisplayOptionsContainerと合わせる）
    export3mfButton->setFixedWidth(300);
    rightPaneLayout->addSpacing(50);
    rightPaneLayout->addWidget(export3mfButton);
    rightPaneLayout->addStretch();
    
    // 右ペインウィジェット作成
    QWidget* rightPaneWidget = new QWidget(vtkWidget);
    rightPaneWidget->setLayout(rightPaneLayout);
    rightPaneWidget->setFixedWidth(320); // コンテンツ幅300px + 左右マージン20px
    rightPaneWidget->setStyleSheet("QWidget { background-color:rgba(45, 45, 45, 0); border-radius: 10px; }");
    rightPaneWidget->raise();
    rightPaneWidget->show();

    vtkWidget->installEventFilter(this);

    resizeDisplayOptionsContainer();
    setupStyle();
}

bool MainWindowUI::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == vtkWidget && event->type() == QEvent::Resize) {
        resizeDisplayOptionsContainer();
    }
    return false;
}

void MainWindowUI::resizeEvent(QResizeEvent* event)
{
    // QWidget::resizeEvent(event); // 呼び出しを削除またはコメントアウト
    resizeDisplayOptionsContainer();
}

void MainWindowUI::resizeDisplayOptionsContainer()
{
    if (!displayOptionsContainer || !vtkWidget) return;
    int margin = 20;
    // 右ペインウィジェット全体の位置を調整
    QWidget* rightPaneWidget = displayOptionsContainer->parentWidget();
    if (rightPaneWidget) {
        int x = vtkWidget->width() - rightPaneWidget->width() - margin;
        int y = margin;
        rightPaneWidget->move(x, y);
    }
}

void MainWindowUI::setupStyle()
{
    mainWindow->setStyleSheet("background-color: #1a1a1a;");
} 