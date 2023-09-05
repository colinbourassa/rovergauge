#pragma once
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QFile>
#include <QLineEdit>
#include <QTextStream>
#include <QThread>
#include <QFrame>
#include <QTableWidget>
#include <QHash>
#include <QMap>
#include <QPair>
#include <QTimer>
#include <QShortcut>
#include <QGraphicsOpacityEffect>
#include <analogwidgets/manometer.h>
#include <qledindicator/qledindicator.h>
#include "optionsdialog.h"
#include "idleaircontroldialog.h"
#include "cuxinterface.h"
#include "aboutbox.h"
#include "logger.h"
#include "commonunits.h"
#include "helpviewer.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow (bool autoconnect, bool autolog, bool doublebaud, bool simulateConnection, QWidget* parent = 0);
  ~MainWindow();

public slots:
  void onDataReady();
  void onConnect();
  void onDisconnect();
  void onReadError();
  void onReadSuccess();
  void onFailedToConnect(QString dev);
  void onFaultCodesReady();
  void onFaultCodesReadFailed();
  void onFaultCodesClearRequested();
  void onBatteryBackedMemReady();
  void onBatteryBackedMemReadFailed();
  void onFuelMapDataReady(unsigned int fuelMapId);
  void onTuneRevisionReady(int tuneRevisionNum, int checksumFixer, int ident);
  void onRPMLimitReady(int rpmLimit);
  void onRPMTableReady();
  void onROMImageReady();
  void onROMImageReadFailed();
  void onInterfaceReady();
  void onNotConnected();
  void onFeedbackModeChanged(c14cux_feedback_mode mode);
  void onFuelMapIndexChanged(unsigned int fuelMapId);

signals:
  void requestToStartPolling();
  void requestThreadShutdown();

protected:
  void closeEvent(QCloseEvent* event);

private:
  Ui::MainWindow* m_ui;

  QTimer m_fuelPumpRefreshTimer;
  QThread* m_cuxThread;
  CUXInterface* m_cux;
  OptionsDialog* m_options;
  IdleAirControlDialog* m_iacDialog;
  AboutBox* m_aboutBox;
  QMessageBox* m_pleaseWaitBox;
  HelpViewer* m_helpViewerDialog;
  bool m_doubleBaudRate;

  QShortcut m_shortcutStartLogging;
  QShortcut m_shortcutStopLogging;

  Logger* m_logger;

  QGraphicsOpacityEffect* m_waterTempGaugeOpacity;
  QGraphicsOpacityEffect* m_fuelTempGaugeOpacity;
  QGraphicsOpacityEffect* m_speedometerOpacity;
  QGraphicsOpacityEffect* m_revCounterOpacity;
  QGraphicsOpacityEffect* m_fuelMapOpacity;
  QGraphicsOpacityEffect* m_idleModeLedOpacity;
  QGraphicsOpacityEffect* m_fuelPumpLedOpacity;

  QMap<SampleType, bool> m_enabledSamples;

  static const float s_speedometerMaxMPH;
  static const float s_speedometerMaxKPH;

  bool m_fuelMapDataIsCurrent;

  static const QHash<SpeedUnits, QString> s_speedUnitSuffix;
  static const QHash<TemperatureUnits, QString> s_tempUnitSuffix;
  static const QHash<TemperatureUnits, QPair<int, int> > s_tempRange;
  static const QHash<TemperatureUnits, QPair<int, int> > s_tempLimits;

  bool m_isLogging;

  void connectInterfaceSignals();
  void doConnect();
  void startLogging();
  void setupWidgets();
  void populateFuelMapDisplay(const QByteArray *data, unsigned int fuelMapMultiplier, unsigned int rowScaler);
  void sendROMImageRequest(QString prompt);
  void dimUnusedControls();
  void setGearLabel(c14cux_gear gearReading);
  void setLambdaTrimIndicators(int lambdaTrimOdd, int lambdaTrimEven);
  void setLambdaWidgetsForFeedbackMode(c14cux_feedback_mode mode, bool coTrimEnabled, bool lambdaEnabled);
  void setSpeedoLabel();
  void moveFuelMapCellHighlight();

private slots:
  void onSaveROMImageSelected();
  void onROMReadCancelled();
  void onExitSelected();
  void onEditOptionsClicked();
  void onHelpContentsClicked();
  void onHelpAboutClicked();
  void onConnectClicked();
  void onDisconnectClicked();
  void onStartLogging();
  void onStopLogging();
  void onFuelPumpRunTimer();
  void onFuelPumpContinuous();
  void onIdleAirControlClicked();
  void onShowFaultCodesClicked();
  void onBatteryBackedMemClicked();
  void onLambdaTrimButtonClicked(QAbstractButton* button);
  void onMAFReadingButtonClicked(QAbstractButton* button);
  void onThrottleTypeButtonClicked(QAbstractButton* button);
};

