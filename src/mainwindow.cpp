#include <QCloseEvent>
#include <QMessageBox>
#include <QDateTime>
#include <QThread>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QIcon>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "faultcodedialog.h"
#include "batterybackeddisplay.h"

const float MainWindow::s_speedometerMaxMPH = 160.0;
const float MainWindow::s_speedometerMaxKPH = 240.0;

#define ICON_PATH ":/icon/icon/rovergauge_48x48.png"

const QHash<TemperatureUnits, QPair<int,int> > MainWindow::s_tempLimits
{
  { Fahrenheit, { 180, 225 } },
  { Celsius,    {  80, 108 } }
};

const QHash<TemperatureUnits, QPair<int,int> > MainWindow::s_tempRange
{
  { Fahrenheit, { -40, 280 } },
  { Celsius,    { -40, 140 } }
};

const QHash<SpeedUnits, QString> MainWindow::s_speedUnitSuffix
{
  { MPH, " MPH"  },
  { KPH, " km/h" }
};

const QHash<TemperatureUnits, QString> MainWindow::s_tempUnitSuffix
{
  { Fahrenheit, " F" },
  { Celsius,    " C" }
};

/**
 * Constructor; sets up main UI
 */
MainWindow::MainWindow (bool autoconnect,
                        bool autolog,
                        bool doublebaud,
                        bool simulateConnection,
                        QWidget* parent)
  : QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_doubleBaudRate(doublebaud),
    m_shortcutStartLogging(QKeySequence(Qt::Key_F5), this),
    m_shortcutStopLogging(QKeySequence(Qt::Key_F7), this),
    m_fuelMapDataIsCurrent(false),
    m_isLogging(false)
{
  // register this special enum type for use in Qt signals/slots
  qRegisterMetaType<c14cux_feedback_mode>("c14cux_feedback_mode");

  m_ui->setupUi(this);
  this->setWindowTitle("RoverGauge " +
                       QString::number(ROVERGAUGE_VER_MAJOR) + "." +
                       QString::number(ROVERGAUGE_VER_MINOR) + "." +
                       QString::number(ROVERGAUGE_VER_PATCH));

  m_options = new OptionsDialog(this->windowTitle(), this);
  m_cux = new CUXInterface(m_options->getSerialDeviceName(), CUXInterface::getBaudRate(doublebaud),
                           m_options->getSpeedUnits(), m_options->getTemperatureUnits(),
                           m_options->getRefreshFuelMap(), simulateConnection);

  m_enabledSamples = m_options->getEnabledSamples();
  m_cux->setEnabledSamples(m_enabledSamples);
  m_cux->setReadIntervals(m_options->getReadIntervals());

  m_iacDialog = new IdleAirControlDialog(this->windowTitle(), *m_cux, this);
  m_logger = new Logger(*m_cux, *m_options);

  m_fuelPumpRefreshTimer.setInterval(1000);

  connectInterfaceSignals();
  setWindowIcon(QIcon(ICON_PATH));
  setupWidgets();
  dimUnusedControls();

  if (autolog)
  {
    startLogging();
  }
  if (autoconnect)
  {
    doConnect();
  }
}

/**
 * Destructor; cleans up instance of 14CUX communications library
 *  and miscellaneous data storage
 */
MainWindow::~MainWindow()
{
  delete m_aboutBox;
  delete m_options;
  delete m_cux;
  delete m_cuxThread;
}

/**
 * Establish the signals/slots connections between the 14CUX interface class
 * and the other RoverGauge components.
 */
void MainWindow::connectInterfaceSignals()
{
  connect(&m_shortcutStartLogging, &QShortcut::activated, this, &MainWindow::onStartLogging);
  connect(&m_shortcutStopLogging,  &QShortcut::activated, this, &MainWindow::onStopLogging);

  connect(m_cux, &CUXInterface::dataReady,                  this, &MainWindow::onDataReady);
  connect(m_cux, &CUXInterface::connected,                  this, &MainWindow::onConnect);
  connect(m_cux, &CUXInterface::disconnected,               this, &MainWindow::onDisconnect);
  connect(m_cux, &CUXInterface::readError,                  this, &MainWindow::onReadError);
  connect(m_cux, &CUXInterface::readSuccess,                this, &MainWindow::onReadSuccess);
  connect(m_cux, &CUXInterface::failedToConnect,            this, &MainWindow::onFailedToConnect);
  connect(m_cux, &CUXInterface::faultCodesReady,            this, &MainWindow::onFaultCodesReady);
  connect(m_cux, &CUXInterface::faultCodesReadFailed,       this, &MainWindow::onFaultCodesReadFailed);
  connect(m_cux, &CUXInterface::batteryBackedMemReady,      this, &MainWindow::onBatteryBackedMemReady);
  connect(m_cux, &CUXInterface::batteryBackedMemReadFailed, this, &MainWindow::onBatteryBackedMemReadFailed);
  connect(m_cux, &CUXInterface::fuelMapReady,               this, &MainWindow::onFuelMapDataReady);
  connect(m_cux, &CUXInterface::revisionNumberReady,        this, &MainWindow::onTuneRevisionReady);
  connect(m_cux, &CUXInterface::interfaceReadyForPolling,   this, &MainWindow::onInterfaceReady);
  connect(m_cux, &CUXInterface::notConnected,               this, &MainWindow::onNotConnected);
  connect(m_cux, &CUXInterface::romImageReady,              this, &MainWindow::onROMImageReady);
  connect(m_cux, &CUXInterface::romImageReadFailed,         this, &MainWindow::onROMImageReadFailed);
  connect(m_cux, &CUXInterface::rpmLimitReady,              this, &MainWindow::onRPMLimitReady);
  connect(m_cux, &CUXInterface::rpmTableReady,              this, &MainWindow::onRPMTableReady);
  connect(m_cux, &CUXInterface::feedbackModeHasChanged,     this, &MainWindow::onFeedbackModeChanged);
  connect(m_cux, &CUXInterface::fuelMapIndexHasChanged,     this, &MainWindow::onFuelMapIndexChanged);
  connect(&m_fuelPumpRefreshTimer, &QTimer::timeout, this, &MainWindow::onFuelPumpRunTimer);
  connect(this, &MainWindow::requestToStartPolling, m_cux, &CUXInterface::onStartPollingRequest);
  connect(this, &MainWindow::requestThreadShutdown, m_cux, &CUXInterface::onShutdownThreadRequest);
}

/**
 * Instantiates widgets used in the main window.
 */
