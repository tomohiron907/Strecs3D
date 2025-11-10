#include <QApplication>
#include "mainwindow.h"
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include <QIcon>
#include <Standard_Version.hxx>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/resources/strecs_icon.png"));

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    MainWindow window;
    window.show();

    return app.exec();
}
