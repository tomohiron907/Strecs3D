#include "mainwindowui.h"
#include "../mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>
#include <QFrame>
#include <QTimer>
#include <iostream>
#include "widgets/Button.h"
#include "ColorManager.h"

MainWindowUI::MainWindowUI(MainWindow* mainWindow)
    : mainWindow(mainWindow)
{
    uiState = new UIState(this);
    setupUI();
    connectUIStateSignals();
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
    if (watched == vtkWidget) {
        if (event->type() == QEvent::Resize) {
            setupWidgetPositioning();
        }
        // Block touch events to fix macOS trackpad issue
        else if (event->type() == QEvent::TouchBegin ||
                 event->type() == QEvent::TouchUpdate ||
                 event->type() == QEvent::TouchEnd) {
            return true; // Block the event
        }
    }
    return false;
}

void MainWindowUI::resizeEvent(QResizeEvent* event)
{
    setupWidgetPositioning();
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
    openStlButton->setVisible(false);

    openVtkButton = new Button("Open VTK File", centralWidget);
    openVtkButton->setIcon(":/resources/icons/vtk.png");
    openVtkButton->setVisible(false);

    // openStepButton, constrainButton, etc. are now managed by ProcessManagerWidget
    
    export3mfButton = new Button("Export 3MF", centralWidget);
    export3mfButton->setIcon(":/resources/icons/export.png");
    export3mfButton->setIconDark(":/resources/icons/export_dark.png");
    export3mfButton->setEnabled(false);
}

void MainWindowUI::createLeftPaneWidget(QWidget* vtkParent)
{
    QVBoxLayout* leftPaneLayout = new QVBoxLayout();
    leftPaneLayout->setContentsMargins(0, 0, 0, 0); // Tight fit
    
    // Process Manager Widget (The main content)
    processManagerWidget = new ProcessManagerWidget(centralWidget);
    // Pass UIState early if available, or wait for connectUIStateSignals
    
    // ModeComboBox (Keeping it visible for now, maybe move to InfillStep later)
    modeComboBox = new ModeComboBox(centralWidget);
    
    // Message Console
    messageConsole = new MessageConsole(centralWidget);
    messageConsole->setMinimumHeight(CONSOLE_MIN_HEIGHT);

    leftPaneLayout->addWidget(processManagerWidget, 1); // Expanding
    
    // Global controls below the process flow
    leftPaneLayout->addWidget(modeComboBox);
    leftPaneLayout->addWidget(messageConsole);

    QWidget* leftPaneWidget = new QWidget(centralWidget);
    leftPaneWidget->setLayout(leftPaneLayout);
    leftPaneWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    leftPaneWidget->setMaximumWidth(LEFT_PANE_MAX_WIDTH);
    leftPaneWidget->setParent(vtkParent);
    leftPaneWidget->setStyleSheet("QWidget { background-color:rgba(45, 45, 45, 0); border-radius: 10px; }");
}

// Implement Legacy Getters
Button* MainWindowUI::getOpenStepButton() const {
    if (processManagerWidget && processManagerWidget->getImportStep()) 
        return processManagerWidget->getImportStep()->getImportButton();
    return nullptr;
}

Button* MainWindowUI::getConstrainButton() const {
    if (processManagerWidget && processManagerWidget->getBoundaryConditionStep())
        return processManagerWidget->getBoundaryConditionStep()->getAddConstraintButton();
    return nullptr;
}

Button* MainWindowUI::getLoadButton() const {
    if (processManagerWidget && processManagerWidget->getBoundaryConditionStep())
        return processManagerWidget->getBoundaryConditionStep()->getAddLoadButton();
    return nullptr;
}

Button* MainWindowUI::getSimulateButton() const {
    if (processManagerWidget && processManagerWidget->getSimulationStep())
        return processManagerWidget->getSimulationStep()->getSimulateButton();
    return nullptr;
}

Button* MainWindowUI::getProcessButton() const {
    if (processManagerWidget && processManagerWidget->getInfillStep())
        return processManagerWidget->getInfillStep()->getProcessButton();
    return nullptr;
}

DensitySlider* MainWindowUI::getRangeSlider() const {
    if (processManagerWidget && processManagerWidget->getInfillStep())
        return processManagerWidget->getInfillStep()->getDensitySlider();
    return nullptr;
}

StressRangeWidget* MainWindowUI::getStressRangeWidget() const {
    if (processManagerWidget && processManagerWidget->getInfillStep())
        return processManagerWidget->getInfillStep()->getStressRangeWidget();
    return nullptr;
}

