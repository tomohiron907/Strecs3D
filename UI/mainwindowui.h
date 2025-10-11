#ifndef MAINWINDOWUI_H
#define MAINWINDOWUI_H

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "widgets/DensitySlider.h"
#include "widgets/StressRangeWidget.h"
#include "widgets/MessageConsole.h"
#include "widgets/Button.h"
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include "widgets/ModeComboBox.h"
#include "widgets/ObjectDisplayOptionsWidget.h"
#include "widgets/DisplayOptionsContainer.h"
#include "../core/ui/UIState.h"
#include <QObject>
#include <QTouchEvent>

class MainWindow;

class MainWindowUI : public QObject {
    Q_OBJECT
public:
    explicit MainWindowUI(MainWindow* mainWindow);
    ~MainWindowUI() = default;

    void setupUI();
    QWidget* getCentralWidget() const { return centralWidget; }
    QVTKOpenGLNativeWidget* getVtkWidget() const { return vtkWidget; }
    vtkSmartPointer<vtkRenderer> getRenderer() const { return renderer; }
    Button* getOpenStlButton() const { return openStlButton; }
    Button* getOpenVtkButton() const { return openVtkButton; }
    Button* getProcessButton() const { return processButton; }
    Button* getExport3mfButton() const { return export3mfButton; }
    ModeComboBox* getModeComboBox() const { return modeComboBox; }
    DensitySlider* getRangeSlider() const { return rangeSlider; }
    StressRangeWidget* getStressRangeWidget() const { return stressRangeWidget; }
    MessageConsole* getMessageConsole() const { return messageConsole; }
    DisplayOptionsContainer* getDisplayOptionsContainer() const { return displayOptionsContainer; }
    UIState* getUIState() const { return uiState; }
    
    // 個別のウィジェットへのアクセサー（後方互換性のため）
    ObjectDisplayOptionsWidget* getObjectDisplayOptionsWidget() const { return displayOptionsContainer->getStlDisplayWidget(); }
    ObjectDisplayOptionsWidget* getVtkDisplayOptionsWidget() const { return displayOptionsContainer->getVtkDisplayWidget(); }
    ObjectDisplayOptionsWidget* getDividedMeshWidget1() const { return displayOptionsContainer->getDividedMeshWidget1(); }
    ObjectDisplayOptionsWidget* getDividedMeshWidget2() const { return displayOptionsContainer->getDividedMeshWidget2(); }
    ObjectDisplayOptionsWidget* getDividedMeshWidget3() const { return displayOptionsContainer->getDividedMeshWidget3(); }
    ObjectDisplayOptionsWidget* getDividedMeshWidget4() const { return displayOptionsContainer->getDividedMeshWidget4(); }

    // アイコンサイズ一括設定
    void setButtonIconSize(const QSize& size);
    
    // UIState連携メソッド
    void connectUIStateSignals();
    void updateUIFromState();
    
private:

public slots:
    void resizeDisplayOptionsContainer();
    void resizeEvent(QResizeEvent* event);
    bool eventFilter(QObject* watched, QEvent* event);

private:
    // Constants
    static constexpr int HEADER_HEIGHT = 70;
    static constexpr int LOGO_SIZE = 40;
    static constexpr int LOGO_TYPE_HEIGHT = 19;
    static constexpr int HEADER_SPACING = 15;
    static constexpr int HEADER_LEFT_MARGIN = 35;
    static constexpr int HEADER_BOTTOM_MARGIN = 3;
    static constexpr int LEFT_PANE_MAX_WIDTH = 300;
    static constexpr int RIGHT_PANE_WIDTH = 320;
    static constexpr int DISPLAY_OPTIONS_WIDTH = 300;
    static constexpr int WIDGET_MARGIN = 20;
    static constexpr int CONSOLE_MIN_HEIGHT = 200;
    static constexpr int ICON_SIZE = 25;
    static constexpr int LAYOUT_SPACING = 10;
    static constexpr int EXPORT_BUTTON_TOP_SPACING = 50;

    // Setup methods
    void setupStyle();
    void createHeaderWidget(QVBoxLayout* outerLayout);
    void createLeftPaneWidget(QWidget* vtkParent);
    void createRightPaneWidget(QWidget* vtkParent);
    void createVtkWidget();
    void createButtons();
    void setupWidgetPositioning();

    MainWindow* mainWindow;
    QWidget* centralWidget;
    QVTKOpenGLNativeWidget* vtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderer> renderer;
    Button* openStlButton;
    Button* openVtkButton;
    Button* processButton;
    Button* export3mfButton;
    ModeComboBox* modeComboBox;
    DensitySlider* rangeSlider;
    StressRangeWidget* stressRangeWidget;
    MessageConsole* messageConsole;
    DisplayOptionsContainer* displayOptionsContainer;
    UIState* uiState;
};

#endif // MAINWINDOWUI_H 