#include "mainwindow.h"
#include "xorlogic.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<QFileInfoList>("QFileInfoList");
    qRegisterMetaType<QFileInfoList*>("QFileInfoList*");
    QApplication a(argc, argv);
    MainWindow w;
    XorLogic logic;
    QThread *workerThread = new QThread();
    logic.moveToThread(workerThread);

    MainWindow::connect(&w, &MainWindow::signal_btn_openPath_clicked, &logic, &XorLogic::slot_btn_openPath_clicked);
    MainWindow::connect(&w, &MainWindow::signal_btn_savePath_clicked, &logic, &XorLogic::slot_btn_savePath_clicked);
    MainWindow::connect(&w, &MainWindow::signal_modifierEdited, &logic, &XorLogic::slot_modifierEdited);
    MainWindow::connect(&w, &MainWindow::signal_overwriteModeChecked, &logic, &XorLogic::slot_overwriteModeChecked);
    MainWindow::connect(&w, &MainWindow::signal_startButton_clicked, &logic, &XorLogic::slot_startButton_clicked);
    MainWindow::connect(&w, &MainWindow::signal_timerModeChecked, &logic, &XorLogic::slot_timerModeChecked);
    MainWindow::connect(&w, &MainWindow::signal_timerMode_time_changed, &logic, &XorLogic::slot_timerMode_time_changed);
    MainWindow::connect(&w, &MainWindow::signal_deleteInputChecked, &logic, &XorLogic::slot_deleteInputChecked);
    MainWindow::connect(&w, &MainWindow::signal_maskChanged, &logic, &XorLogic::slot_maskChanged);

    MainWindow::connect(&logic, &XorLogic::signal_scanComplete, &w, &MainWindow::slot_scanComplete);
    MainWindow::connect(&logic, &XorLogic::signal_progress, &w, &MainWindow::slot_progress);
    MainWindow::connect(&logic, &XorLogic::signal_xor_started, &w, &MainWindow::slot_xor_started);
    MainWindow::connect(&logic, &XorLogic::signal_xor_finished, &w, &MainWindow::slot_xor_finished);


    QObject::connect(workerThread, &QThread::finished, &logic, &XorLogic::deleteLater);
    QObject::connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
    QMetaObject::invokeMethod(&logic, "initTimer", Qt::QueuedConnection);

    workerThread->start();
    w.show();
    return a.exec();
}
