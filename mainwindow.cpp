#include <QCloseEvent>
#include <QMessageBox>
#include <QList>
#include <QDateTime>
#include <QDir>
#include <QHBoxLayout>
#include <QThread>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QCryptographicHash>
#include <QGraphicsOpacityEffect>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "faultcodedialog.h"

/**
 * Constructor; sets up main UI
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_ui(new Ui::MainWindow),
#ifdef ENABLE_SIM_MODE
      m_simDialog(0),
#endif
      m_cuxThread(0),
      m_cux(0),
      m_options(0),
      m_aboutBox(0),
      m_pleaseWaitBox(0),
      m_helpViewerDialog(0),
      m_fuelMapDataIsCurrent(false)
{
    // register this special enum type for use in Qt signals/slots
    qRegisterMetaType<c14cux_feedback_mode>("c14cux_feedback_mode");

    buildSpeedAndTempUnitTables();
    m_ui->setupUi(this);
    this->setWindowTitle("RoverGauge");

    m_options = new OptionsDialog(this->windowTitle(), this);
    m_cux = new CUXInterface(m_options->getSerialDeviceName(), m_options->getSpeedUnits(),
                             m_options->getTemperatureUnits(), m_options->getRefreshFuelMap());

    m_enabledSamples = m_options->getEnabledSamples();
    m_cux->setEnabledSamples(m_enabledSamples);
    m_cux->setReadIntervals(m_options->getReadIntervals());

    m_iacDialog = new IdleAirControlDialog(this->windowTitle(), this);
    connect(m_iacDialog, SIGNAL(requestIdleAirControlMovement(int,int)),
            m_cux, SLOT(onIdleAirControlMovementRequest(int,int)));

    m_logger = new Logger(m_cux);

    m_fuelPumpRefreshTimer = new QTimer(this);
    m_fuelPumpRefreshTimer->setInterval(1000);

    for (int idx = 0; idx < NUM_ACTIVE_FUEL_MAP_CELLS; idx += 1)
    {
        m_lastHighlightedFuelMapCell[idx] = 0;
    }

    connect(m_cux, SIGNAL(dataReady()), this, SLOT(onDataReady()));
    connect(m_cux, SIGNAL(connected()), this, SLOT(onConnect()));
    connect(m_cux, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(m_cux, SIGNAL(readError()), this, SLOT(onReadError()));
    connect(m_cux, SIGNAL(readSuccess()), this, SLOT(onReadSuccess()));
    connect(m_cux, SIGNAL(failedToConnect(QString)), this, SLOT(onFailedToConnect(QString)));
    connect(m_cux, SIGNAL(faultCodesReady()), this, SLOT(onFaultCodesReady()));
    connect(m_cux, SIGNAL(faultCodesReadFailed()), this, SLOT(onFaultCodesReadFailed()));
    connect(m_cux, SIGNAL(fuelMapReady(unsigned int)), this, SLOT(onFuelMapDataReady(unsigned int)));
    connect(m_cux, SIGNAL(revisionNumberReady(int,int,int)), this, SLOT(onTuneRevisionReady(int,int,int)));
    connect(m_cux, SIGNAL(interfaceReadyForPolling()), this, SLOT(onInterfaceReady()));
    connect(m_cux, SIGNAL(notConnected()), this, SLOT(onNotConnected()));
    connect(m_cux, SIGNAL(romImageReady()), this, SLOT(onROMImageReady()));
    connect(m_cux, SIGNAL(romImageReadFailed()), this, SLOT(onROMImageReadFailed()));
    connect(m_cux, SIGNAL(rpmLimitReady(int)), this, SLOT(onRPMLimitReady(int)));
    connect(m_cux, SIGNAL(rpmTableReady()), this, SLOT(onRPMTableReady()));
    connect(m_cux, SIGNAL(feedbackModeHasChanged(c14cux_feedback_mode)), this, SLOT(onFeedbackModeChanged(c14cux_feedback_mode)));
    connect(m_cux, SIGNAL(fuelMapIndexHasChanged(uint)), this, SLOT(onFuelMapIndexChanged(uint)));
#ifdef ENABLE_FORCE_OPEN_LOOP
    connect(m_cux, SIGNAL(forceOpenLoopState(bool)), this, SLOT(onForceOpenLoopStateReceived(bool)));
#endif
    connect(m_fuelPumpRefreshTimer, SIGNAL(timeout()), m_cux, SLOT(onFuelPumpRunRequest()));
    connect(this, SIGNAL(requestToStartPolling()), m_cux, SLOT(onStartPollingRequest()));
    connect(this, SIGNAL(requestThreadShutdown()), m_cux, SLOT(onShutdownThreadRequest()));
    connect(this, SIGNAL(requestFuelMapData(unsigned int)), m_cux, SLOT(onFuelMapRequested(unsigned int)));
    connect(this, SIGNAL(requestROMImage()), m_cux, SLOT(onReadROMImageRequested()));
    connect(this, SIGNAL(requestFuelPumpRun()), m_cux, SLOT(onFuelPumpRunRequest()));

    setWindowIcon(QIcon(":/icons/key.png"));

    setupWidgets();
    dimUnusedControls();
}

/**
 * Destructor; cleans up instance of 14CUX communications library
 *  and miscellaneous data storage
 */
