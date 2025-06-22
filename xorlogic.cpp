#include "xorlogic.h"

XorLogic::XorLogic()
{
    saveDir = new QDir("../");
    openDir = new QDir("../");
    file = new QFile();
    overwriteMode = true;
    filterList = new QStringList();
    deleteInput = false;
}

XorLogic::~XorLogic()
{
    delete saveDir;
    delete openDir;
    delete file;
    delete timer;
    delete filterList;
}

void XorLogic::initTimer()
{
    Q_ASSERT(QThread::currentThread() == this->thread());
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &XorLogic::slot_timeout);
    connect(this, &XorLogic::signal_timeout_start, this, &XorLogic::slot_startButton_clicked);
}

quint32 XorLogic::countFiles()
{
    quint32 count = 0;
    quint32 maxCount = 0;
    QFileInfoList files = saveDir->entryInfoList(QDir::Files);
    for(int i = 0; i < files.size(); i++)
    {
        QRegularExpression expr("(\\d+)");
        QRegularExpressionMatch match = expr.match(files[i].fileName());

        if(match.hasMatch())
        {
            QString capture = match.captured(1);
            qDebug() << "Counter captured from filename:" << capture;
            quint32 count = capture.toInt();
            if(count > maxCount) maxCount = count;
        }
    }
    return maxCount;
}

QFile* XorLogic::createFile(const QFileInfo &fileInfo, bool overwrite)
{
    QFile *createdFile;
    QString suffix = "_output";
    if(!overwrite)
    {
        QString newFilename = saveDir->absolutePath().append("/");
        newFilename.append(fileInfo.baseName());
        newFilename.append(suffix);
        qDebug() << "countFiles() returned:" << countFiles();
        newFilename.append(QString::number(countFiles() + 1));
        newFilename.append(".txt");
        createdFile = new QFile(newFilename);
        qDebug() << "Overwrite mode: OFF; filepath:" << newFilename;
    } else {
        QString newFilename = fileInfo.absoluteFilePath();
        createdFile = new QFile(newFilename);
        qDebug() << "Overwrite mode: ON; filepath:" << newFilename;
    }
    return createdFile;
}

uint64_t XorLogic::invertBinary(uint64_t num)
{
    uint64_t result = 0;
    for(int i = 0; i < 64; i++)
    {
        result <<= 1;
        result |= (num & 1);
        num >>= 1;
    }
    return result;
}

void XorLogic::setupFile()
{
    QFileInfoList files = scanForFiles();
    for(int i = 0; i < files.size(); i++)
    {
    emit signal_xor_started(files.at(i).fileName());
    qDebug() << "Performing XOR on file: " << files.at(i).fileName();
    QFile *file = createFile(files.at(i), true);
    QFile *outFile = createFile(files.at(i), false);
    QBuffer *buffer = new QBuffer();

    writeBuffer(file, buffer);
    if(overwriteMode)
    {
        writeFile(file, buffer);
        emit signal_xor_finished(files.at(i).fileName());
        delete file;
        delete buffer;
        delete outFile;
    }
    else if(!overwriteMode)
    {
        if(deleteInput)
        {
            deleteFile(file);
        }
        writeFile(outFile, buffer);
        emit signal_xor_finished(files.at(i).fileName());
        delete file;
        delete buffer;
        delete outFile;
    }
    }
}

void XorLogic::run()
{
    setupFile();
}

bool XorLogic::deleteFile(QFile *file)
{
    if(file->remove())
    {
        qDebug() << "Deleted file:" << file->fileName();
        return true;
    }
    else return false;
}

bool XorLogic::writeBuffer(QFile *file, QBuffer *buffer)
{
    if(file->open(QIODevice::ReadOnly))
    {
        if(buffer->open(QIODevice::WriteOnly))
        {
            qint64 len = buffer->write(file->readAll());
            qDebug() << "Written to buffer:" << len;
            file->close();
            buffer->close();
            return true;
        }
    }
    return false;
}

bool XorLogic::writeFile(QFile *file, QBuffer *buffer)
{
    QByteArray bytearr(8, '\0');
    quint64 maxProgress = buffer->size();
    quint64 progress = 0;
    if(file->open(QIODevice::WriteOnly))
    {
        if(buffer->open(QIODevice::ReadOnly))
        {

            while(!buffer->atEnd())
            {
                bytearr = buffer->read(8);
                qDebug() << "Written to byte array from buffer:" << bytearr.size();
                progress += bytearr.size();

                emit signal_progress(progress, maxProgress);

                if(bytearr.size() < 8)
                {
                    bytearr.append(QByteArray(8 - bytearr.size(), '\0'));
                }
                bytearr = performXOR(bytearr);
                uint len = file->write(bytearr);
                qDebug() << "Written bytes to file:" << len;
            }
        } else return false;
    } else return false;
    file->close();
    buffer->close();
    return true;
}


