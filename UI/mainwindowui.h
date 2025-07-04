#ifndef MAINWINDOWUI_H
#define MAINWINDOWUI_H

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "DensitySlider.h"
#include "MessageConsole.h"
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>

class MainWindow;

class MainWindowUI {
public:
    explicit MainWindowUI(MainWindow* mainWindow);
    ~MainWindowUI() = default;

    void setupUI();
    QWidget* getCentralWidget() const { return centralWidget; }
    QVTKOpenGLNativeWidget* getVtkWidget() const { return vtkWidget; }
    vtkSmartPointer<vtkRenderer> getRenderer() const { return renderer; }
    QPushButton* getOpenStlButton() const { return openStlButton; }
    QPushButton* getOpenVtkButton() const { return openVtkButton; }
    QPushButton* getProcessButton() const { return processButton; }
    QPushButton* getExport3mfButton() const { return export3mfButton; }
    QComboBox* getModeComboBox() const { return modeComboBox; }
    DensitySlider* getRangeSlider() const { return rangeSlider; }
    MessageConsole* getMessageConsole() const { return messageConsole; }

private:
    void setupStyle();

    MainWindow* mainWindow;
    QWidget* centralWidget;
    QVTKOpenGLNativeWidget* vtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderer> renderer;
    QPushButton* openStlButton;
    QPushButton* openVtkButton;
    QPushButton* processButton;
    QPushButton* export3mfButton;
    QComboBox* modeComboBox;
    DensitySlider* rangeSlider;
    MessageConsole* messageConsole;
};

#endif // MAINWINDOWUI_H 