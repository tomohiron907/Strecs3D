#ifndef MAINWINDOWUI_H
#define MAINWINDOWUI_H

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "widgets/DensitySlider.h"
#include "widgets/MessageConsole.h"
#include "widgets/Button.h"
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include "widgets/ModeComboBox.h"
#include "widgets/ObjectDisplayOptionsWidget.h"
#include "widgets/DisplayOptionsContainer.h"
#include <QObject>

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
    MessageConsole* getMessageConsole() const { return messageConsole; }
    DisplayOptionsContainer* getDisplayOptionsContainer() const { return displayOptionsContainer; }
    
    // 個別のウィジェットへのアクセサー（後方互換性のため）
    ObjectDisplayOptionsWidget* getObjectDisplayOptionsWidget() const { return displayOptionsContainer->getStlDisplayWidget(); }
    ObjectDisplayOptionsWidget* getVtkDisplayOptionsWidget() const { return displayOptionsContainer->getVtkDisplayWidget(); }
    ObjectDisplayOptionsWidget* getDividedMeshWidget1() const { return displayOptionsContainer->getDividedMeshWidget1(); }
    ObjectDisplayOptionsWidget* getDividedMeshWidget2() const { return displayOptionsContainer->getDividedMeshWidget2(); }
    ObjectDisplayOptionsWidget* getDividedMeshWidget3() const { return displayOptionsContainer->getDividedMeshWidget3(); }
    ObjectDisplayOptionsWidget* getDividedMeshWidget4() const { return displayOptionsContainer->getDividedMeshWidget4(); }

    // アイコンサイズ一括設定
    void setButtonIconSize(const QSize& size);

public slots:
    void resizeDisplayOptionsContainer();
    void resizeEvent(QResizeEvent* event);
    bool eventFilter(QObject* watched, QEvent* event);

private:
    void setupStyle();

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
    MessageConsole* messageConsole;
    DisplayOptionsContainer* displayOptionsContainer;
};

#endif // MAINWINDOWUI_H 