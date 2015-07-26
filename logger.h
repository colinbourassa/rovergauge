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
    CUXInterface *m_cux;
    QString m_logExtension;
    QString m_logDir;
    QFile m_logFile;
    QTextStream m_logFileStream;
    QString m_lastAttemptedLog;

    float getRowWithWeighting();
    float getColWithWeighting();
};

#endif // LOGGER_H

