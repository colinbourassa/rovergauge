#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QString>
#include <QHash>
#include "commonunits.h"

namespace Ui
{
    class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    OptionsDialog(QString title, QWidget *parent = 0);
    QString getSerialDeviceName();
    bool getSerialDeviceChanged()                     { return m_serialDeviceChanged; }
    bool getRefreshFuelMap()                          { return m_refreshFuelMap; }
    SpeedUnits getSpeedUnits()                        { return m_speedUnits; }
    TemperatureUnits getTemperatureUnits()            { return m_tempUnits; }
    QHash<SampleType,bool> getEnabledSamples()        { return m_enabledSamples; }
    QHash<SampleType,unsigned int> getReadIntervals() { return m_readIntervalsMs; }

protected:
    void accept();

private slots:
    void checkAll();
    void uncheckAll();

private:
    Ui::OptionsDialog *m_ui;

    QHash<SampleType,QCheckBox*> m_enabledSamplesBoxes;

    QString m_serialDeviceName;
    TemperatureUnits m_tempUnits;
    SpeedUnits m_speedUnits;

    QHash<SampleType,bool> m_enabledSamples;
    QHash<SampleType,QString> m_sampleTypeNames;
    QHash<SampleType,QString> m_sampleTypeLabels;
    QHash<SampleType,unsigned int> m_readIntervalsMs;
    bool m_serialDeviceChanged;
    bool m_refreshFuelMap;

    const QString m_settingsFileName;
    const QString m_settingsGroupName;

    const QString m_settingSerialDev;
    const QString m_settingRefreshFuelMap;
    const QString m_settingSpeedUnits;
    const QString m_settingTemperatureUnits;

    void groupLikeSettings();
    void setupWidgets();
    void readSettings();
    void writeSettings();
};

#endif // OPTIONSDIALOG_H

