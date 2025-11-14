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
    void modifierEdited(uint64_t mod);
    void timerModeChecked(bool isChecked);
    void overwriteModeChecked(bool isChecked);
    void startButton_clicked();
    void maskChanged(QString mask);
    void btn_savePath_clicked(QString savePath);
    void btn_openPath_clicked(QString openPath);
    void timerMode_time_changed(quint32 time);
    void deleteInputChecked(bool isChecked);
    void btn_stop_clicked();
    void initTimer();

private slots:
    void timeout();

signals:
    void timeout_start(uint64_t mod);
    void scanComplete(const QFileInfoList &list);
    void progress(quint64 progress, quint64 maxProgress);
    void xor_started(QString file_name);
    void xor_finished(QString file_name);

public:
    XorLogic();
    ~XorLogic();
    void run();
private:
    // QFile file;
    uint64_t modifier;
    bool overwriteMode;
    bool timerMode;
    bool busy;
    QTimer timer;
    quint64 timerInterval;
    QDir *saveDir;
    QDir *openDir;
    bool deleteInput;
    QStringList filterList;

    quint32 countFiles();
    QFile* createFile(const QFileInfo &fileInfo, bool overwrite);
    uint64_t invertBinary(uint64_t num);
    void performXOR(quint64 modifier, QByteArray& bytearr);
    void setupFile();
    bool writeBuffer(QFile *file, QBuffer *buffer);
    bool writeFile(QFile *file, QBuffer *buffer);
    void startTimer();
    QFileInfoList scanForFiles();
    bool deleteFile(QFile *file);


};

#endif // XORLOGIC_H
