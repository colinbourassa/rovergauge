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
    QStyleOptionProgressBarV2 bar;
    bar.initFrom(this);
    bar.minimum = m_minimumVal;
    bar.maximum = m_maximumVal;
    bar.progress = (currentVal >= 0) ? qAbs(m_maximumVal) : qAbs(m_minimumVal);

    style()->drawControl(QStyle::CE_ProgressBarGroove, &bar, &painter, this);

    // compute the dimensions and location of the bar
    float percentOfWidth = (float)(qAbs(currentVal)) / (float)(m_maximumVal - m_minimumVal);
    int left = bar.rect.topLeft().x();
    int right = bar.rect.topRight().x();
    int top = bar.rect.topLeft().y();
    int height = bar.rect.bottomLeft().y() - top + 1;
    int barWidth = (right - left) * percentOfWidth;
    int midPoint = left + ((right - left) / 2);
    int startPoint = (currentVal >= 0) ? midPoint : midPoint - barWidth;

    bar.rect = QRect(startPoint, top, barWidth + 2, height);
    style()->drawControl(QStyle::CE_ProgressBarContents, &bar, &painter, this);
}
