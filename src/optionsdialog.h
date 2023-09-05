#pragma once
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
  OptionsDialog(QString title, QWidget* parent = 0);

  QString getSerialDeviceName() const;

  inline bool getSerialDeviceChanged() const
  {
    return m_serialDeviceChanged;
  }

  inline bool getRefreshFuelMap() const
  {
    return m_refreshFuelMap;
  }

  inline bool getSoftHighlight() const
  {
    return m_softHighlight;
  }

  inline int getDisplayNumberBase() const
  {
    return m_displayNumberBase;
  }

  inline bool getDisplayNumberBaseChanged() const
  {
    return m_displayNumberBaseChanged;
  }

  inline SpeedUnits getSpeedUnits() const
  {
    return m_speedUnits;
  }

  inline TemperatureUnits getTemperatureUnits() const
  {
    return m_tempUnits;
  }

  inline QMap<SampleType, bool> getEnabledSamples() const
  {
    return m_enabledSamples;
  }

  inline QHash<SampleType, unsigned int> getReadIntervals() const
  {
    return m_readIntervalsMs;
  }

  inline bool getSpeedoAdjust() const
  {
    return m_speedoAdjust;
  }

  inline double getSpeedoMultiplier() const
  {
    return m_speedoMultiplier;
  }

  inline int getSpeedoOffset() const
  {
    return m_speedoOffset;
  }

  inline const QMap<int,QString>& getRAMLabels() const
  {
    return m_ramLocLabels;
  }

protected:
  void accept();
  void reject();

private slots:
  void checkAll();
  void uncheckAll();
  void toggledSpeedoAdjust(bool value);

private:
  Ui::OptionsDialog* m_ui;

  QMap<SampleType, QCheckBox*> m_enabledSamplesBoxes;

  QString m_serialDeviceName;
  TemperatureUnits m_tempUnits;
  SpeedUnits m_speedUnits;

  QMap<SampleType, bool> m_enabledSamples;
  QMap<SampleType, QString> m_sampleTypeNames;
  QMap<SampleType, QString> m_sampleTypeLabels;
  QHash<SampleType, unsigned int> m_readIntervalsMs;
  bool m_serialDeviceChanged;
  bool m_refreshFuelMap;
  bool m_softHighlight;
  bool m_speedoAdjust;
  double m_speedoMultiplier;
  int m_speedoOffset;
  int m_displayNumberBase;
  bool m_displayNumberBaseChanged;
  QMap<int,QString> m_ramLocLabels;

  const QString m_settingsFileName;
  const QString m_settingsGroupName;
  const QString m_settingSerialDev;
  const QString m_settingRefreshFuelMap;
  const QString m_settingSoftHighlight;
  const QString m_settingSpeedUnits;
  const QString m_settingDisplayNumBase;
  const QString m_settingTemperatureUnits;
  const QString m_settingSpeedoAdjust;
  const QString m_settingSpeedoMultiplier;
  const QString m_settingSpeedoOffset;
  const QString m_settingRAMLocGroupName;
  const QString m_ramLabelPrefix;

  void groupLikeSettings();
  void setupWidgets();
  void setWidgetValues();
  void readSettings();
  void writeSettings();
};

