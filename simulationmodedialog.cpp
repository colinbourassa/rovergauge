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
    mafLabel = new QLabel("MAF:", this);
    mafTrimLabel = new QLabel("MAF trim:", this);
    throttlePositionLabel = new QLabel("Throttle position:", this);
    coolantTempLabel = new QLabel("Coolant temperature:", this);
    fuelTempLabel = new QLabel("Fuel temperature:", this);
    neutralSwitchLabel = new QLabel("Neutral switch:", this);
    airConLoadLabel = new QLabel("Air conditioner load:", this);
    roadSpeedLabel = new QLabel("Road speed:", this);
    mainRelayLabel = new QLabel("Main relay voltage:", this);
    tuneResistorLabel = new QLabel("Tune resistor:", this);
    o2SensorReferenceLabel = new QLabel("O2 sensor reference:", this);
    diagnosticPlugLabel = new QLabel("Diagnostic display:", this);
    o2LeftDutyLabel = new QLabel("O2 left duty cycle:", this);
    o2RightDutyLabel = new QLabel("O2 right duty cycle:", this);

    inertiaSwitchVal = new QLabel(this);
    heatedScreenVal = new QLabel(this);
    mafVal = new QLabel(this);
    mafTrimVal = new QLabel(this);
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
    o2LeftDutyVal = new QLabel(this);
    o2RightDutyVal = new QLabel(this);

    inertiaSwitchRawVal = new QLineEdit(this);
    heatedScreenRawVal = new QLineEdit(this);
    mafRawVal = new QLineEdit(this);
    mafTrimRawVal = new QLineEdit(this);
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
    o2LeftDutyRawVal = new QLineEdit(this);
    o2RightDutyRawVal = new QLineEdit(this);

    inertiaSwitchBox = new QCheckBox(this);
    connect(inertiaSwitchBox, SIGNAL(toggled(bool)), this, SLOT(onInertiaSwitchChanged(bool)));

    heatedScreenBox = new QCheckBox(this);
    connect(heatedScreenBox, SIGNAL(toggled(bool)), this, SLOT(onHeatedScreenChanged(bool)));

    airConLoadBox = new QCheckBox(this);
    connect(airConLoadBox, SIGNAL(toggled(bool)), this, SLOT(onAirConLoadChanged(bool)));

    diagnosticPlugBox = new QCheckBox(this);
    connect(diagnosticPlugBox, SIGNAL(toggled(bool)), this, SLOT(onDiagnosticPlugChanged(bool)));

    neutralSwitchBox = new QComboBox(this);
    neutralSwitchBox->addItem("Park/Neutral");
    neutralSwitchBox->addItem("Manual");
    neutralSwitchBox->addItem("Drive/Reverse");
    connect(neutralSwitchBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onNeutralSwitchChanged(int)));

    mafSlider = new QSlider(Qt::Horizontal, this);
    mafSlider->setMinimum(0);
    mafSlider->setMaximum(50);
    mafSlider->setMinimumWidth(200);
    connect(mafSlider, SIGNAL(valueChanged(int)), this, SLOT(onMafChanged(int)));

    mafTrimSlider = new QSlider(Qt::Horizontal, this);
    mafTrimSlider->setMinimum(0);
    mafTrimSlider->setMaximum(50);
    mafTrimSlider->setMinimumWidth(200);
    connect(mafTrimSlider, SIGNAL(valueChanged(int)), this, SLOT(onMafTrimChanged(int)));
    mafTrimSlider->setEnabled(false);

    coolantTempSlider = new QSlider(Qt::Horizontal, this);
    coolantTempSlider->setMinimum(-40);
    coolantTempSlider->setMaximum(250);
    coolantTempSlider->setMinimumWidth(200);
    connect(coolantTempSlider, SIGNAL(valueChanged(int)), this, SLOT(onCoolantTempChanged(int)));

    fuelTempSlider = new QSlider(Qt::Horizontal, this);
    fuelTempSlider->setMinimum(-40);
    fuelTempSlider->setMaximum(250);
    fuelTempSlider->setMinimumWidth(200);
    connect(fuelTempSlider, SIGNAL(valueChanged(int)), this, SLOT(onFuelTempChanged(int)));

    roadSpeedSlider = new QSlider(Qt::Horizontal, this);
    roadSpeedSlider->setMinimum(0);
    roadSpeedSlider->setMaximum(158);
    roadSpeedSlider->setMinimumWidth(200);
    connect(roadSpeedSlider, SIGNAL(valueChanged(int)), this, SLOT(onRoadSpeedChanged(int)));

    throttleSlider = new QSlider(Qt::Horizontal, this);
    throttleSlider->setMinimum(0);
    throttleSlider->setMaximum(100);
    throttleSlider->setMinimumWidth(200);
    connect(throttleSlider, SIGNAL(valueChanged(int)), this, SLOT(onThrottleChanged(int)));

    mainRelaySlider = new QSlider(Qt::Horizontal, this);
    mainRelaySlider->setMinimum(80);
    mainRelaySlider->setMaximum(160);
    mainRelaySlider->setMinimumWidth(200);
    connect(mainRelaySlider, SIGNAL(valueChanged(int)), this, SLOT(onMainRelayVoltageChanged(int)));

    o2LeftDutySlider = new QSlider(Qt::Horizontal, this);
    o2LeftDutySlider->setMinimum(0);
    o2LeftDutySlider->setMaximum(10);
    o2LeftDutySlider->setMinimumWidth(200);
    connect(o2LeftDutySlider, SIGNAL(valueChanged(int)), this, SLOT(onO2LeftDutyChanged(int)));

    o2RightDutySlider = new QSlider(Qt::Horizontal, this);
    o2RightDutySlider->setMinimum(0);
    o2RightDutySlider->setMaximum(10);
    o2RightDutySlider->setMinimumWidth(200);
    connect(o2RightDutySlider, SIGNAL(valueChanged(int)), this, SLOT(onO2RightDutyChanged(int)));

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

    grid->addWidget(mafLabel,               row,   0, Qt::AlignRight);
    grid->addWidget(mafSlider,              row,   1, Qt::AlignCenter);
    grid->addWidget(mafVal,                 row,   2, Qt::AlignLeft);
    grid->addWidget(mafRawVal,              row++, 3, Qt::AlignLeft);

    grid->addWidget(mafTrimLabel,           row,   0, Qt::AlignRight);
    grid->addWidget(mafTrimSlider,          row,   1, Qt::AlignCenter);
    grid->addWidget(mafTrimVal,             row,   2, Qt::AlignLeft);
    grid->addWidget(mafTrimRawVal,          row++, 3, Qt::AlignLeft);

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
    grid->addWidget(airConLoadBox,          row,   1, Qt::AlignCenter);
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

    grid->addWidget(o2LeftDutyLabel,        row,   0, Qt::AlignRight);
    grid->addWidget(o2LeftDutySlider,       row,   1, Qt::AlignCenter);
    grid->addWidget(o2LeftDutyVal,          row,   2, Qt::AlignLeft);
    grid->addWidget(o2LeftDutyRawVal,       row++, 3, Qt::AlignLeft);

    grid->addWidget(o2RightDutyLabel,       row,   0, Qt::AlignRight);
    grid->addWidget(o2RightDutySlider,      row,   1, Qt::AlignCenter);
    grid->addWidget(o2RightDutyVal,         row,   2, Qt::AlignLeft);
    grid->addWidget(o2RightDutyRawVal,      row++, 3, Qt::AlignLeft);

    grid->addWidget(diagnosticPlugLabel,    row,   0, Qt::AlignRight);
    grid->addWidget(diagnosticPlugBox,      row,   1, Qt::AlignCenter);
    grid->addWidget(diagnosticPlugVal,      row,   2, Qt::AlignLeft);
    grid->addWidget(diagnosticPlugRawVal,   row++, 3, Qt::AlignLeft);

    grid->addWidget(writeButton, row, 1);
    grid->addWidget(closeButton, row, 2);

    inertiaSwitchBox->setChecked(true);
    neutralSwitchBox->setCurrentIndex(1);
    coolantTempSlider->setValue(50);
    fuelTempSlider->setValue(50);
    throttleSlider->setValue(4);
    tuneResistorRawVal->setText("0xFF");
    mainRelaySlider->setValue(140);
    o2SensorReferenceRawVal->setText("0x16");
    o2LeftDutySlider->setValue(5);
    o2RightDutySlider->setValue(5);
    onHeatedScreenChanged(false);
    onAirConLoadChanged(false);
    onRoadSpeedChanged(0);
    onMafTrimChanged(0);
    onDiagnosticPlugChanged(false);
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
    vals.maf = mafRawVal->text().toInt(&ok, 16);
    vals.mafTrim = mafTrimRawVal->text().toInt(&ok, 16);
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
    vals.o2LeftDutyCycle = o2LeftDutyRawVal->text().toInt(&ok, 16);
    vals.o2RightDutyCycle = o2RightDutyRawVal->text().toInt(&ok, 16);

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
    float voltage = val / 10.0;
    mainRelayVal->setText(QString("%1 VDC").arg(voltage));
    unsigned int storedVal = convertVoltageToQuadraticCounts(voltage);
    mainRelayRawVal->setText(QString("%1").sprintf("0x%02X", storedVal));
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