void MainWindow::setupWidgets()
{
  // set menu and button icons
  m_ui->m_saveROMImageAction->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
  m_ui->m_exitAction->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
  m_ui->m_showFaultCodesAction->setIcon(style()->standardIcon(QStyle::SP_DialogNoButton));
  m_ui->m_editSettingsAction->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
  m_ui->m_helpContentsAction->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));
  m_ui->m_helpAboutAction->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
  m_ui->m_startLoggingButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
  m_ui->m_stopLoggingButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));

  // connect menu item signals
  connect(m_ui->m_saveROMImageAction,   &QAction::triggered, this, &MainWindow::onSaveROMImageSelected);
  connect(m_ui->m_exitAction,           &QAction::triggered, this, &MainWindow::onExitSelected);
  connect(m_ui->m_idleAirControlAction, &QAction::triggered, this, &MainWindow::onIdleAirControlClicked);
  connect(m_ui->m_showFaultCodesAction, &QAction::triggered, this, &MainWindow::onShowFaultCodesClicked);
  connect(m_ui->m_batteryBackedAction,  &QAction::triggered, this, &MainWindow::onBatteryBackedMemClicked);
  connect(m_ui->m_editSettingsAction,   &QAction::triggered, this, &MainWindow::onEditOptionsClicked);
  connect(m_ui->m_helpContentsAction,   &QAction::triggered, this, &MainWindow::onHelpContentsClicked);
  connect(m_ui->m_helpAboutAction,      &QAction::triggered, this, &MainWindow::onHelpAboutClicked);

  // connect button signals
  connect(m_ui->m_connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
  connect(m_ui->m_disconnectButton, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
  connect(m_ui->m_mafReadingButtonGroup, qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked),
          this, &MainWindow::onMAFReadingButtonClicked);
  connect(m_ui->m_throttleTypeButtonGroup, qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked),
          this, &MainWindow::onThrottleTypeButtonClicked);
  connect(m_ui->m_lambdaTrimButtonGroup, qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked),
          this, &MainWindow::onLambdaTrimButtonClicked);
  connect(m_ui->m_fuelPumpContinuousButton, &QPushButton::clicked, this, &MainWindow::onFuelPumpContinuous);
  connect(m_ui->m_startLoggingButton, &QPushButton::clicked, this, &MainWindow::onStartLogging);
  connect(m_ui->m_stopLoggingButton, &QPushButton::clicked, this, &MainWindow::onStopLogging);

  // set the LED colors
  m_ui->m_milLed->setOnColor1(QColor(255, 0, 0));
  m_ui->m_milLed->setOnColor2(QColor(176, 0, 2));
  m_ui->m_milLed->setOffColor1(QColor(20, 0, 0));
  m_ui->m_milLed->setOffColor2(QColor(90, 0, 2));
  m_ui->m_milLed->setDisabled(true);
  m_ui->m_commsGoodLed->setOnColor1(QColor(102, 255, 102));
  m_ui->m_commsGoodLed->setOnColor2(QColor(82, 204, 82));
  m_ui->m_commsGoodLed->setOffColor1(QColor(0, 102, 0));
  m_ui->m_commsGoodLed->setOffColor2(QColor(0, 51, 0));
  m_ui->m_commsGoodLed->setDisabled(true);
  m_ui->m_commsBadLed->setOnColor1(QColor(255, 0, 0));
  m_ui->m_commsBadLed->setOnColor2(QColor(176, 0, 2));
  m_ui->m_commsBadLed->setOffColor1(QColor(20, 0, 0));
  m_ui->m_commsBadLed->setOffColor2(QColor(90, 0, 2));
  m_ui->m_commsBadLed->setDisabled(true);
  m_ui->m_idleModeLed->setOnColor1(QColor(102, 255, 102));
  m_ui->m_idleModeLed->setOnColor2(QColor(82, 204, 82));
  m_ui->m_idleModeLed->setOffColor1(QColor(0, 102, 0));
  m_ui->m_idleModeLed->setOffColor2(QColor(0, 51, 0));
  m_ui->m_idleModeLed->setDisabled(true);
  m_ui->m_fuelPumpRelayStateLed->setOnColor1(QColor(102, 255, 102));
  m_ui->m_fuelPumpRelayStateLed->setOnColor2(QColor(82, 204, 82));
  m_ui->m_fuelPumpRelayStateLed->setOffColor1(QColor(0, 102, 0));
  m_ui->m_fuelPumpRelayStateLed->setOffColor2(QColor(0, 51, 0));
  m_ui->m_fuelPumpRelayStateLed->setDisabled(true);

  // TODO: can this be done in the FuelMapGrid class?
  // If it needs to stay here, should the 'QTableWidget'
  // change to 'FuelMapGrid'?
  setStyleSheet("QTableWidget {background-color: transparent;}");
  m_ui->m_fuelMapDisplay->setup(m_options->getDisplayNumberBase());
  m_ui->m_logFileNameBox->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss"));
  m_ui->m_injectorDutyCycleBar->setAlignment(Qt::AlignCenter);

  const SpeedUnits speedUnit = m_options->getSpeedUnits();
  m_ui->m_speedo->setMinimum(0.0);
  m_ui->m_speedo->setMaximum((speedUnit == MPH) ? s_speedometerMaxMPH : s_speedometerMaxKPH);
  setSpeedoLabel();
  m_ui->m_speedo->setSuffix(s_speedUnitSuffix.value(speedUnit));
  m_ui->m_speedo->setNominal(1000.0);
  m_ui->m_speedo->setCritical(1000.0);

  m_ui->m_revCounter->setMinimum(0.0);
  m_ui->m_revCounter->setMaximum(8000);
  m_ui->m_revCounter->setSuffix(" RPM");
  m_ui->m_revCounter->setNominal(100000.0);
  m_ui->m_revCounter->setCritical(8000);

  const TemperatureUnits tempUnits = m_options->getTemperatureUnits();
  const int tempMin = s_tempRange.value(tempUnits).first;
  const int tempMax = s_tempRange.value(tempUnits).second;

  m_ui->m_waterTempGauge->setValue(tempMin);
  m_ui->m_waterTempGauge->setMaximum(tempMax);
  m_ui->m_waterTempGauge->setMinimum(tempMin);
  m_ui->m_waterTempGauge->setSuffix(s_tempUnitSuffix.value(tempUnits));
  m_ui->m_waterTempGauge->setNominal(s_tempLimits.value(tempUnits).first);
  m_ui->m_waterTempGauge->setCritical(s_tempLimits.value(tempUnits).second);

  m_ui->m_fuelTempGauge->setValue(tempMin);
  m_ui->m_fuelTempGauge->setMaximum(tempMax);
  m_ui->m_fuelTempGauge->setMinimum(tempMin);
  m_ui->m_fuelTempGauge->setSuffix(s_tempUnitSuffix.value(tempUnits));
  m_ui->m_fuelTempGauge->setNominal(10000.0);
  m_ui->m_fuelTempGauge->setCritical(10000.0);

  m_waterTempGaugeOpacity = new QGraphicsOpacityEffect(this);
  m_waterTempGaugeOpacity->setOpacity(0.5);
  m_waterTempGaugeOpacity->setEnabled(false);
  m_ui->m_waterTempGauge->setGraphicsEffect(m_waterTempGaugeOpacity);

  m_fuelTempGaugeOpacity = new QGraphicsOpacityEffect(this);
  m_fuelTempGaugeOpacity->setOpacity(0.5);
  m_fuelTempGaugeOpacity->setEnabled(false);
  m_ui->m_fuelTempGauge->setGraphicsEffect(m_fuelTempGaugeOpacity);

  m_speedometerOpacity = new QGraphicsOpacityEffect(this);
  m_speedometerOpacity->setOpacity(0.5);
  m_speedometerOpacity->setEnabled(false);
  m_ui->m_speedo->setGraphicsEffect(m_speedometerOpacity);

  m_revCounterOpacity = new QGraphicsOpacityEffect(this);
  m_revCounterOpacity->setOpacity(0.5);
  m_revCounterOpacity->setEnabled(false);
  m_ui->m_revCounter->setGraphicsEffect(m_revCounterOpacity);

  m_fuelMapOpacity = new QGraphicsOpacityEffect(this);
  m_fuelMapOpacity->setOpacity(0.5);
  m_fuelMapOpacity->setEnabled(false);
  m_ui->m_fuelMapDisplay->setGraphicsEffect(m_fuelMapOpacity);

  m_fuelPumpLedOpacity = new QGraphicsOpacityEffect(this);
  m_fuelPumpLedOpacity->setOpacity(0.5);
  m_fuelPumpLedOpacity->setEnabled(false);
  m_ui->m_fuelPumpRelayStateLed->setGraphicsEffect(m_fuelPumpLedOpacity);

  m_idleModeLedOpacity = new QGraphicsOpacityEffect(this);
  m_idleModeLedOpacity->setOpacity(0.5);
  m_idleModeLedOpacity->setEnabled(false);
  m_ui->m_idleModeLed->setGraphicsEffect(m_idleModeLedOpacity);
}

