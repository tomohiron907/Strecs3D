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
    QVBoxLayout* outerLayout = new QVBoxLayout(centralWidget);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    createHeaderWidget(outerLayout);
    createVtkWidget();
    
    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->addWidget(vtkWidget, 1);
    outerLayout->addLayout(mainLayout);
    outerLayout->setSpacing(5);

    createButtons();
    createLeftPaneWidget(vtkWidget);
    createRightPaneWidget(vtkWidget);
    
    setupWidgetPositioning();
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
    
    QWidget* rightPaneWidget = displayOptionsContainer->parentWidget();
    if (rightPaneWidget) {
        int x = vtkWidget->width() - rightPaneWidget->width() - WIDGET_MARGIN;
        int y = WIDGET_MARGIN;
        rightPaneWidget->move(x, y);
    }
}

void MainWindowUI::setupStyle()
{
    mainWindow->setStyleSheet("background-color: #1a1a1a;");
}

void MainWindowUI::createHeaderWidget(QVBoxLayout* outerLayout)
{
    QWidget* headerWidget = new QWidget(centralWidget);
    headerWidget->setFixedHeight(HEADER_HEIGHT);
    headerWidget->setStyleSheet(QString("background-color: %1;").arg(ColorManager::HEADER_COLOR.name()));
    
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* logoLabel = new QLabel(centralWidget);
    QPixmap logoPixmap(":/resources/white_symbol.png");
    logoLabel->setPixmap(logoPixmap.scaled(LOGO_SIZE, LOGO_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    headerLayout->addWidget(logoLabel);
    
    QLabel* logoTypeLabel = new QLabel(centralWidget);
    QPixmap logoTypePixmap(":/resources/logo_type.png");
    logoTypeLabel->setPixmap(logoTypePixmap.scaledToHeight(LOGO_TYPE_HEIGHT, Qt::SmoothTransformation));
    logoTypeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    headerLayout->addWidget(logoTypeLabel);
    
    headerLayout->addStretch();
    headerLayout->setSpacing(HEADER_SPACING);
    headerLayout->setContentsMargins(HEADER_LEFT_MARGIN, 0, 0, HEADER_BOTTOM_MARGIN);
    
    outerLayout->addWidget(headerWidget);
}

void MainWindowUI::createButtons()
{
    Button::setGlobalIconSize(QSize(ICON_SIZE, ICON_SIZE));
    
    openStlButton = new Button("Open STL File", centralWidget);
    openStlButton->setIcon(":/resources/icons/stl.png");
    
    openVtkButton = new Button("Open VTK File", centralWidget);
    openVtkButton->setIcon(":/resources/icons/vtk.png");
    
    processButton = new Button("Process", centralWidget);
    processButton->setIcon(":/resources/icons/process.png");
    processButton->setIconDark(":/resources/icons/process_dark.png");
    processButton->setEnabled(false);
    processButton->setEmphasized(false);
    
    export3mfButton = new Button("Export 3MF", centralWidget);
    export3mfButton->setIcon(":/resources/icons/export.png");
    export3mfButton->setIconDark(":/resources/icons/export_dark.png");
    export3mfButton->setEnabled(false);
}

void MainWindowUI::createLeftPaneWidget(QWidget* vtkParent)
{
    QVBoxLayout* leftPaneLayout = new QVBoxLayout();
    
    rangeSlider = new DensitySlider(centralWidget);
    modeComboBox = new ModeComboBox(centralWidget);
    messageConsole = new MessageConsole(centralWidget);
    messageConsole->setMinimumHeight(CONSOLE_MIN_HEIGHT);

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
    leftPaneWidget->setMaximumWidth(LEFT_PANE_MAX_WIDTH);
    leftPaneWidget->setParent(vtkParent);
    leftPaneWidget->setStyleSheet("QWidget { background-color:rgba(45, 45, 45, 0); border-radius: 10px; }");
}

void MainWindowUI::createRightPaneWidget(QWidget* vtkParent)
{
    QVBoxLayout* rightPaneLayout = new QVBoxLayout();
    rightPaneLayout->setContentsMargins(LAYOUT_SPACING, LAYOUT_SPACING, LAYOUT_SPACING, LAYOUT_SPACING);
    rightPaneLayout->setSpacing(LAYOUT_SPACING);
    
    displayOptionsContainer = new DisplayOptionsContainer(vtkParent);
    displayOptionsContainer->setFixedWidth(DISPLAY_OPTIONS_WIDTH);
    displayOptionsContainer->setMaximumHeight(1000);
    rightPaneLayout->addWidget(displayOptionsContainer);
    
    export3mfButton->setFixedWidth(DISPLAY_OPTIONS_WIDTH);
    rightPaneLayout->addSpacing(EXPORT_BUTTON_TOP_SPACING);
    rightPaneLayout->addWidget(export3mfButton);
    rightPaneLayout->addStretch();
    
    QWidget* rightPaneWidget = new QWidget(vtkParent);
    rightPaneWidget->setLayout(rightPaneLayout);
    rightPaneWidget->setFixedWidth(RIGHT_PANE_WIDTH);
    rightPaneWidget->setStyleSheet("QWidget { background-color:rgba(45, 45, 45, 0); border-radius: 10px; }");
}

void MainWindowUI::createVtkWidget()
{
    vtkWidget = new QVTKOpenGLNativeWidget(centralWidget);
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkWidget->setRenderWindow(renderWindow);
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);
    renderer->SetBackground(0.1, 0.1, 0.1);
}

void MainWindowUI::setupWidgetPositioning()
{
    // Find and position left pane widget
    QWidget* leftPaneWidget = nullptr;
    for (QObject* child : vtkWidget->children()) {
        QWidget* widget = qobject_cast<QWidget*>(child);
        if (widget && widget->maximumWidth() == LEFT_PANE_MAX_WIDTH) {
            leftPaneWidget = widget;
            break;
        }
    }
    
    if (leftPaneWidget) {
        leftPaneWidget->adjustSize();
        leftPaneWidget->move(WIDGET_MARGIN, WIDGET_MARGIN);
        leftPaneWidget->raise();
        leftPaneWidget->show();
    }
    
    // Find and position right pane widget
    QWidget* rightPaneWidget = nullptr;
    for (QObject* child : vtkWidget->children()) {
        QWidget* widget = qobject_cast<QWidget*>(child);
        if (widget && widget->width() == RIGHT_PANE_WIDTH) {
            rightPaneWidget = widget;
            break;
        }
    }
    
    if (rightPaneWidget) {
        rightPaneWidget->raise();
        rightPaneWidget->show();
    }
    
    vtkWidget->installEventFilter(this);
    resizeDisplayOptionsContainer();
}

void MainWindowUI::setButtonIconSize(const QSize& size)
{
    // グローバルアイコンサイズを設定
    Button::setGlobalIconSize(size);
    
    // 既存のボタンのアイコンサイズを更新
    if (openStlButton) {
        openStlButton->setIconSize(size);
    }
    if (openVtkButton) {
        openVtkButton->setIconSize(size);
    }
    if (processButton) {
        processButton->setIconSize(size);
    }
    if (export3mfButton) {
        export3mfButton->setIconSize(size);
    }
} 