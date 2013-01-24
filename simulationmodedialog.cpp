#include <QMessageBox>
#include "simulationmodedialog.h"
#include <math.h>

SimulationModeDialog::SimulationModeDialog(const QString title, QWidget *parent) :
    QDialog(parent)
{
    this->setWindowTitle(title);
    setupWidgets();
}

void SimulationModeDialog::setupWidgets()
{
    grid = new QGridLayout(this);

    inertiaSwitchLabel = new QLabel("Inertia switch:", this);
    heatedScreenLabel = new QLabel("Heated screen:", this);
    airflowALabel = new QLabel("Airflow A:", this);
    airflowBLabel = new QLabel("Airflow B:", this);
    airflowCLabel = new QLabel("Airflow C:", this);
    throttlePositionLabel = new QLabel("Throttle position:", this);
    coolantTempLabel = new QLabel("Coolant temperature:", this);
    fuelTempLabel = new QLabel("Fuel temperature:", this);
    neutralSwitchLabel = new QLabel("Neutral switch:", this);
    airConLoadLabel = new QLabel("Air conditioner load:", this);
    roadSpeedLabel = new QLabel("Road speed:", this);
    mainRelayLabel = new QLabel("Main relay voltage:", this);
    tuneResistorLabel = new QLabel("Tune resistor:", this);
    o2SensorReferenceLabel = new QLabel("O2 sensor reference:", this);
    diagnosticPlugLabel = new QLabel("Diagnostic plug:", this);

    inertiaSwitchVal = new QLabel(this);
    heatedScreenVal = new QLabel(this);
    airflowAVal = new QLabel(this);
    airflowBVal = new QLabel(this);
    airflowCVal = new QLabel(this);
    throttlePositionVal = new QLabel(this);
    coolantTempVal = new QLabel(this);
    fuelTempVal = new QLabel(this);
    neutralSwitchVal = new QLabel(this);
    airConLoadVal = new QLabel(this);
    roadSpeedVal = new QLabel(this);
    mainRelayVal = new QLabel(this);
    tuneResistorVal = new QLabel(this);
    o2SensorReferenceVal = new QLabel(this);
    diagnosticPlugVal = new QLabel(this);

    inertiaSwitchRawVal = new QLineEdit(this);
    heatedScreenRawVal = new QLineEdit(this);
    airflowARawVal = new QLineEdit(this);
    airflowBRawVal = new QLineEdit(this);
    airflowCRawVal = new QLineEdit(this);
    throttlePositionRawVal = new QLineEdit(this);
    coolantTempRawVal = new QLineEdit(this);
    fuelTempRawVal = new QLineEdit(this);
    neutralSwitchRawVal = new QLineEdit(this);
    airConLoadRawVal = new QLineEdit(this);
    roadSpeedRawVal = new QLineEdit(this);
    mainRelayRawVal = new QLineEdit(this);
    tuneResistorRawVal = new QLineEdit(this);
    o2SensorReferenceRawVal = new QLineEdit(this);
    diagnosticPlugRawVal = new QLineEdit(this);

    inertiaSwitchBox = new QCheckBox(this);
    connect(inertiaSwitchBox, SIGNAL(clicked(bool)), this, SLOT(onInertiaSwitchChanged(bool)));

    heatedScreenBox = new QCheckBox(this);
    connect(heatedScreenBox, SIGNAL(clicked(bool)), this, SLOT(onHeatedScreenChanged(bool)));

    neutralSwitchBox = new QComboBox(this);
    neutralSwitchBox->addItem("Park/Neutral");
    neutralSwitchBox->addItem("Manual");
    neutralSwitchBox->addItem("Drive/Reverse");
    connect(neutralSwitchBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onNeutralSwitchChanged(int)));

    coolantTempSlider = new QSlider(Qt::Horizontal, this);
    coolantTempSlider->setMinimum(-40);
    coolantTempSlider->setMaximum(250);
    coolantTempSlider->setMinimumWidth(200);
    connect(coolantTempSlider, SIGNAL(sliderMoved(int)), this, SLOT(onCoolantTempChanged(int)));

    fuelTempSlider = new QSlider(Qt::Horizontal, this);
    fuelTempSlider->setMinimum(-40);
    fuelTempSlider->setMaximum(250);
    fuelTempSlider->setMinimumWidth(200);
    connect(fuelTempSlider, SIGNAL(sliderMoved(int)), this, SLOT(onFuelTempChanged(int)));

    roadSpeedSlider = new QSlider(Qt::Horizontal, this);
    roadSpeedSlider->setMinimum(0);
    roadSpeedSlider->setMaximum(158);
    roadSpeedSlider->setMinimumWidth(200);
    connect(roadSpeedSlider, SIGNAL(sliderMoved(int)), this, SLOT(onRoadSpeedChanged(int)));

    throttleSlider = new QSlider(Qt::Horizontal, this);
    throttleSlider->setMinimum(0);
    throttleSlider->setMaximum(100);
    throttleSlider->setMinimumWidth(200);
    connect(throttleSlider, SIGNAL(sliderMoved(int)), this, SLOT(onThrottleChanged(int)));

    mainRelaySlider = new QSlider(Qt::Horizontal, this);
    mainRelaySlider->setMinimum(80);
    mainRelaySlider->setMaximum(160);
    mainRelaySlider->setMinimumWidth(200);
    connect(mainRelaySlider, SIGNAL(sliderMoved(int)), this, SLOT(onMainRelayVoltageChanged(int)));

    writeButton = new QPushButton("Write", this);
    connect(writeButton, SIGNAL(clicked()), this, SLOT(onWriteClicked()));
    closeButton = new QPushButton("Close", this);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(onCloseClicked()));

    int row = 0;

    grid->addWidget(inertiaSwitchLabel,     row,   0, Qt::AlignRight);
    grid->addWidget(inertiaSwitchBox,       row,   1, Qt::AlignCenter);
    grid->addWidget(inertiaSwitchVal,       row,   2, Qt::AlignLeft);
    grid->addWidget(inertiaSwitchRawVal,    row++, 3, Qt::AlignLeft);

    grid->addWidget(heatedScreenLabel,      row,   0, Qt::AlignRight);
    grid->addWidget(heatedScreenBox,        row,   1, Qt::AlignCenter);
    grid->addWidget(heatedScreenVal,        row,   2, Qt::AlignLeft);
    grid->addWidget(heatedScreenRawVal,     row++, 3, Qt::AlignLeft);

    grid->addWidget(airflowALabel,          row,   0, Qt::AlignRight);
    grid->addWidget(airflowARawVal,         row++, 3, Qt::AlignLeft);

    grid->addWidget(airflowBLabel,          row,   0, Qt::AlignRight);
    grid->addWidget(airflowBRawVal,         row++, 3, Qt::AlignLeft);

    grid->addWidget(airflowCLabel,          row,   0, Qt::AlignRight);
    grid->addWidget(airflowCRawVal,         row++, 3, Qt::AlignLeft);

    grid->addWidget(throttlePositionLabel,  row,   0, Qt::AlignRight);
    grid->addWidget(throttleSlider,         row,   1, Qt::AlignCenter);
    grid->addWidget(throttlePositionVal,    row,   2, Qt::AlignLeft);
    grid->addWidget(throttlePositionRawVal, row++, 3, Qt::AlignLeft);

    grid->addWidget(coolantTempLabel,       row,   0, Qt::AlignRight);
    grid->addWidget(coolantTempSlider,      row,   1, Qt::AlignCenter);
    grid->addWidget(coolantTempVal,         row,   2, Qt::AlignLeft);
    grid->addWidget(coolantTempRawVal,      row++, 3, Qt::AlignLeft);

    grid->addWidget(fuelTempLabel,          row,   0, Qt::AlignRight);
    grid->addWidget(fuelTempSlider,         row,   1, Qt::AlignCenter);
    grid->addWidget(fuelTempVal,            row,   2, Qt::AlignLeft);
    grid->addWidget(fuelTempRawVal,         row++, 3, Qt::AlignLeft);

    grid->addWidget(neutralSwitchLabel,     row,   0, Qt::AlignRight);
    grid->addWidget(neutralSwitchBox,       row,   1, Qt::AlignCenter);
    grid->addWidget(neutralSwitchVal,       row,   2, Qt::AlignLeft);
    grid->addWidget(neutralSwitchRawVal,    row++, 3, Qt::AlignLeft);

    grid->addWidget(airConLoadLabel,        row,   0, Qt::AlignRight);
    grid->addWidget(airConLoadVal,          row,   2, Qt::AlignLeft);
    grid->addWidget(airConLoadRawVal,       row++, 3, Qt::AlignLeft);

    grid->addWidget(roadSpeedLabel,         row,   0, Qt::AlignRight);
    grid->addWidget(roadSpeedSlider,        row,   1, Qt::AlignCenter);
    grid->addWidget(roadSpeedVal,           row,   2, Qt::AlignLeft);
    grid->addWidget(roadSpeedRawVal,        row++, 3, Qt::AlignLeft);

    grid->addWidget(mainRelayLabel,         row,   0, Qt::AlignRight);
    grid->addWidget(mainRelaySlider,        row,   1, Qt::AlignCenter);
    grid->addWidget(mainRelayVal,           row,   2, Qt::AlignLeft);
    grid->addWidget(mainRelayRawVal,        row++, 3, Qt::AlignLeft);

    grid->addWidget(tuneResistorLabel,      row,   0, Qt::AlignRight);
    grid->addWidget(tuneResistorVal,        row,   2, Qt::AlignLeft);
    grid->addWidget(tuneResistorRawVal,     row++, 3, Qt::AlignLeft);

    grid->addWidget(o2SensorReferenceLabel, row,   0, Qt::AlignRight);
    grid->addWidget(o2SensorReferenceVal,   row,   2, Qt::AlignLeft);
    grid->addWidget(o2SensorReferenceRawVal,row++, 3, Qt::AlignLeft);

    grid->addWidget(diagnosticPlugLabel,    row,   0, Qt::AlignRight);
    grid->addWidget(diagnosticPlugVal,      row,   2, Qt::AlignLeft);
    grid->addWidget(diagnosticPlugRawVal,   row++, 3, Qt::AlignLeft);

    grid->addWidget(writeButton, row, 1);
    grid->addWidget(closeButton, row, 2);
}

