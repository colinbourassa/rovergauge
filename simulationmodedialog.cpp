#include <QMessageBox>
#include "simulationmodedialog.h"
#include <math.h>

SimulationModeDialog::SimulationModeDialog(const QString title, QWidget* parent) :
  QDialog(parent)
{
  memset(&m_changes, 0x01, sizeof(SimulationInputChanges));
  qRegisterMetaType<SimulationInputValues>("SimulationInputValues");
  qRegisterMetaType<SimulationInputChanges>("SimulationInputChanges");
  this->setWindowTitle(title);
  setupWidgets();
}

void SimulationModeDialog::setupWidgets()
{
  m_grid = new QGridLayout(this);
  m_buttonLayout = new QHBoxLayout();
  m_statusBar = new QStatusBar(this);

  m_inertiaSwitchLabel = new QLabel("Inertia switch:", this);
  m_heatedScreenLabel = new QLabel("Heated screen:", this);
  m_mafLabel = new QLabel("MAF:", this);
  m_mafTrimLabel = new QLabel("MAF trim:", this);
  m_throttlePositionLabel = new QLabel("Throttle position:", this);
  m_coolantTempLabel = new QLabel("Coolant temperature:", this);
  m_fuelTempLabel = new QLabel("Fuel temperature:", this);
  m_neutralSwitchLabel = new QLabel("Neutral switch:", this);
  m_airConLoadLabel = new QLabel("Air conditioner load:", this);
  m_mainRelayLabel = new QLabel("Main relay voltage:", this);
  m_tuneResistorLabel = new QLabel("Tune resistor:", this);
  m_o2SensorReferenceLabel = new QLabel("O2 sensor reference:", this);
  m_diagnosticPlugLabel = new QLabel("Diagnostic display:", this);
  m_o2OddDutyLabel = new QLabel("O2 (odd) duty cycle:", this);
  m_o2EvenDutyLabel = new QLabel("O2 (even) duty cycle:", this);

  m_inertiaSwitchVal = new QLabel(this);
  m_heatedScreenVal = new QLabel(this);
  m_mafVal = new QLabel(this);
  m_mafTrimVal = new QLabel(this);
  m_throttlePositionVal = new QLabel(this);
  m_coolantTempVal = new QLabel(this);
  m_fuelTempVal = new QLabel(this);
  m_neutralSwitchVal = new QLabel(this);
  m_airConLoadVal = new QLabel(this);
  m_mainRelayVal = new QLabel(this);
  m_tuneResistorVal = new QLabel(this);
  m_o2SensorReferenceVal = new QLabel(this);
  m_diagnosticPlugVal = new QLabel(this);
  m_o2OddDutyVal = new QLabel(this);
  m_o2EvenDutyVal = new QLabel(this);

  m_inertiaSwitchRawVal = new QLineEdit(this);
  m_heatedScreenRawVal = new QLineEdit(this);
  m_mafRawVal = new QLineEdit(this);
  m_mafTrimRawVal = new QLineEdit(this);
  m_throttlePositionRawVal = new QLineEdit(this);
  m_coolantTempRawVal = new QLineEdit(this);
  m_fuelTempRawVal = new QLineEdit(this);
  m_neutralSwitchRawVal = new QLineEdit(this);
  m_airConLoadRawVal = new QLineEdit(this);
  m_mainRelayRawVal = new QLineEdit(this);
  m_tuneResistorRawVal = new QLineEdit(this);
  m_o2SensorReferenceRawVal = new QLineEdit(this);
  m_diagnosticPlugRawVal = new QLineEdit(this);
  m_o2OddDutyRawVal = new QLineEdit(this);
  m_o2EvenDutyRawVal = new QLineEdit(this);

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

  m_o2OddDutySlider = new QSlider(Qt::Horizontal, this);
  m_o2OddDutySlider->setMinimum(0);
  m_o2OddDutySlider->setMaximum(10);
  m_o2OddDutySlider->setMinimumWidth(200);
  connect(m_o2OddDutySlider, SIGNAL(valueChanged(int)), this, SLOT(onO2OddDutyChanged(int)));

  m_o2EvenDutySlider = new QSlider(Qt::Horizontal, this);
  m_o2EvenDutySlider->setMinimum(0);
  m_o2EvenDutySlider->setMaximum(10);
  m_o2EvenDutySlider->setMinimumWidth(200);
  connect(m_o2EvenDutySlider, SIGNAL(valueChanged(int)), this, SLOT(onO2EvenDutyChanged(int)));

  m_enableSimModeButton = new QPushButton("Enable Sim Mode", this);
  connect(m_enableSimModeButton, SIGNAL(clicked()), this, SLOT(onEnabledSimModeClicked()));

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

  m_grid->addWidget(m_mainRelayLabel,         row,   0, Qt::AlignRight);
  m_grid->addWidget(m_mainRelaySlider,        row,   1, Qt::AlignCenter);
  m_grid->addWidget(m_mainRelayVal,           row,   2, Qt::AlignLeft);
  m_grid->addWidget(m_mainRelayRawVal,        row++, 3, Qt::AlignLeft);

  m_grid->addWidget(m_tuneResistorLabel,      row,   0, Qt::AlignRight);
  m_grid->addWidget(m_tuneResistorVal,        row,   2, Qt::AlignLeft);
  m_grid->addWidget(m_tuneResistorRawVal,     row++, 3, Qt::AlignLeft);

  m_grid->addWidget(m_o2SensorReferenceLabel, row,   0, Qt::AlignRight);
  m_grid->addWidget(m_o2SensorReferenceVal,   row,   2, Qt::AlignLeft);
  m_grid->addWidget(m_o2SensorReferenceRawVal, row++, 3, Qt::AlignLeft);

  m_grid->addWidget(m_o2OddDutyLabel,        row,   0, Qt::AlignRight);
  m_grid->addWidget(m_o2OddDutySlider,       row,   1, Qt::AlignCenter);
  m_grid->addWidget(m_o2OddDutyVal,          row,   2, Qt::AlignLeft);
  m_grid->addWidget(m_o2OddDutyRawVal,       row++, 3, Qt::AlignLeft);

  m_grid->addWidget(m_o2EvenDutyLabel,       row,   0, Qt::AlignRight);
  m_grid->addWidget(m_o2EvenDutySlider,      row,   1, Qt::AlignCenter);
  m_grid->addWidget(m_o2EvenDutyVal,         row,   2, Qt::AlignLeft);
  m_grid->addWidget(m_o2EvenDutyRawVal,      row++, 3, Qt::AlignLeft);

  m_grid->addWidget(m_diagnosticPlugLabel,    row,   0, Qt::AlignRight);
  m_grid->addWidget(m_diagnosticPlugBox,      row,   1, Qt::AlignCenter);
  m_grid->addWidget(m_diagnosticPlugVal,      row,   2, Qt::AlignLeft);
  m_grid->addWidget(m_diagnosticPlugRawVal,   row++, 3, Qt::AlignLeft);

  m_grid->addLayout(m_buttonLayout, row++, 0, 1, 4);
  m_buttonLayout->addWidget(m_enableSimModeButton);
  m_buttonLayout->addWidget(m_writeButton);
  m_buttonLayout->addWidget(m_closeButton);

  m_grid->addWidget(m_statusBar, row, 0, 1, 4);
  m_statusBar->showNormal();
  m_statusBar->showMessage("Ready");

  m_inertiaSwitchBox->setChecked(true);
  m_neutralSwitchBox->setCurrentIndex(1);
  m_coolantTempSlider->setValue(50);
  m_fuelTempSlider->setValue(50);
  m_throttleSlider->setValue(4);
  m_mafSlider->setValue(16);
  m_tuneResistorRawVal->setText("0xD7");
  m_mainRelaySlider->setValue(140);
  m_o2SensorReferenceRawVal->setText("0x16");
  m_o2OddDutySlider->setValue(5);
  m_o2EvenDutySlider->setValue(5);
  onHeatedScreenChanged(false);
  onAirConLoadChanged(false);
  onMafTrimChanged(0);
  onDiagnosticPlugChanged(false);
  m_mafTrimRawVal->setText("0x00");
}

