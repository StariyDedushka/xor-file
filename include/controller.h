#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include "xorlogic.h"
#include "mainwindow.h"
#include <QThread>
#include <QDebug>

class Controller : public QObject
{
    Q_OBJECT

public slots:
    void terminate();
    void start();


signals:
    void startLogic();
    void execStopped();

public:
    Controller();
    ~Controller();
    XorLogic* getLogic() { return logic; }
    QThread* getThread() { return thread; }
    void init();

private:
    MainWindow w;
    XorLogic *logic;
    QThread *thread;
    bool stopped;
    void connectSignals();
};

#endif // CONTROLLER_H
