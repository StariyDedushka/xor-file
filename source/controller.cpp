#include "controller.h"

Controller::Controller()
    : logic(new XorLogic())
    , thread(new QThread())
    , stopped(false)
{
    logic->moveToThread(thread);
    connect(this, &Controller::startLogic, logic, &XorLogic::startButton_clicked);
    connect(thread, &QThread::finished, logic, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    // Инициализация таймера
    QMetaObject::invokeMethod(logic, "initTimer", Qt::QueuedConnection);
    connectSignals();
}

Controller::~Controller()
{
    if (thread->isRunning()) {
        thread->quit();
        thread->wait(1000);
    }}


void Controller::start()
{
    if (!thread->isRunning()) {
        thread->start();
    }
    emit startLogic();
}


void Controller::terminate()
{
    if (thread->isRunning()) {
        thread->requestInterruption();
        thread->quit();
        if (!thread->wait(1000)) {
            thread->terminate();
            thread->wait();
        }
    }

    // Пересоздаем объекты для следующего запуска
    logic = new XorLogic();
    thread = new QThread();

    logic->moveToThread(thread);
    connect(this, &Controller::startLogic, logic, &XorLogic::startButton_clicked);
    connect(thread, &QThread::finished, logic, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connectSignals();
}

void Controller::connectSignals()
{
    // Первоначальная настройка соединений
    // Соединения от MainWindow к XorLogic
    QObject::connect(&w, &MainWindow::btn_openPath_clicked, logic, &XorLogic::btn_openPath_clicked);
    QObject::connect(&w, &MainWindow::btn_savePath_clicked, logic, &XorLogic::btn_savePath_clicked);
    QObject::connect(&w, &MainWindow::modifierEdited, logic, &XorLogic::modifierEdited);
    QObject::connect(&w, &MainWindow::overwriteModeChecked, logic, &XorLogic::overwriteModeChecked);
    QObject::connect(&w, &MainWindow::timerModeChecked, logic, &XorLogic::timerModeChecked);
    QObject::connect(&w, &MainWindow::timerMode_time_changed, logic, &XorLogic::timerMode_time_changed);
    QObject::connect(&w, &MainWindow::deleteInputChecked, logic, &XorLogic::deleteInputChecked);
    QObject::connect(&w, &MainWindow::maskChanged, logic, &XorLogic::maskChanged);

    // Соединения от MainWindow к Controller
    QObject::connect(&w, &MainWindow::btn_stop_clicked, this, &Controller::terminate);
    QObject::connect(&w, &MainWindow::startButton_clicked, this, &Controller::start);

    // Соединения от XorLogic к MainWindow
    QObject::connect(logic, &XorLogic::scanComplete, &w, &MainWindow::scanComplete);
    QObject::connect(logic, &XorLogic::progress, &w, &MainWindow::progress);
    QObject::connect(logic, &XorLogic::xor_started, &w, &MainWindow::xor_started);
    QObject::connect(logic, &XorLogic::xor_finished, &w, &MainWindow::xor_finished);

    // Инициализация таймера
    QMetaObject::invokeMethod(logic, "initTimer", Qt::QueuedConnection);
}

void Controller::init()
{
    w.init();
    w.show();
}