/**
 * Handler for the buttonclick on the Connect button
 */
void MainWindow::onConnectClicked()
{
  doConnect();
}

/**
 * Attempts to open the serial device connected to the 14CUX,
 * and starts updating the display with data if successful.
 */
void MainWindow::doConnect()
{
  // If the worker thread hasn't been created yet, do that now.
  if (m_cuxThread == nullptr)
  {
    m_cuxThread = new QThread(this);
    m_cux->moveToThread(m_cuxThread);
    connect(m_cuxThread, &QThread::started, m_cux, &CUXInterface::onParentThreadStarted);
  }

  // If the worker thread is already running, ask it to start polling the ECU.
  // Otherwise, start the worker thread, but don't ask it to begin polling
  // yet; it'll signal us when it's ready.
  if (m_cuxThread->isRunning())
  {
    emit requestToStartPolling();
  }
  else
  {
    m_cuxThread->start();
  }
}

/**
 * Responds to the signal from the worker thread which indicates that it's
 * ready to start polling the ECU. This routine emits the "start polling"
 * command signal.
 */
void MainWindow::onInterfaceReady()
{
  emit requestToStartPolling();
}

/**
 * Sets a flag in the worker thread that tells it to disconnect from the
 * serial device.
 */
void MainWindow::onDisconnectClicked()
{
  m_ui->m_disconnectButton->setEnabled(false);
  m_cux->disconnectFromECU();
  m_logger->onDisconnect();
}

/**
 * Closes the main window and terminates the application.
 */
void MainWindow::onExitSelected()
{
  this->close();
}

/**
 * Opens the fault-code dialog.
 */
void MainWindow::onFaultCodesReady()
{
  c14cux_faultcodes faultCodes = m_cux->getFaultCodes();
  FaultCodeDialog faultDialog(this->windowTitle(), faultCodes);
  connect(&faultDialog, &FaultCodeDialog::clearFaultCodes, this, &MainWindow::onFaultCodesClearRequested);
  connect(m_cux, &CUXInterface::faultCodesClearSuccess,
          &faultDialog, &FaultCodeDialog::onFaultClearSuccess);
  connect(m_cux, &CUXInterface::faultCodesClearFailure, &faultDialog, &FaultCodeDialog::onFaultClearFailure);
  faultDialog.exec();
}

void MainWindow::onFaultCodesClearRequested()
{
  m_cux->enqueueRequest(QueueableRequest_ClearFaultCodes);
}

/**
 * Responds to a signal from the worker thread that indicates there was a
 * problem reading the fault codes. Displays a message box indicating the same.
 */
void MainWindow::onFaultCodesReadFailed()
{
  QMessageBox::warning(this, "Error", "Unable to read fault codes from ECU.", QMessageBox::Ok);
}

/**
 * Opens the battery-backed memory dialog.
 */
void MainWindow::onBatteryBackedMemReady()
{
  BatteryBackedDisplay batteryDialog(this->windowTitle(), m_cux->getBatteryBackedMem(), 0x0040, m_options->getRAMLabels(), this);
  batteryDialog.exec();
}

/**
 * Responds to a signal from the worker thread that indicates there was a
 * problem reading the battery-backed memory. Displays a message box indicating the same.
 */
void MainWindow::onBatteryBackedMemReadFailed()
{
  QMessageBox::warning(this, "Error", "Unable to read battery-backed memory from ECU.", QMessageBox::Ok);
}

/**
 * Moves the highlight for the active cell in the fuel map display, based on the
 * most recent active row/column index data.
 */
void MainWindow::moveFuelMapCellHighlight()
{
  m_ui->m_fuelMapDisplay->moveCellHighlight(
    m_cux->getFuelMapRowIndex(),
    m_cux->getFuelMapRowWeighting(),
    m_cux->getFuelMapColumnIndex(),
    m_cux->getFuelMapColumnWeighting(),
    m_options->getSoftHighlight());
}

/**
 * Uses a fuel map array to populate a 16x8 grid that shows all the fueling
 * values.
 * @param data Pointer to the ByteArray that contains the map data
 */
void MainWindow::populateFuelMapDisplay(const QByteArray* data, unsigned int fuelMapMultiplier, unsigned int rowScaler)
{
  if (data)
  {
    m_ui->m_fuelMapDisplay->setData(*data);
  }

  moveFuelMapCellHighlight();

  if (m_options->getDisplayNumberBase() == 16)
  {
    m_ui->m_fuelMapFactorLabel->setText(
      QString("Multiplier: 0x%1").arg(QString("%1").arg(fuelMapMultiplier, 0, 16).toUpper()));
    m_ui->m_rowScalerLabel->setText(
      QString("Row scaler: 0x%1").arg(QString("%1").arg(rowScaler, 0, 16).toUpper()));
  }
  else if (m_options->getDisplayNumberBase() == 10)
  {
    m_ui->m_fuelMapFactorLabel->setText(QString("Multiplier: %1").arg(fuelMapMultiplier));
    m_ui->m_rowScalerLabel->setText(QString("Row scaler: %1").arg(rowScaler));
  }
}

/**
 * Uses fuel map data to populate the fuel map display grid.
 * @param ID of the fuel map just retrieved (from 1-5)
 */
