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
      m_currentFuelMapIndex(0),
      m_currentFuelMapRow(0),
      m_currentFuelMapCol(0),
      m_fuelMapIndexIsCurrent(false),
      m_fuelMapRowColumnIsCurrent(false),
      m_waitingForFuelMapData(false)
{
    buildSpeedAndTempUnitTables();
    m_ui->setupUi(this);
    this->setWindowTitle("RoverGauge");

    m_options = new OptionsDialog(this->windowTitle(), this);
    m_cux = new CUXInterface(m_options->getSerialDeviceName(), m_options->getSpeedUnits(),
                             m_options->getTemperatureUnits(), m_options->getRefreshFuelMap());

    m_enabledSamples = m_options->getEnabledSamples();
    m_cux->setEnabledSamples(m_enabledSamples);

    m_iacDialog = new IdleAirControlDialog(this->windowTitle(), this);
    connect(m_iacDialog, SIGNAL(requestIdleAirControlMovement(int,int)),
            m_cux, SLOT(onIdleAirControlMovementRequest(int,int)));

    m_logger = new Logger(m_cux);

    m_fuelPumpRefreshTimer = new QTimer(this);
    m_fuelPumpRefreshTimer->setInterval(1000);

    connect(m_cux, SIGNAL(dataReady()), this, SLOT(onDataReady()));
    connect(m_cux, SIGNAL(connected()), this, SLOT(onConnect()));
    connect(m_cux, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(m_cux, SIGNAL(readError()), this, SLOT(onReadError()));
    connect(m_cux, SIGNAL(readSuccess()), this, SLOT(onReadSuccess()));
    connect(m_cux, SIGNAL(failedToConnect(QString)), this, SLOT(onFailedToConnect(QString)));
    connect(m_cux, SIGNAL(faultCodesReady()), this, SLOT(onFaultCodesReady()));
    connect(m_cux, SIGNAL(faultCodesReadFailed()), this, SLOT(onFaultCodesReadFailed()));
    connect(m_cux, SIGNAL(fuelMapReady(unsigned int)), this, SLOT(onFuelMapDataReady(unsigned int)));
    connect(m_cux, SIGNAL(revisionNumberReady(int)), this, SLOT(onTuneRevisionReady(int)));
    connect(m_cux, SIGNAL(interfaceReadyForPolling()), this, SLOT(onInterfaceReady()));
    connect(m_cux, SIGNAL(notConnected()), this, SLOT(onNotConnected()));
    connect(m_cux, SIGNAL(romImageReady()), this, SLOT(onROMImageReady()));
    connect(m_cux, SIGNAL(romImageReadFailed()), this, SLOT(onROMImageReadFailed()));
    connect(m_cux, SIGNAL(rpmLimitReady(int)), this, SLOT(onRPMLimitReady(int)));
    connect(m_cux, SIGNAL(rpmTableReady()), this, SLOT(onRPMTableReady()));
#ifdef ENABLE_FORCE_OPEN_LOOP
    connect(m_cux, SIGNAL(forceOpenLoopState(bool)), this, SLOT(onForceOpenLoopStateReceived(bool)));
#endif
    connect(m_fuelPumpRefreshTimer, SIGNAL(timeout()), m_cux, SLOT(onFuelPumpRunRequest()));
    connect(this, SIGNAL(requestToStartPolling()), m_cux, SLOT(onStartPollingRequest()));
    connect(this, SIGNAL(requestThreadShutdown()), m_cux, SLOT(onShutdownThreadRequest()));
    connect(this, SIGNAL(requestFuelMapData(unsigned int)), m_cux, SLOT(onFuelMapRequested(unsigned int)));
    connect(this, SIGNAL(requestROMImage()), m_cux, SLOT(onReadROMImageRequested()));

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
    m_speedUnitSuffix->insert(FPS, " ft/s");
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
    m_optionsMenu->addSeparator();
    m_simDialogAction = m_optionsMenu->addAction("&Simulation mode control...");
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
    m_ui->m_fuelMapDisplay->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    m_ui->m_fuelMapDisplay->verticalHeader()->setResizeMode(QHeaderView::Stretch);
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

        highlightActiveFuelMapCell();
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
        m_waitingForFuelMapData = false;
        populateFuelMapDisplay(data, m_cux->getFuelMapAdjustmentFactor(fuelMapId));
    }
}

/**
 * Updates the gauges and indicators with the latest data available from
 * the ECU.
 */