QByteArray XorLogic::performXOR(QByteArray bytearr)
{
    QByteArray result(8, '\0');
    quint64 modifierInverted = invertBinary(modifier);
    quint64 orderKey = qToBigEndian(modifierInverted);
    const char* keyBytes = reinterpret_cast<const char*>(&orderKey);

    qDebug() << "Modifier inverted:" << modifierInverted;
    qDebug() << "Input bytes:" << bytearr.toHex();

    for(int i = 0; i < result.size(); i++)
    {
        result[i] = bytearr[i] ^ keyBytes[i];
        qDebug() << QString("Byte %1: 0x%2 XOR 0x%3 = 0x%4")
                        .arg(i)
                        .arg(QString::number((uchar)bytearr[i], 16))
                        .arg(QString::number(keyBytes[i], 16))
                        .arg(QString::number((uchar)result[i], 16));
    }
    qDebug() << "Modified array hex value:" << result.toHex();
    return result;
}

void XorLogic::slot_btn_openPath_clicked(QString openPath)
{
    openDir->cd(openPath);
    scanForFiles();
    qDebug() << "Button open path clicked, open dir: " << openPath;
}

void XorLogic::slot_btn_savePath_clicked(QString savePath)
{
    saveDir->cd(savePath);
    qDebug() << "Button save path clicked, save dir:" << savePath;
}

void XorLogic::slot_modifierEdited(uint64_t mod)
{
    modifier = mod;
    qDebug() << "Saved modifier:" << modifier;
}

void XorLogic::slot_overwriteModeChecked(bool isChecked)
{
    overwriteMode = isChecked;
    qDebug() << "Overwrite mode:" << overwriteMode;
}

void XorLogic::slot_startButton_clicked(uint64_t mod)
{
    if(timerMode) startTimer();
    scanForFiles();
    modifier = mod;
    run();
}
void XorLogic::slot_timerModeChecked(bool isChecked)
{
    timerMode = isChecked;
    if(!timerMode) timer->stop();
    qDebug() << "Timer mode:" << timerMode;
}

void XorLogic::startTimer()
{
    quint64 intervalMs = timerInterval * 1000;
    timer->start(intervalMs); // *1000 т.к. в методе период указывается в миллисекундах
    qDebug() << "Timer started!";
}

void XorLogic::slot_timeout()
{
    if(!timerMode) timer->stop();
    run();
    scanForFiles();
    startTimer();
    qDebug() << "Timeout reached";
}

QFileInfoList XorLogic::scanForFiles()
{
    QFileInfoList files = openDir->entryInfoList(QDir::Files);
    QFileInfoList outputList;
    for(int i = 0; i < files.size(); ++i)
    {
        QStringList parts = files.at(i).fileName().split(".");

        if(parts.size() < 2)
        {
            qDebug() << "Parts list contains no files with extensions, exiting cycle";
            continue;
        }

        QString extension = parts.last();
        for(int j = 0; j < filterList->size(); j++)
        {
            if(QString(filterList->at(j)).contains(extension))
            {
                outputList.push_back(files.at(i));
                qDebug() << "filterList[" << j << "] =" << filterList->at(j);
                qDebug() << "Contains <<" << extension << ">>?:" << QString(filterList->at(j)).contains(extension);
                break;
            }
        }
    }
    emit signal_scanComplete(outputList);
    return outputList;
}

void XorLogic::slot_timerMode_time_changed(quint32 time)
{
    timerInterval = time;
    qDebug() << "Set timer period to" << timerInterval;
}

void XorLogic::slot_deleteInputChecked(bool isChecked)
{
    deleteInput = isChecked;
}

void XorLogic::slot_maskChanged(QString mask)
{
    qDebug() << "Mask list changed:" << mask;
    *filterList = QStringList(mask.split(' ', Qt::SkipEmptyParts));
    for(int i = 0; i < filterList->size(); i++)
    {
        qDebug() << "filterList[" << i << "] = " << filterList->at(i);
    }
}