void MainWindow::onFuelMapDataReady(unsigned int fuelMapId)
{
  const QByteArray* data = m_cux->getFuelMap(fuelMapId);

  if (data)
  {
    populateFuelMapDisplay(data,
                           m_cux->getFuelMapAdjustmentFactor(fuelMapId),
                           m_cux->getRowScaler(fuelMapId));
    m_fuelMapDataIsCurrent = true;

    m_logger->onFuelMapDataReady(fuelMapId);
  }
}

/**
 * Updates the gauges and indicators with the latest data available from
 * the ECU.
 */
void MainWindow::onDataReady()
{
  int rpm = 0;
  float pulseWidth = 0;

  if (!m_requestedTuneID)
  {
    m_cux->enqueueRequest(QueueableRequest_TuneRevID);
    m_requestedTuneID = true;
  }

  m_ui->m_milLed->setChecked(m_cux->isMILOn());

  // if fuel map display updates are enabled...
  if (m_enabledSamples[SampleType_FuelMapRowCol] && m_fuelMapDataIsCurrent)
  {
    moveFuelMapCellHighlight();
  }

  if (m_enabledSamples[SampleType_Throttle])
  {
    m_ui->m_throttleBar->setValue(m_cux->getThrottlePos() * 100);
  }

  if (m_enabledSamples[SampleType_MAF])
  {
    m_ui->m_mafReadingBar->setValue(m_cux->getMAFReading() * 100);
  }

  if (m_enabledSamples[SampleType_IdleBypassPosition])
  {
    m_ui->m_idleBypassPosBar->setValue(m_cux->getIdleBypassPos() * 100);
  }

  if (m_enabledSamples[SampleType_RoadSpeed])
  {
    if (m_options->getSpeedoAdjust())
    {
      const int adjustedSpeed =
        (m_cux->getRoadSpeed() * m_options->getSpeedoMultiplier()) + m_options->getSpeedoOffset();
      m_ui->m_speedo->setValue(adjustedSpeed);
    }
    else
    {
      m_ui->m_speedo->setValue((int)m_cux->getRoadSpeed());
    }
  }

  if (m_enabledSamples[SampleType_EngineRPM])
  {
    rpm = m_cux->getEngineSpeedRPM();
    m_ui->m_revCounter->setValue(rpm);
  }

  if (m_enabledSamples[SampleType_EngineTemperature])
  {
    m_ui->m_waterTempGauge->setValue(m_cux->getCoolantTemp());
  }

  if (m_enabledSamples[SampleType_FuelTemperature])
  {
    m_ui->m_fuelTempGauge->setValue(m_cux->getFuelTemp());
  }

  if (m_enabledSamples[SampleType_MainVoltage])
  {
    m_ui->m_voltage->setText(QString::number(m_cux->getMainVoltage(), 'f', 1) + "V");
  }

  if (m_enabledSamples[SampleType_FuelPumpRelay])
  {
    m_ui->m_fuelPumpRelayStateLed->setChecked(m_cux->getFuelPumpRelayState());
  }

  if (m_enabledSamples[SampleType_InjectorPulseWidth])
  {
    pulseWidth = m_cux->getInjectorPulseWidthMs();

    // if we're also monitoring the engine speed, we can compute the injector duty
    // cycle as a percentage of the time available between spark interrupts
    if (rpm > 0)
    {
      m_ui->m_injectorDutyCycleBar->setValue((pulseWidth / (60.0 / (float)rpm * 1000.0)) * 100);
    }

    m_ui->m_injectorPulseWidthLabel->setText(QString("Pulse width: %1 ms").arg(pulseWidth, 0, 'f', 2));
  }

  if (m_enabledSamples[SampleType_TargetIdleRPM])
  {
    const int targetIdleSpeedRPM = m_cux->getTargetIdleSpeed();
    m_ui->m_targetIdle->setText((targetIdleSpeedRPM > 0) ? QString::number(targetIdleSpeedRPM) : "");
    m_ui->m_idleModeLed->setChecked(m_cux->getIdleMode());
  }

  if ((m_enabledSamples[SampleType_LambdaTrimShort] || m_enabledSamples[SampleType_LambdaTrimLong]) &&
      (m_cux->getFeedbackMode() == C14CUX_FeedbackMode_ClosedLoop))
  {
    setLambdaTrimIndicators(m_cux->getLambdaTrimOdd(), m_cux->getLambdaTrimEven());
  }

  if (m_enabledSamples[SampleType_COTrimVoltage] && (m_cux->getFeedbackMode() == C14CUX_FeedbackMode_OpenLoop))
  {
    m_ui->m_oddFuelTrimBarAndMAFCOLabel->setText(QString::number(m_cux->getCOTrimVoltage(), 'f', 2) + "V");
  }

  if (m_enabledSamples[SampleType_GearSelection])
  {
    setGearLabel(m_cux->getGear());
  }

  m_logger->logData();
}

/**
 * Sets the lambda fuel trim indicators to the provided values
 */
void MainWindow::setLambdaTrimIndicators(int lambdaTrimOdd, int lambdaTrimEven)
{
  const QString oddLabel = (lambdaTrimOdd >= 0) ?
                           QString("+%1%").arg(lambdaTrimOdd * 100 / m_ui->m_oddFuelTrimBar->maximum()) :
                           QString("-%1%").arg(lambdaTrimOdd * 100 / m_ui->m_oddFuelTrimBar->minimum());
  const QString evenLabel = (lambdaTrimEven >= 0) ?
                            QString("+%1%").arg(lambdaTrimEven * 100 / m_ui->m_evenFuelTrimBar->maximum()) :
                            QString("-%1%").arg(lambdaTrimEven * 100 / m_ui->m_evenFuelTrimBar->minimum());

  m_ui->m_oddFuelTrimBar->setValue(lambdaTrimOdd);
  m_ui->m_evenFuelTrimBar->setValue(lambdaTrimEven);

  m_ui->m_oddFuelTrimBarAndMAFCOLabel->setText(oddLabel);
  m_ui->m_evenFuelTrimBarLabel->setText(evenLabel);
}

/**
 * Sets the label indicating the current gear selection
 */
void MainWindow::setGearLabel(c14cux_gear gearReading)
{
  switch (gearReading)
  {
  case C14CUX_Gear_ParkOrNeutral:
    m_ui->m_gear->setText("Park/Neutral");
    break;

  case C14CUX_Gear_DriveOrReverse:
    m_ui->m_gear->setText("Drive/Reverse");
    break;

  case C14CUX_Gear_ManualGearbox:
    m_ui->m_gear->setText("(Manual gearbox)");
    break;

  case C14CUX_Gear_NoReading:
  default:
    m_ui->m_gear->setText("(no reading)");
    break;
  }
}

/**
 * Opens the settings dialog, where the user may change the
 * serial device, timer interval, gauge settings, and
 * correction factors.
 */
