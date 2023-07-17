#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QString>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
  const QString versionStr = QString("%1.%2.%3").arg(ROVERGAUGE_VER_MAJOR).arg(ROVERGAUGE_VER_MINOR).arg(ROVERGAUGE_VER_PATCH);

  QApplication a(argc, argv);
  a.setApplicationVersion(versionStr);
  a.setApplicationName("RoverGauge");

  QCommandLineParser parser;

  parser.setApplicationDescription("Diagnostic utility that will interface with the Lucas 14CUX automotive ECU");

  const QCommandLineOption autoconnectOption
      ({"a", "autoconnect"}, "Automatically connect to ECU when starting.");
  const QCommandLineOption autologOption
      ({"l", "autolog"}, "Automatically start logging to a file on startup.");
  const QCommandLineOption fullscreenOption
      ({"f", "fullscreen"}, "Start in fullscreen mode.");
  QCommandLineOption doublebaudOption
      ({"d", "doublebaud"}, "Connect to an ECU that has customized firmware doubling the serial baud rate.");
  doublebaudOption.setFlags(QCommandLineOption::HiddenFromHelp);
  const QCommandLineOption simulatedData
      ({"s", "simulated"}, "Simulate a connection to the ECU. Generally used only for internal RoverGauge testing.");

  parser.addHelpOption();
  parser.addVersionOption();
  parser.addOption(autoconnectOption);
  parser.addOption(autologOption);
  parser.addOption(fullscreenOption);
  parser.addOption(doublebaudOption);
  parser.addOption(simulatedData);

  parser.process(a);

  MainWindow w (parser.isSet(autoconnectOption),
                parser.isSet(autologOption),
                parser.isSet(doublebaudOption),
                parser.isSet(simulatedData));

  if (parser.isSet(fullscreenOption))
  {
    w.showFullScreen();
  }
  else
  {
    w.show();
  }

  return a.exec();
}

