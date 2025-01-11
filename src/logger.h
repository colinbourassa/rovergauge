#pragma
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include "cuxinterface.h"
#include "optionsdialog.h"

class Logger
{
public:
  Logger(CUXInterface& cuxIFace, OptionsDialog& options);
  bool openLog(QString fileName);
  void closeLog();
  void logData();
  QString getLogPath();
  void onFuelMapDataReady(unsigned int fuelMapId);
  void onDisconnect();

private:
  bool m_fuelMapDataIsReady = false;
  bool m_miscStaticDataIsReady = false;
  unsigned int m_fuelMapId = 0;
  CUXInterface& m_cux;
  OptionsDialog& m_options;
  QString m_logExtension;
  QString m_logDir;
  QFile m_logFile;
  QFile m_staticLogFile;
  QTextStream m_logFileStream;
  QTextStream m_staticLogFileStream;
  QString m_lastAttemptedLog;
  QString m_lastAttemptedStaticLog;
  bool m_staticDataLogged = false;
  QDateTime m_timeOfFirstData;
  bool m_timeOfFirstDataSet = false;

  void logStaticData(unsigned int fuelMapId);
  float getRowWithWeighting() const;
  float getColWithWeighting() const;
  QString getTimestamp(bool forStaticData);

  QMutex m_staticLogLock;
};

