#pragma once
#include <QTableWidget>
#include <QByteArray>

#define NUM_ACTIVE_FUEL_MAP_CELLS 4
#define FUEL_MAP_ROWS 8
#define FUEL_MAP_COLUMNS 16

class FuelMapGrid : public QTableWidget
{
  Q_OBJECT
public:
  explicit FuelMapGrid(QWidget* parent = nullptr);
  void moveCellHighlight(int row, int rowWeight, int col, int colWeight, bool soft);
  void clearCellHighlight();
  void restoreCellHighlight();
  void setData(const QByteArray& data);
  void setNumberBase(int base);
  void setup(int numberBase);

private:
  QColor m_cellColors[FUEL_MAP_ROWS][FUEL_MAP_COLUMNS];
  QPair<int,int> m_lastCellHighlight[NUM_ACTIVE_FUEL_MAP_CELLS];
  int m_numberBase = 16;
  int m_activeRow = 0;
  int m_activeRowWeight = 0;
  int m_activeCol = 0;
  int m_activeColWeight = 0;
  int m_lastCellHighlightCount = 0;

  void moveCellHighlightHard();
  void moveCellHighlightSoft();
  QColor getColorForFuelMapCell(unsigned char value) const;
};

