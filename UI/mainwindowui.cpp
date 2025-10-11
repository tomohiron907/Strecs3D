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
#include "widgets/ObjectDisplayOptionsWidget.h"
#include "widgets/DisplayOptionsContainer.h"

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
            resizeDisplayOptionsContainer();
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
    stressRangeWidget = new StressRangeWidget(centralWidget);
    modeComboBox = new ModeComboBox(centralWidget);
    messageConsole = new MessageConsole(centralWidget);
    messageConsole->setMinimumHeight(CONSOLE_MIN_HEIGHT);

    leftPaneLayout->addWidget(openStlButton);
    leftPaneLayout->addWidget(openVtkButton);
    leftPaneLayout->addWidget(rangeSlider);
    leftPaneLayout->addWidget(stressRangeWidget);
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

void MainWindowUI::connectUIStateSignals()
{
    // StressRangeWidgetの変更をUIStateに反映
    connect(stressRangeWidget, &StressRangeWidget::stressRangeChanged,
            this, [this](double minStress, double maxStress) {
                uiState->setStressRange(minStress, maxStress);
            });

    // DensitySliderの変更をUIStateに反映
    connect(rangeSlider, &DensitySlider::regionPercentsChanged,
            this, [this](const std::vector<double>& percents) {
                auto mappings = rangeSlider->stressDensityMappings();
                uiState->setStressDensityMappings(mappings);
                
                // スライダーの色もUIStateに登録（regionPercentsChanged時も）
                auto colors = rangeSlider->getRegionColors();
                uiState->setDensitySliderColors(colors);
            });
    
    // DensitySliderのハンドル位置変更をUIStateに反映
    connect(rangeSlider, &DensitySlider::handlePositionsChanged,
            this, [this](const std::vector<int>& positions) {
                auto mappings = rangeSlider->stressDensityMappings();
                uiState->setStressDensityMappings(mappings);
                
                // スライダーの色をUIStateに登録
                auto colors = rangeSlider->getRegionColors();
                uiState->setDensitySliderColors(colors);
            });
    
    // 初期色をUIStateに設定
    QTimer::singleShot(0, this, [this]() {
        auto colors = rangeSlider->getRegionColors();
        std::cout << "Debug: Setting initial " << colors.size() << " colors to UIState" << std::endl;
        for (size_t i = 0; i < colors.size(); ++i) {
            std::cout << "  Color " << i << ": RGB(" << colors[i].red() << "," << colors[i].green() << "," << colors[i].blue() << ")" << std::endl;
        }
        uiState->setDensitySliderColors(colors);
    });

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

    // DisplayOptionsContainerの各ウィジェットの変更をUIStateに反映
    if (displayOptionsContainer) {
        auto widgets = displayOptionsContainer->getAllDisplayWidgets();
        
        // STL Display Widget (Mesh)
        if (widgets.size() > 0 && widgets[0]) {
            connect(widgets[0], &ObjectDisplayOptionsWidget::visibilityToggled,
                    this, [this](bool visible) {
                        DisplaySettings settings = uiState->getMeshDisplaySettings();
                        settings.isVisible = visible;
                        uiState->setMeshDisplaySettings(settings);
                    });
            connect(widgets[0], &ObjectDisplayOptionsWidget::opacityChanged,
                    this, [this](double opacity) {
                        DisplaySettings settings = uiState->getMeshDisplaySettings();
                        settings.opacity = opacity;
                        uiState->setMeshDisplaySettings(settings);
                    });
        }
        
        // VTK Display Widget (VTU)
        if (widgets.size() > 1 && widgets[1]) {
            connect(widgets[1], &ObjectDisplayOptionsWidget::visibilityToggled,
                    this, [this](bool visible) {
                        DisplaySettings settings = uiState->getVtuDisplaySettings();
                        settings.isVisible = visible;
                        uiState->setVtuDisplaySettings(settings);
                    });
            connect(widgets[1], &ObjectDisplayOptionsWidget::opacityChanged,
                    this, [this](double opacity) {
                        DisplaySettings settings = uiState->getVtuDisplaySettings();
                        settings.opacity = opacity;
                        uiState->setVtuDisplaySettings(settings);
                    });
        }
        
        // Divided Mesh Widgets
        if (widgets.size() > 2 && widgets[2]) {
            connect(widgets[2], &ObjectDisplayOptionsWidget::visibilityToggled,
                    this, [this](bool visible) {
                        DisplaySettings settings = uiState->getDividedMesh1Settings();
                        settings.isVisible = visible;
                        uiState->setDividedMesh1Settings(settings);
                    });
            connect(widgets[2], &ObjectDisplayOptionsWidget::opacityChanged,
                    this, [this](double opacity) {
                        DisplaySettings settings = uiState->getDividedMesh1Settings();
                        settings.opacity = opacity;
                        uiState->setDividedMesh1Settings(settings);
                    });
        }
        
        if (widgets.size() > 3 && widgets[3]) {
            connect(widgets[3], &ObjectDisplayOptionsWidget::visibilityToggled,
                    this, [this](bool visible) {
                        DisplaySettings settings = uiState->getDividedMesh2Settings();
                        settings.isVisible = visible;
                        uiState->setDividedMesh2Settings(settings);
                    });
            connect(widgets[3], &ObjectDisplayOptionsWidget::opacityChanged,
                    this, [this](double opacity) {
                        DisplaySettings settings = uiState->getDividedMesh2Settings();
                        settings.opacity = opacity;
                        uiState->setDividedMesh2Settings(settings);
                    });
        }
        
        if (widgets.size() > 4 && widgets[4]) {
            connect(widgets[4], &ObjectDisplayOptionsWidget::visibilityToggled,
                    this, [this](bool visible) {
                        DisplaySettings settings = uiState->getDividedMesh3Settings();
                        settings.isVisible = visible;
                        uiState->setDividedMesh3Settings(settings);
                    });
            connect(widgets[4], &ObjectDisplayOptionsWidget::opacityChanged,
                    this, [this](double opacity) {
                        DisplaySettings settings = uiState->getDividedMesh3Settings();
                        settings.opacity = opacity;
                        uiState->setDividedMesh3Settings(settings);
                    });
        }
        
        if (widgets.size() > 5 && widgets[5]) {
            connect(widgets[5], &ObjectDisplayOptionsWidget::visibilityToggled,
                    this, [this](bool visible) {
                        DisplaySettings settings = uiState->getDividedMesh4Settings();
                        settings.isVisible = visible;
                        uiState->setDividedMesh4Settings(settings);
                    });
            connect(widgets[5], &ObjectDisplayOptionsWidget::opacityChanged,
                    this, [this](double opacity) {
                        DisplaySettings settings = uiState->getDividedMesh4Settings();
                        settings.opacity = opacity;
                        uiState->setDividedMesh4Settings(settings);
                    });
        }
    }
}

