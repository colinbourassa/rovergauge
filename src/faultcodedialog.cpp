#include <QMessageBox>
#include "faultcodedialog.h"

/**
 * Constructor. Creates the fault code list, creates widgets, and sets them
 *  appropriately.
 */
FaultCodeDialog::FaultCodeDialog(QString title, c14cux_faultcodes faults):
  m_rows(8)
{
  qRegisterMetaType<c14cux_faultcodes>("c14cux_faultcodes");
  this->setWindowTitle(title);
  populateFaultList();
  setupWidgets();
  lightLEDs(faults);
}

/**
 * Destructor.
 */
FaultCodeDialog::~FaultCodeDialog()
{
}

/**
 * Displays a message box with an error message about failure to clear fault codes.
 */
void FaultCodeDialog::onFaultClearFailure()
{
  QMessageBox::warning(this, "Error", "Unable to clear fault codes. Check connections.",
                       QMessageBox::Ok);
}

/**
 * Displays a message box indicating that fault codes were successfully cleared, and
 * sets the fault LEDs to match the new set of codes.
 */
void FaultCodeDialog::onFaultClearSuccess(c14cux_faultcodes faultCodes)
{
  QMessageBox::information(this, "Success", "Successfully cleared fault codes.", QMessageBox::Ok);
  lightLEDs(faultCodes);
}

/**
 * Creates the list of possible fault codes, and their descriptions.
 */
void FaultCodeDialog::populateFaultList()
{
  m_faultNames.insert(FaultCode_ROMChecksumFailure, QString("(29) ECU checksum error"));
  m_faultNames.insert(FaultCode_LambdaSensorOdd, QString("(44) Lambda sensor (odd)"));
  m_faultNames.insert(FaultCode_LambdaSensorEven, QString("(45) Lambda sensor (even)"));
  m_faultNames.insert(FaultCode_MisfireOdd, QString("(40) Misfire (odd)"));
  m_faultNames.insert(FaultCode_MisfireEven, QString("(50) Misfire (even)"));
  m_faultNames.insert(FaultCode_AirflowMeter, QString("(12) Airflow meter"));
  m_faultNames.insert(FaultCode_TuneResistor, QString("(21) Tune resistor out of range"));
  m_faultNames.insert(FaultCode_InjectorOdd, QString("(34) Injector bank (odd)"));
  m_faultNames.insert(FaultCode_InjectorEven, QString("(36) Injector bank (even)"));
  m_faultNames.insert(FaultCode_CoolantTempSensor, QString("(14) Coolant temp sensor"));
  m_faultNames.insert(FaultCode_ThrottlePot, QString("(17) Throttle pot"));
  m_faultNames.insert(FaultCode_ThrottlePotHiMAFLo, QString("(18) Throttle pot hi / MAF lo"));
  m_faultNames.insert(FaultCode_ThrottlePotLoMAFHi, QString("(19) Throttle pot lo / MAF hi"));
  m_faultNames.insert(FaultCode_PurgeValveLeak, QString("(88) Purge valve leak"));
  m_faultNames.insert(FaultCode_MixtureTooLean, QString("(26) Mixture too lean"));
  m_faultNames.insert(FaultCode_IntakeAirLeak, QString("(28) Intake air leak"));
  m_faultNames.insert(FaultCode_LowFuelPressure, QString("(23) Low fuel pressure"));
  m_faultNames.insert(FaultCode_IdleStepper, QString("(48) Idle Air Control stepper motor"));
  m_faultNames.insert(FaultCode_RoadSpeedSensor, QString("(68) Road speed sensor"));
  m_faultNames.insert(FaultCode_NeutralSwitch, QString("(69) Neutral (gear selector) switch"));
  m_faultNames.insert(FaultCode_FuelPressureOrLeak, QString("(58) Ambiguous: low fuel pressure or air leak"));
  m_faultNames.insert(FaultCode_FuelTempSensor, QString("(15) Fuel temp sensor"));
  m_faultNames.insert(FaultCode_BatteryDisconnected, QString("(02) RAM contents unreliable (battery disconnected)"));
  m_faultNames.insert(FaultCode_RAMChecksumFailure, QString("(03) Bad checksum on battery-backed RAM"));
}

/**
 * Creates, sets up, and places an LED widget for each of the possible fault
 * codes.
 */
