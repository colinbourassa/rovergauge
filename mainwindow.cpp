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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "faultcodedialog.h"
#include "tunerevisiontable.h"

/**
 * Constructor; sets up main UI
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      cuxThread(0),
      cux(0),
      options(0),
      aboutBox(0),
      pleaseWaitBox(0),
      currentFuelMapIndex(-1),
      currentFuelMapRow(-1),
      currentFuelMapCol(-1),
      widthPixels(940),
      heightPixels(620)
{
    QDesktopWidget desktop;
    const QRect screenGeo = desktop.screenGeometry();
    if ((screenGeo.height() * 0.95) < heightPixels)
    {
        heightPixels = screenGeo.height() * 0.9;
    }
    if ((screenGeo.width() * 0.95) < widthPixels)
    {
        widthPixels = screenGeo.width() * 0.9;
    }

    ui->setupUi(this);
    this->setWindowTitle("RoverGauge");
    this->setMinimumSize(widthPixels, heightPixels);

    speedUnitSuffix = new QHash<SpeedUnits,QString>();
    speedUnitSuffix->insert(MPH, " MPH");
    speedUnitSuffix->insert(FPS, " ft/s");
    speedUnitSuffix->insert(KPH, " km/h");

    tempUnitSuffix = new QHash<TemperatureUnits,QString>;
    tempUnitSuffix->insert(Fahrenheit, " F");
    tempUnitSuffix->insert(Celcius, " C");

    tempRange = new QHash<TemperatureUnits,QPair<int, int> >;
    tempRange->insert(Fahrenheit, qMakePair(-40, 280));
    tempRange->insert(Celcius, qMakePair(-40, 140));

    tempLimits = new QHash<TemperatureUnits,QPair<int, int> >;
    tempLimits->insert(Fahrenheit, qMakePair(180, 210));
    tempLimits->insert(Celcius, qMakePair(80, 98));

    options = new OptionsDialog(this->windowTitle(), this);
    cux = new CUXInterface(options->getSerialDeviceName(), options->getPollIntervalMilliseconds(),
                           options->getSpeedUnits(), options->getTemperatureUnits());

    iacDialog = new IdleAirControlDialog(this->windowTitle(), this);
    connect(iacDialog, SIGNAL(requestIdleAirControlMovement(int,int)),
            cux, SLOT(onIdleAirControlMovementRequest(int,int)));

    logger = new Logger(cux);

    fuelPumpRefreshTimer = new QTimer(this);
    fuelPumpRefreshTimer->setInterval(1000);

    connect(cux, SIGNAL(dataReady()), this, SLOT(onDataReady()));
    connect(cux, SIGNAL(connected()), this, SLOT(onConnect()));
    connect(cux, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(cux, SIGNAL(readError()), this, SLOT(onReadError()));
    connect(cux, SIGNAL(readSuccess()), this, SLOT(onReadSuccess()));
    connect(cux, SIGNAL(failedToConnect(QString)), this, SLOT(onFailedToConnect(QString)));
    connect(cux, SIGNAL(faultCodesReady()), this, SLOT(onFaultCodesReady()));
    connect(cux, SIGNAL(faultCodesReadFailed()), this, SLOT(onFaultCodesReadFailed()));
    connect(cux, SIGNAL(fuelMapReady(int)), this, SLOT(onFuelMapDataReady(int)));
    connect(cux, SIGNAL(interfaceReadyForPolling()), this, SLOT(onInterfaceReady()));
    connect(cux, SIGNAL(notConnected()), this, SLOT(onNotConnected()));
    connect(cux, SIGNAL(promImageReady(bool)), this, SLOT(onPROMImageReady(bool)));
    connect(cux, SIGNAL(promImageReadFailed()), this, SLOT(onPROMImageReadFailed()));
    connect(this, SIGNAL(requestToStartPolling()), cux, SLOT(onStartPollingRequest()));
    connect(this, SIGNAL(requestThreadShutdown()), cux, SLOT(onShutdownThreadRequest()));
    connect(this, SIGNAL(requestFuelMapData(int)), cux, SLOT(onFuelMapRequested(int)));
    connect(this, SIGNAL(requestPROMImage(bool)), cux, SLOT(onReadPROMImageRequested(bool)));
    connect(this, SIGNAL(requestFuelPumpRun()), cux, SLOT(onFuelPumpRunRequest()));
    connect(fuelPumpRefreshTimer, SIGNAL(timeout()), this, SLOT(onFuelPumpRefreshTimer()));

    setWindowIcon(QIcon(":/icons/key.png"));
    setupWidgets();
}

/**
 * Destructor; cleans up instance of 14CUX communications library
 */
MainWindow::~MainWindow()
{
    delete tempLimits;
    delete tempRange;
    delete speedUnitSuffix;
    delete tempUnitSuffix;
    delete aboutBox;
    delete options;
    delete cux;
    delete cuxThread;
}

/**
 * Sets up the layout of the main window.
 */