void MainWindow::onEditOptionsClicked()
{
  // if the user doesn't cancel the options dialog...
  if (m_options->exec() == QDialog::Accepted)
  {
    // update the speedo appropriately
    const SpeedUnits speedUnit = m_options->getSpeedUnits();
    m_ui->m_speedo->setMaximum((speedUnit == MPH) ? s_speedometerMaxMPH : s_speedometerMaxKPH);
    m_ui->m_speedo->setSuffix(s_speedUnitSuffix.value(speedUnit));
    m_ui->m_speedo->repaint();

    setSpeedoLabel();

    const TemperatureUnits tempUnits = m_options->getTemperatureUnits();
    const QString tempUnitStr = s_tempUnitSuffix.value(tempUnits);

    const int tempMin = s_tempRange.value(tempUnits).first;
    const int tempMax = s_tempRange.value(tempUnits).second;
    const int tempNominal = s_tempLimits.value(tempUnits).first;
    const int tempCritical = s_tempLimits.value(tempUnits).second;

    m_ui->m_fuelTempGauge->setSuffix(tempUnitStr);
    m_ui->m_fuelTempGauge->setValue(tempMin);
    m_ui->m_fuelTempGauge->setMaximum(tempMax);
    m_ui->m_fuelTempGauge->setMinimum(tempMin);
    m_ui->m_fuelTempGauge->repaint();

    m_ui->m_waterTempGauge->setSuffix(tempUnitStr);
    m_ui->m_waterTempGauge->setValue(tempMin);
    m_ui->m_waterTempGauge->setMaximum(tempMax);
    m_ui->m_waterTempGauge->setMinimum(tempMin);
    m_ui->m_waterTempGauge->setNominal(tempNominal);
    m_ui->m_waterTempGauge->setCritical(tempCritical);
    m_ui->m_waterTempGauge->repaint();

    m_cux->setSpeedUnits(speedUnit);
    m_cux->setTemperatureUnits(tempUnits);
    m_cux->setPeriodicFuelMapRefresh(m_options->getRefreshFuelMap());

    // The fields are updated one at a time, because a replacement of the entire
    // hash table (using the assignment operator) can disrupt other threads that
    // are reading the table at that time
    const QMap<SampleType, bool> samples = m_options->getEnabledSamples();
    foreach(SampleType field, samples.keys())
    {
      if (samples.keys().contains(field))
      {
        m_enabledSamples[field] = samples[field];
      }
    }

    dimUnusedControls();

    m_cux->setEnabledSamples(m_enabledSamples);
    m_cux->setReadIntervals(m_options->getReadIntervals());

    // If the user changed the serial device name and/or the polling
    // interval, stop the timer, re-connect to the 14CUX (if neccessary),
    // and restart the timer
    if (m_options->getSerialDeviceChanged())
    {
      if (m_cux->isConnected())
      {
        m_cux->disconnectFromECU();
      }

      m_cux->setSerialDevice(m_options->getSerialDeviceName());
      m_cux->setBaudRate(CUXInterface::getBaudRate(m_doubleBaudRate));
    }

    if (m_options->getDisplayNumberBaseChanged())
    {
      m_ui->m_fuelMapDisplay->setNumberBase(m_options->getDisplayNumberBase());
      const int fuelMapId = m_cux->getCurrentFuelMapIndex();
      const QByteArray* fuelMapData = m_cux->getFuelMap(fuelMapId);
      populateFuelMapDisplay(fuelMapData,
                             m_cux->getFuelMapAdjustmentFactor(fuelMapId),
                             m_cux->getRowScaler(fuelMapId));
    }
  }
}

/**
 * Sets the speedometer label to display either the default description,
 * or the description that identifies the data as being modified
 * (depending on the value of the checkbox in the Options)
 */
void MainWindow::setSpeedoLabel()
{
  m_ui->m_speedoLabel->setText(
    m_options->getSpeedoAdjust() ? "<b>Adjusted road speed</b>" : "Road speed");
}

/**
 * Dims/grays controls for fields that are disabled, and activates all other controls
 */
void MainWindow::dimUnusedControls()
{
  bool enabled = m_enabledSamples[SampleType_MAF];
  m_ui->m_mafReadingLabel->setEnabled(enabled);
  m_ui->m_mafReadingBar->setEnabled(enabled);
  m_ui->m_mafReadingTypeLabel->setEnabled(enabled);
  m_ui->m_mafReadingDirectButton->setEnabled(enabled);
  m_ui->m_mafReadingLinearButton->setEnabled(enabled);

  if (!enabled)
  {
    m_ui->m_mafReadingBar->setValue(0);
  }

  enabled = m_enabledSamples[SampleType_Throttle];
  m_ui->m_throttleLabel->setEnabled(enabled);
  m_ui->m_throttleBar->setEnabled(enabled);
  m_ui->m_throttleTypeLabel->setEnabled(enabled);
  m_ui->m_throttleTypeAbsoluteButton->setEnabled(enabled);
  m_ui->m_throttleTypeCorrectedButton->setEnabled(enabled);

  if (!enabled)
  {
    m_ui->m_throttleBar->setValue(0);
  }

  enabled = m_enabledSamples[SampleType_IdleBypassPosition];
  m_ui->m_idleBypassLabel->setEnabled(enabled);
  m_ui->m_idleBypassPosBar->setEnabled(enabled);

  if (!enabled)
  {
    m_ui->m_idleBypassPosBar->setValue(0);
  }

  enabled = m_enabledSamples[SampleType_GearSelection];
  m_ui->m_gearLabel->setEnabled(enabled);
  m_ui->m_gear->setEnabled(enabled);

  enabled = m_enabledSamples[SampleType_MainVoltage];
  m_ui->m_voltageLabel->setEnabled(enabled);
  m_ui->m_voltage->setEnabled(enabled);

  enabled = m_enabledSamples[SampleType_TargetIdleRPM];
  m_ui->m_targetIdleLabel->setEnabled(enabled);
  m_ui->m_targetIdle->setEnabled(enabled);
  m_idleModeLedOpacity->setEnabled(!enabled);

  setLambdaWidgetsForFeedbackMode(
    m_cux->getFeedbackMode(),
    m_enabledSamples[SampleType_COTrimVoltage],
    m_enabledSamples[SampleType_LambdaTrimShort] || m_enabledSamples[SampleType_LambdaTrimLong]);

  enabled = m_enabledSamples[SampleType_FuelPumpRelay];
  m_ui->m_fuelPumpRelayStateLabel->setEnabled(enabled);
  m_ui->m_fuelPumpRelayStateLed->setEnabled(enabled);
  m_fuelPumpLedOpacity->setEnabled(!enabled);

  m_ui->m_fuelMapIndexLabel->setEnabled(m_enabledSamples[SampleType_FuelMapIndex]);

  enabled = (m_enabledSamples[SampleType_FuelMapData] && m_enabledSamples[SampleType_FuelMapRowCol]);
  m_ui->m_fuelMapFactorLabel->setEnabled(enabled);
  m_fuelMapOpacity->setEnabled(!enabled);

  enabled = m_enabledSamples[SampleType_InjectorPulseWidth];
  m_ui->m_injectorPulseWidthLabel->setEnabled(enabled);

  // These controls are shown in a disabled state by applying a 50% opacity
  // graphical effect; the 'enabled' bit is therefore inverted because it's
  // controlling the state of the graphical effect (rather than the widget).
  enabled = m_enabledSamples[SampleType_EngineTemperature];
  m_waterTempGaugeOpacity->setEnabled(!enabled);
  m_ui->m_waterTempLabel->setEnabled(enabled);
  enabled = m_enabledSamples[SampleType_FuelTemperature];
  m_fuelTempGaugeOpacity->setEnabled(!enabled);
  m_ui->m_fuelTempLabel->setEnabled(enabled);

  enabled = m_enabledSamples[SampleType_EngineRPM];
  m_revCounterOpacity->setEnabled(!enabled);

  // Computing injector duty cycle requires RPM data
  enabled = (m_enabledSamples[SampleType_InjectorPulseWidth] &&
             m_enabledSamples[SampleType_EngineRPM]);
  m_ui->m_injectorDutyCycleBar->setEnabled(enabled);
  m_ui->m_injectorDutyCycleLabel->setEnabled(enabled);

  if (!enabled)
  {
    m_ui->m_injectorDutyCycleBar->setValue(0);
  }

  m_speedometerOpacity->setEnabled(!m_enabledSamples[SampleType_RoadSpeed]);
}

