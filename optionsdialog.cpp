#include <QSettings>
#include "ui_optionsdialog.h"
#include "optionsdialog.h"
#include "serialdevenumerator.h"
#include "comm14cux.h"

/**
 * Constructor; sets up the options-dialog UI and sets settings-file field names.
 */
OptionsDialog::OptionsDialog(QString title, QWidget* parent) : QDialog(parent),
  m_ui(new Ui::OptionsDialog),
  m_serialDeviceChanged(false),
  m_settingsGroupName("Settings"),
  m_settingSerialDev("SerialDevice"),
  m_settingRefreshFuelMap("RefreshFuelMap"),
  m_settingSoftHighlight("SoftHighlight"),
  m_settingSpeedUnits("SpeedUnits"),
  m_settingDisplayNumBase("FuelMapDisplayNumberBase"),
  m_settingTemperatureUnits("TemperatureUnits"),
  m_settingSpeedoAdjust("SpeedometerAdjustment"),
  m_settingSpeedoMultiplier("SpeedometerMultiplier"),
  m_settingSpeedoOffset("SpeedometerOffset")
{
  m_ui->setupUi(this);

  m_sampleTypeNames[SampleType_EngineTemperature] = "SampleType_EngineTemperature";
  m_sampleTypeNames[SampleType_RoadSpeed] = "SampleType_RoadSpeed";
  m_sampleTypeNames[SampleType_EngineRPM] = "SampleType_EngineRPM";
  m_sampleTypeNames[SampleType_FuelTemperature] = "SampleType_FuelTemperature";
  m_sampleTypeNames[SampleType_MAF] = "SampleType_MAF";
  m_sampleTypeNames[SampleType_Throttle] = "SampleType_Throttle";
  m_sampleTypeNames[SampleType_IdleBypassPosition] = "SampleType_IdleBypassPosition";
  m_sampleTypeNames[SampleType_TargetIdleRPM] = "SampleType_TargetIdleRPM";
  m_sampleTypeNames[SampleType_GearSelection] = "SampleType_GearSelection";
  m_sampleTypeNames[SampleType_MainVoltage] = "SampleType_MainVoltage";
  m_sampleTypeNames[SampleType_LambdaTrimLong] = "SampleType_LambdaTrim";
  m_sampleTypeNames[SampleType_COTrimVoltage] = "SampleType_COTrimVoltage";
  m_sampleTypeNames[SampleType_FuelMapData] = "SampleType_FuelMap";
  m_sampleTypeNames[SampleType_FuelPumpRelay] = "SampleType_FuelPumpRelay";
  m_sampleTypeNames[SampleType_InjectorPulseWidth] = "SampleType_InjectorPulseWidth";

  m_sampleTypeLabels[SampleType_EngineTemperature] = "Engine temperature";
  m_sampleTypeLabels[SampleType_RoadSpeed] = "Road speed";
  m_sampleTypeLabels[SampleType_EngineRPM] = "Engine RPM";
  m_sampleTypeLabels[SampleType_FuelTemperature] = "Fuel temperature";
  m_sampleTypeLabels[SampleType_MAF] = "Mass airflow";
  m_sampleTypeLabels[SampleType_Throttle] = "Throttle position";
  m_sampleTypeLabels[SampleType_IdleBypassPosition] = "Idle bypass position";
  m_sampleTypeLabels[SampleType_TargetIdleRPM] = "Idle mode / target RPM";
  m_sampleTypeLabels[SampleType_GearSelection] = "Gear selection";
  m_sampleTypeLabels[SampleType_MainVoltage] = "Main voltage";
  m_sampleTypeLabels[SampleType_LambdaTrimLong] = "Lambda trim";
  m_sampleTypeLabels[SampleType_COTrimVoltage] = "MAF CO trim";
  m_sampleTypeLabels[SampleType_FuelMapData] = "Fuel map data";
  m_sampleTypeLabels[SampleType_FuelPumpRelay] = "Fuel pump relay";
  m_sampleTypeLabels[SampleType_InjectorPulseWidth] = "Injector pulse width / duty cycle";

  // We try to keep the nonzero intervals prime to avoid statistical
  // clustering of read calls to the library.
  m_readIntervalsMs[SampleType_EngineTemperature]  = 1499;
  m_readIntervalsMs[SampleType_RoadSpeed]          = 997;
  m_readIntervalsMs[SampleType_EngineRPM]          = 0;
  m_readIntervalsMs[SampleType_FuelTemperature]    = 1801;
  m_readIntervalsMs[SampleType_MAF]                = 0;
  m_readIntervalsMs[SampleType_Throttle]           = 0;
  m_readIntervalsMs[SampleType_IdleBypassPosition] = 0;
  m_readIntervalsMs[SampleType_TargetIdleRPM]      = 487;
  m_readIntervalsMs[SampleType_GearSelection]      = 563;
  m_readIntervalsMs[SampleType_MainVoltage]        = 283;
  m_readIntervalsMs[SampleType_LambdaTrimShort]    = 0;
  m_readIntervalsMs[SampleType_LambdaTrimLong]     = 331;
  m_readIntervalsMs[SampleType_COTrimVoltage]      = 317;
  m_readIntervalsMs[SampleType_FuelPumpRelay]      = 313;
  m_readIntervalsMs[SampleType_FuelMapRowCol]      = 0;
  m_readIntervalsMs[SampleType_FuelMapData]        = 3511;
  m_readIntervalsMs[SampleType_FuelMapIndex]       = 1201;
  m_readIntervalsMs[SampleType_InjectorPulseWidth] = 0;
  m_readIntervalsMs[SampleType_MIL]                = 347;

  this->setWindowTitle(title);
  readSettings();
  setupWidgets();
}