void MainWindow::setupLayout()
{
    layout = new QVBoxLayout(ui->centralWidget);

    aboveGaugesRow = new QHBoxLayout();
    layout->addLayout(aboveGaugesRow);

    connectionButtonLayout = new QHBoxLayout();
    aboveGaugesRow->addLayout(connectionButtonLayout);

    commsLedLayout = new QHBoxLayout();
    commsLedLayout->setAlignment(Qt::AlignRight);
    aboveGaugesRow->addLayout(commsLedLayout);

    horizontalLineA = new QFrame(this);
    horizontalLineA->setFrameShape(QFrame::HLine);
    horizontalLineA->setFrameShadow(QFrame::Sunken);
    layout->addWidget(horizontalLineA);

    gaugesLayout = new QHBoxLayout();
    layout->addLayout(gaugesLayout);

    horizontalLineB = new QFrame(this);
    horizontalLineB->setFrameShape(QFrame::HLine);
    horizontalLineB->setFrameShadow(QFrame::Sunken);
    layout->addWidget(horizontalLineB);

    horizontalLineC = new QFrame(this);
    horizontalLineC->setFrameShape(QFrame::HLine);
    horizontalLineC->setFrameShadow(QFrame::Sunken);

    belowGaugesRow = new QHBoxLayout();
    layout->addLayout(belowGaugesRow);

    waterTempLayout = new QVBoxLayout();
    gaugesLayout->addLayout(waterTempLayout);

    speedoLayout = new QVBoxLayout();
    gaugesLayout->addLayout(speedoLayout);

    revCounterLayout = new QVBoxLayout();
    gaugesLayout->addLayout(revCounterLayout);

    fuelTempLayout = new QVBoxLayout();
    gaugesLayout->addLayout(fuelTempLayout);

    belowGaugesLeft = new QGridLayout();
    belowGaugesRow->addLayout(belowGaugesLeft);

    verticalLineA = new QFrame(this);
    verticalLineA->setFrameShape(QFrame::VLine);
    verticalLineA->setFrameShadow(QFrame::Sunken);
    belowGaugesRow->addWidget(verticalLineA);

    belowGaugesRight = new QGridLayout();
    belowGaugesRow->addLayout(belowGaugesRight);
}

/**
 * Instantiates widgets used in the main window.
 */
