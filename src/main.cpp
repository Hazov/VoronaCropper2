#include <iostream>
#include <QApplication>

#include "MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <cstdio>


int main(int argc, char* argv[])
{
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

    return app.exec();
}
