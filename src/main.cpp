#include <iostream>
#include <QApplication>

#include "MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <cstdio>

#include <windows.h>

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
    MainWindow window;
    window.show();
    
    CoUninitialize();

    return app.exec();
}