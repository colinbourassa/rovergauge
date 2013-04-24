#include <QSettings>
#include "optionsdialog.h"
#include "serialdevenumerator.h"

/**
 * Constructor; sets up the options-dialog UI and sets settings-file field names.
 */
OptionsDialog::OptionsDialog(QString title, QWidget *parent) : QDialog(parent),
    m_serialDeviceChanged(false),
    m_settingsGroupName("Settings"),
    m_settingSerialDev("SerialDevice"),
    m_settingRedline("Redline"),
    m_settingSpeedUnits("SpeedUnits"),
    m_settingTemperatureUnits("TemperatureUnits")
{
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
    m_sampleTypeNames[SampleType_LambdaTrim] = "SampleType_LambdaTrim";
    m_sampleTypeNames[SampleType_FuelMap] = "SampleType_FuelMap";
    m_sampleTypeNames[SampleType_FuelPumpRelay] = "SampleType_FuelPumpRelay";

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
    m_sampleTypeLabels[SampleType_LambdaTrim] = "Lambda trim";
    m_sampleTypeLabels[SampleType_FuelMap] = "Fuel map data";
    m_sampleTypeLabels[SampleType_FuelPumpRelay] = "Fuel pump relay";

    this->setWindowTitle(title);
    readSettings();
    setupWidgets();
}

/**
 * Instantiates widgets, connects to their signals, and places them on the form.
 */
