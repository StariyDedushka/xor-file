#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
signals:
    void signal_modifierEdited(qint64 modifier);
    void signal_timerModeChecked(bool isChecked);
    void signal_overwriteModeChecked(bool isChecked);
    void signal_startButton_clicked(quint64 mod);
    void signal_maskChanged(QString mask);
    void signal_btn_savePath_clicked(QString savePath);
    void signal_btn_openPath_clicked(QString openPath);
    void signal_timerMode_time_changed(int time);
    void signal_deleteInputChecked(bool isChecked);
    void signal_btn_stop_clicked();
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void slot_scanComplete(const QFileInfoList &list);
    void slot_progress(quint64 progress, quint64 maxProgress);
    void slot_xor_started(QString file_name);
    void slot_xor_finished(QString file_name);

private slots:
    void on_btn_SelectPath_clicked();

    void on_editMask_editingFinished();

    void on_btn_SavePath_clicked();

    void on_btnOverwrite_clicked();

    void on_btnCreateNew_clicked();

    void on_spinbox_cycle_valueChanged(int arg1);

    void on_editModifier_editingFinished();

    void on_timer_checkBox_stateChanged();

    void on_btn_Start_clicked();

    // void on_btn_Stop_clicked();

    void on_checkbox_deleteInput_stateChanged();
    void on_timer_timeout();

    void on_btn_stop_clicked();

public:
    void init();

private:
    Ui::MainWindow *ui;
    bool timerMode;
    bool deleteInput;
    bool overwriteMode;
    QTimer timer;

    void blockControls(bool block);

    quint32 timeElapsed;
};
#endif // MAINWINDOW_H
