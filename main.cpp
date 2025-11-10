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

    // OpenCASCADEのバージョン情報を出力
    std::cout << "OpenCASCADE Version: "
              << OCC_VERSION_MAJOR << "."
              << OCC_VERSION_MINOR << "."
              << OCC_VERSION_MAINTENANCE << std::endl;

    // アプリケーションアイコンを設定
    app.setWindowIcon(QIcon(":/resources/strecs_icon.png"));

    // QVTKOpenGLNativeWidgetで使用するデフォルトのSurfaceFormatを設定
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    MainWindow window;
    window.show();

    return app.exec();
}