/**
 * Responds to a 'close' event on the main window by first shutting down
 * the other thread.
 * @param event The event itself.
 */
void MainWindow::closeEvent(QCloseEvent* event)
{
  m_logger->closeLog();

  if (m_cuxThread && m_cuxThread->isRunning())
  {
    emit requestThreadShutdown();
    m_cuxThread->wait(2000);
  }

  event->accept();
}

/**
 * Reponds to the "connect" signal from the CUXInterface by enabling/disabling
 * the appropriate buttons and setting a message in the status bar.
 */
void MainWindow::onConnect()
{
  m_ui->m_connectButton->setEnabled(false);
  m_ui->m_disconnectButton->setEnabled(true);
  m_ui->m_commsGoodLed->setChecked(false);
  m_ui->m_commsBadLed->setChecked(false);
  m_ui->m_fuelPumpOneshotButton->setEnabled(true);
  m_ui->m_fuelPumpContinuousButton->setEnabled(true);
}

/**
 * Reponds to the "disconnect" signal from the CUXInterface by enabling/disabling
 * the appropriate buttons and setting a message in the status bar.
 */
void MainWindow::onDisconnect()
{
  m_ui->m_connectButton->setEnabled(true);
  m_ui->m_disconnectButton->setEnabled(false);
  m_ui->m_milLed->setChecked(false);
  m_ui->m_commsGoodLed->setChecked(false);
  m_ui->m_commsBadLed->setChecked(false);
  m_ui->m_fuelPumpOneshotButton->setEnabled(false);
  m_ui->m_fuelPumpContinuousButton->setEnabled(false);
  m_ui->m_tuneRevNumberLabel->setText("Tune:");
  m_ui->m_identLabel->setText("Ident:");
  m_ui->m_checksumFixerLabel->setText("Checksum fixer:");
  m_ui->m_fuelMapIndexLabel->setText("Current fuel map:");
  m_ui->m_fuelMapFactorLabel->setText("Multiplier:");
  m_ui->m_rpmLimitLabel->setText("RPM limit:");
  m_ui->m_rowScalerLabel->setText("Row scaler:");

  m_ui->m_speedo->setValue(0.0);
  m_ui->m_revCounter->setValue(0.0);
  m_ui->m_waterTempGauge->setValue(m_ui->m_waterTempGauge->minimum());
  m_ui->m_fuelTempGauge->setValue(m_ui->m_fuelTempGauge->minimum());
  m_ui->m_throttleBar->setValue(0);
  m_ui->m_mafReadingBar->setValue(0);
  m_ui->m_idleBypassPosBar->setValue(0);
  m_ui->m_idleModeLed->setChecked(false);
  m_ui->m_targetIdle->setText("--");
  m_ui->m_voltage->setText("--");
  m_ui->m_gear->setText("--");
  m_ui->m_fuelPumpRelayStateLed->setChecked(false);
  m_ui->m_oddFuelTrimBar->setValue(0);

  m_ui->m_oddFuelTrimBarAndMAFCOLabel->setText(
    (m_cux->getFeedbackMode() == C14CUX_FeedbackMode_ClosedLoop) ? "+0%" : "");

  m_ui->m_evenFuelTrimBar->setValue(0);
  m_ui->m_evenFuelTrimBarLabel->setText("+0%");
  m_ui->m_injectorDutyCycleBar->setValue(0);
  m_ui->m_injectorPulseWidthLabel->setText("Pulse width:");

  m_ui->m_oddFuelTrimBar->repaint();
  m_ui->m_evenFuelTrimBar->repaint();

  m_ui->m_fuelMapDisplay->clearCellHighlight();

  m_fuelMapDataIsCurrent = false;
  m_cux->invalidateFuelMapData();
  m_requestedTuneID = false;
}

/**
 * Responds to the "read error" signal from the worker thread by turning
 * on a red lamp.
 */
void MainWindow::onReadError()
{
  m_ui->m_commsGoodLed->setChecked(false);
  m_ui->m_commsBadLed->setChecked(true);
}

/**
 * Responds to the "read success" signal from the worker thread by turning
 * on a green lamp.
 */
void MainWindow::onReadSuccess()
{
  m_ui->m_commsGoodLed->setChecked(true);
  m_ui->m_commsBadLed->setChecked(false);
}

/**
 * Opens the log file for writing.
 */
void MainWindow::startLogging()
{
  if (!m_isLogging)
  {
    if (m_logger->openLog(m_ui->m_logFileNameBox->text()))
    {
      m_isLogging = true;
      m_ui->m_logFileNameBox->setEnabled(false);
      m_ui->m_startLoggingButton->setEnabled(false);
      m_ui->m_stopLoggingButton->setEnabled(true);
    }
    else
    {
      QMessageBox::warning(
        this, "Error", "Failed to open log file (" + m_logger->getLogPath() + ")", QMessageBox::Ok);
    }
  }
}

/**
 * Opens the log file for writing.
 */
void MainWindow::onStartLogging()
{
  startLogging();
}

/**
 * Closes the open log file.
 */
void MainWindow::onStopLogging()
{
  m_isLogging = false;
  m_logger->closeLog();
  m_ui->m_logFileNameBox->setEnabled(true);
  m_ui->m_stopLoggingButton->setEnabled(false);
  m_ui->m_startLoggingButton->setEnabled(true);
}

/**
 * Displays an dialog box with information about the program.
 */