void SimulationModeDialog::onEnabledSimModeClicked()
{
  doWrite(true);
}

void SimulationModeDialog::onCloseClicked()
{
  m_statusBar->showMessage("");
  this->hide();
}

void SimulationModeDialog::onWriteClicked()
{
  doWrite(false);
}

void SimulationModeDialog::onWriteSuccess()
{
  m_statusBar->showMessage("Write success");
}

void SimulationModeDialog::onWriteFailure()
{
  m_statusBar->showMessage("Write failure");
}

void SimulationModeDialog::doWrite(bool enableSimMode)
{
  SimulationInputValues vals;
  SimulationInputChanges changes;
  bool ok = true;

  memcpy(&changes, &m_changes, sizeof(SimulationInputChanges));
  memset(&m_changes, 0x00, sizeof(SimulationInputChanges));

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
  vals.throttle = m_throttlePositionRawVal->text().toInt(&ok, 16);
  vals.tuneResistor = m_tuneResistorRawVal->text().toInt(&ok, 16);
  vals.o2OddDutyCycle = m_o2OddDutyRawVal->text().toInt(&ok, 16);
  vals.o2EvenDutyCycle = m_o2EvenDutyRawVal->text().toInt(&ok, 16);

  emit writeSimulationInputValues(enableSimMode, vals, changes);
}

