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
      m_currentFuelMapIndex(-1),
      m_currentFuelMapRow(-1),
      m_currentFuelMapCol(-1),
      m_widthPixels(970),
      m_heightPixels(620)
{
    buildSpeedAndTempUnitTables();

    QDesktopWidget desktop;
    const QRect screenGeo = desktop.screenGeometry();
    if ((screenGeo.height() * 0.95) < m_heightPixels)
    {
        m_heightPixels = screenGeo.height() * 0.9;
    }
    if ((screenGeo.width() * 0.95) < m_widthPixels)
    {
        m_widthPixels = screenGeo.width() * 0.9;
    }

    m_ui->setupUi(this);
    this->setWindowTitle("RoverGauge");
    this->setMinimumSize(m_widthPixels, m_heightPixels);

    m_options = new OptionsDialog(this->windowTitle(), this);
    m_cux = new CUXInterface(m_options->getSerialDeviceName(), m_options->getSpeedUnits(),
                             m_options->getTemperatureUnits());

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
    connect(m_cux, SIGNAL(fuelMapReady(int)), this, SLOT(onFuelMapDataReady(int)));
    connect(m_cux, SIGNAL(revisionNumberReady(int)), this, SLOT(onTuneRevisionReady(int)));
    connect(m_cux, SIGNAL(interfaceReadyForPolling()), this, SLOT(onInterfaceReady()));
    connect(m_cux, SIGNAL(notConnected()), this, SLOT(onNotConnected()));
    connect(m_cux, SIGNAL(promImageReady()), this, SLOT(onPROMImageReady()));
    connect(m_cux, SIGNAL(promImageReadFailed()), this, SLOT(onPROMImageReadFailed()));
    connect(m_cux, SIGNAL(rpmLimitReady(int)), this, SLOT(onRPMLimitReady(int)));
    connect(this, SIGNAL(requestToStartPolling()), m_cux, SLOT(onStartPollingRequest()));
    connect(this, SIGNAL(requestThreadShutdown()), m_cux, SLOT(onShutdownThreadRequest()));
    connect(this, SIGNAL(requestFuelMapData(int)), m_cux, SLOT(onFuelMapRequested(int)));
    connect(this, SIGNAL(requestPROMImage()), m_cux, SLOT(onReadPROMImageRequested()));
    connect(this, SIGNAL(requestFuelPumpRun()), m_cux, SLOT(onFuelPumpRunRequest()));
    connect(m_fuelPumpRefreshTimer, SIGNAL(timeout()), this, SLOT(onFuelPumpRefreshTimer()));

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
 * Sets up the layout of the main window.
 */
void MainWindow::setupLayout()
{
    /*
    m_layout = new QVBoxLayout(m_ui->centralWidget);

    m_aboveGaugesRow = new QHBoxLayout();
    m_layout->addLayout(m_aboveGaugesRow);

    m_connectionButtonLayout = new QHBoxLayout();
    m_aboveGaugesRow->addLayout(m_connectionButtonLayout);

    m_commsLedLayout = new QHBoxLayout();
    m_commsLedLayout->setAlignment(Qt::AlignRight);
    m_aboveGaugesRow->addLayout(m_commsLedLayout);

    m_verticalLineC = new QFrame(this);
    m_verticalLineC->setFrameShape(QFrame::VLine);
    m_verticalLineC->setFrameShadow(QFrame::Sunken);

    m_verticalLineB = new QFrame(this);
    m_verticalLineB->setFrameShape(QFrame::VLine);
    m_verticalLineB->setFrameShadow(QFrame::Sunken);

    m_horizontalLineA = new QFrame(this);
    m_horizontalLineA->setFrameShape(QFrame::HLine);
    m_horizontalLineA->setFrameShadow(QFrame::Sunken);
    m_layout->addWidget(m_horizontalLineA);

    m_gaugesLayout = new QHBoxLayout();
    m_layout->addLayout(m_gaugesLayout);

    m_horizontalLineB = new QFrame(this);
    m_horizontalLineB->setFrameShape(QFrame::HLine);
    m_horizontalLineB->setFrameShadow(QFrame::Sunken);
    m_layout->addWidget(m_horizontalLineB);

    m_horizontalLineC = new QFrame(this);
    m_horizontalLineC->setFrameShape(QFrame::HLine);
    m_horizontalLineC->setFrameShadow(QFrame::Sunken);

    m_belowGaugesRow = new QHBoxLayout();
    m_layout->addLayout(m_belowGaugesRow);

    m_waterTempLayout = new QVBoxLayout();
    m_gaugesLayout->addLayout(m_waterTempLayout);

    m_speedoLayout = new QVBoxLayout();
    m_gaugesLayout->addLayout(m_speedoLayout);

    m_revCounterLayout = new QVBoxLayout();
    m_gaugesLayout->addLayout(m_revCounterLayout);

    m_fuelTempLayout = new QVBoxLayout();
    m_gaugesLayout->addLayout(m_fuelTempLayout);

    m_belowGaugesLeft = new QGridLayout();
    m_belowGaugesRow->addLayout(m_belowGaugesLeft);

    m_idleSpeedLayout = new QHBoxLayout();

    m_verticalLineA = new QFrame(this);
    m_verticalLineA->setFrameShape(QFrame::VLine);
    m_verticalLineA->setFrameShadow(QFrame::Sunken);
    m_belowGaugesRow->addWidget(m_verticalLineA);

    m_belowGaugesRight = new QGridLayout();
    m_belowGaugesRow->addLayout(m_belowGaugesRight);
    */
}

/**
 * Instantiates widgets used in the main window.
 */
void MainWindow::createWidgets()
{
    /*
    m_fileMenu = menuBar()->addMenu("&File");
    m_savePROMImageAction = m_fileMenu->addAction("&Save PROM image...");
    m_savePROMImageAction->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(m_savePROMImageAction, SIGNAL(triggered()), this, SLOT(onSavePROMImageSelected()));
    m_fileMenu->addSeparator();
    m_exitAction = m_fileMenu->addAction("E&xit");
    m_exitAction->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    connect(m_exitAction, SIGNAL(triggered()), this, SLOT(onExitSelected()));

    m_optionsMenu = menuBar()->addMenu("&Options");
    m_showFaultsAction = m_optionsMenu->addAction("Show fault &codes...");
    m_showFaultsAction->setIcon(style()->standardIcon(QStyle::SP_DialogNoButton));
    connect(m_showFaultsAction, SIGNAL(triggered()), m_cux, SLOT(onFaultCodesRequested()));
    m_showIdleAirControlDialog = m_optionsMenu->addAction("&Idle air control...");
    connect(m_showIdleAirControlDialog, SIGNAL(triggered()), this, SLOT(onIdleAirControlClicked()));
    m_editOptionsAction = m_optionsMenu->addAction("&Edit settings...");
    m_editOptionsAction->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));    
    connect(m_editOptionsAction, SIGNAL(triggered()), this, SLOT(onEditOptionsClicked()));
#ifdef ENABLE_SIM_MODE
    m_optionsMenu->addSeparator();
    m_simDialogAction = m_optionsMenu->addAction("&Simulation mode control...");
    connect(m_simDialogAction, SIGNAL(triggered()), this, SLOT(onSimDialogClicked()));
#endif

    m_helpMenu = menuBar()->addMenu("&Help");

    m_helpAction = m_helpMenu->addAction("&Contents...");
    m_helpAction->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));
    connect(m_helpAction, SIGNAL(triggered()), this, SLOT(onHelpContentsClicked()));

    m_aboutAction = m_helpMenu->addAction("&About");
    m_aboutAction->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(onHelpAboutClicked()));

    m_connectButton = new QPushButton("Connect", this);
    connect(m_connectButton, SIGNAL(clicked()), this, SLOT(onConnectClicked()));

    m_disconnectButton = new QPushButton("Disconnect", this);
    m_disconnectButton->setEnabled(false);
    connect(m_disconnectButton, SIGNAL(clicked()), this, SLOT(onDisconnectClicked()));

    m_tuneRevNumberLabel = new QLabel("", this);

    m_milLed = new QLedIndicator(this);
    m_milLed->setOnColor1(QColor(255, 0, 0));
    m_milLed->setOnColor2(QColor(176, 0, 2));
    m_milLed->setOffColor1(QColor(20, 0, 0));
    m_milLed->setOffColor2(QColor(90, 0, 2));
    m_milLed->setDisabled(true);

    m_milLabel = new QLabel("MIL:", this);

    m_commsGoodLed = new QLedIndicator(this);
    m_commsGoodLed->setOnColor1(QColor(102, 255, 102));
    m_commsGoodLed->setOnColor2(QColor(82, 204, 82));
    m_commsGoodLed->setOffColor1(QColor(0, 102, 0));
    m_commsGoodLed->setOffColor2(QColor(0, 51, 0));
    m_commsGoodLed->setDisabled(true);

    m_commsBadLed = new QLedIndicator(this);
    m_commsBadLed->setOnColor1(QColor(255, 0, 0));
    m_commsBadLed->setOnColor2(QColor(176, 0, 2));
    m_commsBadLed->setOffColor1(QColor(20, 0, 0));
    m_commsBadLed->setOffColor2(QColor(90, 0, 2));
    m_commsBadLed->setDisabled(true);

    m_commsLedLabel = new QLabel("Communications:", this);

    m_mafReadingTypeLabel = new QLabel("MAF reading type:", this);
    m_mafReadingLinearButton = new QRadioButton("Linear", this);
    m_mafReadingLinearButton->setChecked(true);
    m_mafReadingDirectButton = new QRadioButton("Direct", this);

    m_mafReadingButtonGroup = new QButtonGroup(this);
    m_mafReadingButtonGroup->addButton(m_mafReadingLinearButton, 1);
    m_mafReadingButtonGroup->addButton(m_mafReadingDirectButton, 2);
    connect(m_mafReadingButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onMAFReadingButtonClicked(int)));

    m_mafReadingLabel = new QLabel("MAF reading:", this);
    m_mafReadingBar = new QProgressBar(this);
    m_mafReadingBar->setRange(0, 100);
    m_mafReadingBar->setValue(0);
    m_mafReadingBar->setMinimumWidth(300);

    m_throttleTypeLabel = new QLabel("Throttle reading type:", this);
    m_throttleTypeAbsoluteButton = new QRadioButton("Absolute", this);
    m_throttleTypeAbsoluteButton->setChecked(true);
    m_throttleTypeCorrectedButton = new QRadioButton("Corrected", this);

    m_throttleTypeButtonGroup = new QButtonGroup(this);
    m_throttleTypeButtonGroup->addButton(m_throttleTypeAbsoluteButton, 1);
    m_throttleTypeButtonGroup->addButton(m_throttleTypeCorrectedButton, 2);
    connect(m_throttleTypeButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onThrottleTypeButtonClicked(int)));

    m_throttleLabel = new QLabel("Throttle position:", this);
    m_throttleBar = new QProgressBar(this);
    m_throttleBar->setRange(0, 100);
    m_throttleBar->setValue(0);
    m_throttleBar->setMinimumWidth(300);

    m_idleBypassLabel = new QLabel("Idle bypass position:", this);
    m_idleBypassPosBar = new QProgressBar(this);
    m_idleBypassPosBar->setRange(0, 100);
    m_idleBypassPosBar->setValue(0);
    m_idleBypassPosBar->setMinimumWidth(300);

    m_lambdaTrimTypeLabel = new QLabel("Lambda trim type:", this);
    m_lambdaTrimShortButton = new QRadioButton("Short term", this);
    m_lambdaTrimShortButton->setChecked(true);
    m_lambdaTrimLongButton = new QRadioButton("Long term", this);

    m_lambdaTrimButtonGroup = new QButtonGroup(this);
    m_lambdaTrimButtonGroup->addButton(m_lambdaTrimShortButton, 1);
    m_lambdaTrimButtonGroup->addButton(m_lambdaTrimLongButton, 2);
    connect(m_lambdaTrimButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onLambdaTrimButtonClicked(int)));

    m_lambdaTrimLowLimitLabel = new QLabel("(Reducing)", this);
    m_lambdaTrimHighLimitLabel = new QLabel("(Increasing)", this);

    m_leftFuelTrimLabel = new QLabel("Lambda fuel trim (left):", this);
    m_leftFuelTrimBar = new FuelTrimBar(this);
    m_leftFuelTrimBar->setValue(0);
    m_leftFuelTrimBarLabel = new QLabel("+0%", this);

    m_rightFuelTrimLabel = new QLabel("Lambda fuel trim (right):", this);
    m_rightFuelTrimBar = new FuelTrimBar(this);
    m_rightFuelTrimBar->setValue(0);
    m_rightFuelTrimBarLabel = new QLabel("+0%", this);

    m_targetIdleLabel = new QLabel("Idle mode / target RPM:", this);
    m_targetIdle = new QLabel("", this);

    m_idleModeLed = new QLedIndicator(this);
    m_idleModeLed->setOnColor1(QColor(102, 255, 102));
    m_idleModeLed->setOnColor2(QColor(82, 204, 82));
    m_idleModeLed->setOffColor1(QColor(0, 102, 0));
    m_idleModeLed->setOffColor2(QColor(0, 51, 0));
    m_idleModeLed->setDisabled(true);

    m_gearLabel = new QLabel("Selected gear:", this);
    m_gear = new QLabel("", this);

    m_voltageLabel = new QLabel("Main voltage:", this);
    m_voltage = new QLabel("", this);

    m_fuelMapIndexLabel = new QLabel("Current fuel map:", this);
    m_fuelMapFactorLabel = new QLabel("Adjustment factor:", this);

    setStyleSheet("QTableWidget {background-color: transparent;}");
    m_fuelMapDisplay = new QTableWidget(8, 16, this);
    m_fuelMapDisplay->verticalHeader()->hide();
    m_fuelMapDisplay->horizontalHeader()->hide();
    m_fuelMapDisplay->resizeColumnsToContents();
    m_fuelMapDisplay->resizeRowsToContents();

    int rowCount = m_fuelMapDisplay->rowCount();
    int colCount = m_fuelMapDisplay->columnCount();
    QTableWidgetItem *item = 0;
    for (int row = 0; row < rowCount; row++)
    {
        for (int col = 0; col < colCount; col++)
        {
            item = new QTableWidgetItem("");
            item->setFlags(0);
            m_fuelMapDisplay->setItem(row, col, item);
        }
    }

    m_fuelPumpRelayStateLabel = new QLabel("Fuel pump relay", this);
    m_fuelPumpRelayStateLed = new QLedIndicator(this);
    m_fuelPumpRelayStateLed->setOnColor1(QColor(102, 255, 102));
    m_fuelPumpRelayStateLed->setOnColor2(QColor(82, 204, 82));
    m_fuelPumpRelayStateLed->setOffColor1(QColor(0, 102, 0));
    m_fuelPumpRelayStateLed->setOffColor2(QColor(0, 51, 0));
    m_fuelPumpRelayStateLed->setDisabled(true);

    m_fuelPumpOneshotButton = new QPushButton("Run pump (one shot)");
    m_fuelPumpOneshotButton->setEnabled(false);
    connect(m_fuelPumpOneshotButton, SIGNAL(clicked()), this, SLOT(onFuelPumpOneshot()));

    m_fuelPumpContinuousButton = new QPushButton("Run pump (continuous)");
    m_fuelPumpContinuousButton->setEnabled(false);
    m_fuelPumpContinuousButton->setCheckable(true);
    connect(m_fuelPumpContinuousButton, SIGNAL(clicked()), this, SLOT(onFuelPumpContinuous()));

    m_logFileNameLabel = new QLabel("Log file name:", this);
    m_logFileNameBox = new QLineEdit(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss"), this);
    m_startLoggingButton = new QPushButton("Start logging");
    m_startLoggingButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_stopLoggingButton = new QPushButton("Stop logging");
    m_stopLoggingButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    m_stopLoggingButton->setEnabled(false);
    connect(m_startLoggingButton, SIGNAL(clicked()), this, SLOT(onStartLogging()));
    connect(m_stopLoggingButton, SIGNAL(clicked()), this, SLOT(onStopLogging()));

    m_speedo = new ManoMeter(this);

    SpeedUnits speedUnit = m_options->getSpeedUnits();

    m_speedo->setMinimum(0.0);
    if (speedUnit == MPH)
    {
        m_speedo->setMaximum(speedometerMaxMPH);
    }
    else
    {
        m_speedo->setMaximum(speedometerMaxKPH);
    }
    m_speedo->setSuffix(m_speedUnitSuffix->value(speedUnit));
    m_speedo->setNominal(1000.0);
    m_speedo->setCritical(1000.0);

    m_revCounter = new ManoMeter(this);
    m_revCounter->setMinimum(0.0);
    m_revCounter->setMaximum(8000);
    m_revCounter->setSuffix(" RPM");
    m_revCounter->setNominal(100000.0);
    m_revCounter->setCritical(8000);

    TemperatureUnits tempUnits = m_options->getTemperatureUnits();
    int tempMin = m_tempRange->value(tempUnits).first;
    int tempMax = m_tempRange->value(tempUnits).second;

    m_waterTempGauge = new ManoMeter(this);
    m_waterTempGauge->setValue(tempMin);
    m_waterTempGauge->setMaximum(tempMax);
    m_waterTempGauge->setMinimum(tempMin);
    m_waterTempGauge->setSuffix(m_tempUnitSuffix->value(tempUnits));
    m_waterTempGauge->setNominal(m_tempLimits->value(tempUnits).first);
    m_waterTempGauge->setCritical(m_tempLimits->value(tempUnits).second);

    m_waterTempLabel = new QLabel("Engine Temperature", this);

    m_fuelTempGauge = new ManoMeter(this);
    m_fuelTempGauge->setValue(tempMin);
    m_fuelTempGauge->setMaximum(tempMax);
    m_fuelTempGauge->setMinimum(tempMin);
    m_fuelTempGauge->setSuffix(m_tempUnitSuffix->value(tempUnits));
    m_fuelTempGauge->setNominal(10000.0);
    m_fuelTempGauge->setCritical(10000.0);

    m_fuelTempLabel = new QLabel("Fuel Temperature", this);

    m_waterTempGaugeOpacity = new QGraphicsOpacityEffect(this);
    m_waterTempGaugeOpacity->setOpacity(0.5);
    m_waterTempGaugeOpacity->setEnabled(false);
    m_waterTempGauge->setGraphicsEffect(m_waterTempGaugeOpacity);

    m_fuelTempGaugeOpacity = new QGraphicsOpacityEffect(this);
    m_fuelTempGaugeOpacity->setOpacity(0.5);
    m_fuelTempGaugeOpacity->setEnabled(false);
    m_fuelTempGauge->setGraphicsEffect(m_fuelTempGaugeOpacity);

    m_speedometerOpacity = new QGraphicsOpacityEffect(this);
    m_speedometerOpacity->setOpacity(0.5);
    m_speedometerOpacity->setEnabled(false);
    m_speedo->setGraphicsEffect(m_speedometerOpacity);

    m_revCounterOpacity = new QGraphicsOpacityEffect(this);
    m_revCounterOpacity->setOpacity(0.5);
    m_revCounterOpacity->setEnabled(false);
    m_revCounter->setGraphicsEffect(m_revCounterOpacity);

    m_fuelMapOpacity = new QGraphicsOpacityEffect(this);
    m_fuelMapOpacity->setOpacity(0.5);
    m_fuelMapOpacity->setEnabled(false);
    m_fuelMapDisplay->setGraphicsEffect(m_fuelMapOpacity);

    m_fuelPumpLedOpacity = new QGraphicsOpacityEffect(this);
    m_fuelPumpLedOpacity->setOpacity(0.5);
    m_fuelPumpLedOpacity->setEnabled(false);
    m_fuelPumpRelayStateLed->setGraphicsEffect(m_fuelPumpLedOpacity);

    m_idleModeLedOpacity = new QGraphicsOpacityEffect(this);
    m_idleModeLedOpacity->setOpacity(0.5);
    m_idleModeLedOpacity->setEnabled(false);
    m_idleModeLed->setGraphicsEffect(m_idleModeLedOpacity);
    */
}

/**
 * Adds the created widgets to the form's layout
 */
void MainWindow::placeWidgets()
{
    /*
    m_connectionButtonLayout->addWidget(m_connectButton);
    m_connectionButtonLayout->addWidget(m_disconnectButton);

    m_commsLedLayout->addWidget(m_tuneRevNumberLabel);
    m_commsLedLayout->addWidget(m_verticalLineC);
    m_commsLedLayout->addWidget(m_milLabel);
    m_commsLedLayout->addWidget(m_milLed);
    m_commsLedLayout->addWidget(m_verticalLineB);
    m_commsLedLayout->addWidget(m_commsLedLabel);
    m_commsLedLayout->addWidget(m_commsGoodLed);
    m_commsLedLayout->addWidget(m_commsBadLed);

    m_speedoLayout->addWidget(m_speedo);
    m_revCounterLayout->addWidget(m_revCounter);

    m_waterTempLayout->addWidget(m_waterTempGauge);
    m_waterTempLayout->addWidget(m_waterTempLabel, 0, Qt::AlignCenter);

    m_fuelTempLayout->addWidget(m_fuelTempGauge);
    m_fuelTempLayout->addWidget(m_fuelTempLabel, 0, Qt::AlignCenter);

    unsigned char row = 0;

    m_belowGaugesLeft->setColumnStretch(0, 0);
    m_belowGaugesLeft->addWidget(m_mafReadingTypeLabel,    row,   0, 1, 1,  Qt::AlignRight);
    m_belowGaugesLeft->addWidget(m_mafReadingLinearButton, row,   1, 1, 1);
    m_belowGaugesLeft->addWidget(m_mafReadingDirectButton, row++, 2, 1, 1);

    m_belowGaugesLeft->addWidget(m_mafReadingLabel,    row,   0,        Qt::AlignRight);
    m_belowGaugesLeft->addWidget(m_mafReadingBar,      row++, 1, 1, 3);

    m_belowGaugesLeft->addWidget(m_throttleTypeLabel,           row,   0, 1, 1,  Qt::AlignRight);
    m_belowGaugesLeft->addWidget(m_throttleTypeAbsoluteButton,  row,   1, 1, 1);
    m_belowGaugesLeft->addWidget(m_throttleTypeCorrectedButton, row++, 2, 1, 1);

    m_belowGaugesLeft->addWidget(m_throttleLabel,      row,   0,        Qt::AlignRight);
    m_belowGaugesLeft->addWidget(m_throttleBar,        row++, 1, 1, 3);

    m_belowGaugesLeft->addWidget(m_idleBypassLabel,    row,   0,        Qt::AlignRight);
    m_belowGaugesLeft->addWidget(m_idleBypassPosBar,   row++, 1, 1, 3);

    m_belowGaugesLeft->addWidget(m_targetIdleLabel,    row,   0,        Qt::AlignRight);
    m_belowGaugesLeft->addLayout(m_idleSpeedLayout,    row++, 1, 1, 3);
    m_idleSpeedLayout->addWidget(m_idleModeLed);
    m_idleSpeedLayout->addWidget(m_targetIdle);
    m_idleSpeedLayout->addStretch(0);

    m_belowGaugesLeft->addWidget(m_gearLabel,          row,   0,        Qt::AlignRight);
    m_belowGaugesLeft->addWidget(m_gear,               row++, 1, 1, 3);

    m_belowGaugesLeft->addWidget(m_voltageLabel,       row,   0,        Qt::AlignRight);
    m_belowGaugesLeft->addWidget(m_voltage,            row++, 1, 1, 3);

    m_belowGaugesLeft->addWidget(m_lambdaTrimTypeLabel,   row,   0, 1, 1,  Qt::AlignRight);
    m_belowGaugesLeft->addWidget(m_lambdaTrimShortButton, row,   1, 1, 1);
    m_belowGaugesLeft->addWidget(m_lambdaTrimLongButton,  row++, 2, 1, 1);

    m_belowGaugesLeft->addWidget(m_leftFuelTrimLabel,     row,   0, 1, 1,  Qt::AlignRight);
    m_belowGaugesLeft->addWidget(m_leftFuelTrimBarLabel,  row,   1, 1, 1,  Qt::AlignRight);
    m_belowGaugesLeft->addWidget(m_leftFuelTrimBar,       row++, 2, 1, 2);

    m_belowGaugesLeft->addWidget(m_rightFuelTrimLabel,    row,   0, 1, 1,  Qt::AlignRight);
    m_belowGaugesLeft->addWidget(m_rightFuelTrimBarLabel, row,   1, 1, 1,  Qt::AlignRight);
    m_belowGaugesLeft->addWidget(m_rightFuelTrimBar,      row++, 2, 1, 2);

    m_belowGaugesLeft->addWidget(m_lambdaTrimLowLimitLabel,  row,   2, 1, 1, Qt::AlignLeft);
    m_belowGaugesLeft->addWidget(m_lambdaTrimHighLimitLabel, row++, 3, 1, 1, Qt::AlignRight);

    row = 0;
    m_belowGaugesRight->setColumnMinimumWidth(0, 20);
    m_belowGaugesRight->setColumnStretch(0, 0);
    m_belowGaugesRight->addWidget(m_fuelMapIndexLabel,        row,   0, 1, 2);
    m_belowGaugesRight->addWidget(m_fuelMapFactorLabel,       row++, 2, 1, 2);
    m_belowGaugesRight->addWidget(m_fuelMapDisplay,           row++, 0, 1, 4);
    m_belowGaugesRight->addWidget(m_fuelPumpRelayStateLed,    row,   0, 1, 1);
    m_belowGaugesRight->addWidget(m_fuelPumpRelayStateLabel,  row,   1, 1, 1);
    m_belowGaugesRight->addWidget(m_fuelPumpOneshotButton,    row,   2, 1, 1);
    m_belowGaugesRight->addWidget(m_fuelPumpContinuousButton, row++, 3, 1, 1);

    m_belowGaugesRight->addWidget(m_horizontalLineC,    row++, 0, 1, 4);
    m_belowGaugesRight->addWidget(m_logFileNameLabel,   row,   0, 1, 2);
    m_belowGaugesRight->addWidget(m_logFileNameBox,     row++, 2, 1, 2);

    m_belowGaugesRight->addWidget(m_startLoggingButton, row,   2, 1, 1);
    m_belowGaugesRight->addWidget(m_stopLoggingButton,  row++, 3, 1, 1);
    */
}

/**
 * Instantiates widgets, connects to their signals, and places them on the form.
 */
void MainWindow::setupWidgets()
{
    /*
    setupLayout();
    createWidgets();
    placeWidgets();
    */
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
    m_disconnectButton->setEnabled(false);
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
        int rowCount = m_fuelMapDisplay->rowCount();
        int colCount = m_fuelMapDisplay->columnCount();
        QTableWidgetItem *item = 0;
        unsigned char byte = 0;

        // populate all the cells with the data for this map
        for (int row = 0; row < rowCount; row++)
        {
            for (int col = 0; col < colCount; col++)
            {
                item = m_fuelMapDisplay->item(row, col);
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

        // ensure that the cells show the contents, but that
        // they're not larger than necessary
        m_fuelMapDisplay->resizeColumnsToContents();
        m_fuelMapDisplay->resizeRowsToContents();

        QString adjFactorLabel =
            QString("%1").arg(fuelMapAdjustmentFactor, 0, 16).toUpper();
        m_fuelMapFactorLabel->setText(QString("Adjustment factor: 0x") + adjFactorLabel);

        highlightActiveFuelMapCell();
    }
}

/**
 * Uses fuel map data to populate the fuel map display grid.
 * @param ID of the fuel map just retrieved (from 1-5)
 */
void MainWindow::onFuelMapDataReady(int fuelMapId)
{
    QByteArray *data = m_cux->getFuelMap(fuelMapId);
    if (data != 0)
    {
        populateFuelMapDisplay(data, m_cux->getFuelMapAdjustmentFactor(fuelMapId));
    }
}

/**
 * Updates the gauges and indicators with the latest data available from
 * the ECU.
 */
void MainWindow::onDataReady()
{
    m_milLed->setChecked(m_cux->isMILOn());

    // if fuel map display updates are enabled...
    if (m_enabledSamples[SampleType_FuelMap])
    {
        int newFuelMapIndex = m_cux->getCurrentFuelMapIndex();
        int newFuelMapRow = m_cux->getFuelMapRowIndex();
        int newFuelMapCol = m_cux->getFuelMapColumnIndex();
        QByteArray *fuelMapData = 0;

        // if the active fuel map has changed, prepare to update the display
        if (m_currentFuelMapIndex != newFuelMapIndex)
        {
            m_currentFuelMapIndex = newFuelMapIndex;
            m_fuelMapIndexLabel->setText(QString("Current fuel map: %1").arg(m_currentFuelMapIndex));
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
                emit requestFuelMapData(m_currentFuelMapIndex);
            }
        }

        // if the row/column index into the fuel map has changed
        if ((m_currentFuelMapRow != newFuelMapRow) || (m_currentFuelMapCol != newFuelMapCol))
        {
            // if the fuel map data hasn't been retrieved on this pass, that means
            // that the fuel map itself hasn't changed and the currently-displayed
            // map needs an update
            if ((fuelMapData == 0) &&
                (m_currentFuelMapRow >= 0) && (m_currentFuelMapRow < m_fuelMapDisplay->rowCount()) &&
                (m_currentFuelMapCol >= 0) && (m_currentFuelMapCol < m_fuelMapDisplay->columnCount()))
            {
                // set the currently-highlighted cell back to its original colors
                QTableWidgetItem *item = m_fuelMapDisplay->item(m_currentFuelMapRow, m_currentFuelMapCol);
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
        m_throttleBar->setValue(m_cux->getThrottlePos() * 100);

    if (m_enabledSamples[SampleType_MAF])
        m_mafReadingBar->setValue(m_cux->getMAFReading() * 100);

    if (m_enabledSamples[SampleType_IdleBypassPosition])
        m_idleBypassPosBar->setValue(m_cux->getIdleBypassPos() * 100);

    if (m_enabledSamples[SampleType_RoadSpeed])
        m_speedo->setValue(m_cux->getRoadSpeed());

    if (m_enabledSamples[SampleType_EngineRPM])
        m_revCounter->setValue(m_cux->getEngineSpeedRPM());

    if (m_enabledSamples[SampleType_EngineTemperature])
        m_waterTempGauge->setValue(m_cux->getCoolantTemp());

    if (m_enabledSamples[SampleType_FuelTemperature])
        m_fuelTempGauge->setValue(m_cux->getFuelTemp());

    if (m_enabledSamples[SampleType_MainVoltage])
        m_voltage->setText(QString::number(m_cux->getMainVoltage(), 'f', 1) + "VDC");

    if (m_enabledSamples[SampleType_FuelPumpRelay])
        m_fuelPumpRelayStateLed->setChecked(m_cux->getFuelPumpRelayState());

    if (m_enabledSamples[SampleType_TargetIdleRPM])
    {
        int targetIdleSpeedRPM = m_cux->getTargetIdleSpeed();

        if (targetIdleSpeedRPM > 0)
            m_targetIdle->setText(QString::number(targetIdleSpeedRPM));
        else
            m_targetIdle->setText("");

        m_idleModeLed->setChecked(m_cux->getIdleMode());
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
            QString("+%1%").arg(leftLambdaTrim * 100 / m_leftFuelTrimBar->maximum()) :
            QString("-%1%").arg(leftLambdaTrim * 100 / m_leftFuelTrimBar->minimum());
        QString rightLabel = (rightLambdaTrim >= 0) ?
            QString("+%1%").arg(rightLambdaTrim * 100 / m_rightFuelTrimBar->maximum()) :
            QString("-%1%").arg(rightLambdaTrim * 100 / m_rightFuelTrimBar->minimum());

        m_leftFuelTrimBar->setEnabled(true);
        m_leftFuelTrimBar->setValue(leftLambdaTrim);
        m_rightFuelTrimBar->setEnabled(true);
        m_rightFuelTrimBar->setValue(rightLambdaTrim);

        m_leftFuelTrimBarLabel->setText(leftLabel);
        m_rightFuelTrimBarLabel->setText(rightLabel);
    }
    else
    {
        m_leftFuelTrimBar->setValue(0);
        m_leftFuelTrimBar->setEnabled(false);
        m_rightFuelTrimBar->setValue(0);
        m_rightFuelTrimBar->setEnabled(false);

        m_leftFuelTrimBarLabel->setText("+0%");
        m_rightFuelTrimBarLabel->setText("+0%");
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
            m_gear->setText("Park/Neutral");
            break;
        case C14CUX_Gear_DriveOrReverse:
            m_gear->setText("Drive/Reverse");
            break;
        case C14CUX_Gear_ManualGearbox:
            m_gear->setText("(Manual gearbox)");
            break;
        case C14CUX_Gear_NoReading:
        default:
            m_gear->setText("(no reading)");
            break;
    }
}

/**
 * Paints a highlight on the fuel map display cell that represents the
 * most-recently-read fueling index.
 */
void MainWindow::highlightActiveFuelMapCell()
{
    if ((m_currentFuelMapRow >= 0) && (m_currentFuelMapRow < m_fuelMapDisplay->rowCount()) &&
        (m_currentFuelMapCol >= 0) && (m_currentFuelMapCol < m_fuelMapDisplay->columnCount()))
    {
        QTableWidgetItem *item = m_fuelMapDisplay->item(m_currentFuelMapRow, m_currentFuelMapCol);
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
            m_speedo->setMaximum(speedometerMaxMPH);
        }
        else
        {
            m_speedo->setMaximum(speedometerMaxKPH);
        }
        m_speedo->setSuffix(m_speedUnitSuffix->value(speedUnit));
        m_speedo->repaint();

        TemperatureUnits tempUnits = m_options->getTemperatureUnits();
        QString tempUnitStr = m_tempUnitSuffix->value(tempUnits);

        int tempMin = m_tempRange->value(tempUnits).first;
        int tempMax = m_tempRange->value(tempUnits).second;
        int tempNominal = m_tempLimits->value(tempUnits).first;
        int tempCritical = m_tempLimits->value(tempUnits).second;

        m_fuelTempGauge->setSuffix(tempUnitStr);
        m_fuelTempGauge->setValue(tempMin);
        m_fuelTempGauge->setMaximum(tempMax);
        m_fuelTempGauge->setMinimum(tempMin);
        m_fuelTempGauge->repaint();

        m_waterTempGauge->setSuffix(tempUnitStr);
        m_waterTempGauge->setValue(tempMin);
        m_waterTempGauge->setMaximum(tempMax);
        m_waterTempGauge->setMinimum(tempMin);
        m_waterTempGauge->setNominal(tempNominal);
        m_waterTempGauge->setCritical(tempCritical);
        m_waterTempGauge->repaint();

        m_cux->setSpeedUnits(speedUnit);
        m_cux->setTemperatureUnits(tempUnits);

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
    m_mafReadingLabel->setEnabled(enabled);
    m_mafReadingBar->setEnabled(enabled);
    m_mafReadingTypeLabel->setEnabled(enabled);
    m_mafReadingDirectButton->setEnabled(enabled);
    m_mafReadingLinearButton->setEnabled(enabled);
    if (!enabled)
        m_mafReadingBar->setValue(0);

    enabled = m_enabledSamples[SampleType_Throttle];
    m_throttleLabel->setEnabled(enabled);
    m_throttleBar->setEnabled(enabled);
    m_throttleTypeLabel->setEnabled(enabled);
    m_throttleTypeAbsoluteButton->setEnabled(enabled);
    m_throttleTypeCorrectedButton->setEnabled(enabled);
    if (!enabled)
        m_throttleBar->setValue(0);

    enabled = m_enabledSamples[SampleType_IdleBypassPosition];
    m_idleBypassLabel->setEnabled(enabled);
    m_idleBypassPosBar->setEnabled(enabled);
    if (!enabled)
        m_idleBypassPosBar->setValue(0);

    enabled = m_enabledSamples[SampleType_GearSelection];
    m_gearLabel->setEnabled(enabled);
    m_gear->setEnabled(enabled);

    enabled = m_enabledSamples[SampleType_MainVoltage];
    m_voltageLabel->setEnabled(enabled);
    m_voltage->setEnabled(enabled);

    enabled = m_enabledSamples[SampleType_TargetIdleRPM];
    m_targetIdleLabel->setEnabled(enabled);
    m_targetIdle->setEnabled(enabled);
    m_idleModeLedOpacity->setEnabled(!enabled);

    enabled = m_enabledSamples[SampleType_LambdaTrim];
    m_lambdaTrimTypeLabel->setEnabled(enabled);
    m_lambdaTrimLowLimitLabel->setEnabled(enabled);
    m_lambdaTrimHighLimitLabel->setEnabled(enabled);
    m_lambdaTrimShortButton->setEnabled(enabled);
    m_lambdaTrimLongButton->setEnabled(enabled);
    m_leftFuelTrimBar->setEnabled(enabled);
    m_leftFuelTrimLabel->setEnabled(enabled);
    m_leftFuelTrimBarLabel->setEnabled(enabled);
    m_rightFuelTrimBar->setEnabled(enabled);
    m_rightFuelTrimBarLabel->setEnabled(enabled);
    m_rightFuelTrimLabel->setEnabled(enabled);
    if (!enabled)
    {
        m_leftFuelTrimBar->setValue(0);
        m_leftFuelTrimBarLabel->setText("");
        m_rightFuelTrimBar->setValue(0);
        m_rightFuelTrimBarLabel->setText("");
    }

    enabled = m_enabledSamples[SampleType_FuelPumpRelay];
    m_fuelPumpRelayStateLabel->setEnabled(enabled);
    m_fuelPumpRelayStateLed->setEnabled(enabled);
    m_fuelPumpLedOpacity->setEnabled(!enabled);

    enabled = m_enabledSamples[SampleType_FuelMap];
    m_fuelMapIndexLabel->setEnabled(enabled);
    m_fuelMapFactorLabel->setEnabled(enabled);
    m_fuelMapOpacity->setEnabled(!enabled);

    // These controls are shown in a disabled state by applying a 50% opacity
    // graphical effect; the 'enabled' bit is therefore inverted because it's
    // controlling the state of the graphical effect (rather than the widget).
    enabled = m_enabledSamples[SampleType_EngineTemperature];
    m_waterTempGaugeOpacity->setEnabled(!enabled);
    m_waterTempLabel->setEnabled(enabled);
    enabled = m_enabledSamples[SampleType_FuelTemperature];
    m_fuelTempGaugeOpacity->setEnabled(!enabled);
    m_fuelTempLabel->setEnabled(enabled);

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
    m_connectButton->setEnabled(false);
    m_disconnectButton->setEnabled(true);
    m_commsGoodLed->setChecked(false);
    m_commsBadLed->setChecked(false);
    m_fuelPumpOneshotButton->setEnabled(true);
    m_fuelPumpContinuousButton->setEnabled(true);
}

/**
 * Reponds to the "disconnect" signal from the CUXInterface by enabling/disabling
 * the appropriate buttons and setting a message in the status bar.
 */
void MainWindow::onDisconnect()
{
    m_connectButton->setEnabled(true);
    m_disconnectButton->setEnabled(false);
    m_milLed->setChecked(false);
    m_commsGoodLed->setChecked(false);
    m_commsBadLed->setChecked(false);
    m_fuelPumpOneshotButton->setEnabled(false);
    m_fuelPumpContinuousButton->setEnabled(false);

    m_speedo->setValue(0.0);
    m_revCounter->setValue(0.0);
    m_waterTempGauge->setValue(m_waterTempGauge->minimum());
    m_fuelTempGauge->setValue(m_fuelTempGauge->minimum());
    m_throttleBar->setValue(0);
    m_mafReadingBar->setValue(0);
    m_idleBypassPosBar->setValue(0);
    m_idleModeLed->setChecked(false);
    m_targetIdleLabel->setText("");
    m_voltage->setText("");
    m_gear->setText("");
    m_fuelPumpRelayStateLed->setChecked(false);
    m_leftFuelTrimBar->setValue(0);
    m_leftFuelTrimBarLabel->setText("+0%");
    m_rightFuelTrimBar->setValue(0);
    m_rightFuelTrimBarLabel->setText("+0%");

    m_leftFuelTrimBar->repaint();
    m_rightFuelTrimBar->repaint();

    m_currentFuelMapIndex = -1;
    m_currentFuelMapRow = -1;
    m_currentFuelMapCol = -1;
}

/**
 * Responds to the "read error" signal from the worker thread by turning
 * on a red lamp.
 */
void MainWindow::onReadError()
{
    m_commsGoodLed->setChecked(false);
    m_commsBadLed->setChecked(true);
}

/**
 * Responds to the "read success" signal from the worker thread by turning
 * on a green lamp.
 */
void MainWindow::onReadSuccess()
{
    m_commsGoodLed->setChecked(true);
    m_commsBadLed->setChecked(false);
}

/**
 * Opens the log file for writing.
 */
void MainWindow::onStartLogging()
{
    if (m_logger->openLog(m_logFileNameBox->text()))
    {
        m_startLoggingButton->setEnabled(false);
        m_stopLoggingButton->setEnabled(true);
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
    m_stopLoggingButton->setEnabled(false);
    m_startLoggingButton->setEnabled(true);
}

/**
 * Displays an dialog box with information about the program.
 */
void MainWindow::onHelpAboutClicked()
{
    if (m_aboutBox == 0)
    {
        m_aboutBox = new AboutBox(style(), this->windowTitle(), m_cux->getVersion());
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
void MainWindow::onSavePROMImageSelected()
{
    sendPROMImageRequest(
        QString("Read the PROM image from the ECU? This will take approximately 25 seconds."));
}

/**
 * Prompts the user to continue, and sends a request to read the PROM image.
 * @param prompt String used to prompt the user to continue.
 * @param displayTune True to determine the tune number after the image has been read; false to skip this.
 */
void MainWindow::sendPROMImageRequest(QString prompt)
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
                    QString("Please wait while the PROM image is read.\n\n"),
                    0, this, Qt::Dialog);
                m_pleaseWaitBox->setStandardButtons(QMessageBox::Cancel);
                connect(m_pleaseWaitBox, SIGNAL(finished(int)), this, SLOT(onPROMReadCancelled()));
            }
            m_pleaseWaitBox->show();

            emit requestPROMImage();
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
void MainWindow::onPROMReadCancelled()
{
    m_cux->cancelRead();
}

/**
 * Prompts the user for a file in which to save the PROM image.
 * @param displayTuneNumber True to determine the tune number after the image has been read; false to skip this.
 */
void MainWindow::onPROMImageReady()
{
    if (m_pleaseWaitBox != 0)
    {
        m_pleaseWaitBox->hide();
    }

    QByteArray *promData = m_cux->getPROMImage();

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
 * Displays a message box indicating that reading the PROM image has failed.
 */
void MainWindow::onPROMImageReadFailed()
{
    if (m_pleaseWaitBox != 0)
    {
        m_pleaseWaitBox->hide();
    }

    QMessageBox::warning(this, "Error",
        "Communications error. PROM image could not be read.", QMessageBox::Ok);
}

/**
 * Signals the worker thread to run the fuel pump.
 */
void MainWindow::onFuelPumpOneshot()
{
    emit requestFuelPumpRun();
}

/**
 * Starts a timer that periodically re-sends the signal to run the fuel
 * pump, thus keeping the pump running continuously.
 */
void MainWindow::onFuelPumpContinuous()
{
    if (m_fuelPumpContinuousButton->isChecked())
    {
        emit requestFuelPumpRun();
        m_fuelPumpRefreshTimer->start();
        m_fuelPumpOneshotButton->setEnabled(false);
    }
    else
    {
        m_fuelPumpRefreshTimer->stop();
        m_fuelPumpOneshotButton->setEnabled(true);
    }
}

/**
 * Signals the worker thread to run the fuel pump.
 */
void MainWindow::onFuelPumpRefreshTimer()
{
    emit requestFuelPumpRun();
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
void MainWindow::onLambdaTrimButtonClicked(int id)
{
    if (id == 1)
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
void MainWindow::onMAFReadingButtonClicked(int id)
{
    if (id == 1)
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
void MainWindow::onThrottleTypeButtonClicked(int id)
{
    if (id == 1)
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
    m_tuneRevNumberLabel->setText(QString("Tune revision: R%04").arg(tuneRevisionNum));
}

/**
 * Paints a redline on the tachometer, based on the electronic RPM limit
 * @param rpmLimit Engine speed limit in RPM
 */
void MainWindow::onRPMLimitReady(int rpmLimit)
{
    m_revCounter->setCritical((double)rpmLimit);
}

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

