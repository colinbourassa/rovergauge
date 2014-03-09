#include <QSettings>
#include "ui_optionsdialog.h"
#include "optionsdialog.h"
#include "serialdevenumerator.h"

/**
 * Constructor; sets up the options-dialog UI and sets settings-file field names.
 */
OptionsDialog::OptionsDialog(QString title, QWidget *parent) : QDialog(parent),
    m_serialDeviceChanged(false),
    m_settingsGroupName("Settings"),
    m_settingSerialDev("SerialDevice"),
    m_settingRefreshFuelMap("RefreshFuelMap"),
    m_settingSpeedUnits("SpeedUnits"),
    m_settingTemperatureUnits("TemperatureUnits"),
    m_ui(new Ui::OptionsDialog)
{
    m_ui->setupUi(this);
    this->setLayout(m_ui->m_mainLayout);

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

    // Ultimately, to make the read intervals configurable, we'll want to create controls
    // on this form to handle the input. For now, they're just reasonable hardcoded values.
    m_readIntervalsMs[SampleType_EngineTemperature] = 1500;
    m_readIntervalsMs[SampleType_RoadSpeed] = 203;
    m_readIntervalsMs[SampleType_EngineRPM] = 0;
    m_readIntervalsMs[SampleType_FuelTemperature] = 1805;
    m_readIntervalsMs[SampleType_MAF] = 0;
    m_readIntervalsMs[SampleType_Throttle] = 0;
    m_readIntervalsMs[SampleType_IdleBypassPosition] = 0;
    m_readIntervalsMs[SampleType_TargetIdleRPM] = 485;
    m_readIntervalsMs[SampleType_GearSelection] = 560;
    m_readIntervalsMs[SampleType_MainVoltage] = 292;
    m_readIntervalsMs[SampleType_LambdaTrimShort] = 0;
    m_readIntervalsMs[SampleType_LambdaTrimLong] = 327;
    m_readIntervalsMs[SampleType_COTrimVoltage] = 317;
    m_readIntervalsMs[SampleType_FuelPumpRelay] = 333;
    m_readIntervalsMs[SampleType_FuelMapRowCol] = 0;
    m_readIntervalsMs[SampleType_FuelMapData] = 3500;
    m_readIntervalsMs[SampleType_FuelMapIndex] = 1200;
    m_readIntervalsMs[SampleType_MIL] = 333;

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
    unsigned char numCheckboxesPerColumn;

    foreach (SampleType sType, m_sampleTypeNames.keys())
    {
        m_enabledSamplesBoxes.insert(sType, new QCheckBox(m_sampleTypeLabels[sType], this));
        m_enabledSamplesBoxes[sType]->setChecked(m_enabledSamples[sType]);
    }
    numCheckboxesPerColumn = m_enabledSamplesBoxes.count() / 2;

    SerialDevEnumerator serialDevs;
    m_ui->m_serialDeviceBox->addItems(serialDevs.getSerialDevList(m_serialDeviceName));

    m_ui->m_speedUnitsBox->addItem("MPH");
    m_ui->m_speedUnitsBox->addItem("ft/s");
    m_ui->m_speedUnitsBox->addItem("km/h");
    m_ui->m_speedUnitsBox->setCurrentIndex((int)m_speedUnits);

    m_ui->m_temperatureUnitsBox->addItem("Fahrenheit");
    m_ui->m_temperatureUnitsBox->addItem("Celcius");
    m_ui->m_temperatureUnitsBox->setCurrentIndex((int)m_tempUnits);

    foreach (QCheckBox *sampleCheckBox, m_enabledSamplesBoxes)
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
    QString newSerialDeviceName = m_ui->m_serialDeviceBox->currentText();

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

    m_tempUnits = (TemperatureUnits)(m_ui->m_temperatureUnitsBox->currentIndex());
    m_speedUnits = (SpeedUnits)(m_ui->m_speedUnitsBox->currentIndex());
    m_refreshFuelMap = m_ui->m_refreshFuelMapCheckbox->isChecked();

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
    m_refreshFuelMap = settings.value(m_settingRefreshFuelMap, false).toBool();       

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
    settings.setValue(m_settingRefreshFuelMap, m_refreshFuelMap);

    foreach (SampleType sType, m_sampleTypeNames.keys())
    {
        settings.setValue(m_sampleTypeNames[sType], m_enabledSamples[sType]);
    }

    settings.endGroup();
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