void MainWindow::createWidgets()
{
    fileMenu = menuBar()->addMenu("&File");
    savePROMImageAction = fileMenu->addAction("&Save PROM image...");
    savePROMImageAction->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(savePROMImageAction, SIGNAL(triggered()), this, SLOT(onSavePROMImageSelected()));
    fileMenu->addSeparator();
    exitAction = fileMenu->addAction("E&xit");
    exitAction->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(onExitSelected()));

    optionsMenu = menuBar()->addMenu("&Options");
    showFaultsAction = optionsMenu->addAction("Show fault &codes...");
    showFaultsAction->setIcon(style()->standardIcon(QStyle::SP_DialogNoButton));
    connect(showFaultsAction, SIGNAL(triggered()), cux, SLOT(onFaultCodesRequested()));
    showIdleAirControlDialog = optionsMenu->addAction("&Idle air control...");
    connect(showIdleAirControlDialog, SIGNAL(triggered()), this, SLOT(onIdleAirControlClicked()));
    checkPROMRevisionAction = optionsMenu->addAction("Check PROM &revision...");
    connect(checkPROMRevisionAction, SIGNAL(triggered()), this, SLOT(onCheckPROMRevisionSelected()));
    editOptionsAction = optionsMenu->addAction("&Edit settings...");
    editOptionsAction->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));    
    connect(editOptionsAction, SIGNAL(triggered()), this, SLOT(onEditOptionsClicked()));

    helpMenu = menuBar()->addMenu("&Help");
    aboutAction = helpMenu->addAction("&About");
    aboutAction->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(onHelpAboutClicked()));

    connectButton = new QPushButton("Connect");
    connect(connectButton, SIGNAL(clicked()), this, SLOT(onConnectClicked()));

    disconnectButton = new QPushButton("Disconnect");
    disconnectButton->setEnabled(false);
    connect(disconnectButton, SIGNAL(clicked()), this, SLOT(onDisconnectClicked()));

    commsGoodLed = new QLedIndicator(this);
    commsGoodLed->setOnColor1(QColor(102, 255, 102));
    commsGoodLed->setOnColor2(QColor(82, 204, 82));
    commsGoodLed->setOffColor1(QColor(0, 102, 0));
    commsGoodLed->setOffColor2(QColor(0, 51, 0));
    commsGoodLed->setDisabled(true);

    commsBadLed = new QLedIndicator(this);
    commsBadLed->setOnColor1(QColor(255, 0, 0));
    commsBadLed->setOnColor2(QColor(176, 0, 2));
    commsBadLed->setOffColor1(QColor(20, 0, 0));
    commsBadLed->setOffColor2(QColor(90, 0, 2));
    commsBadLed->setDisabled(true);

    commsLedLabel = new QLabel("Communications:");

    mafReadingTypeLabel = new QLabel("MAF reading type:", this);
    mafReadingLinearButton = new QRadioButton("Linear", this);
    mafReadingLinearButton->setChecked(true);
    mafReadingDirectButton = new QRadioButton("Direct", this);

    mafReadingButtonGroup = new QButtonGroup(this);
    mafReadingButtonGroup->addButton(mafReadingLinearButton, 1);
    mafReadingButtonGroup->addButton(mafReadingDirectButton, 2);
    connect(mafReadingButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onMAFReadingButtonClicked(int)));

    mafReadingLabel = new QLabel("MAF reading:", this);
    mafReadingBar = new QProgressBar(this);
    mafReadingBar->setRange(0, 100);
    mafReadingBar->setValue(0);
    mafReadingBar->setMinimumWidth(300);

    throttleTypeLabel = new QLabel("Throttle reading type:", this);
    throttleTypeAbsoluteButton = new QRadioButton("Absolute", this);
    throttleTypeAbsoluteButton->setChecked(true);
    throttleTypeCorrectedButton = new QRadioButton("Corrected", this);

    throttleTypeButtonGroup = new QButtonGroup(this);
    throttleTypeButtonGroup->addButton(throttleTypeAbsoluteButton, 1);
    throttleTypeButtonGroup->addButton(throttleTypeCorrectedButton, 2);
    connect(throttleTypeButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onThrottleTypeButtonClicked(int)));

    throttleLabel = new QLabel("Throttle position:", this);
    throttleBar = new QProgressBar(this);
    throttleBar->setRange(0, 100);
    throttleBar->setValue(0);
    throttleBar->setMinimumWidth(300);

    idleBypassLabel = new QLabel("Idle bypass position:", this);
    idleBypassPosBar = new QProgressBar(this);
    idleBypassPosBar->setRange(0, 100);
    idleBypassPosBar->setValue(0);
    idleBypassPosBar->setMinimumWidth(300);

    lambdaTrimTypeLabel = new QLabel("Lambda trim type:", this);
    lambdaTrimShortButton = new QRadioButton("Short term", this);
    lambdaTrimShortButton->setChecked(true);
    lambdaTrimLongButton = new QRadioButton("Long term", this);

    lambdaTrimButtonGroup = new QButtonGroup(this);
    lambdaTrimButtonGroup->addButton(lambdaTrimShortButton, 1);
    lambdaTrimButtonGroup->addButton(lambdaTrimLongButton, 2);
    connect(lambdaTrimButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onLambdaTrimButtonClicked(int)));

    lambdaTrimLowLimitLabel = new QLabel("(Reducing)", this);
    lambdaTrimHighLimitLabel = new QLabel("(Increasing)", this);

    leftFuelTrimLabel = new QLabel("Lambda fuel trim (left):", this);
    leftFuelTrimBar = new FuelTrimBar(this);
    leftFuelTrimBar->setValue(0);
    leftFuelTrimBarLabel = new QLabel("+0%", this);

    rightFuelTrimLabel = new QLabel("Lambda fuel trim (right):", this);
    rightFuelTrimBar = new FuelTrimBar(this);
    rightFuelTrimBar->setValue(0);
    rightFuelTrimBarLabel = new QLabel("+0%", this);

    targetIdleLabel = new QLabel("Target idle RPM:", this);
    targetIdle = new QLabel("", this);

    gearLabel = new QLabel("Selected gear:", this);
    gear = new QLabel("", this);

    voltageLabel = new QLabel("Main voltage:", this);
    voltage = new QLabel("", this);

    fuelMapIndexLabel = new QLabel("Current fuel map:", this);
    fuelMapFactorLabel = new QLabel("Adjustment factor:", this);

    setStyleSheet("QTableWidget {background-color: transparent;}");
    fuelMapDisplay = new QTableWidget(8, 16, this);
    fuelMapDisplay->verticalHeader()->hide();
    fuelMapDisplay->horizontalHeader()->hide();
    fuelMapDisplay->resizeColumnsToContents();
    fuelMapDisplay->resizeRowsToContents();

    int rowCount = fuelMapDisplay->rowCount();
    int colCount = fuelMapDisplay->columnCount();
    QTableWidgetItem *item = 0;
    for (int row = 0; row < rowCount; row++)
    {
        for (int col = 0; col < colCount; col++)
        {
            item = new QTableWidgetItem("");
            item->setFlags(0);
            fuelMapDisplay->setItem(row, col, item);
        }
    }

    fuelPumpRelayStateLabel = new QLabel("Fuel pump relay", this);
    fuelPumpRelayStateLed = new QLedIndicator(this);
    fuelPumpRelayStateLed->setOnColor1(QColor(102, 255, 102));
    fuelPumpRelayStateLed->setOnColor2(QColor(82, 204, 82));
    fuelPumpRelayStateLed->setOffColor1(QColor(0, 102, 0));
    fuelPumpRelayStateLed->setOffColor2(QColor(0, 51, 0));
    fuelPumpRelayStateLed->setDisabled(true);

    fuelPumpOneshotButton = new QPushButton("Run pump (one shot)");
    fuelPumpOneshotButton->setEnabled(false);
    connect(fuelPumpOneshotButton, SIGNAL(clicked()), this, SLOT(onFuelPumpOneshot()));

    fuelPumpContinuousButton = new QPushButton("Run pump (continuous)");
    fuelPumpContinuousButton->setEnabled(false);
    fuelPumpContinuousButton->setCheckable(true);
    connect(fuelPumpContinuousButton, SIGNAL(clicked()), this, SLOT(onFuelPumpContinuous()));

    logFileNameLabel = new QLabel("Log file name:", this);
    logFileNameBox = new QLineEdit(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss"), this);
    startLoggingButton = new QPushButton("Start logging");
    startLoggingButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    stopLoggingButton = new QPushButton("Stop logging");
    stopLoggingButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    stopLoggingButton->setEnabled(false);
    connect(startLoggingButton, SIGNAL(clicked()), this, SLOT(onStartLogging()));
    connect(stopLoggingButton, SIGNAL(clicked()), this, SLOT(onStopLogging()));

    speedo = new ManoMeter(this);
    speedo->setMinimum(0.0);
    speedo->setMaximum(160.0);
    speedo->setMaximum((double)options->getSpeedMax());
    speedo->setSuffix(speedUnitSuffix->value(options->getSpeedUnits()));
    speedo->setNominal(1000.0);
    speedo->setCritical(1000.0);

    revCounter = new ManoMeter(this);
    revCounter->setMinimum(0.0);
    revCounter->setMaximum(8000);
    revCounter->setSuffix(" RPM");
    revCounter->setNominal(100000.0);
    revCounter->setCritical(options->getRedline());

    TemperatureUnits tempUnits = options->getTemperatureUnits();
    int tempMin = tempRange->value(tempUnits).first;
    int tempMax = tempRange->value(tempUnits).second;

    waterTempGauge = new ManoMeter(this);
    waterTempGauge->setValue(tempMin);
    waterTempGauge->setMaximum(tempMax);
    waterTempGauge->setMinimum(tempMin);
    waterTempGauge->setSuffix(tempUnitSuffix->value(tempUnits));
    waterTempGauge->setNominal(tempLimits->value(tempUnits).first);
    waterTempGauge->setCritical(tempLimits->value(tempUnits).second);

    waterTempLabel = new QLabel("Engine Temperature", this);

    fuelTempGauge = new ManoMeter(this);
    fuelTempGauge->setValue(tempMin);
    fuelTempGauge->setMaximum(tempMax);
    fuelTempGauge->setMinimum(tempMin);
    fuelTempGauge->setSuffix(tempUnitSuffix->value(tempUnits));
    fuelTempGauge->setNominal(10000.0);
    fuelTempGauge->setCritical(10000.0);

    fuelTempLabel = new QLabel("Fuel Temperature", this);
}

