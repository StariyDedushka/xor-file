#include "include/xorlogic.h"
#define READ_SIZE 8 * 1024

XorLogic::XorLogic()
    : saveDir(new QDir("../"))
    , openDir(new QDir("../"))
    , overwriteMode(false)
    , deleteInput(false)
    , timerMode(false)
{
    initTimer();
}

XorLogic::~XorLogic()
{
    delete saveDir;
    delete openDir;
}

void XorLogic::initTimer()
{
    Q_ASSERT(QThread::currentThread() == this->thread());
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, &XorLogic::timeout);
    connect(this, &XorLogic::timeout_start, this, &XorLogic::startButton_clicked);
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
    QRegularExpression regexp("(\\d+)");
    QRegularExpressionMatch match;
    if(!overwrite)
    {
        match = regexp.match(fileInfo.baseName());
        QString newFilename = saveDir->absolutePath().append("/");
        newFilename.append(fileInfo.baseName());
        match = regexp.match(newFilename);
        if(newFilename.contains("_output"))
            newFilename.remove(match.capturedStart(), match.captured().size());
        else newFilename.append(suffix);

        newFilename.append(QString::number(countFiles() + 1));

        qDebug() << "countFiles() returned:" << countFiles();
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
    bool skip = false;
    QFileInfoList files = scanForFiles();
    QList<QFileInfo> deletedFiles;
    for(int i = 0; i < files.size(); i++)
    {
        emit xor_started(files.at(i).fileName());
        qDebug() << "Performing XOR on file: " << files.at(i).fileName();
        QFile *file = createFile(files.at(i), true);
        QFile *outFile = createFile(files.at(i), false);
        QBuffer *buffer = new QBuffer();

        if(!writeBuffer(file, buffer))
            throw std::logic_error("Couldn't write to buffer");
        if(overwriteMode)
        {
            writeFile(file, buffer);
            emit xor_finished(files.at(i).fileName());
        }
        else if(!overwriteMode)
        {
            if(deleteInput)
            {
                if(!deletedFiles.empty())
                {
                    for(QFileInfo info : deletedFiles)
                    {
                        if(file->fileName() == info.fileName())
                        {
                            skip = true;
                            break;
                        }
                    }
                }
                if(skip) break;
                deleteFile(file);
                deletedFiles.append(*file);
            }
            writeFile(outFile, buffer);
            emit xor_finished(files.at(i).fileName());
        }
        skip = false;
        delete file;
        delete buffer;
        delete outFile;
    }
}

void XorLogic::run()
{
    if(QThread::currentThread()->isInterruptionRequested())
    {
        return;
    }
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
            // qDebug() << "Written to buffer:" << len;
            file->close();
            buffer->close();
            return true;
        }
    }
    return false;
}

bool XorLogic::writeFile(QFile *file, QBuffer *buffer)
{
    QByteArray bytearr(READ_SIZE, '\0');
    quint64 maxProgress = buffer->size();
    quint64 progressed = 0;
    if(file->open(QIODevice::WriteOnly))
    {
        if(buffer->open(QIODevice::ReadOnly))
        {

            while(!buffer->atEnd())
            {
                if(QThread::currentThread()->isInterruptionRequested()) // при запрошенном прерывании прекращаем работу
                    return false;

                bytearr = buffer->read(READ_SIZE);
                // qDebug() << "Written to byte array from buffer:" << bytearr.size();
                progressed += bytearr.size();

                emit progress(progressed, maxProgress);

                // если нужно дополнение до кратного восьми размера
                // if(bytearr.size() < 8)
                // {
                //     bytearr.append(QByteArray(8 - bytearr.size(), '\0'));
                // }
                performXOR(bytearr);
                uint len = file->write(bytearr);
                // qDebug() << "Written bytes to file:" << len;
            }
        } else return false;
    } else return false;
    file->close();
    buffer->close();
    return true;
}


void XorLogic::performXOR(QByteArray& bytearr)
{
    quint64 modifierInverted = invertBinary(modifier);
    quint64 orderKey = qToBigEndian(modifierInverted);
    const char* keyBytes = reinterpret_cast<const char*>(&orderKey);

    // qDebug() << "Modifier inverted:" << modifierInverted;
    // qDebug() << "Input bytes:" << bytearr.toHex();

    for(int i = 0; i < bytearr.size(); ++i)
    {
        bytearr[i] = bytearr[i] ^ keyBytes[i % 8];
        // qDebug() << QString("Byte %1: 0x%2 XOR 0x%3 = 0x%4")
        //             .arg(i)
        //             .arg(QString::number((uchar)*bytearr[i], 16))
        //             .arg(QString::number(keyBytes[i], 16));
    }
}

void XorLogic::btn_openPath_clicked(QString openPath)
{
    openDir->cd(openPath);
    scanForFiles();
    qDebug() << "Button open path clicked, open dir: " << openPath;
}

void XorLogic::btn_savePath_clicked(QString savePath)
{
    saveDir->cd(savePath);
    qDebug() << "Button save path clicked, save dir:" << savePath;
}

void XorLogic::modifierEdited(uint64_t mod)
{
    modifier = mod;
    qDebug() << "Saved modifier:" << modifier;
}

void XorLogic::overwriteModeChecked(bool isChecked)
{
    overwriteMode = isChecked;
    qDebug() << "Overwrite mode:" << overwriteMode;
}

void XorLogic::startButton_clicked()
{
    if(timerMode)
        startTimer();
    scanForFiles();
    run();
}

void XorLogic::btn_stop_clicked()
{
    timer.stop();
    qDebug() << "Stop button clicked";
}

void XorLogic::timerModeChecked(bool isChecked)
{
    timerMode = isChecked;
    if(!timerMode) timer.stop();
    qDebug() << "Timer mode:" << timerMode;
}

void XorLogic::startTimer()
{
    quint64 intervalMs = timerInterval * 1000;
    timer.start(intervalMs);
    qDebug() << "Timer started!";
}

void XorLogic::timeout()
{
    if(!timerMode) timer.stop();
    if(busy)
    {
        timer.stop();
        return;
    }
    qDebug() << "Timeout reached";
    scanForFiles();
    run();
    startTimer();
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
        for(int j = 0; j < filterList.size(); j++)
        {
            if(QString(filterList.at(j)).contains(extension))
            {
                outputList.push_back(files.at(i));
                qDebug() << "filterList[" << j << "] =" << filterList.at(j);
                qDebug() << "Contains <<" << extension << ">>?:" << QString(filterList.at(j)).contains(extension);
                break;
            }
        }
    }
    emit scanComplete(outputList);
    return outputList;
}

void XorLogic::timerMode_time_changed(quint32 time)
{
    timerInterval = time;
    qDebug() << "Set timer period to" << timerInterval;
}

void XorLogic::deleteInputChecked(bool isChecked)
{
    deleteInput = isChecked;
}

void XorLogic::maskChanged(QString mask)
{
    qDebug() << "Mask list changed:" << mask;
    filterList = QStringList(mask.split(' ', Qt::SkipEmptyParts));
    for(int i = 0; i < filterList.size(); i++)
    {
        qDebug() << "filterList[" << i << "] = " << filterList.at(i);
    }
}
