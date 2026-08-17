#include <iostream>
#include <QApplication>

#include "MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <cstdio>

#include <windows.h>

#include <QFile>

int main(int argc, char* argv[])
{
    
    CoInitializeEx(
        nullptr,
        COINIT_APARTMENTTHREADED
    );
    
    qInstallMessageHandler(
        [](QtMsgType,
           const QMessageLogContext&,
           const QString& message)
        {
            fprintf(
                stderr,
                "%s\n",
                message.toLocal8Bit().constData()
            );
            fflush(stderr);
        }
    );


    QApplication app(argc, argv);
    
    QIcon icon(":/logo.ico");

    app.setWindowIcon(icon);
    
    MainWindow window;
    window.show();
    window.setWindowIcon(icon);
    CoUninitialize();

    return app.exec();
}