void MainWindow::onDataReady()
{
    m_ui->m_milLed->setChecked(m_cux->isMILOn());

    // if fuel map display updates are enabled...
    if (m_enabledSamples[SampleType_FuelMap])
    {
        int newFuelMapIndex = m_cux->getCurrentFuelMapIndex();
        int newFuelMapRow = m_cux->getFuelMapRowIndex();
        int newFuelMapCol = m_cux->getFuelMapColumnIndex();
        QByteArray *fuelMapData = 0;

        // if the active fuel map has changed, prepare to update the display
        if (((m_currentFuelMapIndex != newFuelMapIndex) || !m_fuelMapIndexIsCurrent) &&
            !m_waitingForFuelMapData)
        {
            m_currentFuelMapIndex = newFuelMapIndex;
            m_fuelMapIndexIsCurrent = true;
            m_ui->m_fuelMapIndexLabel->setText(QString("Current fuel map: %1").arg(m_currentFuelMapIndex));
            fuelMapData = m_cux->getFuelMap(m_currentFuelMapIndex);

            if (fuelMapData != 0)
            {
                populateFuelMapDisplay(fuelMapData, m_currentFuelMapIndex);
            }
            else
            {
                // The data for the current fuel map hasn't been read out of the
                // ECU yet, so put in a request. We'll update the display when
                // we receive the signal that the new data is ready.
                m_waitingForFuelMapData = true;
                emit requestFuelMapData(m_currentFuelMapIndex);
            }
        }

        // if the row/column index into the fuel map has changed
        if ((m_currentFuelMapRow != newFuelMapRow) || (m_currentFuelMapCol != newFuelMapCol) || !m_fuelMapRowColumnIsCurrent)
        {
            // if the fuel map data hasn't been retrieved on this pass, that means
            // that the fuel map itself hasn't changed and the currently-displayed
            // map needs an update
            if ((fuelMapData == 0) &&
                (m_currentFuelMapRow < m_ui->m_fuelMapDisplay->rowCount()) &&
                (m_currentFuelMapCol < m_ui->m_fuelMapDisplay->columnCount()))
            {
                // set the currently-highlighted cell back to its original colors
                QTableWidgetItem *item = m_ui->m_fuelMapDisplay->item(m_currentFuelMapRow, m_currentFuelMapCol);
                bool ok = false;
                unsigned char value = (unsigned char)(item->text().toInt(&ok, 16));
                if (ok)
                {
                    item->setBackgroundColor(getColorForFuelMapCell(value));
                    item->setTextColor(Qt::black);
                }
            }

            m_currentFuelMapRow = newFuelMapRow;
            m_currentFuelMapCol = newFuelMapCol;

            highlightActiveFuelMapCell();
        }
    }

    if (m_enabledSamples[SampleType_Throttle])
        m_ui->m_throttleBar->setValue(m_cux->getThrottlePos() * 100);

    if (m_enabledSamples[SampleType_MAF])
        m_ui->m_mafReadingBar->setValue(m_cux->getMAFReading() * 100);

    if (m_enabledSamples[SampleType_IdleBypassPosition])
        m_ui->m_idleBypassPosBar->setValue(m_cux->getIdleBypassPos() * 100);

    if (m_enabledSamples[SampleType_RoadSpeed])
        m_ui->m_speedo->setValue(m_cux->getRoadSpeed());

    if (m_enabledSamples[SampleType_EngineRPM])
        m_ui->m_revCounter->setValue(m_cux->getEngineSpeedRPM());

    if (m_enabledSamples[SampleType_EngineTemperature])
        m_ui->m_waterTempGauge->setValue(m_cux->getCoolantTemp());

    if (m_enabledSamples[SampleType_FuelTemperature])
        m_ui->m_fuelTempGauge->setValue(m_cux->getFuelTemp());

    if (m_enabledSamples[SampleType_MainVoltage])
        m_ui->m_voltage->setText(QString::number(m_cux->getMainVoltage(), 'f', 1) + "VDC");

    if (m_enabledSamples[SampleType_FuelPumpRelay])
        m_ui->m_fuelPumpRelayStateLed->setChecked(m_cux->getFuelPumpRelayState());

    if (m_enabledSamples[SampleType_TargetIdleRPM])
    {
        int targetIdleSpeedRPM = m_cux->getTargetIdleSpeed();

        if (targetIdleSpeedRPM > 0)
            m_ui->m_targetIdle->setText(QString::number(targetIdleSpeedRPM));
        else
            m_ui->m_targetIdle->setText("");

        m_ui->m_idleModeLed->setChecked(m_cux->getIdleMode());
    }

    if (m_enabledSamples[SampleType_LambdaTrim])
        setLambdaTrimIndicators(m_cux->getLeftLambdaTrim(), m_cux->getRightLambdaTrim());

    if (m_enabledSamples[SampleType_GearSelection])
        setGearLabel(m_cux->getGear());

    m_logger->logData();
}

