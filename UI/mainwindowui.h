#ifndef MAINWINDOWUI_H
#define MAINWINDOWUI_H

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "widgets/process/ProcessManagerWidget.h"
#include "widgets/process/steps/ImportStepWidget.h"
#include "widgets/process/steps/BoundaryConditionStepWidget.h"
#include "widgets/process/steps/SimulationStepWidget.h"
#include "widgets/process/steps/InfillStepWidget.h"
#include "widgets/DensitySlider.h"
#include "widgets/StressRangeWidget.h"
#include "widgets/MessageConsole.h"
#include "widgets/Button.h"
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include "widgets/ModeComboBox.h"
#include "widgets/ObjectListWidget.h"
#include "widgets/PropertyWidget.h"
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
    
    // Legacy getters mapped to new ProcessManager structure
    Button* getOpenStepButton() const;
    Button* getConstrainButton() const;
    Button* getLoadButton() const;
    Button* getSimulateButton() const;
    Button* getProcessButton() const;
    
    Button* getExport3mfButton() const { return export3mfButton; }
    ModeComboBox* getModeComboBox() const { return modeComboBox; }
    DensitySlider* getRangeSlider() const;
    StressRangeWidget* getStressRangeWidget() const;
    MessageConsole* getMessageConsole() const { return messageConsole; }
    ObjectListWidget* getObjectListWidget() const { return objectListWidget; }
    PropertyWidget* getPropertyWidget() const { return propertyWidget; }
    UIState* getUIState() const { return uiState; }

    // アイコンサイズ一括設定
    void setButtonIconSize(const QSize& size);
    
    // UIState連携メソッド
    ProcessManagerWidget* getProcessManagerWidget() const { return processManagerWidget; }
    void connectUIStateSignals();
    void updateUIFromState();
    
private:

public slots:
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
    ProcessManagerWidget* processManagerWidget = nullptr;
    
    Button* openStlButton;
    Button* openVtkButton;
    // Button* openStepButton; // Replaced
    // Button* constrainButton; // Replaced
    // Button* loadButton; // Replaced
    // Button* simulateButton; // Replaced
    // Button* processButton; // Replaced
    Button* export3mfButton;
    ModeComboBox* modeComboBox;
    // DensitySlider* rangeSlider; // Replaced
    // StressRangeWidget* stressRangeWidget; // Replaced
    MessageConsole* messageConsole;
    ObjectListWidget* objectListWidget;
    PropertyWidget* propertyWidget;
    QWidget* rightPaneWidget;
    UIState* uiState;
};

#endif // MAINWINDOWUI_H 