void SimulationModeDialog::onCloseClicked()
{
    this->hide();
}

void SimulationModeDialog::onWriteClicked()
{
    SimulationInputValues vals;
    bool ok = true;
    vals.airConLoad = airConLoadRawVal->text().toInt(&ok, 16);
    vals.airflowA = airflowARawVal->text().toInt(&ok, 16);
    vals.airflowB = airflowBRawVal->text().toInt(&ok, 16);
    vals.airflowC = airflowCRawVal->text().toInt(&ok, 16);
    vals.coolantTemp = coolantTempRawVal->text().toInt(&ok, 16);
    vals.diagnosticPlug = diagnosticPlugRawVal->text().toInt(&ok, 16);
    vals.fuelTemp = fuelTempRawVal->text().toInt(&ok, 16);
    vals.heatedScreen = heatedScreenRawVal->text().toInt(&ok, 16);
    vals.inertiaSwitch = inertiaSwitchRawVal->text().toInt(&ok, 16);
    vals.mainRelay = mainRelayRawVal->text().toInt(&ok, 16);
    vals.neutralSwitch = neutralSwitchRawVal->text().toInt(&ok, 16);
    vals.o2SensorReference = o2SensorReferenceRawVal->text().toInt(&ok, 16);
    vals.roadSpeed = roadSpeedRawVal->text().toInt(&ok, 16);
    vals.throttle = throttlePositionRawVal->text().toInt(&ok, 16);
    vals.tuneResistor = tuneResistorRawVal->text().toInt(&ok, 16);

    emit writeSimulationInputValues(vals);
}

