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
 * Force a repaint when the value has been set to zero.
 * Not sure why the framework doesn't do this.
 */
void FuelTrimBar::setValue(int value)
{
  QProgressBar::setValue(value);

  if (value == 0)
  {
    repaint();
  }
}

/**
 * Draws a progress bar that extends left or right from the center point.
 */
void FuelTrimBar::paintEvent(QPaintEvent*)
{
  int currentVal = this->value();
  int max = this->maximum();
  int min = this->minimum();
  QStylePainter painter(this);
  QStyleOptionProgressBar opt;

  opt.initFrom(this);
  opt.minimum = min;
  opt.maximum = max;
  opt.progress = (currentVal >= 0) ? qAbs(max) : qAbs(min);

  // Draw the groove. This should simply be the same size as the outside dimensions of the widget.
  style()->drawControl(QStyle::CE_ProgressBarGroove, &opt, &painter, this);

  // Get the dimensions for the bar itself (within the groove). Depending on the active graphical
  // style, this may be several pixels smaller that the dimensions of the widget, so that there's
  // a narrow gutter inside the groove not occupied by the bar.
  opt.rect = style()->subElementRect(QStyle::SE_ProgressBarContents, &opt, this);

  float percentOfWidth = (float)(qAbs(currentVal)) / (float)(max - min);
  int left = opt.rect.topLeft().x();
  int right = opt.rect.topRight().x();
  int top = opt.rect.topLeft().y();
  int height = opt.rect.bottomLeft().y() - top + 1;
  int barWidth = (right - left) * percentOfWidth;
  int midPoint = left + ((right - left) / 2);
  int startPoint = (currentVal >= 0) ? midPoint : midPoint - barWidth;

  opt.rect = QRect(startPoint, top, barWidth + 2, height);
  style()->drawControl(QStyle::CE_ProgressBarContents, &opt, &painter, this);
}
