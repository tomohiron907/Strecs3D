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
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QDesktopServices>
#include <QUrl>
#include <QPainter>

#include "../utils/SettingsManager.h"
#include "widgets/Button.h"
#include "widgets/TabButton.h"
#include "widgets/TabGroup.h"
#include "widgets/SettingsWidget.h"
#include "widgets/ShowcaseWidget.h"
#include "widgets/GuideWidget.h"
#include "../utils/ColorManager.h"
#include "../utils/StyleManager.h"

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
    outerLayout->setSpacing(0);

    createHeaderWidget(outerLayout);
    createMainContentStack(outerLayout);

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

    // Handle hover icon swap for link buttons
    if (event->type() == QEvent::Enter || event->type() == QEvent::Leave) {
        QPushButton* btn = qobject_cast<QPushButton*>(watched);
        if (btn && btn->property("hoverPix").isValid()) {
            QPixmap pix = (event->type() == QEvent::Enter)
                ? btn->property("hoverPix").value<QPixmap>()
                : btn->property("normalPix").value<QPixmap>();
            btn->setIcon(QIcon(pix));
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

    // Creates a container for the logo to center it relative to the left pane
    QWidget* logoContainer = new QWidget(centralWidget);
    logoContainer->setFixedWidth(LEFT_PANE_MAX_WIDTH);
    QHBoxLayout* logoLayout = new QHBoxLayout(logoContainer);
    logoLayout->setContentsMargins(0, 0, 0, 0);
    logoLayout->setAlignment(Qt::AlignCenter);
    logoLayout->setSpacing(10); // Spacing between logo and text

    QLabel* logoLabel = new QLabel(centralWidget);
    QPixmap logoPixmap(":/resources/white_symbol.png");
    logoLabel->setPixmap(logoPixmap.scaled(LOGO_SIZE, LOGO_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLayout->addWidget(logoLabel);

    QLabel* logoTypeLabel = new QLabel(centralWidget);
    QPixmap logoTypePixmap(":/resources/logo_type.png");
    logoTypeLabel->setPixmap(logoTypePixmap.scaledToHeight(LOGO_TYPE_HEIGHT, Qt::SmoothTransformation));
    logoTypeLabel->setAlignment(Qt::AlignCenter);
    logoLayout->addWidget(logoTypeLabel);

    headerLayout->addWidget(logoContainer);

    // Add stretch to push tabs to center
    headerLayout->addStretch();

    // Add tab buttons
    createTabButtons(headerLayout);

    headerLayout->addStretch();
    
    // Right container with web/GitHub links (same width as logo container for centering balance)
    QWidget* rightContainer = new QWidget(centralWidget);
    rightContainer->setFixedWidth(LEFT_PANE_MAX_WIDTH);
    QHBoxLayout* rightLayout = new QHBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 40, 0);
    rightLayout->setSpacing(12);
    rightLayout->addStretch();

    auto tintedPixmap = [](const QString& iconPath, const QColor& color, int size) {
        QPixmap src(iconPath);
        QPixmap pixmap = src.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pixmap.rect(), color);
        painter.end();
        return pixmap;
    };

    auto createLinkButton = [&](const QString& iconPath, const QString& url) {
        QPushButton* btn = new QPushButton(rightContainer);
        int iconSize = 28;
        QPixmap normalPix = tintedPixmap(iconPath, QColor(255, 255, 255, 100), iconSize * 2);
        QPixmap hoverPix = tintedPixmap(iconPath, QColor(255, 255, 255, 190), iconSize * 2);
        btn->setIcon(QIcon(normalPix));
        btn->setIconSize(QSize(iconSize, iconSize));
        btn->setFixedSize(36, 36);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton { background: transparent; border: none; }"
        );
        btn->setProperty("normalPix", normalPix);
        btn->setProperty("hoverPix", hoverPix);
        btn->installEventFilter(this);
        QObject::connect(btn, &QPushButton::clicked, [url]() {
            QDesktopServices::openUrl(QUrl(url));
        });
        return btn;
    };

    rightLayout->addWidget(createLinkButton(":/resources/icons/web.svg", "https://strecs3d.xyz"));
    rightLayout->addWidget(createLinkButton(":/resources/icons/GitHub.svg", "https://github.com/tomohiron907/Strecs3D"));

    headerLayout->addWidget(rightContainer);

    headerLayout->setSpacing(HEADER_SPACING);
    headerLayout->setContentsMargins(WIDGET_MARGIN, 0, WIDGET_MARGIN, HEADER_BOTTOM_MARGIN);

    outerLayout->addWidget(headerWidget);
}