void SimulationModeDialog::onCoolantTempChanged(int val)
{
    coolantTempRawVal->setText(QString("%1").sprintf("0x%02X", (int)Peak_LorentzianModifiedPeakG_model((double)val)));
    coolantTempVal->setText(QString("%1 F").arg(val));
}

void SimulationModeDialog::onFuelTempChanged(int val)
{
    fuelTempRawVal->setText(QString("%1").sprintf("0x%02X", (int)Peak_LorentzianModifiedPeakG_model((double)val)));
    fuelTempVal->setText(QString("%1 F").arg(val));
}

void SimulationModeDialog::onThrottleChanged(int val)
{
    throttlePositionVal->setText(QString("%1%").arg(val));
    throttlePositionRawVal->setText(QString("%1").sprintf("0x%04X", (val*1024)/100));
}

void SimulationModeDialog::onRoadSpeedChanged(int val)
{
    roadSpeedRawVal->setText(QString("%1").sprintf("0x%02X", (int)(val*1.6093)));
    roadSpeedVal->setText(QString("%1 MPH").arg(val));
}

void SimulationModeDialog::onMainRelayVoltageChanged(int val)
{
    mainRelayVal->setText(QString("%1 VDC").arg(val / 10.0));
}

void SimulationModeDialog::onNeutralSwitchChanged(int val)
{
    QString rawVal = "0x00";

    switch (val)
    {
    case 1:
        rawVal = "0x80";
        break;
    case 2:
        rawVal = "0xFF";
        break;
    }

    neutralSwitchRawVal->setText(rawVal);
}

void SimulationModeDialog::onHeatedScreenChanged(bool checked)
{
    if (checked)
    {
        heatedScreenVal->setText("On");
        heatedScreenRawVal->setText("0xFF");
    }
    else
    {
        heatedScreenVal->setText("Off");
        heatedScreenRawVal->setText("0x00");
    }
}

void SimulationModeDialog::onInertiaSwitchChanged(bool checked)
{
    if (checked)
    {
        inertiaSwitchVal->setText("Closed");
        inertiaSwitchRawVal->setText("0xFF");
    }
    else
    {
        inertiaSwitchVal->setText("Open");
        inertiaSwitchRawVal->setText("0x00");
    }
}

double SimulationModeDialog::Peak_LorentzianModifiedPeakG_model(double x_in)
{
    double temp;
    temp = 0.0;
    // coefficients
    double a = 2.5666277010262502E+02;
    double b = -2.5052427248638512E+02;
    double c = 3.3328371288671514E+02;
    double d = 7.0575194903787395E+00;
    temp = a / (1.0 + pow((x_in-b)/c, d));
    return temp;
}

