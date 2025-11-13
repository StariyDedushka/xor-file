#include "mainwindow.h"
#include "xorlogic.h"
#include "controller.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    qRegisterMetaType<QFileInfoList>("QFileInfoList");
    qRegisterMetaType<QFileInfoList*>("QFileInfoList*");
    QApplication a(argc, argv);

    Controller controller;
    controller.init();

    return a.exec();
}
