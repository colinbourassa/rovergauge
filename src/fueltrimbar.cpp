#include <QStylePainter>
#include <QStyleOptionProgressBar>
#include <QFrame>
#include "fueltrimbar.h"

/**
 * Construct with fixed max and min values.
 */
FuelTrimBar::FuelTrimBar(QWidget* parent) :
  QProgressBar(parent)
{
  this->setMaximum(255);
  this->setMinimum(-256);
}

/**
 * Call update() when the value changes; this avoids the problem of the
 * bar not getting repainted when a small value (< 3 and > -3) is set.
 */
void FuelTrimBar::setValue(int value)
{
  QProgressBar::setValue(value);
  update();
}

/**
 * Draws a progress bar that extends left or right from the center point.
 */
void FuelTrimBar::paintEvent(QPaintEvent*)
{
  const int currentVal = this->value();
  const int max = this->maximum();
  const int min = this->minimum();
  QStylePainter painter(this);
  QStyleOptionProgressBar opt;

  opt.initFrom(this);
  opt.minimum = min;
  opt.maximum = max;
  opt.progress = (currentVal >= 0) ? qAbs(max) : qAbs(min);
  opt.state |= QStyle::State_Horizontal;

  // Draw the groove. This should simply be the same size as the outside dimensions of the widget.
  style()->drawControl(QStyle::CE_ProgressBarGroove, &opt, &painter, this);

  // Get the dimensions for the bar itself (within the groove). Depending on the active graphical
  // style, this may be several pixels smaller that the dimensions of the widget, so that there's
  // a narrow gutter inside the groove not occupied by the bar.
  opt.rect = style()->subElementRect(QStyle::SE_ProgressBarContents, &opt, this);

  const float percentOfWidth = (float)(qAbs(currentVal)) / (float)(max - min);
  const int left = opt.rect.topLeft().x();
  const int right = opt.rect.topRight().x();
  const int top = opt.rect.topLeft().y();
  const int height = opt.rect.bottomLeft().y() - top + 1;
  const int barWidth = (right - left) * percentOfWidth;
  const int midPoint = left + ((right - left) / 2);
  const int startPoint = (currentVal >= 0) ? midPoint : midPoint - barWidth;

  opt.rect = QRect(startPoint, top, barWidth + 2, height);
  style()->drawControl(QStyle::CE_ProgressBarContents, &opt, &painter, this);
}