MainWindow::~MainWindow()
{
    delete m_tempLimits;
    delete m_tempRange;
    delete m_speedUnitSuffix;
    delete m_tempUnitSuffix;
    delete m_aboutBox;
    delete m_options;
    delete m_cux;
    delete m_cuxThread;
}

/**
 * Resizes the main window if the screen is small
 */
void MainWindow::resizeForSmallScreens()
{
    QDesktopWidget desktop;
    const QRect screenGeo = desktop.screenGeometry();
    int heightPixels = this->minimumHeight();
    int widthPixels = this->minimumWidth();
    if ((screenGeo.height() * 0.95) < heightPixels)
    {
        heightPixels = screenGeo.height() * 0.9;
    }
    if ((screenGeo.width() * 0.95) < widthPixels)
    {
        widthPixels = screenGeo.width() * 0.9;
    }

    this->setMinimumSize(widthPixels, heightPixels);
}

/**
 * Populates hash tables with unit-of-measure suffixes and temperature thresholds
 */
void MainWindow::buildSpeedAndTempUnitTables()
{
    m_speedUnitSuffix = new QHash<SpeedUnits,QString>();
    m_speedUnitSuffix->insert(MPH, " MPH");
    m_speedUnitSuffix->insert(KPH, " km/h");

    m_tempUnitSuffix = new QHash<TemperatureUnits,QString>;
    m_tempUnitSuffix->insert(Fahrenheit, " F");
    m_tempUnitSuffix->insert(Celcius, " C");

    m_tempRange = new QHash<TemperatureUnits,QPair<int, int> >;
    m_tempRange->insert(Fahrenheit, qMakePair(-40, 280));
    m_tempRange->insert(Celcius, qMakePair(-40, 140));

    m_tempLimits = new QHash<TemperatureUnits,QPair<int, int> >;
    m_tempLimits->insert(Fahrenheit, qMakePair(180, 210));
    m_tempLimits->insert(Celcius, qMakePair(80, 98));
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
    connect(m_ui->m_saveROMImageAction, SIGNAL(triggered()), this, SLOT(onSaveROMImageSelected()));
    connect(m_ui->m_exitAction, SIGNAL(triggered()), this, SLOT(onExitSelected()));
    connect(m_ui->m_showFaultCodesAction, SIGNAL(triggered()), m_cux, SLOT(onFaultCodesRequested()));
    connect(m_ui->m_idleAirControlAction, SIGNAL(triggered()), this, SLOT(onIdleAirControlClicked()));
    connect(m_ui->m_editSettingsAction, SIGNAL(triggered()), this, SLOT(onEditOptionsClicked()));
    connect(m_ui->m_helpContentsAction, SIGNAL(triggered()), this, SLOT(onHelpContentsClicked()));
    connect(m_ui->m_helpAboutAction, SIGNAL(triggered()), this, SLOT(onHelpAboutClicked()));

#ifdef ENABLE_FORCE_OPEN_LOOP
    connect(m_ui->m_forceOpenLoopCheckbox, SIGNAL(clicked(bool)), m_cux, SLOT(onForceOpenLoopRequest(bool)));
#endif

#ifdef ENABLE_SIM_MODE
    m_ui->m_optionsMenu->addSeparator();
    m_simDialogAction = m_ui->m_optionsMenu->addAction("&Simulation mode control...");
    connect(m_simDialogAction, SIGNAL(triggered()), this, SLOT(onSimDialogClicked()));
#endif

    // connect button signals
    connect(m_ui->m_connectButton, SIGNAL(clicked()), this, SLOT(onConnectClicked()));
    connect(m_ui->m_disconnectButton, SIGNAL(clicked()), this, SLOT(onDisconnectClicked()));
    connect(m_ui->m_mafReadingButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onMAFReadingButtonClicked(QAbstractButton*)));
    connect(m_ui->m_throttleTypeButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onThrottleTypeButtonClicked(QAbstractButton*)));
    connect(m_ui->m_lambdaTrimButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onLambdaTrimButtonClicked(QAbstractButton*)));
    connect(m_ui->m_fuelPumpOneshotButton, SIGNAL(clicked()), m_cux, SLOT(onFuelPumpRunRequest()));
    connect(m_ui->m_fuelPumpContinuousButton, SIGNAL(clicked()), this, SLOT(onFuelPumpContinuous()));
    connect(m_ui->m_startLoggingButton, SIGNAL(clicked()), this, SLOT(onStartLogging()));
    connect(m_ui->m_stopLoggingButton, SIGNAL(clicked()), this, SLOT(onStopLogging()));

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

    // set up the fuel map display
    setStyleSheet("QTableWidget {background-color: transparent;}");

    m_ui->m_fuelMapDisplay->horizontalHeader()->setStyleSheet("QHeaderView { font-size: 9pt; }");
    m_ui->m_fuelMapDisplay->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_ui->m_fuelMapDisplay->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    unsigned int rowCount = m_ui->m_fuelMapDisplay->rowCount();
    unsigned int colCount = m_ui->m_fuelMapDisplay->columnCount();
    QTableWidgetItem *item = 0;
    for (int col = 0; col < colCount; col++)
    {
        m_ui->m_fuelMapDisplay->setHorizontalHeaderItem(col, new QTableWidgetItem(""));
        for (int row = 0; row < rowCount; row++)
        {
            item = new QTableWidgetItem("");
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(0);
            m_ui->m_fuelMapDisplay->setItem(row, col, item);
        }
    }

    m_ui->m_logFileNameBox->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss"));
    m_ui->m_injectorDutyCycleBar->setAlignment(Qt::AlignCenter);

    SpeedUnits speedUnit = m_options->getSpeedUnits();

    m_ui->m_speedo->setMinimum(0.0);
    if (speedUnit == MPH)
    {
        m_ui->m_speedo->setMaximum(speedometerMaxMPH);
    }
    else
    {
        m_ui->m_speedo->setMaximum(speedometerMaxKPH);
    }
    m_ui->m_speedo->setSuffix(m_speedUnitSuffix->value(speedUnit));
    m_ui->m_speedo->setNominal(1000.0);
    m_ui->m_speedo->setCritical(1000.0);

    m_ui->m_revCounter->setMinimum(0.0);
    m_ui->m_revCounter->setMaximum(8000);
    m_ui->m_revCounter->setSuffix(" RPM");
    m_ui->m_revCounter->setNominal(100000.0);
    m_ui->m_revCounter->setCritical(8000);

    TemperatureUnits tempUnits = m_options->getTemperatureUnits();
    int tempMin = m_tempRange->value(tempUnits).first;
    int tempMax = m_tempRange->value(tempUnits).second;

    m_ui->m_waterTempGauge->setValue(tempMin);
    m_ui->m_waterTempGauge->setMaximum(tempMax);
    m_ui->m_waterTempGauge->setMinimum(tempMin);
    m_ui->m_waterTempGauge->setSuffix(m_tempUnitSuffix->value(tempUnits));
    m_ui->m_waterTempGauge->setNominal(m_tempLimits->value(tempUnits).first);
    m_ui->m_waterTempGauge->setCritical(m_tempLimits->value(tempUnits).second);

    m_ui->m_fuelTempGauge->setValue(tempMin);
    m_ui->m_fuelTempGauge->setMaximum(tempMax);
    m_ui->m_fuelTempGauge->setMinimum(tempMin);
    m_ui->m_fuelTempGauge->setSuffix(m_tempUnitSuffix->value(tempUnits));
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
 * Attempts to open the serial device connected to the 14CUX,
 * and starts updating the display with data if successful.
 */
