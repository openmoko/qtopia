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
#include <qmainwindow.h>  
#include <qcanvas.h>
#include <qlabel.h>

#ifndef SNAKEGAME
#define SNAKEGAME

#include "snake.h"
#include "target.h"
#include "obstacle.h"
#include "border.h"

// class QCanvas;

#ifdef QTOPIA_PHONE
# include <qtopia/contextmenu.h>
#endif

class QCanvasText;
class QCanvasRectangle;

class SnakeGame : public QMainWindow {
    Q_OBJECT

public:
    SnakeGame(QWidget* parent=0, const char* name=0, WFlags f=0);
    ~SnakeGame();

    void clear();
    void createTargets();

protected:
#ifndef QTOPIA_PHONE
    void closeEvent( QCloseEvent* );
#endif
    virtual void keyPressEvent(QKeyEvent*);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void focusOutEvent(QFocusEvent *);
    virtual void focusInEvent(QFocusEvent *);

signals:
    void moveFaster();

private slots:
    void newGame();
    void gameOver();
    void levelUp();
    void scoreInc();

    void endWait();

private:
    void showScore(int);
    void setupWalls(void);
    void showMessage(const QString &);
    QCanvasView* cv;
    QLabel* scorelabel;
    QCanvas canvas;
    Snake* snake;
    int last;
    int level;
    int stage;
    int targetamount;
    int notargets;
    bool waitover;
    bool gamestopped;

    QList<Obstacle> obs;

    QCanvasText *gamemessage;
    QCanvasRectangle *gamemessagebkg;
    
#ifdef QTOPIA_PHONE
    ContextMenu *contextMenu;
#endif
};

#endif
