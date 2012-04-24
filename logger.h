#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include "cuxinterface.h"

class Logger
{
public:
    Logger(CUXInterface *cuxIFace);
    bool openLog(QString fileName);
    void closeLog();
    void logData();
    QString getLogPath();

private:
    CUXInterface *cux;
    QString logExtension;
    QString logDir;
    QFile logFile;
    QTextStream logFileStream;
    QString lastAttemptedLog;
};

#endif // LOGGER_H
