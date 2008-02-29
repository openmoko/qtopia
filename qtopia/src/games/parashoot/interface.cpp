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

#include "interface.h"
#include "man.h"

#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>

#include <qlabel.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qstyle.h>
#include <qtopia/qpetoolbar.h>
#include <qtoolbutton.h>
#include <qaction.h>

#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#endif
 
ParaShoot::ParaShoot(QWidget* parent, const char* name, WFlags f) :
    QMainWindow(parent,name,f),
    cannon(NULL),
    base(NULL),
    updatespeed(80),
    gamestopped(true),
    finished(true),
    waitover(true),
    fanfare("level_up"),
    score(0)
{
    QPEApplication::grabKeyboard();
    QPEApplication::setInputMethodHint( this, QPEApplication::AlwaysOff );
    canvas.setAdvancePeriod(80);

    pb = new QCanvasView(&canvas, this);

    setToolBarsMovable( FALSE );

    QPEToolBar* toolbar = new QPEToolBar(this);
    toolbar->setHorizontalStretchable( TRUE );

    setCaption( tr("ParaShoot") );

#ifdef QTOPIA_PHONE

    ContextMenu *menu = new ContextMenu( this );
    QAction *action;
    action = new QAction( tr("New Game"), QString::null, 0, this, 0 );
    connect( action, SIGNAL(activated()), this, SLOT(newGame()) );
    action->setWhatsThis( tr("Start a new game") );
    action->addTo( menu );
    action->setEnabled( TRUE );

#else
    new QToolButton( Resource::loadPixmap("ParaShoot"), tr("New Game"), 0,
                           this, SLOT(newGame()), toolbar, "New Game");
#endif

    levelscore = new QLabel(toolbar);
    levelscore->setBackgroundMode( PaletteButton );
    levelscore->setAlignment( AlignRight | AlignVCenter | ExpandTabs );
    toolbar->setStretchableWidget( levelscore );
    showScore(0,0);
    
    setCentralWidget(pb);

    autoDropTimer = new QTimer(this);
    connect (autoDropTimer, SIGNAL(timeout()), this, SLOT(play()) );
  
    pauseTimer = new QTimer(this);
    connect(pauseTimer, SIGNAL(timeout()), this, SLOT(wait()) ); 

    setFocusPolicy(StrongFocus);

#ifdef QTOPIA_PHONE
    newGame();
#endif
}

#ifndef QTOPIA_PHONE
void ParaShoot::closeEvent( QCloseEvent* e )
{
    clear();
    pauseTimer->stop();
    showScore(0,0);
    finished = gamestopped = false;
    waitover = true;
    e->accept();
}
#endif

void ParaShoot::resizeEvent(QResizeEvent *)
{
    QSize s = centralWidget()->size();
    int fw = style().defaultFrameWidth();
    canvas.resize( s.width() - fw - 2, s.height() - fw - 2);

    QImage bgimage = Resource::loadImage("parashoot/sky");
    QPixmap bgpixmap;

    bgpixmap.convertFromImage(bgimage.smoothScale(canvas.width(),
	canvas.height()), QPixmap::Auto);
    canvas.setBackgroundPixmap(bgpixmap);

    if (base) {
	base->reposition();
    }

    if (cannon) {
	cannon->reposition();
    }
}

void ParaShoot::focusOutEvent (QFocusEvent *) 
{
    if ( !gamestopped )
    {
	canvas.setAdvancePeriod(-1);
	gamestopped = true;
    }
}

void ParaShoot::focusInEvent (QFocusEvent *) 
{
    if ( gamestopped )
    {
	canvas.setAdvancePeriod(updatespeed);
	gamestopped = false;
    }
}

void ParaShoot::showScore( int score, int level )
{
    levelscore->setText(tr("     Level: %1       Score: %2   ").arg(score).arg(level) );
}


void ParaShoot::newGame()
{
    clear();
    if (pauseTimer->isActive()) 
	pauseTimer->stop();
    clear();
    Man::setManCount(0);
    score = 0;
    Bullet::setShotCount(0);
    Bullet::setNobullets(0);
    nomen = 2;
    Bullet::setLimit(nomen);
    level = 0;
    updatespeed = 80;
    showScore(0,0);
    finished = gamestopped = false;
    Helicopter::deleteAll();
    waitover = true;
    base = new Base(&canvas);
    cannon = new Cannon(&canvas);
    connect( cannon, SIGNAL(score(int)), this, SLOT(increaseScore(int)));
    autoDropTimer->start(100);
}