/**
 * Adds the created widgets to the form's layout
 */
void MainWindow::placeWidgets()
{
    connectionButtonLayout->addWidget(connectButton);
    connectionButtonLayout->addWidget(disconnectButton);

    commsLedLayout->addWidget(commsLedLabel);
    commsLedLayout->addWidget(commsGoodLed);
    commsLedLayout->addWidget(commsBadLed);

    speedoLayout->addWidget(speedo);
    revCounterLayout->addWidget(revCounter);

    waterTempLayout->addWidget(waterTempGauge);
    waterTempLayout->addWidget(waterTempLabel, 0, Qt::AlignCenter);

    fuelTempLayout->addWidget(fuelTempGauge);
    fuelTempLayout->addWidget(fuelTempLabel, 0, Qt::AlignCenter);

    unsigned char row = 0;

    belowGaugesLeft->addWidget(mafReadingTypeLabel,    row,   0, 1, 1,  Qt::AlignRight);
    belowGaugesLeft->addWidget(mafReadingLinearButton, row,   1, 1, 1);
    belowGaugesLeft->addWidget(mafReadingDirectButton, row++, 2, 1, 1);

    belowGaugesLeft->addWidget(mafReadingLabel,    row,   0,        Qt::AlignRight);
    belowGaugesLeft->addWidget(mafReadingBar,      row++, 1, 1, 3);

    belowGaugesLeft->addWidget(throttleTypeLabel,           row,   0, 1, 1,  Qt::AlignRight);
    belowGaugesLeft->addWidget(throttleTypeAbsoluteButton,  row,   1, 1, 1);
    belowGaugesLeft->addWidget(throttleTypeCorrectedButton, row++, 2, 1, 1);

    belowGaugesLeft->addWidget(throttleLabel,      row,   0,        Qt::AlignRight);
    belowGaugesLeft->addWidget(throttleBar,        row++, 1, 1, 3);

    belowGaugesLeft->addWidget(idleBypassLabel,    row,   0,        Qt::AlignRight);
    belowGaugesLeft->addWidget(idleBypassPosBar,   row++, 1, 1, 3);

    belowGaugesLeft->addWidget(targetIdleLabel,    row,   0,        Qt::AlignRight);
    belowGaugesLeft->addWidget(targetIdle,         row++, 1, 1, 3);

    belowGaugesLeft->addWidget(gearLabel,          row,   0,        Qt::AlignRight);
    belowGaugesLeft->addWidget(gear,               row++, 1, 1, 3);

    belowGaugesLeft->addWidget(voltageLabel,       row,   0,        Qt::AlignRight);
    belowGaugesLeft->addWidget(voltage,            row++, 1, 1, 3);

    belowGaugesLeft->addWidget(lambdaTrimTypeLabel,   row,   0, 1, 1,  Qt::AlignRight);
    belowGaugesLeft->addWidget(lambdaTrimShortButton, row,   1, 1, 1);
    belowGaugesLeft->addWidget(lambdaTrimLongButton,  row++, 2, 1, 1);

    belowGaugesLeft->addWidget(leftFuelTrimLabel,  row,   0, 1, 1,  Qt::AlignRight);
    belowGaugesLeft->addWidget(leftFuelTrimBarLabel, row, 1, 1, 1,  Qt::AlignRight);
    belowGaugesLeft->addWidget(leftFuelTrimBar,    row++, 2, 1, 2);

    belowGaugesLeft->addWidget(rightFuelTrimLabel, row,   0, 1, 1,  Qt::AlignRight);
    belowGaugesLeft->addWidget(rightFuelTrimBarLabel,row, 1, 1, 1,  Qt::AlignRight);
    belowGaugesLeft->addWidget(rightFuelTrimBar,   row++, 2, 1, 2);

    belowGaugesLeft->addWidget(lambdaTrimLowLimitLabel,  row,   2, 1, 1, Qt::AlignLeft);
    belowGaugesLeft->addWidget(lambdaTrimHighLimitLabel, row++, 3, 1, 1, Qt::AlignRight);

    row = 0;
    belowGaugesRight->setColumnMinimumWidth(0, 20);
    belowGaugesRight->setColumnStretch(0, 0);
    belowGaugesRight->addWidget(fuelMapIndexLabel,        row,   0, 1, 2);
    belowGaugesRight->addWidget(fuelMapFactorLabel,       row++, 2, 1, 2);
    belowGaugesRight->addWidget(fuelMapDisplay,           row++, 0, 1, 4);
    belowGaugesRight->addWidget(fuelPumpRelayStateLed,    row,   0, 1, 1);
    belowGaugesRight->addWidget(fuelPumpRelayStateLabel,  row,   1, 1, 1);
    belowGaugesRight->addWidget(fuelPumpOneshotButton,    row,   2, 1, 1);
    belowGaugesRight->addWidget(fuelPumpContinuousButton, row++, 3, 1, 1);

    belowGaugesRight->addWidget(horizontalLineC,    row++, 0, 1, 4);
    belowGaugesRight->addWidget(logFileNameLabel,   row,   0, 1, 2);
    belowGaugesRight->addWidget(logFileNameBox,     row++, 2, 1, 2);

    belowGaugesRight->addWidget(startLoggingButton, row,   2, 1, 1);
    belowGaugesRight->addWidget(stopLoggingButton,  row++, 3, 1, 1);
}

