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
    m_grid = new QGridLayout(this);

    m_inertiaSwitchLabel = new QLabel("Inertia switch:", this);
    m_heatedScreenLabel = new QLabel("Heated screen:", this);
    m_mafLabel = new QLabel("MAF:", this);
    m_mafTrimLabel = new QLabel("MAF trim:", this);
    m_throttlePositionLabel = new QLabel("Throttle position:", this);
    m_coolantTempLabel = new QLabel("Coolant temperature:", this);
    m_fuelTempLabel = new QLabel("Fuel temperature:", this);
    m_neutralSwitchLabel = new QLabel("Neutral switch:", this);
    m_airConLoadLabel = new QLabel("Air conditioner load:", this);
    m_roadSpeedLabel = new QLabel("Road speed:", this);
    m_mainRelayLabel = new QLabel("Main relay voltage:", this);
    m_tuneResistorLabel = new QLabel("Tune resistor:", this);
    m_o2SensorReferenceLabel = new QLabel("O2 sensor reference:", this);
    m_diagnosticPlugLabel = new QLabel("Diagnostic display:", this);
    m_o2LeftDutyLabel = new QLabel("O2 left duty cycle:", this);
    m_o2RightDutyLabel = new QLabel("O2 right duty cycle:", this);

    m_inertiaSwitchVal = new QLabel(this);
    m_heatedScreenVal = new QLabel(this);
    m_mafVal = new QLabel(this);
    m_mafTrimVal = new QLabel(this);
    m_throttlePositionVal = new QLabel(this);
    m_coolantTempVal = new QLabel(this);
    m_fuelTempVal = new QLabel(this);
    m_neutralSwitchVal = new QLabel(this);
    m_airConLoadVal = new QLabel(this);
    m_roadSpeedVal = new QLabel(this);
    m_mainRelayVal = new QLabel(this);
    m_tuneResistorVal = new QLabel(this);
    m_o2SensorReferenceVal = new QLabel(this);
    m_diagnosticPlugVal = new QLabel(this);
    m_o2LeftDutyVal = new QLabel(this);
    m_o2RightDutyVal = new QLabel(this);

    m_inertiaSwitchRawVal = new QLineEdit(this);
    m_heatedScreenRawVal = new QLineEdit(this);
    m_mafRawVal = new QLineEdit(this);
    m_mafTrimRawVal = new QLineEdit(this);
    m_throttlePositionRawVal = new QLineEdit(this);
    m_coolantTempRawVal = new QLineEdit(this);
    m_fuelTempRawVal = new QLineEdit(this);
    m_neutralSwitchRawVal = new QLineEdit(this);
    m_airConLoadRawVal = new QLineEdit(this);
    m_roadSpeedRawVal = new QLineEdit(this);
    m_mainRelayRawVal = new QLineEdit(this);
    m_tuneResistorRawVal = new QLineEdit(this);
    m_o2SensorReferenceRawVal = new QLineEdit(this);
    m_diagnosticPlugRawVal = new QLineEdit(this);
    m_o2LeftDutyRawVal = new QLineEdit(this);
    m_o2RightDutyRawVal = new QLineEdit(this);

    m_inertiaSwitchBox = new QCheckBox(this);
    connect(m_inertiaSwitchBox, SIGNAL(toggled(bool)), this, SLOT(onInertiaSwitchChanged(bool)));

    m_heatedScreenBox = new QCheckBox(this);
    connect(m_heatedScreenBox, SIGNAL(toggled(bool)), this, SLOT(onHeatedScreenChanged(bool)));

    m_airConLoadBox = new QCheckBox(this);
    connect(m_airConLoadBox, SIGNAL(toggled(bool)), this, SLOT(onAirConLoadChanged(bool)));

    m_diagnosticPlugBox = new QCheckBox(this);
    connect(m_diagnosticPlugBox, SIGNAL(toggled(bool)), this, SLOT(onDiagnosticPlugChanged(bool)));

    m_neutralSwitchBox = new QComboBox(this);
    m_neutralSwitchBox->addItem("Park/Neutral");
    m_neutralSwitchBox->addItem("Manual");
    m_neutralSwitchBox->addItem("Drive/Reverse");
    connect(m_neutralSwitchBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onNeutralSwitchChanged(int)));

    m_mafSlider = new QSlider(Qt::Horizontal, this);
    m_mafSlider->setMinimum(0);
    m_mafSlider->setMaximum(50);
    m_mafSlider->setMinimumWidth(200);
    connect(m_mafSlider, SIGNAL(valueChanged(int)), this, SLOT(onMafChanged(int)));

    m_mafTrimSlider = new QSlider(Qt::Horizontal, this);
    m_mafTrimSlider->setMinimum(0);
    m_mafTrimSlider->setMaximum(50);
    m_mafTrimSlider->setMinimumWidth(200);
    connect(m_mafTrimSlider, SIGNAL(valueChanged(int)), this, SLOT(onMafTrimChanged(int)));
    m_mafTrimSlider->setEnabled(false);

    m_coolantTempSlider = new QSlider(Qt::Horizontal, this);
    m_coolantTempSlider->setMinimum(-40);
    m_coolantTempSlider->setMaximum(250);
    m_coolantTempSlider->setMinimumWidth(200);
    connect(m_coolantTempSlider, SIGNAL(valueChanged(int)), this, SLOT(onCoolantTempChanged(int)));

    m_fuelTempSlider = new QSlider(Qt::Horizontal, this);
    m_fuelTempSlider->setMinimum(-40);
    m_fuelTempSlider->setMaximum(250);
    m_fuelTempSlider->setMinimumWidth(200);
    connect(m_fuelTempSlider, SIGNAL(valueChanged(int)), this, SLOT(onFuelTempChanged(int)));

    m_roadSpeedSlider = new QSlider(Qt::Horizontal, this);
    m_roadSpeedSlider->setMinimum(0);
    m_roadSpeedSlider->setMaximum(158);
    m_roadSpeedSlider->setMinimumWidth(200);
    connect(m_roadSpeedSlider, SIGNAL(valueChanged(int)), this, SLOT(onRoadSpeedChanged(int)));

    m_throttleSlider = new QSlider(Qt::Horizontal, this);
    m_throttleSlider->setMinimum(0);
    m_throttleSlider->setMaximum(100);
    m_throttleSlider->setMinimumWidth(200);
    connect(m_throttleSlider, SIGNAL(valueChanged(int)), this, SLOT(onThrottleChanged(int)));

    m_mainRelaySlider = new QSlider(Qt::Horizontal, this);
    m_mainRelaySlider->setMinimum(80);
    m_mainRelaySlider->setMaximum(160);
    m_mainRelaySlider->setMinimumWidth(200);
    connect(m_mainRelaySlider, SIGNAL(valueChanged(int)), this, SLOT(onMainRelayVoltageChanged(int)));

    m_o2LeftDutySlider = new QSlider(Qt::Horizontal, this);
    m_o2LeftDutySlider->setMinimum(0);
    m_o2LeftDutySlider->setMaximum(10);
    m_o2LeftDutySlider->setMinimumWidth(200);
    connect(m_o2LeftDutySlider, SIGNAL(valueChanged(int)), this, SLOT(onO2LeftDutyChanged(int)));

    m_o2RightDutySlider = new QSlider(Qt::Horizontal, this);
    m_o2RightDutySlider->setMinimum(0);
    m_o2RightDutySlider->setMaximum(10);
    m_o2RightDutySlider->setMinimumWidth(200);
    connect(m_o2RightDutySlider, SIGNAL(valueChanged(int)), this, SLOT(onO2RightDutyChanged(int)));

    m_writeButton = new QPushButton("Write", this);
    connect(m_writeButton, SIGNAL(clicked()), this, SLOT(onWriteClicked()));

    m_closeButton = new QPushButton("Close", this);
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(onCloseClicked()));

    int row = 0;

    m_grid->addWidget(m_inertiaSwitchLabel,     row,   0, Qt::AlignRight);
    m_grid->addWidget(m_inertiaSwitchBox,       row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_inertiaSwitchVal,       row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_inertiaSwitchRawVal,    row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_heatedScreenLabel,      row,   0, Qt::AlignRight);
    m_grid->addWidget(m_heatedScreenBox,        row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_heatedScreenVal,        row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_heatedScreenRawVal,     row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_mafLabel,               row,   0, Qt::AlignRight);
    m_grid->addWidget(m_mafSlider,              row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_mafVal,                 row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_mafRawVal,              row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_mafTrimLabel,           row,   0, Qt::AlignRight);
    m_grid->addWidget(m_mafTrimSlider,          row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_mafTrimVal,             row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_mafTrimRawVal,          row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_throttlePositionLabel,  row,   0, Qt::AlignRight);
    m_grid->addWidget(m_throttleSlider,         row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_throttlePositionVal,    row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_throttlePositionRawVal, row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_coolantTempLabel,       row,   0, Qt::AlignRight);
    m_grid->addWidget(m_coolantTempSlider,      row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_coolantTempVal,         row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_coolantTempRawVal,      row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_fuelTempLabel,          row,   0, Qt::AlignRight);
    m_grid->addWidget(m_fuelTempSlider,         row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_fuelTempVal,            row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_fuelTempRawVal,         row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_neutralSwitchLabel,     row,   0, Qt::AlignRight);
    m_grid->addWidget(m_neutralSwitchBox,       row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_neutralSwitchVal,       row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_neutralSwitchRawVal,    row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_airConLoadLabel,        row,   0, Qt::AlignRight);
    m_grid->addWidget(m_airConLoadBox,          row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_airConLoadVal,          row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_airConLoadRawVal,       row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_roadSpeedLabel,         row,   0, Qt::AlignRight);
    m_grid->addWidget(m_roadSpeedSlider,        row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_roadSpeedVal,           row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_roadSpeedRawVal,        row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_mainRelayLabel,         row,   0, Qt::AlignRight);
    m_grid->addWidget(m_mainRelaySlider,        row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_mainRelayVal,           row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_mainRelayRawVal,        row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_tuneResistorLabel,      row,   0, Qt::AlignRight);
    m_grid->addWidget(m_tuneResistorVal,        row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_tuneResistorRawVal,     row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_o2SensorReferenceLabel, row,   0, Qt::AlignRight);
    m_grid->addWidget(m_o2SensorReferenceVal,   row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_o2SensorReferenceRawVal,row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_o2LeftDutyLabel,        row,   0, Qt::AlignRight);
    m_grid->addWidget(m_o2LeftDutySlider,       row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_o2LeftDutyVal,          row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_o2LeftDutyRawVal,       row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_o2RightDutyLabel,       row,   0, Qt::AlignRight);
    m_grid->addWidget(m_o2RightDutySlider,      row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_o2RightDutyVal,         row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_o2RightDutyRawVal,      row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_diagnosticPlugLabel,    row,   0, Qt::AlignRight);
    m_grid->addWidget(m_diagnosticPlugBox,      row,   1, Qt::AlignCenter);
    m_grid->addWidget(m_diagnosticPlugVal,      row,   2, Qt::AlignLeft);
    m_grid->addWidget(m_diagnosticPlugRawVal,   row++, 3, Qt::AlignLeft);

    m_grid->addWidget(m_writeButton, row, 1);
    m_grid->addWidget(m_closeButton, row, 2);

    m_inertiaSwitchBox->setChecked(true);
    m_neutralSwitchBox->setCurrentIndex(1);
    m_coolantTempSlider->setValue(50);
    m_fuelTempSlider->setValue(50);
    m_throttleSlider->setValue(4);
    m_mafSlider->setValue(16);
    m_tuneResistorRawVal->setText("0xFF");
    m_mainRelaySlider->setValue(140);
    m_o2SensorReferenceRawVal->setText("0x16");
    m_o2LeftDutySlider->setValue(5);
    m_o2RightDutySlider->setValue(5);
    onHeatedScreenChanged(false);
    onAirConLoadChanged(false);
    onRoadSpeedChanged(0);
    onMafTrimChanged(0);
    onDiagnosticPlugChanged(false);
    m_mafTrimRawVal->setText("0x00");
}

