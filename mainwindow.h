#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVTKOpenGLNativeWidget.h>
#include <QComboBox>
#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include "core/ApplicationController.h"
#include "UI/mainwindowui.h"
#include "UI/MessageConsole.h"
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

private:
    std::unique_ptr<ApplicationController> appController;
    std::unique_ptr<MainWindowUI> ui;
};

#endif // MAINWINDOW_H