/**
 * Instantiates widgets, connects to their signals, and places them on the form.
 */
void MainWindow::setupWidgets()
{    
    setupLayout();
    createWidgets();
    placeWidgets();
}

/**
 * Attempts to open the serial device connected to the 14CUX,
 * and starts updating the display with data if successful.
 */
void MainWindow::onConnectClicked()
{
    // If the worker thread hasn't been created yet, do that now.
    if (cuxThread == 0)
    {
        cuxThread = new QThread(this);
        cux->moveToThread(cuxThread);
        connect(cuxThread, SIGNAL(started()), cux, SLOT(onParentThreadStarted()));
    }

    // If the worker thread is alreay running, ask it to start polling the ECU.
    // Otherwise, start the worker thread, but don't ask it to begin polling
    // yet; it'll signal us when it's ready.
    if (cuxThread->isRunning())
    {
        emit requestToStartPolling();
    }
    else
    {
        cuxThread->start();
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
    disconnectButton->setEnabled(false);
    cux->disconnectFromECU();
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
    Comm14CUXFaultCodes faultCodes = cux->getFaultCodes();
    FaultCodeDialog faultDialog(this->windowTitle(), faultCodes);
    connect(&faultDialog, SIGNAL(clearFaultCodes()), cux, SLOT(onFaultCodesClearRequested()));
    connect(cux, SIGNAL(faultCodesClearSuccess(Comm14CUXFaultCodes)),
            &faultDialog, SLOT(onFaultClearSuccess(Comm14CUXFaultCodes)));
    connect(cux, SIGNAL(faultCodesClearFailure()), &faultDialog, SLOT(onFaultClearFailure()));
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
void MainWindow::populateFuelMapDisplay(QByteArray *data)
{
    if (data != 0)
    {
        int rowCount = fuelMapDisplay->rowCount();
        int colCount = fuelMapDisplay->columnCount();
        QTableWidgetItem *item = 0;
        unsigned char byte = 0;

        // populate all the cells with the data for this map
        for (int row = 0; row < rowCount; row++)
        {
            for (int col = 0; col < colCount; col++)
            {
                item = fuelMapDisplay->item(row, col);
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
        fuelMapDisplay->resizeColumnsToContents();
        fuelMapDisplay->resizeRowsToContents();

        highlightActiveFuelMapCell();
    }
}

/**
 * Uses fuel map data to populate the fuel map display grid.
 * @param ID of the fuel map just retrieved (from 1-5)
 */
void MainWindow::onFuelMapDataReady(int fuelMapId)
{
    QByteArray *data = cux->getFuelMap(fuelMapId);

    if (data != 0)
    {
        populateFuelMapDisplay(data);
        QString hexPart =
            QString("%1").arg(cux->getFuelMapAdjustmentFactor(), 0, 16).toUpper();
        fuelMapFactorLabel->setText(QString("Adjustment factor: 0x") + hexPart);
    }
}

/**
 * Updates the gauges and indicators with the latest data available from
 * the ECU.
 */
void MainWindow::onDataReady()
{
    int roadSpeed = cux->getRoadSpeed();
    int engineSpeedRPM = cux->getEngineSpeedRPM();
    int targetIdleSpeedRPM = cux->getTargetIdleSpeed();
    int waterTemp = cux->getCoolantTemp();
    int fuelTemp = cux->getFuelTemp();
    int throttlePos = cux->getThrottlePos() * 100;
    Comm14CUXGear gearReading = cux->getGear();
    float mainVoltage = cux->getMainVoltage();
    int mafReading = cux->getMAFReading() * 100;
    int idleBypassPos = cux->getIdleBypassPos() * 100;
    bool fuelPumpRelay = cux->getFuelPumpRelayState();
    int leftLambdaTrim = cux->getLeftLambdaTrim();
    int rightLambdaTrim = cux->getRightLambdaTrim();

    int newFuelMapIndex = cux->getCurrentFuelMapIndex();
    int newFuelMapRow = cux->getFuelMapRowIndex();
    int newFuelMapCol = cux->getFuelMapColumnIndex();

    QByteArray *fuelMapData = 0;

    // if the active fuel map has changed, prepare to update the display
    if (currentFuelMapIndex != newFuelMapIndex)
    {
        currentFuelMapIndex = newFuelMapIndex;
        fuelMapIndexLabel->setText(QString("Current fuel map: %1").arg(currentFuelMapIndex));
        fuelMapData = cux->getFuelMap(currentFuelMapIndex);

        if (fuelMapData != 0)
        {
            populateFuelMapDisplay(fuelMapData);
        }
        else
        {
            // The data for the current fuel map hasn't been read out of the
            // ECU yet, so put in a request. We'll update the display when
            // we receive the signal that the new data is ready.
            emit requestFuelMapData(currentFuelMapIndex);
        }
    }

    // if the row/column index into the fuel map has changed
    if ((currentFuelMapRow != newFuelMapRow) || (currentFuelMapCol != newFuelMapCol))
    {
        // if the fuel map data hasn't been retrieved on this pass, that means
        // that the fuel map itself hasn't changed and the currently-displayed
        // map needs an update
        if ((fuelMapData == 0) &&
            (currentFuelMapRow >= 0) && (currentFuelMapRow < fuelMapDisplay->rowCount()) &&
            (currentFuelMapCol >= 0) && (currentFuelMapCol < fuelMapDisplay->columnCount()))
        {
            // set the currently-highlighted cell back to its original colors
            QTableWidgetItem *item = fuelMapDisplay->item(currentFuelMapRow, currentFuelMapCol);
            bool ok = false;
            unsigned char value = (unsigned char)(item->text().toInt(&ok, 16));
            if (ok)
            {
                item->setBackgroundColor(getColorForFuelMapCell(value));
                item->setTextColor(Qt::black);
            }
        }

        currentFuelMapRow = newFuelMapRow;
        currentFuelMapCol = newFuelMapCol;

        highlightActiveFuelMapCell();
    }

    // clip the throttle percentage at the min/max
    if (throttlePos < throttleBar->minimum())
    {
        throttlePos = throttleBar->minimum();
    }
    else if (throttlePos > throttleBar->maximum())
    {
        throttlePos = throttleBar->maximum();
    }

    // clip the MAF reading at min/max
    if (mafReading < mafReadingBar->minimum())
    {
        mafReading = mafReadingBar->minimum();
    }
    else if (mafReading > mafReadingBar->maximum())
    {
        mafReading = mafReadingBar->maximum();
    }

    // clip the idle bypass reading at min/max
    if (idleBypassPos < idleBypassPosBar->minimum())
    {
        idleBypassPos = idleBypassPosBar->minimum();
    }
    else if (idleBypassPos > idleBypassPosBar->maximum())
    {
        idleBypassPos = idleBypassPosBar->maximum();
    }

    speedo->setValue(roadSpeed);
    revCounter->setValue(engineSpeedRPM);
    waterTempGauge->setValue(waterTemp);
    fuelTempGauge->setValue(fuelTemp);
    throttleBar->setValue(throttlePos);
    mafReadingBar->setValue(mafReading);
    idleBypassPosBar->setValue(idleBypassPos);
    voltage->setText(QString::number(mainVoltage, 'f', 1) + "VDC");
    fuelPumpRelayStateLed->setChecked(fuelPumpRelay);

    if (targetIdleSpeedRPM > 0)
    {
        targetIdle->setText(QString::number(targetIdleSpeedRPM));
    }
    else
    {
        targetIdle->setText("");
    }

    setLambdaTrimIndicators(leftLambdaTrim, rightLambdaTrim);
    setGearLabel(gearReading);
    logger->logData();
}

/**
 * Sets the lambda fuel trim indicators to the provided values
 */
void MainWindow::setLambdaTrimIndicators(int leftLambdaTrim, int rightLambdaTrim)
{
    if ((currentFuelMapIndex == 0) ||
        (currentFuelMapIndex == 4) ||
        (currentFuelMapIndex == 5))
    {        
        QString leftLabel = (leftLambdaTrim >= 0) ?
            QString("+%1%").arg(leftLambdaTrim * 100 / leftFuelTrimBar->maximum()) :
            QString("-%1%").arg(leftLambdaTrim * 100 / leftFuelTrimBar->minimum());
        QString rightLabel = (rightLambdaTrim >= 0) ?
            QString("+%1%").arg(rightLambdaTrim * 100 / rightFuelTrimBar->maximum()) :
            QString("-%1%").arg(rightLambdaTrim * 100 / rightFuelTrimBar->minimum());

        leftFuelTrimBar->setEnabled(true);
        leftFuelTrimBar->setValue(leftLambdaTrim);
        rightFuelTrimBar->setEnabled(true);
        rightFuelTrimBar->setValue(rightLambdaTrim);

        leftFuelTrimBarLabel->setText(leftLabel);
        rightFuelTrimBarLabel->setText(rightLabel);
    }
    else
    {
        leftFuelTrimBar->setValue(0);
        leftFuelTrimBar->setEnabled(false);
        rightFuelTrimBar->setValue(0);
        rightFuelTrimBar->setEnabled(false);

        leftFuelTrimBarLabel->setText("+0%");
        rightFuelTrimBarLabel->setText("+0%");
    }
}

/**
 * Sets the label indicating the current gear selection
 */
void MainWindow::setGearLabel(Comm14CUXGear gearReading)
{
    switch (gearReading)
    {
        case Comm14CUXGear_ParkOrNeutral:
            gear->setText("Park/Neutral");
            break;
        case Comm14CUXGear_DriveOrReverse:
            gear->setText("Drive/Reverse");
            break;
        case Comm14CUXGear_ManualGearbox:
            gear->setText("(Manual gearbox)");
            break;
        case Comm14CUXGear_NoReading:
        default:
            gear->setText("(no reading)");
            break;
    }
}

/**
 * Paints a highlight on the fuel map display cell that represents the
 * most-recently-read fueling index.
 */
void MainWindow::highlightActiveFuelMapCell()
{
    if ((currentFuelMapRow >= 0) && (currentFuelMapRow < fuelMapDisplay->rowCount()) &&
        (currentFuelMapCol >= 0) && (currentFuelMapCol < fuelMapDisplay->columnCount()))
    {
        QTableWidgetItem *item = fuelMapDisplay->item(currentFuelMapRow, currentFuelMapCol);
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
    if (options->exec() == QDialog::Accepted)
    {
        // update the speedo and tach appropriately
        SpeedUnits speedUnits = options->getSpeedUnits();
        speedo->setMaximum((double)options->getSpeedMax());
        speedo->setSuffix(speedUnitSuffix->value(speedUnits));
        speedo->repaint();

        revCounter->setCritical(options->getRedline());

        TemperatureUnits tempUnits = options->getTemperatureUnits();
        QString tempUnitStr = tempUnitSuffix->value(tempUnits);

        int tempMin = tempRange->value(tempUnits).first;
        int tempMax = tempRange->value(tempUnits).second;
        int tempNominal = tempLimits->value(tempUnits).first;
        int tempCritical = tempLimits->value(tempUnits).second;

        fuelTempGauge->setSuffix(tempUnitStr);
        fuelTempGauge->setValue(tempMin);
        fuelTempGauge->setMaximum(tempMax);
        fuelTempGauge->setMinimum(tempMin);
        fuelTempGauge->repaint();

        waterTempGauge->setSuffix(tempUnitStr);
        waterTempGauge->setValue(tempMin);
        waterTempGauge->setMaximum(tempMax);
        waterTempGauge->setMinimum(tempMin);
        waterTempGauge->setNominal(tempNominal);
        waterTempGauge->setCritical(tempCritical);
        waterTempGauge->repaint();

        cux->setSpeedUnits(speedUnits);
        cux->setTemperatureUnits(tempUnits);

        // if the user changed the serial device name and/or the polling
        // interval, stop the timer, re-connect to the 14CUX (if neccessary),
        // and restart the timer
        if (options->getSerialDeviceChanged())
        {
            if (cux->isConnected())
            {
                cux->disconnectFromECU();
            }
            cux->setSerialDevice(options->getSerialDeviceName());
        }

        if (options->getPollIntervalChanged())
        {
            cux->setIntervalMsecs(options->getPollIntervalMilliseconds());
        }
    }
}

/**
 * Responds to a 'close' event on the main window by first shutting down
 * the other thread.
 * @param event The event itself.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    logger->closeLog();

    if ((cuxThread != 0) && cuxThread->isRunning())
    {
        emit requestThreadShutdown();
        cuxThread->wait(2000);
    }

    event->accept();
}

/**
 * Reponds to the "connect" signal from the CUXInterface by enabling/disabling
 * the appropriate buttons and setting a message in the status bar.
 */
void MainWindow::onConnect()
{
    connectButton->setEnabled(false);
    disconnectButton->setEnabled(true);
    commsGoodLed->setChecked(false);
    commsBadLed->setChecked(false);
    fuelPumpOneshotButton->setEnabled(true);
    fuelPumpContinuousButton->setEnabled(true);
}

/**
 * Reponds to the "disconnect" signal from the CUXInterface by enabling/disabling
 * the appropriate buttons and setting a message in the status bar.
 */
void MainWindow::onDisconnect()
{
    connectButton->setEnabled(true);
    disconnectButton->setEnabled(false);
    commsGoodLed->setChecked(false);
    commsBadLed->setChecked(false);
    fuelPumpOneshotButton->setEnabled(false);
    fuelPumpContinuousButton->setEnabled(false);

    speedo->setValue(0.0);
    revCounter->setValue(0.0);
    waterTempGauge->setValue(waterTempGauge->minimum());
    fuelTempGauge->setValue(fuelTempGauge->minimum());
    throttleBar->setValue(0);
    mafReadingBar->setValue(0);
    idleBypassPosBar->setValue(0);
    voltage->setText("");
    gear->setText("");
    fuelPumpRelayStateLed->setChecked(false);
    leftFuelTrimBar->setValue(0);
    leftFuelTrimBarLabel->setText("+0%");
    rightFuelTrimBar->setValue(0);
    rightFuelTrimBarLabel->setText("+0%");

    leftFuelTrimBar->repaint();
    rightFuelTrimBar->repaint();

    currentFuelMapIndex = -1;
    currentFuelMapRow = -1;
    currentFuelMapCol = -1;
}

/**
 * Responds to the "read error" signal from the worker thread by turning
 * on a red lamp.
 */
void MainWindow::onReadError()
{
    commsGoodLed->setChecked(false);
    commsBadLed->setChecked(true);
}

/**
 * Responds to the "read success" signal from the worker thread by turning
 * on a green lamp.
 */
void MainWindow::onReadSuccess()
{
    commsGoodLed->setChecked(true);
    commsBadLed->setChecked(false);
}

/**
 * Opens the log file for writing.
 */
void MainWindow::onStartLogging()
{
    if (logger->openLog(logFileNameBox->text()))
    {
        startLoggingButton->setEnabled(false);
        stopLoggingButton->setEnabled(true);
    }
    else
    {
        QMessageBox::warning(this, "Error",
            "Failed to open log file (" + logger->getLogPath() + ")", QMessageBox::Ok);
    }
}

/**
 * Closes the open log file.
 */
void MainWindow::onStopLogging()
{
    logger->closeLog();
    stopLoggingButton->setEnabled(false);
    startLoggingButton->setEnabled(true);
}

/**
 * Displays an dialog box with information about the program.
 */
void MainWindow::onHelpAboutClicked()
{
    if (aboutBox == 0)
    {
        aboutBox = new AboutBox(style(), this->windowTitle(), cux->getVersion());
    }
    aboutBox->exec();
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
    if (pleaseWaitBox != 0)
    {
        pleaseWaitBox->hide();
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
        QString("Read the PROM image from the ECU? This will take approximately 25 seconds."), false);
}

/**
 * Requests the PROM image so that its tune number can be identified.
 * Also allows the option of saving the image to disk.
 */
void MainWindow::onCheckPROMRevisionSelected()
{
    sendPROMImageRequest(
        QString("Checking the PROM revision requires reading the entire image from the ECU.\n") +
        QString("This will take approximately 25 seconds. Proceed?"), true);
}

/**
 * Prompts the user to continue, and sends a request to read the PROM image.
 * @param prompt String used to prompt the user to continue.
 * @param displayTune True to determine the tune number after the image has been read; false to skip this.
 */
void MainWindow::sendPROMImageRequest(QString prompt, bool displayTune)
{
    if (cux->isConnected())
    {
        if (QMessageBox::question(this, "Confirm", prompt,
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            if (pleaseWaitBox == 0)
            {
                pleaseWaitBox = new QMessageBox(
                    QMessageBox::Information, "In Progress",
                    QString("Please wait while the PROM image is read.\n\n"),
                    0, this, Qt::Dialog);
                pleaseWaitBox->setStandardButtons(QMessageBox::Cancel);
                connect(pleaseWaitBox, SIGNAL(finished(int)), this, SLOT(onPROMReadCancelled()));
            }
            pleaseWaitBox->show();

            emit requestPROMImage(displayTune);
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
    cux->cancelRead();
}

/**
 * Prompts the user for a file in which to save the PROM image.
 * @param displayTuneNumber True to determine the tune number after the image has been read; false to skip this.
 */
void MainWindow::onPROMImageReady(bool displayTuneNumber)
{
    if (pleaseWaitBox != 0)
    {
        pleaseWaitBox->hide();
    }

    QByteArray *promData = cux->getPROMImage();
    bool saveToFile = true;

    if (promData != 0)
    {
        if (displayTuneNumber)
        {
            QByteArray md5SumBin = QCryptographicHash::hash(*promData, QCryptographicHash::Md5);
            QString md5sum = byteArrayToHexString(md5SumBin);
            TuneRevisionTable table;
            QMessageBox tuneRevMsgBox(
                QMessageBox::Information, "Tune information",
                QString("Tune revision:\n%1\n\nDo you wish to save the PROM image to a file?").arg(
                            table.lookup(md5sum)),
                0, this, Qt::Dialog);
            tuneRevMsgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            if (tuneRevMsgBox.exec() == QMessageBox::No)
            {
                saveToFile = false;
            }
        }

        if (saveToFile)
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
}

/**
 * Displays a message box indicating that reading the PROM image has failed.
 */
void MainWindow::onPROMImageReadFailed()
{
    if (pleaseWaitBox != 0)
    {
        pleaseWaitBox->hide();
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
    if (fuelPumpContinuousButton->isChecked())
    {
        emit requestFuelPumpRun();
        fuelPumpRefreshTimer->start();
        fuelPumpOneshotButton->setEnabled(false);
    }
    else
    {
        fuelPumpRefreshTimer->stop();
        fuelPumpOneshotButton->setEnabled(true);
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
    iacDialog->show();
}

/**
 * Sets the type of lambda trim to read from the ECU.
 * @param Set to 1 for short-term, 2 for long-term
 */
void MainWindow::onLambdaTrimButtonClicked(int id)
{
    cux->setLambdaTrimType(id);
    leftFuelTrimBar->setValue(0);
    leftFuelTrimBarLabel->setText("+0%");
    rightFuelTrimBar->setValue(0);
    rightFuelTrimBarLabel->setText("+0%");
}

/**
 * Sets the type of MAF reading to read from the ECU.
 * @param Set to 1 for Linearized, 2 for Direct
 */
void MainWindow::onMAFReadingButtonClicked(int id)
{
    if (id == 1)
    {
        cux->setMAFReadingType(Comm14CUXAirflowType_Linearized);
    }
    else
    {
        cux->setMAFReadingType(Comm14CUXAirflowType_Direct);
    }
    mafReadingBar->setValue(0);
}

/**
 * Sets the type of throttle position to display.
 * @param Set to 1 for Absolute, 2 for Corrected
 */
void MainWindow::onThrottleTypeButtonClicked(int id)
{
    if (id == 1)
    {
        cux->setThrottleReadingType(Comm14CUXThrottlePosType_Absolute);
    }
    else
    {
        cux->setThrottleReadingType(Comm14CUXThrottlePosType_Corrected);
    }
}

/**
 * Converts a QByteArray to a hex string that represents each byte consecutively
 * (two printable digits per byte, in the style of md5sum output.)
 * @param bytes Input byte array
 * @return Hex string representation of the byte array
 */
QString MainWindow::byteArrayToHexString(QByteArray bytes)
{
    QString str("");
    QChar fillChar('0');

    for (int index = 0; index < bytes.length(); index++)
    {
        str.append(QString("%1").arg((unsigned char)bytes.at(index),2,16,fillChar));
    }

    return str.toUpper();
}
