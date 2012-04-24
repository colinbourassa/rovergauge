#include <QDir>
#include <QDateTime>
#include "logger.h"

/**
 * Constructor. Sets the 14CUX interface class pointer as
 * well as log directory and log file extension.
 */
Logger::Logger(CUXInterface *cuxIFace) :
    logExtension(".txt"), logDir("logs")
{
    cux = cuxIFace;
}

/**
 * Attempts to open a log file with the name specified.
 * @return True on success, false otherwise
 */
bool Logger::openLog(QString fileName)
{
    bool success = false;

    lastAttemptedLog = logDir + QDir::separator() + fileName + logExtension;

    // if the 'logs' directory exists, or if we're able to create it...
    if (!logFile.isOpen() && (QDir(logDir).exists() || QDir().mkdir(logDir)))
    {
        // set the name of the log file and open it for writing
        bool alreadyExists = QFileInfo(lastAttemptedLog).exists();
        logFile.setFileName(lastAttemptedLog);
        if (logFile.open(QFile::WriteOnly | QFile::Append))
        {
            logFileStream.setDevice(&logFile);

            if (!alreadyExists)
            {
                logFileStream << "#time,roadSpeed,engineSpeed,waterTemp,fuelTemp," <<
                                 "throttlePos,mainVoltage,currentFuelMapIndex," <<
                                 "currentFuelMapRow,currentFuelMapCol" << endl;
            }

            success = true;
        }
    }

    return success;
}

/**
 * Close the log file.
 */
void Logger::closeLog()
{
    logFile.close();
}

/**
 * Commands the logger to query the 14CUX interface for the currently
 * buffered data, and write it to the file.
 */
void Logger::logData()
{
    if (logFile.isOpen() && (logFileStream.status() == QTextStream::Ok))
    {
        logFileStream << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << ","
                      << cux->getRoadSpeed() << ","
                      << cux->getEngineSpeedRPM() << ","
                      << cux->getCoolantTemp() << ","
                      << cux->getFuelTemp() << ","
                      << cux->getThrottlePos() << ","
                      << cux->getMainVoltage() << ","
                      << cux->getCurrentFuelMapIndex() << ","
                      << cux->getFuelMapRowIndex() << ","
                      << cux->getFuelMapColumnIndex()
                      << endl;
    }
}

/**
 * Returns the full path to the last log that we attempted to open.
 * @return Full path to last log file
 */
QString Logger::getLogPath()
{
    return lastAttemptedLog;
}