void MainWindow::onConnectClicked()
{
    // If the worker thread hasn't been created yet, do that now.
    if (m_cuxThread == 0)
    {
        m_cuxThread = new QThread(this);
        m_cux->moveToThread(m_cuxThread);
        connect(m_cuxThread, SIGNAL(started()), m_cux, SLOT(onParentThreadStarted()));
    }

    // If the worker thread is alreay running, ask it to start polling the ECU.
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
    connect(&faultDialog, SIGNAL(clearFaultCodes()), m_cux, SLOT(onFaultCodesClearRequested()));
    connect(m_cux, SIGNAL(faultCodesClearSuccess(c14cux_faultcodes)),
            &faultDialog, SLOT(onFaultClearSuccess(c14cux_faultcodes)));
    connect(m_cux, SIGNAL(faultCodesClearFailure()), &faultDialog, SLOT(onFaultClearFailure()));
    faultDialog.exec();
}

/**
 * Responds to a signal from the worker thread that indicates there was a
 * problem reading the fault codes. Displays a message box indicating the same.
 */
void MainWindow::onFaultCodesReadFailed()
{
    QMessageBox::warning(this, "Error",
        "Unable to read fault codes from ECU.",
        QMessageBox::Ok);
}

/**
 * Uses a fuel map array to populate a 16x8 grid that shows all the fueling
 * values.
 * @param data Pointer to the ByteArray that contains the map data
 */
void MainWindow::populateFuelMapDisplay(QByteArray *data, int fuelMapAdjustmentFactor)
{
    if (data != 0)
    {        
        int rowCount = m_ui->m_fuelMapDisplay->rowCount();
        int colCount = m_ui->m_fuelMapDisplay->columnCount();
        QTableWidgetItem *item = 0;
        unsigned char byte = 0;

        removeFuelMapCellHighlight();

        // populate all the cells with the data for this map
        for (int row = 0; row < rowCount; row++)
        {
            for (int col = 0; col < colCount; col++)
            {
                item = m_ui->m_fuelMapDisplay->item(row, col);
                if (item != 0)
                {
                    // retrieve the fuel map value at the current row/col
                    byte = data->at(row*colCount + col);

                    item->setText(QString("%1").arg(byte, 2, 16, QChar('0')).toUpper());
                    item->setBackgroundColor(getColorForFuelMapCell(byte));
                    item->setTextColor(Qt::black);
                }
            }
        }

        QString adjFactorLabel =
            QString("%1").arg(fuelMapAdjustmentFactor, 0, 16).toUpper();
        m_ui->m_fuelMapFactorLabel->setText(QString("Multiplier: 0x") + adjFactorLabel);

        highlightActiveFuelMapCells();
    }
}