void MainWindow::onHelpAboutClicked()
{
  if (m_aboutBox == nullptr)
  {
    m_aboutBox = new AboutBox(style(), this->windowTitle(), m_cux->getVersion(), this);
  }
  m_aboutBox->exec();
}

/**
 * Displays the online help.
 */
void MainWindow::onHelpContentsClicked()
{
  if (m_helpViewerDialog == nullptr)
  {
    m_helpViewerDialog = new HelpViewer(this->windowTitle(), this);
  }
  m_helpViewerDialog->show();
}

/**
 * Displays a message box indicating that an error in connecting to the
 * serial device.
 * @param Name of the serial device that the software attempted to open
 */
void MainWindow::onFailedToConnect(QString dev)
{
  if (dev.isEmpty() || dev.isNull())
  {
    QMessageBox::warning(this, "Error",
                         QString("Error connecting to 14CUX. No serial port name specified.\n\n") +
                         QString("Set a serial device using \"Options\" --> \"Edit Settings\""),
                         QMessageBox::Ok);
  }
  else
  {
    QMessageBox::warning(this, "Error",
                         "Error connecting to 14CUX. Could not open serial device: " + dev,
                         QMessageBox::Ok);
  }
}

/**
 * Displays a message box indicating that an action cannot be completed due
 * to the software not being connected to the ECU.
 */
void MainWindow::onNotConnected()
{
  if (m_pleaseWaitBox)
  {
    m_pleaseWaitBox->hide();
  }

  QMessageBox::warning(
    this, "Error", "This requires that the software first be connected to the ECU (using the \"Connect\" button.)",
    QMessageBox::Ok);
}

/**
 * Requests the ROM image so that it can be saved to disk.
 */
void MainWindow::onSaveROMImageSelected()
{
  sendROMImageRequest(
    QString("Read the ROM image from the ECU? This will take approximately 25 seconds."));
}

/**
 * Prompts the user to continue, and sends a request to read the ROM image.
 * @param prompt String used to prompt the user to continue.
 */