void SimulationModeDialog::onCloseClicked()
{
    this->hide();
}

void SimulationModeDialog::onWriteClicked()
{
    SimulationInputValues vals;
    bool ok = true;
    vals.airConLoad = m_airConLoadRawVal->text().toInt(&ok, 16);
    vals.maf = m_mafRawVal->text().toInt(&ok, 16);
    vals.mafTrim = m_mafTrimRawVal->text().toInt(&ok, 16);
    vals.coolantTemp = m_coolantTempRawVal->text().toInt(&ok, 16);
    vals.diagnosticPlug = m_diagnosticPlugRawVal->text().toInt(&ok, 16);
    vals.fuelTemp = m_fuelTempRawVal->text().toInt(&ok, 16);
    vals.heatedScreen = m_heatedScreenRawVal->text().toInt(&ok, 16);
    vals.inertiaSwitch = m_inertiaSwitchRawVal->text().toInt(&ok, 16);
    vals.mainRelay = m_mainRelayRawVal->text().toInt(&ok, 16);
    vals.neutralSwitch = m_neutralSwitchRawVal->text().toInt(&ok, 16);
    vals.o2SensorReference = m_o2SensorReferenceRawVal->text().toInt(&ok, 16);
    vals.roadSpeed = m_roadSpeedRawVal->text().toInt(&ok, 16);
    vals.throttle = m_throttlePositionRawVal->text().toInt(&ok, 16);
    vals.tuneResistor = m_tuneResistorRawVal->text().toInt(&ok, 16);
    vals.o2LeftDutyCycle = m_o2LeftDutyRawVal->text().toInt(&ok, 16);
    vals.o2RightDutyCycle = m_o2RightDutyRawVal->text().toInt(&ok, 16);

    emit writeSimulationInputValues(vals);
}