/**
 * Uses fuel map data to populate the fuel map display grid.
 * @param ID of the fuel map just retrieved (from 1-5)
 */
void MainWindow::onFuelMapDataReady(unsigned int fuelMapId)
{
    QByteArray *data = m_cux->getFuelMap(fuelMapId);
    if (data != 0)
    {
        populateFuelMapDisplay(data, m_cux->getFuelMapAdjustmentFactor(fuelMapId));
        m_fuelMapDataIsCurrent = true;
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

    m_ui->m_milLed->setChecked(m_cux->isMILOn());

    // if fuel map display updates are enabled...
    if (m_enabledSamples[SampleType_FuelMapRowCol] && m_fuelMapDataIsCurrent)
    {
        removeFuelMapCellHighlight();
        highlightActiveFuelMapCells();
    }

    if (m_enabledSamples[SampleType_Throttle])
        m_ui->m_throttleBar->setValue(m_cux->getThrottlePos() * 100);

    if (m_enabledSamples[SampleType_MAF])
        m_ui->m_mafReadingBar->setValue(m_cux->getMAFReading() * 100);

    if (m_enabledSamples[SampleType_IdleBypassPosition])
        m_ui->m_idleBypassPosBar->setValue(m_cux->getIdleBypassPos() * 100);

    if (m_enabledSamples[SampleType_RoadSpeed])
        m_ui->m_speedo->setValue((int)m_cux->getRoadSpeed());

    if (m_enabledSamples[SampleType_EngineRPM])
    {
        rpm = m_cux->getEngineSpeedRPM();
        m_ui->m_revCounter->setValue(rpm);
    }

    if (m_enabledSamples[SampleType_EngineTemperature])
        m_ui->m_waterTempGauge->setValue(m_cux->getCoolantTemp());

    if (m_enabledSamples[SampleType_FuelTemperature])
        m_ui->m_fuelTempGauge->setValue(m_cux->getFuelTemp());

    if (m_enabledSamples[SampleType_MainVoltage])
        m_ui->m_voltage->setText(QString::number(m_cux->getMainVoltage(), 'f', 1) + "VDC");

    if (m_enabledSamples[SampleType_FuelPumpRelay])
        m_ui->m_fuelPumpRelayStateLed->setChecked(m_cux->getFuelPumpRelayState());

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
        int targetIdleSpeedRPM = m_cux->getTargetIdleSpeed();

        if (targetIdleSpeedRPM > 0)
            m_ui->m_targetIdle->setText(QString::number(targetIdleSpeedRPM));
        else
            m_ui->m_targetIdle->setText("");

        m_ui->m_idleModeLed->setChecked(m_cux->getIdleMode());
    }

    if (m_enabledSamples[SampleType_LambdaTrimShort] || m_enabledSamples[SampleType_LambdaTrimLong])
    {
        if (m_cux->getFeedbackMode() == C14CUX_FeedbackMode_ClosedLoop)
            setLambdaTrimIndicators(m_cux->getLambdaTrimOdd(), m_cux->getLambdaTrimEven());
        else
            m_ui->m_oddFuelTrimBarLabel->setText(QString::number(m_cux->getCOTrimVoltage(), 'f', 2) + "VDC");
    }

    if (m_enabledSamples[SampleType_GearSelection])
        setGearLabel(m_cux->getGear());

    m_logger->logData();
}

/**
 * Sets the lambda fuel trim indicators to the provided values
 */