void MainWindowUI::createRightPaneWidget(QWidget* vtkParent)
{
    // 右側ペインのコンテナ作成
    rightPaneWidget = new QWidget(centralWidget);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPaneWidget);
    rightLayout->setContentsMargins(10, 10, 10, 10);
    rightLayout->setSpacing(10);

    // オブジェクトリスト作成
    objectListWidget = new ObjectListWidget(rightPaneWidget);
    objectListWidget->setUIState(uiState);
    rightLayout->addWidget(objectListWidget, 1);

    // プロパティウィジェット作成
    propertyWidget = new PropertyWidget(rightPaneWidget);
    propertyWidget->setUIState(uiState);
    rightLayout->addWidget(propertyWidget, 1);

    // Exportボタンをここに移動
    export3mfButton->setParent(rightPaneWidget);
    export3mfButton->setFixedWidth(DISPLAY_OPTIONS_WIDTH - 20); // マージン考慮
    rightLayout->addWidget(export3mfButton);

    // コンテナの設定
    rightPaneWidget->setParent(vtkParent);
    rightPaneWidget->setStyleSheet("QWidget { background-color:rgba(45, 45, 45, 0); border-radius: 10px; }");
    rightPaneWidget->setFixedWidth(RIGHT_PANE_WIDTH);
    
    // シグナル接続
    connect(objectListWidget, &ObjectListWidget::objectSelected, 
            propertyWidget, &PropertyWidget::onObjectSelected);
}

void MainWindowUI::createVtkWidget()
{
    vtkWidget = new QVTKOpenGLNativeWidget(centralWidget);
    
    // Multiple approaches to disable touch event processing to fix macOS trackpad issue
    // See: https://gitlab.kitware.com/vtk/vtk/-/issues/19073
    vtkWidget->setAttribute(Qt::WA_AcceptTouchEvents, false);
    vtkWidget->setAttribute(Qt::WA_TouchPadAcceptSingleTouchEvents, false);
    
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

    // Position right pane widget
    if (rightPaneWidget && vtkWidget) {
        int x = vtkWidget->width() - rightPaneWidget->width() - WIDGET_MARGIN;
        int y = WIDGET_MARGIN;
        // 高さは親ウィジェットに合わせる（マージン分引く）
        int h = vtkWidget->height() - (WIDGET_MARGIN * 2);
        
        rightPaneWidget->setGeometry(x, y, RIGHT_PANE_WIDTH, h);
        rightPaneWidget->raise();
        rightPaneWidget->show();
    }

    static bool eventFilterInstalled = false;
    if (!eventFilterInstalled) {
        vtkWidget->installEventFilter(this);
        eventFilterInstalled = true;
    }
}

void MainWindowUI::setButtonIconSize(const QSize& size)
{
    // グローバルアイコンサイズを設定
    Button::setGlobalIconSize(size);

    // 既存のボタンのアイコンサイズを更新
    if (openStlButton) openStlButton->setIconSize(size);
    if (openVtkButton) openVtkButton->setIconSize(size);
    
    // Process widgets buttons
    if (Button* btn = getOpenStepButton()) btn->setIconSize(size);
    if (Button* btn = getConstrainButton()) btn->setIconSize(size);
    if (Button* btn = getLoadButton()) btn->setIconSize(size);
    if (Button* btn = getSimulateButton()) btn->setIconSize(size);
    if (Button* btn = getProcessButton()) btn->setIconSize(size);

    if (export3mfButton) export3mfButton->setIconSize(size);
}

void MainWindowUI::connectUIStateSignals()
{
    // ProcessManager handles its own connections
    if (processManagerWidget) {
        processManagerWidget->setUIState(uiState);
    }
    
    // ModeComboBoxの変更をUIStateに反映
    connect(modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                ProcessingMode mode;
                switch(index) {
                    case 0: mode = ProcessingMode::CURA; break;
                    case 1: mode = ProcessingMode::BAMBU; break;
                    case 2: mode = ProcessingMode::PRUSA; break;
                    default: mode = ProcessingMode::CURA; break;
                }
                uiState->setProcessingMode(mode);
            });
}

void MainWindowUI::updateUIFromState()
{
    // UIStateの値をUIコンポーネントに反映
    if (StressRangeWidget* srw = getStressRangeWidget()) {
        srw->setStressRange(uiState->getMinStress(), uiState->getMaxStress());
    }

    if (modeComboBox) {
        int index = 0;
        switch(uiState->getProcessingMode()) {
            case ProcessingMode::BAMBU: index = 0; break;
            case ProcessingMode::CURA: index = 1; break;
            case ProcessingMode::PRUSA: index = 2; break;
        }
        modeComboBox->setCurrentIndex(index);
    }
}
 