void SimulationModeDialog::onCoolantTempChanged(int val)
{
    m_coolantTempRawVal->setText(QString("%1").sprintf("0x%02X", (int)Peak_LorentzianModifiedPeakG_model((double)val)));
    m_coolantTempVal->setText(QString("%1 F").arg(val));
}

void SimulationModeDialog::onFuelTempChanged(int val)
{
    m_fuelTempRawVal->setText(QString("%1").sprintf("0x%02X", (int)Peak_LorentzianModifiedPeakG_model((double)val)));
    m_fuelTempVal->setText(QString("%1 F").arg(val));
}

void SimulationModeDialog::onThrottleChanged(int val)
{
    m_throttlePositionVal->setText(QString("%1%").arg(val));
    m_throttlePositionRawVal->setText(QString("%1").sprintf("0x%04X", (val*1024)/100));
}

void SimulationModeDialog::onRoadSpeedChanged(int val)
{
    m_roadSpeedRawVal->setText(QString("%1").sprintf("0x%02X", (int)(val*1.6093)));
    m_roadSpeedVal->setText(QString("%1 MPH").arg(val));
}

void SimulationModeDialog::onMainRelayVoltageChanged(int val)
{
    float voltage = val / 10.0;
    m_mainRelayVal->setText(QString("%1 VDC").arg(voltage));
    unsigned int storedVal = convertVoltageToQuadraticCounts(voltage);
    m_mainRelayRawVal->setText(QString("%1").sprintf("0x%02X", storedVal));
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

    m_neutralSwitchRawVal->setText(rawVal);
}