void MainWindow::setLambdaTrimIndicators(int lambdaTrimOdd, int lambdaTrimEven)
{
    QString oddLabel = (lambdaTrimOdd >= 0) ?
        QString("+%1%").arg(lambdaTrimOdd * 100 / m_ui->m_oddFuelTrimBar->maximum()) :
        QString("-%1%").arg(lambdaTrimOdd * 100 / m_ui->m_oddFuelTrimBar->minimum());
    QString evenLabel = (lambdaTrimEven >= 0) ?
        QString("+%1%").arg(lambdaTrimEven * 100 / m_ui->m_evenFuelTrimBar->maximum()) :
        QString("-%1%").arg(lambdaTrimEven * 100 / m_ui->m_evenFuelTrimBar->minimum());

    m_ui->m_oddFuelTrimBar->setEnabled(true);
    m_ui->m_oddFuelTrimBar->setValue(lambdaTrimOdd);
    m_ui->m_evenFuelTrimBar->setEnabled(true);
    m_ui->m_evenFuelTrimBar->setValue(lambdaTrimEven);

    m_ui->m_oddFuelTrimBarLabel->setText(oddLabel);
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
 * Paints a highlight on the fuel map display cells that represent the
 * most-recently-read fueling index.
 */
void MainWindow::highlightActiveFuelMapCells()
{
    int fuelMapRow = m_cux->getFuelMapRowIndex();
    int fuelMapCol = m_cux->getFuelMapColumnIndex();
    int rowWeight = m_cux->getFuelMapRowWeighting();
    int colWeight = m_cux->getFuelMapColWeighting();

    if (m_options->getSoftHighlight())
    {
        int idx;
        QColor currentColor;
        QColor newColor;

        // Compute the distribution of shading that should be applied left/right
        // and top/bottom to the block of four cells.
        float leftPercent = 1.0 - (colWeight / 15.0);
        float rightPercent = 1.0 - leftPercent;
        float topPercent = 1.0 - (rowWeight / 15.0);
        float bottomPercent = 1.0 - topPercent;

        float shadePercentage[NUM_ACTIVE_FUEL_MAP_CELLS];

        // We subtract from 1.0 here because these values will be used as multipliers
        // against the un-highlighted cell's R/G/B components to produce a shade of
        // the appropriate darkness.
        shadePercentage[0] = 1.0 - (leftPercent * topPercent);
        shadePercentage[1] = 1.0 - (rightPercent * topPercent);
        shadePercentage[2] = 1.0 - (leftPercent * bottomPercent);
        shadePercentage[3] = 1.0 - (rightPercent * bottomPercent);

        m_lastHighlightedFuelMapCell[0] = m_ui->m_fuelMapDisplay->item(fuelMapRow, fuelMapCol);
        m_lastHighlightedFuelMapCell[1] = m_ui->m_fuelMapDisplay->item(fuelMapRow, fuelMapCol + 1);
        m_lastHighlightedFuelMapCell[2] = m_ui->m_fuelMapDisplay->item(fuelMapRow + 1, fuelMapCol);
        m_lastHighlightedFuelMapCell[3] = m_ui->m_fuelMapDisplay->item(fuelMapRow + 1, fuelMapCol + 1);

        for (idx = 0; idx < NUM_ACTIVE_FUEL_MAP_CELLS; idx += 1)
        {
            if (m_lastHighlightedFuelMapCell[idx] != 0)
            {
                currentColor = m_lastHighlightedFuelMapCell[idx]->backgroundColor();
                newColor.setRgb(currentColor.red() * shadePercentage[idx],
                                currentColor.green() * shadePercentage[idx],
                                currentColor.blue() * shadePercentage[idx]);
                m_lastHighlightedFuelMapCell[idx]->setBackgroundColor(newColor);
                m_lastHighlightedFuelMapCell[idx]->setTextColor((newColor.value() > 128) ? Qt::black : Qt::white);
            }
        }
    }
    else
    {
        // we're not doing a soft-highlight using the weightings, so simply use the row/column
        // weight to round up to the next row/col index if appropriate
        if (rowWeight >= 8)
        {
            fuelMapRow += 1;
            if (fuelMapRow >= FUEL_MAP_ROWS)
            {
                fuelMapRow = FUEL_MAP_ROWS - 1;
            }
        }
        if (colWeight >= 8)
        {
            fuelMapCol += 1;
            if (fuelMapCol >= FUEL_MAP_COLUMNS)
            {
                fuelMapCol = FUEL_MAP_COLUMNS - 1;
            }
        }

        // we're only highlighting a single cell (not a block of four), so zero the other pointers
        m_lastHighlightedFuelMapCell[0] = m_ui->m_fuelMapDisplay->item(fuelMapRow, fuelMapCol);
        m_lastHighlightedFuelMapCell[1] = 0;
        m_lastHighlightedFuelMapCell[2] = 0;
        m_lastHighlightedFuelMapCell[3] = 0;

        m_lastHighlightedFuelMapCell[0]->setBackgroundColor(Qt::black);
        m_lastHighlightedFuelMapCell[0]->setTextColor(Qt::white);
    }
}

/**
 * Sets the background of the last highlighted fuel map cell(s) back to its
 * un-highligted color (which is dependent on the value for that cell).
 */
void MainWindow::removeFuelMapCellHighlight()
{
    for (int idx = 0; idx < NUM_ACTIVE_FUEL_MAP_CELLS; idx += 1)
    {
        if (m_lastHighlightedFuelMapCell[idx] != 0)
        {
            bool ok = false;
            unsigned char value = (unsigned char)(m_lastHighlightedFuelMapCell[idx]->text().toInt(&ok, 16));
            if (ok)
            {
                m_lastHighlightedFuelMapCell[idx]->setBackgroundColor(getColorForFuelMapCell(value));
                m_lastHighlightedFuelMapCell[idx]->setTextColor(Qt::black);
            }
        }
    }
}

/**
 * Generates a color whose intensity corresponds to a fueling value
 */
QColor MainWindow::getColorForFuelMapCell(unsigned char value)
{
    return QColor::fromRgb(255, (value / 2 * -1) + 255, 255.0 - value);
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
        SpeedUnits speedUnit = m_options->getSpeedUnits();
        if (speedUnit == MPH)
        {
            m_ui->m_speedo->setMaximum(speedometerMaxMPH);
        }
        else
        {
            m_ui->m_speedo->setMaximum(speedometerMaxKPH);
        }
        m_ui->m_speedo->setSuffix(m_speedUnitSuffix->value(speedUnit));
        m_ui->m_speedo->repaint();

        TemperatureUnits tempUnits = m_options->getTemperatureUnits();
        QString tempUnitStr = m_tempUnitSuffix->value(tempUnits);

        int tempMin = m_tempRange->value(tempUnits).first;
        int tempMax = m_tempRange->value(tempUnits).second;
        int tempNominal = m_tempLimits->value(tempUnits).first;
        int tempCritical = m_tempLimits->value(tempUnits).second;

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
        QMap<SampleType,bool> samples = m_options->getEnabledSamples();
        foreach (SampleType field, samples.keys())
        {
            if (samples.keys().contains(field))
            {
                m_enabledSamples[field] = samples[field];
            }
        }

        m_cux->setEnabledSamples(m_enabledSamples);
        m_cux->setReadIntervals(m_options->getReadIntervals());

        dimUnusedControls();

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
        }
    }
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
        m_ui->m_mafReadingBar->setValue(0);

    enabled = m_enabledSamples[SampleType_Throttle];
    m_ui->m_throttleLabel->setEnabled(enabled);
    m_ui->m_throttleBar->setEnabled(enabled);
    m_ui->m_throttleTypeLabel->setEnabled(enabled);
    m_ui->m_throttleTypeAbsoluteButton->setEnabled(enabled);
    m_ui->m_throttleTypeCorrectedButton->setEnabled(enabled);
    if (!enabled)
        m_ui->m_throttleBar->setValue(0);

    enabled = m_enabledSamples[SampleType_IdleBypassPosition];
    m_ui->m_idleBypassLabel->setEnabled(enabled);
    m_ui->m_idleBypassPosBar->setEnabled(enabled);
    if (!enabled)
        m_ui->m_idleBypassPosBar->setValue(0);

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

    enabled = (m_enabledSamples[SampleType_LambdaTrimShort] || m_enabledSamples[SampleType_LambdaTrimLong]);
    m_ui->m_lambdaTrimTypeLabel->setEnabled(enabled);
    m_ui->m_lambdaTrimLowLimitLabel->setEnabled(enabled);
    m_ui->m_lambdaTrimHighLimitLabel->setEnabled(enabled);
    m_ui->m_lambdaTrimShortButton->setEnabled(enabled);
    m_ui->m_lambdaTrimLongButton->setEnabled(enabled);
    m_ui->m_oddFuelTrimBar->setEnabled(enabled);
    m_ui->m_oddFuelTrimLabel->setEnabled(enabled);
    m_ui->m_oddFuelTrimBarLabel->setEnabled(enabled);
    m_ui->m_evenFuelTrimBar->setEnabled(enabled);
    m_ui->m_evenFuelTrimBarLabel->setEnabled(enabled);
    m_ui->m_evenFuelTrimLabel->setEnabled(enabled);
    if (!enabled)
    {
        m_ui->m_oddFuelTrimBar->setValue(0);
        m_ui->m_oddFuelTrimBarLabel->setText("");
        m_ui->m_evenFuelTrimBar->setValue(0);
        m_ui->m_evenFuelTrimBarLabel->setText("");
    }

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
    enabled &= m_enabledSamples[SampleType_InjectorPulseWidth];
    m_ui->m_injectorDutyCycleBar->setEnabled(enabled);
    m_ui->m_injectorDutyCycleLabel->setEnabled(enabled);
    if (!enabled)
        m_ui->m_injectorDutyCycleBar->setValue(0);

    m_speedometerOpacity->setEnabled(!m_enabledSamples[SampleType_RoadSpeed]);
}