void ParaShoot::clear()
{
   autoDropTimer->stop();
//   QCanvasItem* item;
   QCanvasItemList l = canvas.allItems();
   for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {
        delete *it; 
   }
}

void ParaShoot::gameOver()
{
#define DEFAULT_TEXT_SIZE 12 

    QCanvasItem* item;
    QCanvasItemList l = canvas.allItems();
    for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) { 
         item = *it;
         if ((item->rtti()==1500) || (item->rtti()==1600) || item->rtti()==1900)
         item->setAnimated(false);
    }
    autoDropTimer->stop();
    Helicopter::silenceAll();

    int shots = Bullet::getShotCount();

    int shotsFired = cannon->shotsFired();
    if ( shotsFired == 0 ) 
	shotsFired = 1;
    QCanvasText* gameover = new QCanvasText(
        tr( "GAME OVER!\n"
        "Your Score: %1\n"
        "Parachuters Killed: %2\n"
        "Accuracy: %3% " )
        .arg( score ).arg( shots ).arg( shots * 100 / shotsFired ), &canvas );
    gameover->setTextFlags( Qt::AlignCenter );
    gameover->setColor( Qt::yellow );
    
    int size = DEFAULT_TEXT_SIZE;
    QFont fnt( QApplication::font() );
    fnt.setBold( TRUE );
    fnt.setPointSize( size );
    
    do gameover->setFont( fnt );
    while( ( gameover->boundingRect().width() > canvas.width() ||
        gameover->boundingRect().height() > canvas.height() ) && --size );
    
    gameover->move( canvas.width() / 2, canvas.height() / 2 );
    
    gameover->setZ(500);
    gameover->show();
    finished = gamestopped = true;
    waitover = false;
    pauseTimer->start(3000);
}

void ParaShoot::wait()
{
   waitover = true;
   pauseTimer->stop();
}

void ParaShoot::play()
{   
     if (Man::getManCount() < nomen ) {
          new Man(&canvas);
     }
     if (Base::baseDestroyed()) {
          gameOver();
          return;
     }
}

void ParaShoot::increaseScore(int x)
{
    score += x;
    if ( score / 150 != (score-x) / 150 )
       levelUp(); 
    showScore(level,score);
}

void ParaShoot::levelUp()
{
    level++; 
    int stage = level % 3;
    switch(stage) {
      case 0:
	nomen++;
	Bullet::setLimit(nomen);
	fanfare.play();
	break;
      case 1:
	new Helicopter(&canvas);
	break; 
      case 2:
	moveFaster();
	fanfare.play();
	break; 
      default: return;
    }
}

void ParaShoot::moveFaster()
{
   if (updatespeed > 50)
       updatespeed = updatespeed-5;
   else
       updatespeed = updatespeed-3;
   canvas.setAdvancePeriod(updatespeed);
}

void ParaShoot::keyPressEvent(QKeyEvent* event)
{
#ifdef QTOPIA_PHONE
    if (event->key() == Key_Back || event->key() == Key_No)
	QMainWindow::keyPressEvent(event);
#endif
    if ( gamestopped || finished ) {
	if ( finished && waitover )
	    newGame();
	else 
	    return;
    } else {
	switch(event->key()) {
	  case Key_Up:
	  case Key_F1:
	  case Key_F9:
	  case Key_Space:
#ifdef QTOPIA_PHONE
	  case Key_Select:
#endif
	    cannon->shoot();
	    break;
	  case Key_Left:
	    cannon->pointCannon(Cannon::Left);
	    lastcannonkey=Key_Left;
	    break;
	  case Key_Right:
	    cannon->pointCannon(Cannon::Right);
	    lastcannonkey=Key_Right;
	    break;
	  default:
	    return;
	}
    }
}

void ParaShoot::keyReleaseEvent(QKeyEvent* event)
{
    if ( lastcannonkey == event->key() )
	cannon->pointCannon(Cannon::NoDir);
}