void OptionsDialog::setupWidgets()
{
    unsigned int row = 0;
    unsigned char numCheckboxesPerColumn = 0;
    unsigned char checkboxIndex = 0;
    unsigned int checkboxStartingRow = row;

    m_grid = new QGridLayout(this);

    m_serialDeviceLabel = new QLabel("Serial device name:", this);
    m_serialDeviceBox = new QComboBox(this);

    m_speedUnitsLabel = new QLabel("Speed units:", this);
    m_speedUnitsBox = new QComboBox(this);

    m_temperatureUnitsLabel = new QLabel("Temperature units:", this);
    m_temperatureUnitsBox = new QComboBox(this);

    m_horizontalLineA = new QFrame(this);
    m_horizontalLineA->setFrameShape(QFrame::HLine);
    m_horizontalLineA->setFrameShadow(QFrame::Sunken);

    m_horizontalLineB = new QFrame(this);
    m_horizontalLineB->setFrameShape(QFrame::HLine);
    m_horizontalLineB->setFrameShadow(QFrame::Sunken);

    m_enabledSamplesLabel = new QLabel("Enabled readings:", this);
    foreach (SampleType sType, m_sampleTypeNames.keys())
    {
        m_enabledSamplesBoxes.insert(sType, new QCheckBox(m_sampleTypeLabels[sType], this));
        m_enabledSamplesBoxes[sType]->setChecked(m_enabledSamples[sType]);
    }
    numCheckboxesPerColumn = m_enabledSamplesBoxes.count() / 2;

    m_checkAllButton = new QPushButton("Enable all", this);
    m_uncheckAllButton = new QPushButton("Disable all", this);

    m_okButton = new QPushButton("OK", this);
    m_cancelButton = new QPushButton("Cancel", this);

    SerialDevEnumerator serialDevs;
    m_serialDeviceBox->addItems(serialDevs.getSerialDevList(m_serialDeviceName));
    m_serialDeviceBox->setEditable(true);
    m_serialDeviceBox->setMinimumWidth(150);

    m_speedUnitsBox->setEditable(false);
    m_speedUnitsBox->addItem("MPH");
    m_speedUnitsBox->addItem("ft/s");
    m_speedUnitsBox->addItem("km/h");
    m_speedUnitsBox->setCurrentIndex((int)m_speedUnits);

    m_temperatureUnitsBox->setEditable(false);
    m_temperatureUnitsBox->addItem("Fahrenheit");
    m_temperatureUnitsBox->addItem("Celcius");
    m_temperatureUnitsBox->setCurrentIndex((int)m_tempUnits);

    m_grid->addWidget(m_serialDeviceLabel, row, 0);
    m_grid->addWidget(m_serialDeviceBox, row++, 1);

    m_grid->addWidget(m_speedUnitsLabel, row, 0);
    m_grid->addWidget(m_speedUnitsBox, row++, 1);

    m_grid->addWidget(m_temperatureUnitsLabel, row, 0);
    m_grid->addWidget(m_temperatureUnitsBox, row++, 1);

    m_grid->addWidget(m_horizontalLineA, row++, 0, 1, 2);

    m_grid->addWidget(m_enabledSamplesLabel, row++, 0);

    m_grid->addWidget(m_checkAllButton, row, 0);
    m_grid->addWidget(m_uncheckAllButton, row++, 1);

    checkboxStartingRow = row;

    foreach (QCheckBox *sampleCheckBox, m_enabledSamplesBoxes)
    {
        m_grid->addWidget(sampleCheckBox, row, (checkboxIndex <= numCheckboxesPerColumn) ? 0 : 1);
        row = (checkboxIndex == numCheckboxesPerColumn) ? checkboxStartingRow : (row + 1);
        checkboxIndex += 1;
    }

    if (m_enabledSamplesBoxes.count() % 2 == 1)
    {
        row++;
    }

    m_grid->addWidget(m_horizontalLineB, row++, 0, 1, 2);

    m_grid->addWidget(m_okButton, row, 0);
    m_grid->addWidget(m_cancelButton, row++, 1);

    connect(m_checkAllButton, SIGNAL(clicked()), this, SLOT(checkAll()));
    connect(m_uncheckAllButton, SIGNAL(clicked()), this, SLOT(uncheckAll()));

    connect(m_okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void OptionsDialog::checkAll()
{
    foreach (QCheckBox *box, m_enabledSamplesBoxes)
    {
        box->setChecked(true);
    }
}

void OptionsDialog::uncheckAll()
{
    foreach (QCheckBox *box, m_enabledSamplesBoxes)
    {
        box->setChecked(false);
    }
}

/**
 * Reads the new settings from the form controls.
 */
void OptionsDialog::accept()
{
    QString newSerialDeviceName = m_serialDeviceBox->currentText();

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

    m_tempUnits = (TemperatureUnits)(m_temperatureUnitsBox->currentIndex());
    m_speedUnits = (SpeedUnits)(m_speedUnitsBox->currentIndex());

    foreach (SampleType sType, m_sampleTypeNames.keys())
    {
        m_enabledSamples[sType] = m_enabledSamplesBoxes[sType]->isChecked();
    }

    writeSettings();
    done(QDialog::Accepted);
}

/**
 * Reads values for all the settings (either from the settings file, or by return defaults.)
 */
void OptionsDialog::readSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RoverGauge");

    settings.beginGroup(m_settingsGroupName);
    m_serialDeviceName = settings.value(m_settingSerialDev, "").toString();
    m_speedUnits = (SpeedUnits)(settings.value(m_settingSpeedUnits, MPH).toInt());
    m_tempUnits = (TemperatureUnits)(settings.value(m_settingTemperatureUnits, Fahrenheit).toInt());

    foreach (SampleType sType, m_sampleTypeNames.keys())
    {
        m_enabledSamples[sType] = settings.value(m_sampleTypeNames[sType], true).toBool();
    }

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

    foreach (SampleType sType, m_sampleTypeNames.keys())
    {
        settings.setValue(m_sampleTypeNames[sType], m_enabledSamples[sType]);
    }

    settings.endGroup();
}

QHash<SampleType,bool> OptionsDialog::getEnabledSamples()
{
    return m_enabledSamples;
}

/**
 * Returns a flag indicating whether the serial-device name has been changed.
 */
bool OptionsDialog::getSerialDeviceChanged()
{
    return m_serialDeviceChanged;
}

/**
 * Returns the name of the serial device.
 */
QString OptionsDialog::getSerialDeviceName()
{
#ifdef WIN32
    return QString("\\\\.\\%1").arg(m_serialDeviceName);
#else
    return m_serialDeviceName;
#endif
}

/**
 * Returns the units used for the speedometer
 */
SpeedUnits OptionsDialog::getSpeedUnits()
{
    return m_speedUnits;
}

/**
 * Returns the units used for the temperature gauges
 */
TemperatureUnits OptionsDialog::getTemperatureUnits()
{
    return m_tempUnits;
}
