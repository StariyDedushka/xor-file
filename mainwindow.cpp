#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    timerMode = false;
    emit signal_timerModeChecked(timerMode);
    emit signal_timerMode_time_changed(ui->spinbox_cycle->value());
    emit signal_maskChanged(ui->editMask->text());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_SelectPath_clicked()
{
    QString openPath = QFileDialog::getExistingDirectory(0, "Выберите папку", "../");
    ui->labelOpenPath->setText(openPath);
    if(!openPath.isEmpty())
    emit signal_btn_openPath_clicked(openPath);
    else QMessageBox::warning(this, "Ошибка", "Неправильная директория!");
}


void MainWindow::on_editMask_editingFinished()
{
    emit signal_maskChanged(ui->editMask->text());
}


void MainWindow::on_btn_SavePath_clicked()
{
    QString savePath = QFileDialog::getExistingDirectory(0, "Выберите путь сохранения", "../");
    ui->savePathLabel->setText(savePath);
    if(!savePath.isEmpty())
    emit signal_btn_savePath_clicked(savePath);
    else
    QMessageBox::warning(this, "Ошибка", "Выберите правильный путь сохранения!");
}

void MainWindow::on_btnOverwrite_clicked()
{
    overwriteMode = ui->btnOverwrite->isChecked();
    emit signal_overwriteModeChecked(overwriteMode);
    if(overwriteMode) ui->checkbox_deleteInput->setEnabled(false);
    else ui->checkbox_deleteInput->setEnabled(true);
}


void MainWindow::on_btnCreateNew_clicked()
{
    // состояние режима перезаписи будем наблюдать через состояние одной кнопки, т.к. она radioButton
    overwriteMode = ui->btnOverwrite->isChecked();
    if(overwriteMode) ui->checkbox_deleteInput->setEnabled(false);
    else ui->checkbox_deleteInput->setEnabled(true);

    emit signal_overwriteModeChecked(overwriteMode);
}


void MainWindow::on_spinbox_cycle_valueChanged(int time)
{
    emit signal_timerMode_time_changed(time);
}


void MainWindow::on_editModifier_editingFinished()
{
    quint64 mod = ui->editModifier->text().toULongLong();
    if(mod <= 18446744073709551615 && mod > 0)
    emit signal_modifierEdited(mod);
    else if(mod <= 0) ui->editModifier->setText("18446744073709551615");
}


void MainWindow::on_timer_checkBox_stateChanged()
{
    timerMode = ui->timer_checkBox->isChecked();
    emit signal_timerModeChecked(timerMode);
}


void MainWindow::on_btn_Start_clicked()
{
    uint64_t modifier = ui->editModifier->text().toULongLong();
    emit signal_startButton_clicked(modifier);
}


// void MainWindow::on_btn_Stop_clicked()
// {

// }

void MainWindow::slot_scanComplete(const QFileInfoList &list)
{
    ui->displayFiles->clear();
    for(auto it = list.begin(); it != list.end(); ++it)
    {
        QFileInfo file = *it;
        ui->displayFiles->append(file.absoluteFilePath());
    }
}

void MainWindow::on_checkbox_deleteInput_stateChanged()
{
    deleteInput = ui->checkbox_deleteInput->isChecked();
    if(deleteInput) ui->btnOverwrite->setEnabled(false);
    else ui->btnOverwrite->setEnabled(true);

    emit signal_deleteInputChecked(deleteInput);
}

void MainWindow::slot_progress(quint64 progress, quint64 maxProgress)
{
    ui->progressBar->setMaximum(maxProgress);
    ui->progressBar->setValue(progress);
}

void MainWindow::slot_xor_started(QString file_name)
{
    ui->displayFiles->append(QString("Начата обработка файла ").append(file_name));

    ui->btnCreateNew->setEnabled(false);
    ui->btnOverwrite->setEnabled(false);
    ui->btn_SavePath->setEnabled(false);
    ui->btn_SelectPath->setEnabled(false);
    ui->btn_Start->setEnabled(false);

    ui->checkbox_deleteInput->setEnabled(false);
    ui->timer_checkBox->setEnabled(false);
}

void MainWindow::slot_xor_finished(QString file_name)
{
    ui->displayFiles->append(QString("Закончена обработка файла ").append(file_name));
    ui->btnCreateNew->setEnabled(true);
    ui->btnOverwrite->setEnabled(true);
    ui->btn_SavePath->setEnabled(true);
    ui->btn_SelectPath->setEnabled(true);
    ui->btn_Start->setEnabled(true);
}
