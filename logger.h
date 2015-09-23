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
    void onFuelMapDataReady(unsigned int fuelMapId);
    void onDisconnect();

private:
    bool m_fuelMapDataIsReady;
    unsigned int m_fuelMapId;
    CUXInterface *m_cux;
    QString m_logExtension;
    QString m_logDir;
    QFile m_logFile;
    QFile m_staticLogFile;
    QTextStream m_logFileStream;
    QTextStream m_staticLogFileStream;
    QString m_lastAttemptedLog;
    QString m_lastAttemptedStaticLog;
    bool m_staticDataLogged;

    void logStaticData(unsigned int fuelMapId);
    float getRowWithWeighting();
    float getColWithWeighting();
};

#endif // LOGGER_H