void MainWindowUI::createTabButtons(QHBoxLayout* headerLayout)
{
    // Create TabGroup
    m_tabGroup = new TabGroup(centralWidget);
    
    m_processTab = new TabButton("Process", m_tabGroup);
    m_settingsTab = new TabButton("Settings", m_tabGroup);
    m_showcaseTab = new TabButton("Showcase", m_tabGroup);
    m_guideTab = new TabButton("Guide", m_tabGroup);

    m_tabGroup->addTab(m_processTab);
    m_tabGroup->addTab(m_settingsTab);
    m_tabGroup->addTab(m_showcaseTab);
    m_tabGroup->addTab(m_guideTab);

    headerLayout->addWidget(m_tabGroup);

    connect(m_tabGroup, &TabGroup::tabSelected, this, [this](int index) {
        switchToTab(index);
    });
}

void MainWindowUI::createMainContentStack(QVBoxLayout* outerLayout)
{
    m_mainContentStack = new QStackedWidget(centralWidget);

    // Process content (Index 0)
    m_processContent = new QWidget(m_mainContentStack);
    QHBoxLayout* processLayout = new QHBoxLayout(m_processContent);
    processLayout->setContentsMargins(0, 0, 0, 0);

    createVtkWidget();
    processLayout->addWidget(vtkWidget, 1);

    createButtons();
    createLeftPaneWidget(vtkWidget);
    createRightPaneWidget(vtkWidget);

    m_mainContentStack->addWidget(m_processContent);

    // Settings content (Index 1)
    m_settingsWidget = new SettingsWidget(m_mainContentStack);
    m_mainContentStack->addWidget(m_settingsWidget);

    // Showcase content (Index 2)
    m_showcaseContent = new ShowcaseWidget(m_mainContentStack);
    m_mainContentStack->addWidget(m_showcaseContent);

    // Guide content (Index 3)
    m_guideContent = new GuideWidget(m_mainContentStack);
    m_mainContentStack->addWidget(m_guideContent);

    // Start with Process tab
    m_mainContentStack->setCurrentIndex(0);

    outerLayout->addWidget(m_mainContentStack, 1);
}