/**
 * Sets the lambda fuel trim indicators to the provided values
 */
void MainWindow::setLambdaTrimIndicators(int leftLambdaTrim, int rightLambdaTrim)
{
    // only fuel maps 0, 4, and 5 operate in a closed-loop manner,
    // so the lambda feedback will only be relevant if one of those
    // maps is selected
    if ((m_currentFuelMapIndex == 0) ||
        (m_currentFuelMapIndex == 4) ||
        (m_currentFuelMapIndex == 5))
    {        
        QString leftLabel = (leftLambdaTrim >= 0) ?
            QString("+%1%").arg(leftLambdaTrim * 100 / m_ui->m_leftFuelTrimBar->maximum()) :
            QString("-%1%").arg(leftLambdaTrim * 100 / m_ui->m_leftFuelTrimBar->minimum());
        QString rightLabel = (rightLambdaTrim >= 0) ?
            QString("+%1%").arg(rightLambdaTrim * 100 / m_ui->m_rightFuelTrimBar->maximum()) :
            QString("-%1%").arg(rightLambdaTrim * 100 / m_ui->m_rightFuelTrimBar->minimum());

        m_ui->m_leftFuelTrimBar->setEnabled(true);
        m_ui->m_leftFuelTrimBar->setValue(leftLambdaTrim);
        m_ui->m_rightFuelTrimBar->setEnabled(true);
        m_ui->m_rightFuelTrimBar->setValue(rightLambdaTrim);

        m_ui->m_leftFuelTrimBarLabel->setText(leftLabel);
        m_ui->m_rightFuelTrimBarLabel->setText(rightLabel);
    }
    else
    {
        m_ui->m_leftFuelTrimBar->setValue(0);
        m_ui->m_leftFuelTrimBar->setEnabled(false);
        m_ui->m_rightFuelTrimBar->setValue(0);
        m_ui->m_rightFuelTrimBar->setEnabled(false);

        m_ui->m_leftFuelTrimBarLabel->setText("+0%");
        m_ui->m_rightFuelTrimBarLabel->setText("+0%");
    }
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
 * Paints a highlight on the fuel map display cell that represents the
 * most-recently-read fueling index.
 */
void MainWindow::highlightActiveFuelMapCell()
{
    if ((m_currentFuelMapRow < m_ui->m_fuelMapDisplay->rowCount()) &&
        (m_currentFuelMapCol < m_ui->m_fuelMapDisplay->columnCount()))
    {
        QTableWidgetItem *item = m_ui->m_fuelMapDisplay->item(m_currentFuelMapRow, m_currentFuelMapCol);
        item->setBackgroundColor(Qt::black);
        item->setTextColor(Qt::white);
    }
}

/**
 * Generates a color whose intensity corresponds to a fueling value
 * @return Color that corresponds to a particular fuel map fueling value
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

        // the fields are updated one at a time, because a replacement of the entire
        // hash table (using the assignment operator) can disrupt other threads that
        // are reading the table at that time
        QHash<SampleType,bool> samples = m_options->getEnabledSamples();
        foreach (SampleType field, samples.keys())
        {
            m_enabledSamples[field] = samples[field];
        }

        m_cux->setEnabledSamples(m_enabledSamples);

        dimUnusedControls();

        // if the user changed the serial device name and/or the polling
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

    enabled = m_enabledSamples[SampleType_LambdaTrim];
    m_ui->m_lambdaTrimTypeLabel->setEnabled(enabled);
    m_ui->m_lambdaTrimLowLimitLabel->setEnabled(enabled);
    m_ui->m_lambdaTrimHighLimitLabel->setEnabled(enabled);
    m_ui->m_lambdaTrimShortButton->setEnabled(enabled);
    m_ui->m_lambdaTrimLongButton->setEnabled(enabled);
    m_ui->m_leftFuelTrimBar->setEnabled(enabled);
    m_ui->m_leftFuelTrimLabel->setEnabled(enabled);
    m_ui->m_leftFuelTrimBarLabel->setEnabled(enabled);
    m_ui->m_rightFuelTrimBar->setEnabled(enabled);
    m_ui->m_rightFuelTrimBarLabel->setEnabled(enabled);
    m_ui->m_rightFuelTrimLabel->setEnabled(enabled);
    if (!enabled)
    {
        m_ui->m_leftFuelTrimBar->setValue(0);
        m_ui->m_leftFuelTrimBarLabel->setText("");
        m_ui->m_rightFuelTrimBar->setValue(0);
        m_ui->m_rightFuelTrimBarLabel->setText("");
    }

    enabled = m_enabledSamples[SampleType_FuelPumpRelay];
    m_ui->m_fuelPumpRelayStateLabel->setEnabled(enabled);
    m_ui->m_fuelPumpRelayStateLed->setEnabled(enabled);
    m_fuelPumpLedOpacity->setEnabled(!enabled);

    enabled = m_enabledSamples[SampleType_FuelMap];
    m_ui->m_fuelMapIndexLabel->setEnabled(enabled);
    m_ui->m_fuelMapFactorLabel->setEnabled(enabled);
    m_fuelMapOpacity->setEnabled(!enabled);

    // These controls are shown in a disabled state by applying a 50% opacity
    // graphical effect; the 'enabled' bit is therefore inverted because it's
    // controlling the state of the graphical effect (rather than the widget).
    enabled = m_enabledSamples[SampleType_EngineTemperature];
    m_waterTempGaugeOpacity->setEnabled(!enabled);
    m_ui->m_waterTempLabel->setEnabled(enabled);
    enabled = m_enabledSamples[SampleType_FuelTemperature];
    m_fuelTempGaugeOpacity->setEnabled(!enabled);
    m_ui->m_fuelTempLabel->setEnabled(enabled);

    m_revCounterOpacity->setEnabled(!m_enabledSamples[SampleType_EngineRPM]);
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
    m_ui->m_leftFuelTrimBar->setValue(0);
    m_ui->m_leftFuelTrimBarLabel->setText("+0%");
    m_ui->m_rightFuelTrimBar->setValue(0);
    m_ui->m_rightFuelTrimBarLabel->setText("+0%");

    m_ui->m_leftFuelTrimBar->repaint();
    m_ui->m_rightFuelTrimBar->repaint();

    m_currentFuelMapIndex = 0;
    m_currentFuelMapRow = 0;
    m_currentFuelMapCol = 0;
    m_fuelMapIndexIsCurrent = false;
    m_fuelMapRowColumnIsCurrent = false;
    m_cux->invalidateFuelMapData(m_currentFuelMapIndex);
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
 * Requests the PROM image so that it can be saved to disk.
 */
void MainWindow::onSaveROMImageSelected()
{
    sendROMImageRequest(
        QString("Read the ROM image from the ECU? This will take approximately 25 seconds."));
}

/**
 * Prompts the user to continue, and sends a request to read the PROM image.
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
                connect(m_pleaseWaitBox, SIGNAL(finished(int)), this, SLOT(onPROMReadCancelled()));
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
 * Sets a flag that indicates we should ignore any PROM image that is returned.
 */
void MainWindow::onROMReadCancelled()
{
    m_cux->cancelRead();
}

/**
 * Prompts the user for a file in which to save the PROM image.
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
                QFileDialog::getSaveFileName(this, "Select output file for PROM image:");

        if (!saveFileName.isNull() && !saveFileName.isEmpty())
        {
            QFile saveFile(saveFileName);

            if (saveFile.open(QIODevice::WriteOnly))
            {
                if (saveFile.write(*promData) != promData->capacity())
                {
                    QMessageBox::warning(this, "Error",
                        QString("Error writing the PROM image file:\n%1").arg(saveFileName), QMessageBox::Ok);
                }

                saveFile.close();
            }
            else
            {
                QMessageBox::warning(this, "Error",
                    QString("Error writing the PROM image file:\n%1").arg(saveFileName), QMessageBox::Ok);
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
        m_cux->onFuelPumpRunRequest();
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
 * @param Set to 1 for short-term, 2 for long-term
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
 * @param Set to 1 for Linearized, 2 for Direct
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
 * @param Set to 1 for Absolute, 2 for Corrected
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
 * Displays the tune revision number (read from the PROM)
 * @param tuneRevisionNum Decimal representation of the revision number
 */
void MainWindow::onTuneRevisionReady(int tuneRevisionNum)
{
    m_ui->m_tuneRevNumberLabel->setText(QString("Tune: R%04").arg(tuneRevisionNum));
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

