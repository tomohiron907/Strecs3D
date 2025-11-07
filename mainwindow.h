#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVTKOpenGLNativeWidget.h>
#include <QComboBox>
#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include "core/application/ApplicationController.h"
#include "core/application/MainWindowUIAdapter.h"
#include "core/commands/Command.h"
#include "UI/mainwindowui.h"
#include "UI/widgets/MessageConsole.h"
#include <QString>
#include <functional>
#include <memory>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
    QString getCurrentMode() const;
    QString getCurrentStlFilename() const;
    void logMessage(const QString& message);
    
    // UIState access
    UIState* getUIState() const;
    void printUIStateDebugInfo() const;

public slots:
    // File operation slots (using commands)
    void onOpenStlButtonClicked();
    void onOpenVtkButtonClicked();
    void onProcessButtonClicked();
    void onExport3mfButtonClicked();

    // Display control slots (using commands)
    void onObjectVisibilityChanged(bool visible);
    void onObjectOpacityChanged(double opacity);
    void onVtkObjectVisibilityChanged(bool visible);
    void onVtkObjectOpacityChanged(double opacity);

    // Parameter change slots (using commands)
    void onStressRangeChanged(double minStress, double maxStress);
    void onModeChanged(int index);
    void onParametersChanged();

    void updateProcessButtonState(); // Processボタンの有効/無効状態を更新
    void showUIStateDebugInfo(); // UIStateのデバッグ情報をコンソールに表示

private:
    // Initialization methods
    void initializeComponents();
    void setupWindow();
    void connectSignals();

    // Signal connection methods
    void connectUISignals();
    void connectDisplayWidgetSignals();
    void connectMessageSignals();

    // UI update methods
    void updateButtonsAfterProcessing(bool success);
    void resetExportButton();
    void updateUIStateFromWidgets(); // UIウィジェットからUIStateを更新

    // Command execution
    void executeCommand(std::unique_ptr<Command> command);

    std::unique_ptr<ApplicationController> appController;
    std::unique_ptr<MainWindowUI> ui;
    std::unique_ptr<MainWindowUIAdapter> uiAdapter;
};

#endif // MAINWINDOW_H