void MainWindowUI::switchToTab(int index)
{
    int currentIndex = m_mainContentStack->currentIndex();
    if (currentIndex == index) {
        return;
    }

    if (m_tabAnimationRunning) {
        return;
    }
    m_tabAnimationRunning = true;

    // Update tab button states via TabGroup
    m_tabGroup->setActiveIndex(index);

    QWidget* currentWidget = m_mainContentStack->widget(currentIndex);
    QWidget* nextWidget = m_mainContentStack->widget(index);

    int width = m_mainContentStack->width();
    int height = m_mainContentStack->height();
    int direction = (index > currentIndex) ? 1 : -1;

    // Set size explicitly, then position off-screen using pos
    nextWidget->resize(width, height);
    nextWidget->move(direction * width, 0);
    nextWidget->show();
    nextWidget->raise();

    // Animation for current widget (slide out) - animate pos only
    QPropertyAnimation* currentAnim = new QPropertyAnimation(currentWidget, "pos", this);
    currentAnim->setDuration(450);
    currentAnim->setStartValue(QPoint(0, 0));
    currentAnim->setEndValue(QPoint(-direction * width, 0));
    currentAnim->setEasingCurve(QEasingCurve::InOutQuad);

    // Animation for next widget (slide in) - animate pos only
    QPropertyAnimation* nextAnim = new QPropertyAnimation(nextWidget, "pos", this);
    nextAnim->setDuration(450);
    nextAnim->setStartValue(QPoint(direction * width, 0));
    nextAnim->setEndValue(QPoint(0, 0));
    nextAnim->setEasingCurve(QEasingCurve::InOutQuad);

    // Run animations in parallel
    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
    group->addAnimation(currentAnim);
    group->addAnimation(nextAnim);

    connect(group, &QParallelAnimationGroup::finished, this, [this, index, group]() {
        m_mainContentStack->setCurrentIndex(index);
        m_tabAnimationRunning = false;
        group->deleteLater();
    });

    group->start();
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
    
    QString slicerType = QString::fromStdString(SettingsManager::instance().slicerType());
    export3mfButton = new Button(QString("Export 3MF for %1").arg(slicerType), centralWidget);
    export3mfButton->setIcon(":/resources/icons/export.png");
    export3mfButton->setIconDark(":/resources/icons/export.png");
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
    // Removed
    
    leftPaneLayout->addWidget(processManagerWidget, 1); // Expanding
    
    // Global controls below the process flow
    // leftPaneLayout->addWidget(modeComboBox); // Removed
    // Message Console removed

    QWidget* leftPaneWidget = new QWidget(centralWidget);
    leftPaneWidget->setLayout(leftPaneLayout);
    leftPaneWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    leftPaneWidget->setMaximumWidth(LEFT_PANE_MAX_WIDTH);
    leftPaneWidget->setParent(vtkParent);
    leftPaneWidget->setStyleSheet(QString("QWidget { background-color:rgba(45, 45, 45, 0); border-radius: %1px; }")
        .arg(StyleManager::CONTAINER_RADIUS));
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

AdaptiveDensitySlider* MainWindowUI::getRangeSlider() const {
    if (processManagerWidget && processManagerWidget->getInfillStep())
        return processManagerWidget->getInfillStep()->getDensitySlider();
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
    export3mfButton->setFixedWidth(RIGHT_PANE_WIDTH - 20); // マージン考慮
    rightLayout->addWidget(export3mfButton);

    // コンテナの設定
    rightPaneWidget->setParent(vtkParent);
    rightPaneWidget->setStyleSheet(QString("QWidget { background-color:rgba(45, 45, 45, 0); border-radius: %1px; }")
        .arg(StyleManager::CONTAINER_RADIUS));
    rightPaneWidget->setFixedWidth(RIGHT_PANE_WIDTH);
    
    // シグナル接続
    connect(objectListWidget, &ObjectListWidget::objectSelected,
            propertyWidget, &PropertyWidget::onObjectSelected);

    // ステップ変更をPropertyWidgetに通知
    connect(processManagerWidget, &ProcessManagerWidget::stepChanged,
            propertyWidget, &PropertyWidget::setCurrentStep);
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
        // adjustSize() ensures the width fits the content (up to max width)
        leftPaneWidget->adjustSize();
        
        // Force height to match window height (minus margins)
        int h = vtkWidget->height() - (WIDGET_MARGIN * 2);
        leftPaneWidget->resize(leftPaneWidget->width(), h);
        
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
    
    // Connect SettingsWidget's regionCountChanged signal to AdaptiveDensitySlider's setRegionCount
    if (m_settingsWidget) {
        if (AdaptiveDensitySlider* slider = getRangeSlider()) {
             connect(m_settingsWidget, &SettingsWidget::regionCountChanged,
                    slider, &AdaptiveDensitySlider::setRegionCount);
        }
    }
    
    // ModeComboBoxの変更をUIStateに反映 (Removed)
}

void MainWindowUI::updateUIFromState()
{
}
 