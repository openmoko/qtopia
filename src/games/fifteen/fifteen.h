/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef __fifteenapplet_h__
#define __fifteenapplet_h__

#include <qmainwindow.h>
#include <qtableview.h>
#include <qarray.h>
#include <qpointarray.h>

class QPopupMenu;

class PiecesTable : public QTableView
{
  Q_OBJECT

 public:
  PiecesTable(QWidget* parent = 0, const char* name = 0);
  ~PiecesTable();

 protected slots:
  void slotRandomize();
  void slotReset();

 protected:
  void keyPressEvent(QKeyEvent*);
  void resizeEvent(QResizeEvent*);
  void mousePressEvent(QMouseEvent*);

  void paintCell(QPainter *, int row, int col);

  void initMap();
  void initColors();
  void randomizeMap();
  void checkwin();
  void readConfig();
  void writeConfig();

 private:
  void push(int col, int row);
  QArray<int>    _map;
  QArray<QColor> _colors;
  QPopupMenu     *_menu;
  bool            _randomized;
  QPointArray	light_border;
  QPointArray	dark_border;

  enum MenuOp { mRandomize = 1, mReset = 2 };
};

class FifteenWidget : public QWidget
{
    Q_OBJECT

public:
    FifteenWidget(QWidget *parent = 0, const char *name = 0);

private:
    PiecesTable *_table;
};

class FifteenMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    FifteenMainWindow(QWidget *parent=0, const char* name=0, WFlags fl=0);
};

#endif
