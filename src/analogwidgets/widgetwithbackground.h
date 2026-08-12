/***************************************************************************
 *   Copyright (C) 2006-2008 by Tomasz Ziobrowski                          *
 *   http://www.3electrons.com                                             *
 *   e-mail: t.ziobrowski@3electrons.com                                   *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef WIDGETWITHBACKGROUND_H
#define WIDGETWITHBACKGROUND_H

#include <QWidget>

class QPixmap;

class WidgetWithBackground : public QWidget
{
	Q_OBJECT
  public:
     /** Konstruktor */
     WidgetWithBackground(QWidget * parent = 0);

     /** Destruktor - usuwa bitmapk�.  */
     ~WidgetWithBackground ();

     /**
      * Operacja odmalowywania kontrolki  -
      * wkleja bitmap� z t�em na kontrolk� -
      * generuj�� j� wcze�niej je�eli zajdzie taka potrzeba.
      */
     void  drawBackground ();

     /** Odmalowywuje kontrolk� bezwarunkowo odmalowywuj�c t�o. */
     void  updateWithBackground ();
    
     /** 
     * Zawraca informacje czy kontrolka zosta�a zmodyfikowana
     * Ta informacja jest ustawiana gdy bitmapka t�a si� zmienia 
     * to znaczy zmienia si� rozmiar komponentu lub istnieje 
     * potrzeba przemalowania t�a. 
     */ 
     bool doRepaintBackground(); 
     
  protected:
    /** Wywo�uje paintBackground - odmalowywuj�c t�o na nowo */
    void repaintBackground();

    /**
     * Odmalowywuje t�o kontrolki
     * @param painter urz�dzenie na ktr�ym mamy malowa�.
     * @param background to t�o kontrolki
     */
    virtual void  paintBackground (QPainter & painer) = 0;
    
    

  protected:
     /** Bufor na t�o. */
     QPixmap * m_pixmap;
     /** Device pixel ratio that m_pixmap was allocated for. */
     qreal m_pixmapRatio;
     /**
     * Ustawia t� zmienn� po zmianie w�a�ciwo�ci
     */
     bool m_modified;
};
#endif //WIDGETWITHBACKGROUND_H

