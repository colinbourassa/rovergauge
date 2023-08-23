#include "fuelmapgrid.h"
#include <QTableWidgetItem>
#include <QColor>
#include <QFont>
#include <QHeaderView>

FuelMapGrid::FuelMapGrid(QWidget* parent) :
  QTableWidget(parent),
	m_numberBase(16),
	m_activeRow(0),
	m_activeRowWeight(0),
	m_activeCol(0),
	m_activeColWeight(0),
	m_lastCellHighlightCount(0)
{
  horizontalHeader()->setStyleSheet("QHeaderView { font-size: 11pt; }");
  horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  horizontalHeader()->setVisible(true);
  verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  verticalHeader()->setVisible(false);
}

void FuelMapGrid::setup(int numberBase)
{
  m_numberBase = numberBase;
  QTableWidgetItem* item = 0;
  for (unsigned int col = 0; col < columnCount(); col++)
  {
    setHorizontalHeaderItem(col, new QTableWidgetItem(""));
    for (unsigned int row = 0; row < rowCount(); row++)
    {
      item = new QTableWidgetItem("");
      item->setTextAlignment(Qt::AlignCenter);
      item->setFlags(Qt::NoItemFlags);
      setItem(row, col, item);
    }
  }
}

void FuelMapGrid::moveCellHighlight(int row, int rowWeight, int col, int colWeight, bool soft)
{
  m_activeRow = row;
	m_activeRowWeight = rowWeight;
	m_activeCol = col;
	m_activeColWeight = colWeight;
  clearCellHighlight();

	if (soft)
	{
	  moveCellHighlightSoft();
  }
	else
	{
	  moveCellHighlightHard();
	}
}

void FuelMapGrid::moveCellHighlightHard()
{
  int highlightedRow = m_activeRow;
  int highlightedCol = m_activeCol;

  // We're not doing a soft-highlight using the weightings, so simply use the row/column
  // weight to round up to the next row/col index if appropriate.
  if ((m_activeRowWeight >= 8) && (highlightedRow < (FUEL_MAP_ROWS - 1)))
  {
    highlightedRow++;
  }

  if ((m_activeColWeight >= 8) && (highlightedCol < (FUEL_MAP_COLUMNS - 1)))
  {
    highlightedCol++;
  }

  // We're only highlighting a single cell (not a block of four).
  m_lastCellHighlight[0].first = highlightedRow;
  m_lastCellHighlight[0].second = highlightedCol;
  m_lastCellHighlightCount = 1;

  QTableWidgetItem* const cell = item(highlightedRow, highlightedCol);
  cell->setBackground(Qt::black);
  cell->setForeground(Qt::white);
}

void FuelMapGrid::moveCellHighlightSoft()
{
	// Compute the distribution of shading that should be applied left/right
	// and top/bottom to the block of four cells.
  const float leftPercent = 1.0 - (m_activeColWeight / 15.0);
  const float rightPercent = 1.0 - leftPercent;
  const float topPercent = 1.0 - (m_activeRowWeight / 15.0);
  const float bottomPercent = 1.0 - topPercent;
  float shadePercentage[NUM_ACTIVE_FUEL_MAP_CELLS];

  // We subtract from 1.0 here because these values will be used as multipliers
  // against the un-highlighted cell's R/G/B components to produce a shade of
  // the appropriate darkness.
  shadePercentage[0] = 1.0 - (leftPercent * topPercent);
  shadePercentage[1] = 1.0 - (rightPercent * topPercent);
  shadePercentage[2] = 1.0 - (leftPercent * bottomPercent);
  shadePercentage[3] = 1.0 - (rightPercent * bottomPercent);

  // Save the row/column positions for the fuel map cells that will be highlighted.
  m_lastCellHighlight[0].first = m_activeRow;
  m_lastCellHighlight[0].second = m_activeCol;
  m_lastCellHighlight[1].first = m_activeRow;
  m_lastCellHighlight[1].second = m_activeCol + 1;
  m_lastCellHighlight[2].first = m_activeRow + 1;
  m_lastCellHighlight[2].second = m_activeCol;
  m_lastCellHighlight[3].first = m_activeRow + 1;
  m_lastCellHighlight[3].second = m_activeCol + 1;
  m_lastCellHighlightCount = 4;

  // Set the shaded color as the background for the highlighted cells.
  for (int idx = 0; idx < m_lastCellHighlightCount; idx++)
  {
    const int row = m_lastCellHighlight[idx].first;
    const int col = m_lastCellHighlight[idx].second;
    QTableWidgetItem* const cell = item(row, col);
    if (cell)
    {
      const QColor& currentColor = m_cellColors[row][col];
      QColor newColor;
      newColor.setRgb(currentColor.red() * shadePercentage[idx],
                      currentColor.green() * shadePercentage[idx],
                      currentColor.blue() * shadePercentage[idx]);
      cell->setBackground(newColor);
      cell->setForeground((newColor.value() > 128) ? Qt::black : Qt::white);
    }
  }
}

void FuelMapGrid::clearCellHighlight()
{
  for (int idx = 0; idx < m_lastCellHighlightCount; idx++)
  {
    const int row = m_lastCellHighlight[idx].first;
    const int col = m_lastCellHighlight[idx].second;
    if ((row >= 0) && (row < rowCount()) &&
        (col >= 0) && (col < columnCount()))
    {
      QTableWidgetItem* cell = item(row, col);
      cell->setBackground(m_cellColors[row][col]);
      cell->setForeground(Qt::black);
    }
  }
}

void FuelMapGrid::restoreCellHighlight()
{
  moveCellHighlight(m_activeRow, m_activeRowWeight, m_activeCol, m_activeColWeight, (m_lastCellHighlightCount == 4));
}

void FuelMapGrid::setData(const QByteArray& data)
{
	const int numRows = rowCount();
	const int numCols = columnCount();
	QTableWidgetItem* cellItem = nullptr;
  unsigned char byte = 0;

	clearCellHighlight();
  for (int row = 0; row < numRows; row++)
  {
    for (int col = 0; col < numCols; col++)
    {
      cellItem = item(row, col);
      if (cellItem)
      {
        // retrieve the fuel map value at the current row/col
        byte = data.at(row * numCols + col);

        if (m_numberBase == 16)
        {
          cellItem->setText(QString("%1").arg(byte, 2, 16, QChar('0')).toUpper());
        }
        else if (m_numberBase == 10)
        {
          cellItem->setText(QString("%1").arg(byte));
        }
        m_cellColors[row][col] = getColorForFuelMapCell(byte);
        cellItem->setBackground(m_cellColors[row][col]);
        cellItem->setForeground(Qt::black);
      }
    }
  }
  restoreCellHighlight();
}

void FuelMapGrid::setNumberBase(int base)
{
  m_numberBase = base;
}

QColor FuelMapGrid::getColorForFuelMapCell(unsigned char value) const
{
  return QColor::fromRgb(255, (value / 2 * -1) + 255, 255.0 - value);
}

