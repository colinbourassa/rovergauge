#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QString>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
  const QString versionStr = QString("%1.%2.%3").arg(ROVERGAUGE_VER_MAJOR).arg(ROVERGAUGE_VER_MINOR).arg(ROVERGAUGE_VER_PATCH);

  // Qt 5 does not scale to high-DPI displays unless asked; Qt 6 always does,
  // and these attributes are deprecated there. macOS handles Retina displays
  // natively in either case, so this mainly affects Windows and Linux.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

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
  QCommandLineOption simulatedData
    ({"s", "simulated"}, "Simulate a connection to the ECU. Generally used only for internal RoverGauge testing.");
  simulatedData.setFlags(QCommandLineOption::HiddenFromHelp);

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