/**
 * Responds to a 'close' event on the main window by first shutting down
 * the other thread.
 * @param event The event itself.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    m_logger->closeLog();

    if ((m_cuxThread != 0) && m_cuxThread->isRunning())
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
#ifdef ENABLE_FORCE_OPEN_LOOP
    m_ui->m_forceOpenLoopCheckbox->setEnabled(true);
#endif
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
#ifdef ENABLE_FORCE_OPEN_LOOP
    m_ui->m_forceOpenLoopCheckbox->setEnabled(false);
#endif
    m_ui->m_tuneRevNumberLabel->setText("Tune:");
    m_ui->m_identLabel->setText("Ident:");
    m_ui->m_checksumFixerLabel->setText("Checksum fixer:");

    m_ui->m_speedo->setValue(0.0);
    m_ui->m_revCounter->setValue(0.0);
    m_ui->m_waterTempGauge->setValue(m_ui->m_waterTempGauge->minimum());
    m_ui->m_fuelTempGauge->setValue(m_ui->m_fuelTempGauge->minimum());
    m_ui->m_throttleBar->setValue(0);
    m_ui->m_mafReadingBar->setValue(0);
    m_ui->m_idleBypassPosBar->setValue(0);
    m_ui->m_idleModeLed->setChecked(false);
    m_ui->m_targetIdle->setText("");
    m_ui->m_voltage->setText("");
    m_ui->m_gear->setText("");
    m_ui->m_fuelPumpRelayStateLed->setChecked(false);
    m_ui->m_oddFuelTrimBar->setValue(0);
    if (m_ui->m_oddFuelTrimBar->isVisible())
        m_ui->m_oddFuelTrimBarLabel->setText("+0%");
    else
        m_ui->m_oddFuelTrimBarLabel->setText("0.0VDC");
    m_ui->m_evenFuelTrimBar->setValue(0);
    m_ui->m_evenFuelTrimBarLabel->setText("+0%");
    m_ui->m_injectorDutyCycleBar->setValue(0);
    m_ui->m_injectorPulseWidthLabel->setText("Pulse width:");

    m_ui->m_oddFuelTrimBar->repaint();
    m_ui->m_evenFuelTrimBar->repaint();

    removeFuelMapCellHighlight();

    m_fuelMapDataIsCurrent = false;
    m_cux->invalidateFuelMapData();
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
void MainWindow::onStartLogging()
{
    if (m_logger->openLog(m_ui->m_logFileNameBox->text()))
    {
        m_ui->m_startLoggingButton->setEnabled(false);
        m_ui->m_stopLoggingButton->setEnabled(true);
    }
    else
    {
        QMessageBox::warning(this, "Error",
            "Failed to open log file (" + m_logger->getLogPath() + ")", QMessageBox::Ok);
    }
}

/**
 * Closes the open log file.
 */