void SimulationModeDialog::onCoolantTempChanged(int val)
{
  m_coolantTempRawVal->setText(QString("%1").sprintf("0x%02X", (int)Peak_LorentzianModifiedPeakG_model((double)val)));
  m_coolantTempVal->setText(QString("%1 F").arg(val));
  m_changes.coolantTemp = true;
}

void SimulationModeDialog::onFuelTempChanged(int val)
{
  m_fuelTempRawVal->setText(QString("%1").sprintf("0x%02X", (int)Peak_LorentzianModifiedPeakG_model((double)val)));
  m_fuelTempVal->setText(QString("%1 F").arg(val));
  m_changes.fuelTemp = true;
}

void SimulationModeDialog::onThrottleChanged(int val)
{
  m_throttlePositionVal->setText(QString("%1%").arg(val));
  m_throttlePositionRawVal->setText(QString("%1").sprintf("0x%04X", (val * 1024) / 100));
  m_changes.throttle = true;
}

void SimulationModeDialog::onMainRelayVoltageChanged(int val)
{
  float voltage = val / 10.0;
  m_mainRelayVal->setText(QString("%1 VDC").arg(voltage));
  unsigned int storedVal = (voltage + 0.09) / 0.07;
  m_mainRelayRawVal->setText(QString("%1").sprintf("0x%02X", storedVal));
  m_changes.mainRelay = true;
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
  m_changes.neutralSwitch = true;
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

  m_changes.heatedScreen = true;
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

  m_changes.inertiaSwitch = true;
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

  m_changes.airConLoad = true;
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

  m_changes.diagnosticPlug = true;
}

void SimulationModeDialog::onMafChanged(int val)
{
  m_mafVal->setText(QString("%1 VDC").arg(val / 10.0));
  m_mafRawVal->setText(QString("%1").sprintf("0x%04X", (val * 1024) / 50));
  m_changes.maf = true;
}

void SimulationModeDialog::onMafTrimChanged(int val)
{
  m_changes.mafTrim = true;
}

void SimulationModeDialog::onO2OddDutyChanged(int val)
{
  m_o2OddDutyVal->setText(QString("%1% duty").arg(val * 10));
  m_o2OddDutyRawVal->setText(QString("%1").sprintf("0x%02X", val));
  m_changes.o2OddDutyCycle = true;
}

void SimulationModeDialog::onO2EvenDutyChanged(int val)
{
  m_o2EvenDutyVal->setText(QString("%1% duty").arg(val * 10));
  m_o2EvenDutyRawVal->setText(QString("%1").sprintf("0x%02X", val));
  m_changes.o2EvenDutyCycle = true;
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
  temp = a / (1.0 + pow((x_in - b) / c, d));
  return temp;
}
