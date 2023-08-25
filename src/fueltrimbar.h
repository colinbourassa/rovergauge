#ifndef FUELTRIMBAR_H
#define FUELTRIMBAR_H

#include <QProgressBar>

class FuelTrimBar : public QProgressBar
{
  Q_OBJECT
public:
  explicit FuelTrimBar(QWidget* parent = 0);
  void paintEvent(QPaintEvent* e);
  void setValue(int value);
};

#endif // FUELTRIMBAR_H
