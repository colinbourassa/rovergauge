#include <QStylePainter>
#include <QStyleOptionProgressBarV2>
#include <QFrame>
#include "fueltrimbar.h"

/**
 * Construct with fixed max and min values.
 */
FuelTrimBar::FuelTrimBar(QWidget *parent) :
    QProgressBar(parent),
    m_minimumVal(-256),
    m_maximumVal(255)

{
    this->setMaximum(m_maximumVal);
    this->setMinimum(m_minimumVal);
}

/**
 * Clips at the minimum and maximum values.
 */
void FuelTrimBar::setValue(int value)
{
    int adjVal = value;

    if (adjVal > m_maximumVal)
    {
        adjVal = m_maximumVal;
    }
    else if (adjVal < m_minimumVal)
    {
        adjVal = m_minimumVal;
    }

    QProgressBar::setValue(adjVal);

    // For some reason, paintEvent() was not being called by the framework
    // when small values were set on this control. This ensures that the
    // control will always be redrawn.
    if (value < 5)
    {
        repaint();
    }
}

/**
 * Draws a progress bar that extends left or right from the center point.
 */
void FuelTrimBar::paintEvent(QPaintEvent *)
{
    int currentVal = this->value();
    QStylePainter painter(this);
    QStyleOptionProgressBarV2 opt;

    opt.initFrom(this);
    opt.minimum = m_minimumVal;
    opt.maximum = m_maximumVal;
    opt.progress = (currentVal >= 0) ? qAbs(m_maximumVal) : qAbs(m_minimumVal);

    // Draw the groove. This should simply be the same size as the outside dimensions of the widget.
    style()->drawControl(QStyle::CE_ProgressBarGroove, &opt, &painter, this);

    // Get the dimensions for the bar itself (within the groove). Depending on the active graphical
    // style, this may be several pixels smaller that the dimensions of the widget, so that there's
    // a narrow gutter inside the groove not occupied by the bar.
    opt.rect = style()->subElementRect(QStyle::SE_ProgressBarContents, &opt, this);

    float percentOfWidth = (float)(qAbs(currentVal)) / (float)(m_maximumVal - m_minimumVal);
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
