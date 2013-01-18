#include <QSettings>
#include "optionsdialog.h"
#include "serialdevenumerator.h"

/**
 * Constructor; sets up the options-dialog UI and sets settings-file field names.
 */
OptionsDialog::OptionsDialog(QString title, QWidget *parent) : QDialog(parent),
    serialDeviceChanged(false),
    settingsGroupName("Settings"),
    settingSerialDev("SerialDevice"),
    settingSpeedMax("SpeedometerMax"),
    settingRedline("Redline"),
    settingSpeedUnits("SpeedUnits"),
    settingTemperatureUnits("TemperatureUnits")
{
    sampleTypeNames[SampleType_EngineTemperature] = "SampleType_EngineTemperature";
    sampleTypeNames[SampleType_RoadSpeed] = "SampleType_RoadSpeed";
    sampleTypeNames[SampleType_EngineRPM] = "SampleType_EngineRPM";
    sampleTypeNames[SampleType_FuelTemperature] = "SampleType_FuelTemperature";
    sampleTypeNames[SampleType_MAF] = "SampleType_MAF";
    sampleTypeNames[SampleType_Throttle] = "SampleType_Throttle";
    sampleTypeNames[SampleType_IdleBypassPosition] = "SampleType_IdleBypassPosition";
    sampleTypeNames[SampleType_TargetIdleRPM] = "SampleType_TargetIdleRPM";
    sampleTypeNames[SampleType_GearSelection] = "SampleType_GearSelection";
    sampleTypeNames[SampleType_MainVoltage] = "SampleType_MainVoltage";
    sampleTypeNames[SampleType_LambdaTrim] = "SampleType_LambdaTrim";
    sampleTypeNames[SampleType_FuelMap] = "SampleType_FuelMap";
    sampleTypeNames[SampleType_FuelPumpRelay] = "SampleType_FuelPumpRelay";

    sampleTypeLabels[SampleType_EngineTemperature] = "Engine temperature";
    sampleTypeLabels[SampleType_RoadSpeed] = "Road speed";
    sampleTypeLabels[SampleType_EngineRPM] = "Engine RPM";
    sampleTypeLabels[SampleType_FuelTemperature] = "Fuel temperature";
    sampleTypeLabels[SampleType_MAF] = "Mass airflow";
    sampleTypeLabels[SampleType_Throttle] = "Throttle position";
    sampleTypeLabels[SampleType_IdleBypassPosition] = "Idle bypass position";
    sampleTypeLabels[SampleType_TargetIdleRPM] = "Idle mode / target RPM";
    sampleTypeLabels[SampleType_GearSelection] = "Gear selection";
    sampleTypeLabels[SampleType_MainVoltage] = "Main voltage";
    sampleTypeLabels[SampleType_LambdaTrim] = "Lambda trim";
    sampleTypeLabels[SampleType_FuelMap] = "Fuel map data";
    sampleTypeLabels[SampleType_FuelPumpRelay] = "Fuel pump relay";

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

    grid = new QGridLayout(this);

    serialDeviceLabel = new QLabel("Serial device name:", this);
    serialDeviceBox = new QComboBox(this);

    speedMaxLabel = new QLabel("Maximum speed on speedometer:", this);
    speedMaxBox = new QSpinBox(this);

    speedUnitsLabel = new QLabel("Speed units:", this);
    speedUnitsBox = new QComboBox(this);

    temperatureUnitsLabel = new QLabel("Temperature units:", this);
    temperatureUnitsBox = new QComboBox(this);

    horizontalLineA = new QFrame(this);
    horizontalLineA->setFrameShape(QFrame::HLine);
    horizontalLineA->setFrameShadow(QFrame::Sunken);

    horizontalLineB = new QFrame(this);
    horizontalLineB->setFrameShape(QFrame::HLine);
    horizontalLineB->setFrameShadow(QFrame::Sunken);

    enabledSamplesLabel = new QLabel("Enabled readings:", this);
    foreach (SampleType sType, sampleTypeNames.keys())
    {
        enabledSamplesBoxes.insert(sType, new QCheckBox(sampleTypeLabels[sType], this));
        enabledSamplesBoxes[sType]->setChecked(enabledSamples[sType]);
    }
    numCheckboxesPerColumn = enabledSamplesBoxes.count() / 2;

    checkAllButton = new QPushButton("Enable all", this);
    uncheckAllButton = new QPushButton("Disable all", this);

    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Cancel", this);

    SerialDevEnumerator serialDevs;
    serialDeviceBox->addItems(serialDevs.getSerialDevList(serialDeviceName));
    serialDeviceBox->setEditable(true);
    serialDeviceBox->setMinimumWidth(150);

    speedMaxBox->setMaximum(250);
    speedMaxBox->setMinimum(10);
    speedMaxBox->setValue(speedMax);

    speedUnitsBox->setEditable(false);
    speedUnitsBox->addItem("MPH");
    speedUnitsBox->addItem("ft/s");
    speedUnitsBox->addItem("km/h");
    speedUnitsBox->setCurrentIndex((int)speedUnits);

    temperatureUnitsBox->setEditable(false);
    temperatureUnitsBox->addItem("Fahrenheit");
    temperatureUnitsBox->addItem("Celcius");
    temperatureUnitsBox->setCurrentIndex((int)tempUnits);

    grid->addWidget(serialDeviceLabel, row, 0);
    grid->addWidget(serialDeviceBox, row++, 1);

    grid->addWidget(speedMaxLabel, row, 0);
    grid->addWidget(speedMaxBox, row++, 1);

    grid->addWidget(speedUnitsLabel, row, 0);
    grid->addWidget(speedUnitsBox, row++, 1);

    grid->addWidget(temperatureUnitsLabel, row, 0);
    grid->addWidget(temperatureUnitsBox, row++, 1);

    grid->addWidget(horizontalLineA, row++, 0, 1, 2);

    grid->addWidget(enabledSamplesLabel, row++, 0);

    grid->addWidget(checkAllButton, row, 0);
    grid->addWidget(uncheckAllButton, row++, 1);

    checkboxStartingRow = row;

    foreach (QCheckBox *sampleCheckBox, enabledSamplesBoxes)
    {
        grid->addWidget(sampleCheckBox, row, (checkboxIndex <= numCheckboxesPerColumn) ? 0 : 1);
        row = (checkboxIndex == numCheckboxesPerColumn) ? checkboxStartingRow : (row + 1);
        checkboxIndex += 1;
    }

    if (enabledSamplesBoxes.count() % 2 == 1)
    {
        row++;
    }

    grid->addWidget(horizontalLineB, row++, 0, 1, 2);

    grid->addWidget(okButton, row, 0);
    grid->addWidget(cancelButton, row++, 1);

    connect(checkAllButton, SIGNAL(clicked()), this, SLOT(checkAll()));
    connect(uncheckAllButton, SIGNAL(clicked()), this, SLOT(uncheckAll()));

    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void OptionsDialog::checkAll()
{
    foreach (QCheckBox *box, enabledSamplesBoxes)
    {
        box->setChecked(true);
    }
}

void OptionsDialog::uncheckAll()
{
    foreach (QCheckBox *box, enabledSamplesBoxes)
    {
        box->setChecked(false);
    }
}

/**
 * Reads the new settings from the form controls.
 */
void OptionsDialog::accept()
{
    QString newSerialDeviceName = serialDeviceBox->currentText();

    // set a flag if the serial device has been changed;
    // the main application needs to know if it should
    // reconnect to the 14CUX
    if (serialDeviceName.compare(newSerialDeviceName) != 0)
    {
        serialDeviceName = newSerialDeviceName;
        serialDeviceChanged = true;
    }
    else
    {
        serialDeviceChanged = false;
    }

    speedMax = speedMaxBox->value();
    tempUnits = (TemperatureUnits)(temperatureUnitsBox->currentIndex());
    speedUnits = (SpeedUnits)(speedUnitsBox->currentIndex());

    foreach (SampleType sType, sampleTypeNames.keys())
    {
        enabledSamples[sType] = enabledSamplesBoxes[sType]->isChecked();
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

    settings.beginGroup(settingsGroupName);
    serialDeviceName = settings.value(settingSerialDev, "").toString();
    speedMax = settings.value(settingSpeedMax, 160).toInt();
    redline = settings.value(settingRedline, 5500).toInt();
    speedUnits = (SpeedUnits)(settings.value(settingSpeedUnits, MPH).toInt());
    tempUnits = (TemperatureUnits)(settings.value(settingTemperatureUnits, Fahrenheit).toInt());

    foreach (SampleType sType, sampleTypeNames.keys())
    {
        enabledSamples[sType] = settings.value(sampleTypeNames[sType], true).toBool();
    }

    settings.endGroup();
}

/**
 * Writes settings out to a file on disk.
 */
void OptionsDialog::writeSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RoverGauge");

    settings.beginGroup(settingsGroupName);
    settings.setValue(settingSerialDev, serialDeviceName);
    settings.setValue(settingSpeedMax, speedMax);
    settings.setValue(settingRedline, redline);
    settings.setValue(settingSpeedUnits, speedUnits);
    settings.setValue(settingTemperatureUnits, tempUnits);

    foreach (SampleType sType, sampleTypeNames.keys())
    {
        settings.setValue(sampleTypeNames[sType], enabledSamples[sType]);
    }

    settings.endGroup();
}

QHash<SampleType,bool> OptionsDialog::getEnabledSamples()
{
    return enabledSamples;
}

/**
 * Returns a flag indicating whether the serial-device name has been changed.
 */
bool OptionsDialog::getSerialDeviceChanged()
{
    return serialDeviceChanged;
}

/**
 * Returns the name of the serial device.
 */
QString OptionsDialog::getSerialDeviceName()
{
#ifdef WIN32
    return QString("\\\\.\\%1").arg(serialDeviceName);
#else
    return serialDeviceName;
#endif
}

/**
 * Returns the maximum value to be shown on the speedometer.
 */
int OptionsDialog::getSpeedMax()
{
    return speedMax;
}

/**
 * Returns the redline to be shown on the tachometer.
 */
int OptionsDialog::getRedline()
{
    return redline;
}

/**
 * Returns the units used for the speedometer
 */
SpeedUnits OptionsDialog::getSpeedUnits()
{
    return speedUnits;
}

/**
 * Returns the units used for the temperature gauges
 */
TemperatureUnits OptionsDialog::getTemperatureUnits()
{
    return tempUnits;
}
