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

#include "cannon.h"
#include "base.h"
#include "helicopter.h"

#include <qtopia/sound.h>

#include <qmainwindow.h>
#include <qtimer.h>
#include <qlabel.h>

class QCanvas;
class Helicopter;

//enum Direction{
//      left, right, up, down };   

class ParaShoot : public QMainWindow {
    Q_OBJECT
 
public:
    ParaShoot(QWidget* parent=0, const char* name=0, WFlags f=0);
    
    void clear();
    void gameOver();
    int mancount;
    void levelUp();
    void moveFaster();

protected:
#ifndef QTOPIA_PHONE
    void closeEvent( QCloseEvent* );
#endif
    virtual void keyPressEvent(QKeyEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void focusOutEvent(QFocusEvent *);
    virtual void focusInEvent(QFocusEvent *);
 
private slots:
    void increaseScore(int);
    void newGame();
    void play();
    void wait();

private:
    void showScore( int score, int level );
    QCanvasView* pb;
    QCanvas canvas;
    Cannon* cannon;
    Base* base;
    QCanvasText* gameover;
    QLabel* levelscore;
    int nomen;
    int level;
    int oldscore;
    int updatespeed;
    QTimer* autoDropTimer;
    QTimer* pauseTimer;
    bool gamestopped;
    bool finished, waitover;
    Sound fanfare;
    int score;
    int lastcannonkey;
};           