void MainWindow::sendROMImageRequest(QString prompt)
{
  if (m_cux->isConnected())
  {
    if (QMessageBox::question(this, "Confirm", prompt,
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
      if (m_pleaseWaitBox == nullptr)
      {
        m_pleaseWaitBox = new QMessageBox(
          QMessageBox::Information, "In Progress",
          QString("Please wait while the ROM image is read.\n\n"),
          QMessageBox::Ok, this, Qt::Dialog);
        m_pleaseWaitBox->setStandardButtons(QMessageBox::Cancel);
        connect(m_pleaseWaitBox, &QMessageBox::finished, this, &MainWindow::onROMReadCancelled);
      }

      m_pleaseWaitBox->show();
      m_cux->enqueueRequest(QueueableRequest_ROMImage);
    }
  }
  else
  {
    onNotConnected();
  }
}

/**
 * Sets a flag that indicates we should ignore any ROM image that is returned.
 */
void MainWindow::onROMReadCancelled()
{
  m_cux->cancelRead();
}

/**
 * Prompts the user for a file in which to save the ROM image.
 */
void MainWindow::onROMImageReady()
{
  if (m_pleaseWaitBox)
  {
    m_pleaseWaitBox->hide();
  }

  const QByteArray& promData = m_cux->getROMImage();

  const QString saveFileName =
    QFileDialog::getSaveFileName(this, "Select output file for ROM image:");

  if (!saveFileName.isNull() && !saveFileName.isEmpty())
  {
    QFile saveFile(saveFileName);

    if (saveFile.open(QIODevice::WriteOnly))
    {
      if (saveFile.write(promData) != promData.capacity())
      {
        QMessageBox::warning(
          this, "Error", QString("Error writing the ROM image file:\n%1").arg(saveFileName), QMessageBox::Ok);
      }

      saveFile.close();
    }
    else
    {
      QMessageBox::warning(
        this, "Error", QString("Error writing the ROM image file:\n%1").arg(saveFileName), QMessageBox::Ok);
    }
  }
}

/**
 * Displays a message box indicating that reading the ROM image has failed.
 */
void MainWindow::onROMImageReadFailed()
{
  if (m_pleaseWaitBox)
  {
    m_pleaseWaitBox->hide();
  }

  QMessageBox::warning(
    this, "Error", "Communications error. ROM image could not be read.", QMessageBox::Ok);
}

/**
 * Starts a timer that periodically re-sends the signal to run the fuel
 * pump, thus keeping the pump running continuously.
 */
void MainWindow::onFuelPumpContinuous()
{
  if (m_ui->m_fuelPumpContinuousButton->isChecked())
  {
    m_cux->enqueueRequest(QueueableRequest_FuelPumpRun);
    m_fuelPumpRefreshTimer.start();
    m_ui->m_fuelPumpOneshotButton->setEnabled(false);
  }
  else
  {
    m_fuelPumpRefreshTimer.stop();
    m_ui->m_fuelPumpOneshotButton->setEnabled(true);
  }
}

/**
 * Responds to the fuel pump run timer by re-sending the command to run
 * the fuel pump.
 */
void MainWindow::onFuelPumpRunTimer()
{
  m_cux->enqueueRequest(QueueableRequest_FuelPumpRun);
}

/**
 * Displays the idle-air-control dialog.
 */
void MainWindow::onIdleAirControlClicked()
{
  m_iacDialog->show();
}

/**
 * Queues a request to read the fault codes.
 */
void MainWindow::onShowFaultCodesClicked()
{
  if (m_cux->isConnected())
  {
    m_cux->enqueueRequest(QueueableRequest_FaultCodes);
  }
  else
  {
    onNotConnected();
  }
}

/**
 * Queues a request to read the battery-backed memory.
 */
void MainWindow::onBatteryBackedMemClicked()
{
  if (m_cux->isConnected())
  {
    m_cux->enqueueRequest(QueueableRequest_BatteryBackedMem);
  }
  else
  {
    onNotConnected();
  }
}

/**
 * Sets the type of lambda trim to read from the ECU.
 */
void MainWindow::onLambdaTrimButtonClicked(QAbstractButton* button)
{
  m_cux->setLambdaTrimType((button == m_ui->m_lambdaTrimShortButton) ?
                           C14CUX_LambdaTrimType_ShortTerm : C14CUX_LambdaTrimType_LongTerm);
}

/**
 * Sets the type of MAF reading to read from the ECU.
 */
void MainWindow::onMAFReadingButtonClicked(QAbstractButton* button)
{
  m_cux->setMAFReadingType((button == m_ui->m_mafReadingLinearButton) ?
                           C14CUX_AirflowType_Linearized : C14CUX_AirflowType_Direct);
}

/**
 * Sets the type of throttle position to display.
 */
void MainWindow::onThrottleTypeButtonClicked(QAbstractButton* button)
{
  m_cux->setThrottleReadingType((button == m_ui->m_throttleTypeAbsoluteButton) ?
                                C14CUX_ThrottlePosType_Absolute : C14CUX_ThrottlePosType_Corrected);
}

/**
 * Displays the tune revision number (read from the ROM)
 * @param tuneRevisionNum Decimal representation of the revision number
 * @param checksumFixer Value of byte used to forced a checksum of 0x01 on the ROM contents
 * @param ident Value of "Ident" byte that was sometimes incremented separately from the tune number
 */
void MainWindow::onTuneRevisionReady(int tuneRevisionNum, int checksumFixer, int ident)
{
  m_ui->m_tuneRevNumberLabel->setText(QString("Tune: R%04").arg(tuneRevisionNum));
  m_ui->m_identLabel->setText(QString("Ident: ") + QString("%1").arg(ident, 4, 16).toUpper());
  m_ui->m_checksumFixerLabel->setText(
    QString("Checksum fixer: ") + QString("%1").arg(checksumFixer, 2, 16, QChar('0')).toUpper());
}

/**
 * Paints a redline on the tachometer, based on the electronic RPM limit
 * @param rpmLimit Engine speed limit in RPM
 */
void MainWindow::onRPMLimitReady(int rpmLimit)
{
  m_ui->m_revCounter->setCritical((double)rpmLimit);
  m_ui->m_rpmLimitLabel->setText(QString("RPM limit: ") + QString("%1").arg(rpmLimit));
}

/**
 * Sets the fuel map column header text to reflect the engine speed thresholds
 * used in the ECU
 */
void MainWindow::onRPMTableReady()
{
  const c14cux_rpmtable table = m_cux->getRPMTable();

  for (int col = 0; col < FUEL_MAP_COLUMNS; col++)
  {
    m_ui->m_fuelMapDisplay->horizontalHeaderItem(col)->setText(QString::number(table.rpm[col]));
  }
}

/**
 * Sets the visibility, labeling, and default values for the widgets that display
 * lambda feedback information and MAF CO trim information.
 */
void MainWindow::setLambdaWidgetsForFeedbackMode(c14cux_feedback_mode mode, bool coTrimEnabled, bool lambdaEnabled)
{
  if (mode == C14CUX_FeedbackMode_OpenLoop)
  {
    m_ui->m_lambdaTrimTypeLabel->setEnabled(false);
    m_ui->m_lambdaTrimLongButton->setEnabled(false);
    m_ui->m_lambdaTrimShortButton->setEnabled(false);

    m_ui->m_oddFuelTrimAndMAFCOLabel->setText("MAF CO trim:");
    m_ui->m_oddFuelTrimAndMAFCOLabel->setEnabled(coTrimEnabled);
    m_ui->m_oddFuelTrimBarAndMAFCOLabel->setText(QString::number(m_cux->getCOTrimVoltage(), 'f', 2) + "V");
    m_ui->m_oddFuelTrimBarAndMAFCOLabel->setAlignment(Qt::AlignLeft);
    m_ui->m_oddFuelTrimBarAndMAFCOLabel->setEnabled(coTrimEnabled);

    m_ui->m_lambdaTrimHighLimitLabel->setVisible(false);
    m_ui->m_lambdaTrimLowLimitLabel->setVisible(false);
    m_ui->m_evenFuelTrimBar->setVisible(false);
    m_ui->m_evenFuelTrimBarLabel->setVisible(false);
    m_ui->m_evenFuelTrimLabel->setVisible(false);
    m_ui->m_oddFuelTrimBar->setVisible(false);
  }
  else
  {
    m_ui->m_lambdaTrimTypeLabel->setEnabled(lambdaEnabled);
    m_ui->m_lambdaTrimLongButton->setEnabled(lambdaEnabled);
    m_ui->m_lambdaTrimShortButton->setEnabled(lambdaEnabled);
    m_ui->m_oddFuelTrimBar->setEnabled(lambdaEnabled);
    m_ui->m_evenFuelTrimBar->setEnabled(lambdaEnabled);
    m_ui->m_evenFuelTrimBarLabel->setEnabled(lambdaEnabled);
    m_ui->m_evenFuelTrimLabel->setEnabled(lambdaEnabled);
    m_ui->m_lambdaTrimHighLimitLabel->setEnabled(lambdaEnabled);
    m_ui->m_lambdaTrimLowLimitLabel->setEnabled(lambdaEnabled);

    m_ui->m_oddFuelTrimAndMAFCOLabel->setText("Lambda fuel trim (odd):");
    m_ui->m_oddFuelTrimAndMAFCOLabel->setEnabled(lambdaEnabled);
    m_ui->m_oddFuelTrimBarAndMAFCOLabel->setEnabled(lambdaEnabled);
    m_ui->m_oddFuelTrimBarAndMAFCOLabel->setAlignment(Qt::AlignRight);
    setLambdaTrimIndicators(m_cux->getLambdaTrimOdd(), m_cux->getLambdaTrimEven());

    m_ui->m_lambdaTrimHighLimitLabel->setVisible(true);
    m_ui->m_lambdaTrimLowLimitLabel->setVisible(true);
    m_ui->m_evenFuelTrimBar->setVisible(true);
    m_ui->m_evenFuelTrimBarLabel->setVisible(true);
    m_ui->m_evenFuelTrimLabel->setVisible(true);
    m_ui->m_oddFuelTrimBar->setVisible(true);
  }
}

/**
 * Called when the fueling feedback mode (open- vs closed-loop) changes.
 * @param mode The new feedback mode
 */
void MainWindow::onFeedbackModeChanged(c14cux_feedback_mode mode)
{
  setLambdaWidgetsForFeedbackMode(
    mode,
    m_enabledSamples[SampleType_COTrimVoltage],
    m_enabledSamples[SampleType_LambdaTrimLong] || m_enabledSamples[SampleType_LambdaTrimShort]);
}

/**
 * Called when the fuel map index changes. This would only happen (a) when the map
 * index is read for the first time, or (b) if the tune resistor was changed while
 * running.
 * @param fuelMapId New active fuel map ID
 */
void MainWindow::onFuelMapIndexChanged(unsigned int fuelMapId)
{
  m_ui->m_fuelMapIndexLabel->setText(QString("Current fuel map: %1").arg(fuelMapId));
  m_ui->m_fuelMapFactorLabel->setText(QString("Multiplier:"));

  const QByteArray* fuelMapData = m_cux->getFuelMap(fuelMapId);

  if (fuelMapData)
  {
    populateFuelMapDisplay(fuelMapData,
                           m_cux->getFuelMapAdjustmentFactor(fuelMapId),
                           m_cux->getRowScaler(fuelMapId));
    m_fuelMapDataIsCurrent = true;
  }
  else
  {
    // The data for the current fuel map hasn't been read out of the
    // ECU yet, so put in a request. We'll update the display when
    // we receive the signal that the new data is ready.
    m_fuelMapDataIsCurrent = false;
    m_cux->enqueueRequest(QueueableRequest_FuelMapData, fuelMapId);
    m_cux->enqueueRequest(QueueableRequest_RPMTable);
  }
}