void SimulationModeDialog::onAirConLoadChanged(bool checked)
{
    if (checked)
    {
        airConLoadVal->setText("On");
        airConLoadRawVal->setText("0xFF");
    }
    else
    {
        airConLoadVal->setText("Off");
        airConLoadRawVal->setText("0x00");
    }
}

void SimulationModeDialog::onDiagnosticPlugChanged(bool checked)
{
    if (checked)
    {
        diagnosticPlugVal->setText("Connected");
        diagnosticPlugRawVal->setText("0xFF");
    }
    else
    {
        diagnosticPlugVal->setText("Disconnected");
        diagnosticPlugRawVal->setText("0x00");
    }
}

void SimulationModeDialog::onMafChanged(int val)
{
    mafVal->setText(QString("%1 VDC").arg(val / 10.0));
}

void SimulationModeDialog::onMafTrimChanged(int val)
{
    mafTrimRawVal->setText("0x00");
}

void SimulationModeDialog::onO2LeftDutyChanged(int val)
{
    o2LeftDutyVal->setText(QString("%1% duty").arg(val * 10));
    o2LeftDutyRawVal->setText(QString("%1").sprintf("0x%02X", val));
}

void SimulationModeDialog::onO2RightDutyChanged(int val)
{
    o2RightDutyVal->setText(QString("%1% duty").arg(val * 10));
    o2RightDutyRawVal->setText(QString("%1").sprintf("0x%02X", val));
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

unsigned int SimulationModeDialog::convertVoltageToQuadraticCounts(float voltage)
{
    uint8_t  adc = (voltage + 0.09) / 0.07;
    uint8_t  a   = (adc * adc >> 8);
    uint16_t b   = ((0x64 * a) - (adc * 0xBD) + 0x6180) >> 2;

    return b;
}