/**
 * Instantiates widgets, connects to their signals, and places them on the form.
 */
void OptionsDialog::setupWidgets()
{
  unsigned char column = 0;
  unsigned char row = 0;
  unsigned char checkboxIndex = 0;

  foreach(const SampleType sType, m_sampleTypeNames.keys())
  {
    m_enabledSamplesBoxes.insert(sType, new QCheckBox(m_sampleTypeLabels[sType], this));
  }

  const unsigned char numCheckboxesPerColumn = m_enabledSamplesBoxes.count() / 2;

  SerialDevEnumerator serialDevs;
  m_ui->m_serialDeviceBox->addItems(serialDevs.getSerialDevList(m_serialDeviceName));

  setWidgetValues();

  foreach(QCheckBox * sampleCheckBox, m_enabledSamplesBoxes)
  {
    m_ui->m_checkboxLayout->addWidget(sampleCheckBox, row, column);
    column = (checkboxIndex < numCheckboxesPerColumn) ? 0 : 1;
    row = (checkboxIndex == numCheckboxesPerColumn) ? 0 : (row + 1);
    checkboxIndex += 1;
  }

  connect(m_ui->m_checkAllButton, SIGNAL(clicked()), this, SLOT(checkAll()));
  connect(m_ui->m_uncheckAllButton, SIGNAL(clicked()), this, SLOT(uncheckAll()));

  connect(m_ui->m_okButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(m_ui->m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  connect(m_ui->m_adjustSpeedoCheckbox, SIGNAL(toggled(bool)), this, SLOT(toggledSpeedoAdjust(bool)));
}

void OptionsDialog::setWidgetValues()
{
  foreach(SampleType sType, m_sampleTypeNames.keys())
  {
    m_enabledSamplesBoxes[sType]->setChecked(m_enabledSamples[sType]);
  }

  m_ui->m_serialDeviceBox->setCurrentText(m_serialDeviceName);
  m_ui->m_speedUnitsBox->setCurrentIndex((int)m_speedUnits);
  m_ui->m_temperatureUnitsBox->setCurrentIndex((int)m_tempUnits);
  // set to combo box index 0 for decimal, index 1 otherwise (since hex is the only other option)
  m_ui->m_fuelMapDispBaseBox->setCurrentIndex((m_displayNumberBase == 10) ? 0 : 1);

  m_ui->m_refreshFuelMapCheckbox->setChecked(m_refreshFuelMap);
  m_ui->m_softHighlightCheckbox->setChecked(m_softHighlight);

  m_ui->m_adjustSpeedoCheckbox->setChecked(m_speedoAdjust);
  m_ui->m_speedoMultiplierSpinbox->setValue(m_speedoMultiplier);
  m_ui->m_speedoOffsetSpinbox->setValue(m_speedoOffset);

  m_ui->m_speedoMultiplierLabel->setEnabled(m_speedoAdjust);
  m_ui->m_speedoMultiplierSpinbox->setEnabled(m_speedoAdjust);
  m_ui->m_speedoOffsetLabel->setEnabled(m_speedoAdjust);
  m_ui->m_speedoOffsetSpinbox->setEnabled(m_speedoAdjust);
}

void OptionsDialog::toggledSpeedoAdjust(bool value)
{
  m_ui->m_speedoMultiplierLabel->setEnabled(value);
  m_ui->m_speedoMultiplierSpinbox->setEnabled(value);
  m_ui->m_speedoOffsetLabel->setEnabled(value);
  m_ui->m_speedoOffsetSpinbox->setEnabled(value);
}

void OptionsDialog::checkAll()
{
  foreach(QCheckBox* box, m_enabledSamplesBoxes)
  {
    box->setChecked(true);
  }
}

void OptionsDialog::uncheckAll()
{
  foreach(QCheckBox* box, m_enabledSamplesBoxes)
  {
    box->setChecked(false);
  }
}

/**
 * Reads the new settings from the form controls.
 */
void OptionsDialog::accept()
{
  const QString newSerialDeviceName = m_ui->m_serialDeviceBox->currentText();

  // set a flag if the serial device has been changed;
  // the main application needs to know if it should
  // reconnect to the 14CUX
  if (m_serialDeviceName.compare(newSerialDeviceName) != 0)
  {
    m_serialDeviceName = newSerialDeviceName;
    m_serialDeviceChanged = true;
  }
  else
  {
    m_serialDeviceChanged = false;
  }

  m_tempUnits        = (TemperatureUnits)(m_ui->m_temperatureUnitsBox->currentIndex());
  m_speedUnits       = (SpeedUnits)(m_ui->m_speedUnitsBox->currentIndex());
  m_refreshFuelMap   = m_ui->m_refreshFuelMapCheckbox->isChecked();
  m_softHighlight    = m_ui->m_softHighlightCheckbox->isChecked();
  m_speedoAdjust     = m_ui->m_adjustSpeedoCheckbox->isChecked();
  m_speedoMultiplier = m_ui->m_speedoMultiplierSpinbox->value();
  m_speedoOffset     = m_ui->m_speedoOffsetSpinbox->value();

  // display number based (used for rendering fuel map cell data:
  // first entry in list is 'decimal', otherwise use hex
  m_displayNumberBase = (m_ui->m_fuelMapDispBaseBox->currentIndex() == 0) ? 10 : 16;

  foreach(SampleType sType, m_sampleTypeNames.keys())
  {
    m_enabledSamples[sType] = m_enabledSamplesBoxes[sType]->isChecked();
  }

  // special case for the MIL; this is always enabled
  m_enabledSamples[SampleType_MIL] = true;

  writeSettings();
  done(QDialog::Accepted);
}

/**
 * Cancels any changes in the options dialog and keeps the existing settings.
 */
void OptionsDialog::reject()
{
  // the latest changes are being discarded, so re-read the last saved
  // settings and adjust the widgets to match
  readSettings();
  setWidgetValues();
  done(QDialog::Rejected);
}

/**
 * Reads values for all the settings (either from the settings file, or by returning defaults.)
 */
void OptionsDialog::readSettings()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RoverGauge");

  settings.beginGroup(m_settingsGroupName);
  m_serialDeviceName = settings.value(m_settingSerialDev, "").toString();
  m_speedUnits = (SpeedUnits)(settings.value(m_settingSpeedUnits, MPH).toInt());
  m_tempUnits = (TemperatureUnits)(settings.value(m_settingTemperatureUnits, Fahrenheit).toInt());
  m_displayNumberBase = settings.value(m_settingDisplayNumBase, 16).toInt();
  m_refreshFuelMap = settings.value(m_settingRefreshFuelMap, false).toBool();
  m_softHighlight = settings.value(m_settingSoftHighlight, false).toBool();
  m_speedoAdjust = settings.value(m_settingSpeedoAdjust, false).toBool();
  m_speedoMultiplier = settings.value(m_settingSpeedoMultiplier, 1.0).toDouble();
  m_speedoOffset = settings.value(m_settingSpeedoOffset, 0).toInt();

  foreach(SampleType sType, m_sampleTypeNames.keys())
  {
    m_enabledSamples[sType] = settings.value(m_sampleTypeNames[sType], true).toBool();
  }

  // special case for the MIL; this is always enabled
  m_enabledSamples[SampleType_MIL] = true;

  groupLikeSettings();

  settings.endGroup();
}

/**
 * Writes settings out to a file on disk.
 */
void OptionsDialog::writeSettings()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RoverGauge");

  settings.beginGroup(m_settingsGroupName);
  settings.setValue(m_settingSerialDev, m_serialDeviceName);
  settings.setValue(m_settingSpeedUnits, m_speedUnits);
  settings.setValue(m_settingTemperatureUnits, m_tempUnits);
  settings.setValue(m_settingDisplayNumBase, m_displayNumberBase);
  settings.setValue(m_settingRefreshFuelMap, m_refreshFuelMap);
  settings.setValue(m_settingSoftHighlight, m_softHighlight);
  settings.setValue(m_settingSpeedoAdjust, m_speedoAdjust);
  settings.setValue(m_settingSpeedoMultiplier, m_speedoMultiplier);
  settings.setValue(m_settingSpeedoOffset, m_speedoOffset);

  foreach(SampleType sType, m_sampleTypeNames.keys())
  {
    settings.setValue(m_sampleTypeNames[sType], m_enabledSamples[sType]);
  }

  groupLikeSettings();

  settings.endGroup();
}

/**
 * Sets all enabled samples in the same group to the same value.
 */
void OptionsDialog::groupLikeSettings()
{
  // There are a few readings that we don't let the user adjust invidivually, so
  // just force all the readings in a particular group to the same value.
  // This includes both long- and short-term lambda trim, and the three fuel
  // map related pieces of data (row/col, index, and the map data itself.)
  m_enabledSamples[SampleType_LambdaTrimShort] = m_enabledSamples[SampleType_LambdaTrimLong];
  m_enabledSamples[SampleType_FuelMapRowCol] = m_enabledSamples[SampleType_FuelMapData];
  m_enabledSamples[SampleType_FuelMapIndex] = m_enabledSamples[SampleType_FuelMapData];
}

/**
 * Returns the name of the serial device.
 */
QString OptionsDialog::getSerialDeviceName() const
{
#ifdef WIN32
  return QString("\\\\.\\%1").arg(m_serialDeviceName);
#else
  return m_serialDeviceName;
#endif
}

