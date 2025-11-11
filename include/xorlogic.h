#ifndef XORLOGIC_H
#define XORLOGIC_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QFileInfo>
#include <QBuffer>
#include <QDir>
#include <QDebug>
#include <QDataStream>
#include <QtEndian>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QThread>

class XorLogic : public QObject
{
    Q_OBJECT
public slots:
    void slot_modifierEdited(uint64_t mod);
    void slot_timerModeChecked(bool isChecked);
    void slot_overwriteModeChecked(bool isChecked);
    void slot_startButton_clicked(uint64_t mod);
    void slot_maskChanged(QString mask);
    void slot_btn_savePath_clicked(QString savePath);
    void slot_btn_openPath_clicked(QString openPath);
    void slot_timerMode_time_changed(quint32 time);
    void slot_deleteInputChecked(bool isChecked);
    void initTimer();

private slots:
    void slot_timeout();

signals:
    void signal_timeout_start(uint64_t mod);
    void signal_scanComplete(const QFileInfoList &list);
    void signal_progress(quint64 progress, quint64 maxProgress);
    void signal_xor_started(QString file_name);
    void signal_xor_finished(QString file_name);

public:
    XorLogic();
    ~XorLogic();
    void run();
private:
    QFile *file;
    uint64_t modifier;
    bool overwriteMode;
    bool timerMode;
    QTimer *timer;
    quint64 timerInterval;
    QDir *saveDir;
    QDir *openDir;
    bool deleteInput;
    QStringList *filterList;

    quint32 countFiles();
    QFile* createFile(const QFileInfo &fileInfo, bool overwrite);
    uint64_t invertBinary(uint64_t num);
    void performXOR(QByteArray *bytearr);
    void setupFile();
    bool writeBuffer(QFile *file, QBuffer *buffer);
    bool writeFile(QFile *file, QBuffer *buffer);
    void startTimer();
    QFileInfoList scanForFiles();
    bool deleteFile(QFile *file);


};

#endif // XORLOGIC_H