void SimulationModeDialog::onHeatedScreenChanged(bool checked)
{
    if (checked)
    {
        m_heatedScreenVal->setText("On");
        m_heatedScreenRawVal->setText("0xFF");
    }
    else
    {
        m_heatedScreenVal->setText("Off");
        m_heatedScreenRawVal->setText("0x00");
    }
}

void SimulationModeDialog::onInertiaSwitchChanged(bool checked)
{
    if (checked)
    {
        m_inertiaSwitchVal->setText("Closed");
        m_inertiaSwitchRawVal->setText("0xFF");
    }
    else
    {
        m_inertiaSwitchVal->setText("Open");
        m_inertiaSwitchRawVal->setText("0x00");
    }
}

void SimulationModeDialog::onAirConLoadChanged(bool checked)
{
    if (checked)
    {
        m_airConLoadVal->setText("On");
        m_airConLoadRawVal->setText("0xFF");
    }
    else
    {
        m_airConLoadVal->setText("Off");
        m_airConLoadRawVal->setText("0x00");
    }
}

void SimulationModeDialog::onDiagnosticPlugChanged(bool checked)
{
    if (checked)
    {
        m_diagnosticPlugVal->setText("Connected");
        m_diagnosticPlugRawVal->setText("0xFF");
    }
    else
    {
        m_diagnosticPlugVal->setText("Disconnected");
        m_diagnosticPlugRawVal->setText("0x00");
    }
}

void SimulationModeDialog::onMafChanged(int val)
{
    m_mafVal->setText(QString("%1 VDC").arg(val / 10.0));
    m_mafRawVal->setText(QString("%1").sprintf("0x%04X", (val*1024)/50));
}

void SimulationModeDialog::onMafTrimChanged(int val)
{
}

void SimulationModeDialog::onO2LeftDutyChanged(int val)
{
    m_o2LeftDutyVal->setText(QString("%1% duty").arg(val * 10));
    m_o2LeftDutyRawVal->setText(QString("%1").sprintf("0x%02X", val));
}

void SimulationModeDialog::onO2RightDutyChanged(int val)
{
    m_o2RightDutyVal->setText(QString("%1% duty").arg(val * 10));
    m_o2RightDutyRawVal->setText(QString("%1").sprintf("0x%02X", val));
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