void FaultCodeDialog::setupWidgets()
{
  m_grid = new QGridLayout(this);

  QLedIndicator* currentLed;
  QLabel* currentLabel;
  const QList<FaultCode> faultTypes = m_faultNames.keys();
  int position = 0;
  foreach(FaultCode fault, faultTypes)
  {
    // create a new LED and set its color to red
    currentLed = new QLedIndicator(this);
    currentLed->setOnColor1(QColor(255, 0, 0));
    currentLed->setOnColor2(QColor(176, 0, 2));
    currentLed->setOffColor1(QColor(20, 0, 0));
    currentLed->setOffColor2(QColor(90, 0, 2));
    currentLed->setDisabled(true);
    m_faultLights.insert(fault, currentLed);

    // create a label for this fault code
    currentLabel = new QLabel(m_faultNames[fault], this);
    m_faultLabels.insert(fault, currentLabel);

    m_grid->addWidget(currentLed, position % m_rows, position / m_rows * 2, Qt::AlignRight);
    m_grid->addWidget(currentLabel, position % m_rows, (position / m_rows * 2) + 1, Qt::AlignLeft);

    position++;
  }

  m_clearButton = new QPushButton("Clear codes", this);
  m_grid->addWidget(m_clearButton, m_rows, position / m_rows * 2 - 1, Qt::AlignCenter);
  connect(m_clearButton, &QPushButton::clicked, this, &FaultCodeDialog::clearFaultCodes);

  m_closeButton = new QPushButton("Close", this);
  m_grid->addWidget(m_closeButton, m_rows + 1, position / m_rows * 2 - 1, Qt::AlignCenter);
  connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

/**
 * Lights the appropriate fault lamps based on the provided faultcode structure.
 * @param faults Fault codes read from the 14CUX
 */
void FaultCodeDialog::lightLEDs(c14cux_faultcodes faults)
{
  m_faultLights[FaultCode_ROMChecksumFailure]->setChecked(faults.ROM_Checksum_Failure);
  m_faultLights[FaultCode_LambdaSensorOdd]->setChecked(faults.Lambda_Sensor_Odd);
  m_faultLights[FaultCode_LambdaSensorEven]->setChecked(faults.Lambda_Sensor_Even);
  m_faultLights[FaultCode_MisfireOdd]->setChecked(faults.Misfire_Odd_Bank);
  m_faultLights[FaultCode_MisfireEven]->setChecked(faults.Misfire_Even_Bank);
  m_faultLights[FaultCode_AirflowMeter]->setChecked(faults.Airflow_Meter);
  m_faultLights[FaultCode_TuneResistor]->setChecked(faults.Tune_Resistor_Out_of_Range);
  m_faultLights[FaultCode_InjectorOdd]->setChecked(faults.Injector_Odd_Bank);
  m_faultLights[FaultCode_InjectorEven]->setChecked(faults.Injector_Even_Bank);
  m_faultLights[FaultCode_CoolantTempSensor]->setChecked(faults.Coolant_Temp_Sensor);
  m_faultLights[FaultCode_ThrottlePot]->setChecked(faults.Throttle_Pot);
  m_faultLights[FaultCode_ThrottlePotHiMAFLo]->setChecked(faults.Throttle_Pot_Hi_MAF_Lo);
  m_faultLights[FaultCode_ThrottlePotLoMAFHi]->setChecked(faults.Throttle_Pot_Lo_MAF_Hi);
  m_faultLights[FaultCode_PurgeValveLeak]->setChecked(faults.Purge_Valve_Leak);
  m_faultLights[FaultCode_MixtureTooLean]->setChecked(faults.Mixture_Too_Lean);
  m_faultLights[FaultCode_IntakeAirLeak]->setChecked(faults.Intake_Air_Leak);
  m_faultLights[FaultCode_LowFuelPressure]->setChecked(faults.Low_Fuel_Pressure);
  m_faultLights[FaultCode_IdleStepper]->setChecked(faults.Idle_Valve_Stepper_Motor);
  m_faultLights[FaultCode_RoadSpeedSensor]->setChecked(faults.Road_Speed_Sensor);
  m_faultLights[FaultCode_NeutralSwitch]->setChecked(faults.Neutral_Switch);
  m_faultLights[FaultCode_FuelPressureOrLeak]->setChecked(faults.Low_Fuel_Pressure_or_Air_Leak);
  m_faultLights[FaultCode_FuelTempSensor]->setChecked(faults.Fuel_Temp_Sensor);
  m_faultLights[FaultCode_BatteryDisconnected]->setChecked(faults.Battery_Disconnected);
  m_faultLights[FaultCode_RAMChecksumFailure]->setChecked(faults.RAM_Checksum_Failure);
}

/**
 * Closes the dialog.
 */
void FaultCodeDialog::accept()
{
  done(QDialog::Accepted);
}