void MainWindow::onStopLogging()
{
    m_logger->closeLog();
    m_ui->m_stopLoggingButton->setEnabled(false);
    m_ui->m_startLoggingButton->setEnabled(true);
}

/**
 * Displays an dialog box with information about the program.
 */
void MainWindow::onHelpAboutClicked()
{
    if (m_aboutBox == 0)
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
    if (m_helpViewerDialog == 0)
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
    if (m_pleaseWaitBox != 0)
    {
        m_pleaseWaitBox->hide();
    }

    QMessageBox::warning(this, "Error",
        "This requires that the software first be connected to the ECU (using the \"Connect\" button.)",
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
            if (m_pleaseWaitBox == 0)
            {
                m_pleaseWaitBox = new QMessageBox(
                    QMessageBox::Information, "In Progress",
                    QString("Please wait while the ROM image is read.\n\n"),
                    0, this, Qt::Dialog);
                m_pleaseWaitBox->setStandardButtons(QMessageBox::Cancel);
                connect(m_pleaseWaitBox, SIGNAL(finished(int)), this, SLOT(onROMReadCancelled()));
            }
            m_pleaseWaitBox->show();

            emit requestROMImage();
        }
    }
    else
    {
        QMessageBox::warning(this, "Error",
            "This requires that the software first be connected to the ECU (using the \"Connect\" button.)",
            QMessageBox::Ok);
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
    if (m_pleaseWaitBox != 0)
    {
        m_pleaseWaitBox->hide();
    }

    QByteArray *promData = m_cux->getROMImage();

    if (promData != 0)
    {
        QString saveFileName =
                QFileDialog::getSaveFileName(this, "Select output file for ROM image:");

        if (!saveFileName.isNull() && !saveFileName.isEmpty())
        {
            QFile saveFile(saveFileName);

            if (saveFile.open(QIODevice::WriteOnly))
            {
                if (saveFile.write(*promData) != promData->capacity())
                {
                    QMessageBox::warning(this, "Error",
                        QString("Error writing the ROM image file:\n%1").arg(saveFileName), QMessageBox::Ok);
                }

                saveFile.close();
            }
            else
            {
                QMessageBox::warning(this, "Error",
                    QString("Error writing the ROM image file:\n%1").arg(saveFileName), QMessageBox::Ok);
            }
        }
    }
}

/**
 * Displays a message box indicating that reading the ROM image has failed.
 */
void MainWindow::onROMImageReadFailed()
{
    if (m_pleaseWaitBox != 0)
    {
        m_pleaseWaitBox->hide();
    }

    QMessageBox::warning(this, "Error",
        "Communications error. ROM image could not be read.", QMessageBox::Ok);
}

/**
 * Starts a timer that periodically re-sends the signal to run the fuel
 * pump, thus keeping the pump running continuously.
 */
void MainWindow::onFuelPumpContinuous()
{
    if (m_ui->m_fuelPumpContinuousButton->isChecked())
    {
        emit requestFuelPumpRun();
        m_fuelPumpRefreshTimer->start();
        m_ui->m_fuelPumpOneshotButton->setEnabled(false);
    }
    else
    {
        m_fuelPumpRefreshTimer->stop();
        m_ui->m_fuelPumpOneshotButton->setEnabled(true);
    }
}

/**
 * Displays the idle-air-control dialog.
 */
void MainWindow::onIdleAirControlClicked()
{
    m_iacDialog->show();
}

/**
 * Sets the type of lambda trim to read from the ECU.
 */
void MainWindow::onLambdaTrimButtonClicked(QAbstractButton *button)
{
    if (button == m_ui->m_lambdaTrimShortButton)
    {
        m_cux->setLambdaTrimType(C14CUX_LambdaTrimType_ShortTerm);
    }
    else
    {
        m_cux->setLambdaTrimType(C14CUX_LambdaTrimType_LongTerm);
    }
}

/**
 * Sets the type of MAF reading to read from the ECU.
 */
void MainWindow::onMAFReadingButtonClicked(QAbstractButton *button)
{
    if (button == m_ui->m_mafReadingLinearButton)
    {
        m_cux->setMAFReadingType(C14CUX_AirflowType_Linearized);
    }
    else
    {
        m_cux->setMAFReadingType(C14CUX_AirflowType_Direct);
    }
}

