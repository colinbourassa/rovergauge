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
  int m_numberBase;
  int m_activeRow;
  int m_activeRowWeight;
  int m_activeCol;
  int m_activeColWeight;
  int m_lastCellHighlightCount;

  void moveCellHighlightHard();
  void moveCellHighlightSoft();
  QColor getColorForFuelMapCell(unsigned char value) const;
};

