#pragma once
#include <QProgressBar>

class FuelTrimBar : public QProgressBar
{
  Q_OBJECT
public:
  explicit FuelTrimBar(QWidget* parent = nullptr);
  void paintEvent(QPaintEvent* e);
  void setValue(int value);
};