/**
 * Sets the type of throttle position to display.
 */
void MainWindow::onThrottleTypeButtonClicked(QAbstractButton *button)
{
    if (button == m_ui->m_throttleTypeAbsoluteButton)
    {
        m_cux->setThrottleReadingType(C14CUX_ThrottlePosType_Absolute);
    }
    else
    {
        m_cux->setThrottleReadingType(C14CUX_ThrottlePosType_Corrected);
    }
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
    m_ui->m_checksumFixerLabel->setText(QString("Checksum fixer: ") + QString("%1").arg(checksumFixer, 2, 16, QChar('0')).toUpper());
}

/**
 * Paints a redline on the tachometer, based on the electronic RPM limit
 * @param rpmLimit Engine speed limit in RPM
 */
void MainWindow::onRPMLimitReady(int rpmLimit)
{
    m_ui->m_revCounter->setCritical((double)rpmLimit);
}

/**
 * Sets the fuel map column header text to reflect the engine speed thresholds
 * used in the ECU
 */
void MainWindow::onRPMTableReady()
{
    c14cux_rpmtable table = m_cux->getRPMTable();
    for (int col = 0; col < FUEL_MAP_COLUMNS; col++)
    {
        m_ui->m_fuelMapDisplay->horizontalHeaderItem(col)->setText(QString::number(table.rpm[col]));
    }
}

/**
 * Called when the fueling feedback mode (open- vs closed-loop) changes.
 * @param mode The new feedback mode
 */
void MainWindow::onFeedbackModeChanged(c14cux_feedback_mode mode)
{
    if (mode == C14CUX_FeedbackMode_OpenLoop)
    {
        m_ui->m_lambdaTrimTypeLabel->setEnabled(false);
        m_ui->m_lambdaTrimLongButton->setEnabled(false);
        m_ui->m_lambdaTrimShortButton->setEnabled(false);
        m_ui->m_lambdaTrimHighLimitLabel->setVisible(false);
        m_ui->m_lambdaTrimLowLimitLabel->setVisible(false);
        m_ui->m_evenFuelTrimBar->setVisible(false);
        m_ui->m_evenFuelTrimBarLabel->setVisible(false);
        m_ui->m_evenFuelTrimLabel->setVisible(false);
        m_ui->m_oddFuelTrimBar->setVisible(false);
        m_ui->m_oddFuelTrimLabel->setText("MAF CO trim:");
    }
    else
    {
        m_ui->m_lambdaTrimTypeLabel->setEnabled(true);
        m_ui->m_lambdaTrimLongButton->setEnabled(true);
        m_ui->m_lambdaTrimShortButton->setEnabled(true);
        m_ui->m_lambdaTrimHighLimitLabel->setVisible(true);
        m_ui->m_lambdaTrimLowLimitLabel->setVisible(true);
        m_ui->m_evenFuelTrimBar->setVisible(true);
        m_ui->m_evenFuelTrimBarLabel->setVisible(true);
        m_ui->m_evenFuelTrimLabel->setVisible(true);
        m_ui->m_oddFuelTrimBar->setVisible(true);
        m_ui->m_oddFuelTrimLabel->setText("Lambda fuel trim (odd):");
    }
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

    QByteArray *fuelMapData = m_cux->getFuelMap(fuelMapId);

    if (fuelMapData != 0)
    {
        populateFuelMapDisplay(fuelMapData, m_cux->getFuelMapAdjustmentFactor(fuelMapId));
        m_fuelMapDataIsCurrent = true;
    }
    else
    {
        // The data for the current fuel map hasn't been read out of the
        // ECU yet, so put in a request. We'll update the display when
        // we receive the signal that the new data is ready.
        m_fuelMapDataIsCurrent = false;
        emit requestFuelMapData(fuelMapId);
    }
}

#ifdef ENABLE_FORCE_OPEN_LOOP
/**
 * Changes the state of the "force open loop" checkbox depending on the state of a bit in the ECU
 * @param forceOpen True when the bit is set, false otherwise
 */
void MainWindow::onForceOpenLoopStateReceived(bool forceOpen)
{
    m_ui->m_forceOpenLoopCheckbox->setChecked(forceOpen);
}
#endif

#ifdef ENABLE_SIM_MODE
void MainWindow::onSimDialogClicked()
{
    if (m_simDialog == 0)
    {
        m_simDialog = new SimulationModeDialog(QString(this->windowTitle() + " - Simulation Mode"), this);
        connect(m_cux, SIGNAL(simModeWriteSuccess()), m_simDialog, SLOT(onWriteSuccess()));
        connect(m_cux, SIGNAL(simModeWriteFailure()), m_simDialog, SLOT(onWriteFailure()));
        connect(m_simDialog, SIGNAL(writeSimulationInputValues(bool,SimulationInputValues,SimulationInputChanges)),
                m_cux, SLOT(onSimModeWriteRequest(bool,SimulationInputValues,SimulationInputChanges)));

    }
    m_simDialog->show();
}
#endif