void MainWindowUI::updateUIFromState()
{
    // UIStateの値をUIコンポーネントに反映
    if (stressRangeWidget) {
        stressRangeWidget->setStressRange(uiState->getMinStress(), uiState->getMaxStress());
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
    
    if (displayOptionsContainer) {
        auto widgets = displayOptionsContainer->getAllDisplayWidgets();
        
        // Update visibility and opacity settings
        if (widgets.size() > 0 && widgets[0]) {
            widgets[0]->setVisibleState(uiState->getMeshDisplaySettings().isVisible);
            widgets[0]->setOpacity(uiState->getMeshDisplaySettings().opacity);
        }
        if (widgets.size() > 1 && widgets[1]) {
            widgets[1]->setVisibleState(uiState->getVtuDisplaySettings().isVisible);
            widgets[1]->setOpacity(uiState->getVtuDisplaySettings().opacity);
        }
        if (widgets.size() > 2 && widgets[2]) {
            widgets[2]->setVisibleState(uiState->getDividedMesh1Settings().isVisible);
            widgets[2]->setOpacity(uiState->getDividedMesh1Settings().opacity);
        }
        if (widgets.size() > 3 && widgets[3]) {
            widgets[3]->setVisibleState(uiState->getDividedMesh2Settings().isVisible);
            widgets[3]->setOpacity(uiState->getDividedMesh2Settings().opacity);
        }
        if (widgets.size() > 4 && widgets[4]) {
            widgets[4]->setVisibleState(uiState->getDividedMesh3Settings().isVisible);
            widgets[4]->setOpacity(uiState->getDividedMesh3Settings().opacity);
        }
        if (widgets.size() > 5 && widgets[5]) {
            widgets[5]->setVisibleState(uiState->getDividedMesh4Settings().isVisible);
            widgets[5]->setOpacity(uiState->getDividedMesh4Settings().opacity);
        }
    }
} 