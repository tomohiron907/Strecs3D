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
#include "UI/mainwindowui.h"
#include "UI/widgets/MessageConsole.h"
#include <QString>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
    QString getCurrentMode() const;
    QString getCurrentStlFilename() const;
    void logMessage(const QString& message);

public slots:
    void openVTKFile();
    void openSTLFile();
    void processFiles();
    void export3mfFile();
    void onObjectVisibilityChanged(bool visible);
    void onObjectOpacityChanged(double opacity);
    void onVtkObjectVisibilityChanged(bool visible);
    void onVtkObjectOpacityChanged(double opacity);
    void onParametersChanged(); // DensitySliderやModeComboBoxが変更された時の処理
    void updateProcessButtonState(); // Processボタンの有効/無効状態を更新

private:
    void setupSignalSlotConnections();
    
    std::unique_ptr<ApplicationController> appController;
    std::unique_ptr<MainWindowUI> ui;
    std::unique_ptr<MainWindowUIAdapter> uiAdapter;
};

#endif // MAINWINDOW_H
