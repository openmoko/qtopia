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

#include <qtopia/resource.h>

#include <qtopia/qpetoolbar.h>
#include <qtoolbutton.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/contextbar.h>

SnakeGame::SnakeGame(QWidget* parent, const char* name, WFlags f) :
    QMainWindow(parent,name,f),
    canvas(232, 258), gamemessage(0), gamemessagebkg(0)
{
    setCaption( tr("Snake") );
    canvas.setBackgroundPixmap(*(SpriteDB::spriteCache()->image(SpriteDB::ground())));
    canvas.setUpdatePeriod(100);
    snake = 0;

    // when turned to true, get segfaults.
    // shouldn't though... not sure what is happening.
    obs.setAutoDelete(FALSE);
    cv = new QCanvasView(&canvas, this);
    cv->setFrameStyle(QFrame::NoFrame);
#ifndef QTOPIA_PHONE
    setToolBarsMovable( FALSE );

    QPEToolBar* toolbar = new QPEToolBar( this);
    toolbar->setHorizontalStretchable( TRUE );
    (void)new QToolButton(Resource::loadPixmap("Snake"), tr("New Game"), 0,
                            this, SLOT(newGame()), toolbar, "New Game");

    scorelabel = new QLabel(toolbar);
    showScore(0);
    scorelabel->setBackgroundMode( PaletteButton );
    scorelabel->setAlignment( AlignRight | AlignVCenter | ExpandTabs );
    toolbar->setStretchableWidget( scorelabel );
#endif
    setFocusPolicy(StrongFocus);

    setCentralWidget(cv);

    gamestopped = true;
    waitover = true;
   QPEApplication::setInputMethodHint( this, QPEApplication::AlwaysOff );

#ifdef QTOPIA_PHONE
    contextMenu = new ContextMenu( this );
    ContextBar::setLabel( this, Qt::Key_Select, ContextBar::Select );
#endif
}

SnakeGame::~SnakeGame()
{
   delete snake;
}

#ifndef QTOPIA_PHONE
void SnakeGame::closeEvent( QCloseEvent* e )
{
    clear();
    setupWalls();
    gamestopped = true;
    waitover = true;
    e->accept();
}
#endif

void SnakeGame::resizeEvent(QResizeEvent *)
{
    QSize s = centralWidget()->size();
    int fw = style().defaultFrameWidth();
    canvas.resize( s.width() - fw - 2, s.height() - fw - 2);

    if (snake) {
	newGame();
    } else {
        clear();
	setupWalls();
    }
}

void SnakeGame::focusOutEvent(QFocusEvent *) {
    if (snake)
	snake->pause();
}

void SnakeGame::focusInEvent(QFocusEvent *) {
    if (snake)
	snake->pause();
}

void SnakeGame::newGame()
{
#ifdef QTOPIA_PHONE
    ContextBar::setLabel( this, Qt::Key_Select, ContextBar::NoLabel );
#endif   
    clear();
    snake = new Snake(&canvas);
    connect(snake, SIGNAL(dead()), this, SLOT(gameOver()) );
    connect(snake, SIGNAL(targethit()), this, SLOT(levelUp()) );
    connect(snake, SIGNAL(scorechanged()), this, SLOT(scoreInc()) );
    connect(this, SIGNAL(moveFaster()), snake, SLOT(increaseSpeed()) );
    last = 0;
    targetamount = 1;
    notargets = 1;
    level = 1;
    stage = 1;
    showScore(0);
    gamestopped = false;
    waitover = true;

    setupWalls();
    createTargets();
}

//
// Create walls and obstacles.
//
void SnakeGame::setupWalls(void)
{
    obs.clear();

    int tilesize = SpriteDB::tileSize();
    int screenwidth = canvas.width() / tilesize;
    if ((canvas.width() % tilesize) > (tilesize >>1))
	screenwidth++;
    int screenheight = canvas.height() / tilesize;
    if ((canvas.height() % tilesize) > (tilesize >>1))
	screenheight++;
    
    // now for walls.
    //corners
    obs.append(new Obstacle(&canvas, 0, 0, FALSE, TRUE, FALSE, TRUE));
    obs.append(new Obstacle(&canvas, tilesize*screenwidth-tilesize,
		tilesize*screenheight-tilesize,
		TRUE, FALSE, TRUE, FALSE));
    obs.append(new Obstacle(&canvas, tilesize*screenwidth-tilesize, 0,
		TRUE, FALSE, FALSE, TRUE));
    obs.append(new Obstacle(&canvas, 0, tilesize*screenheight-tilesize,
		FALSE, TRUE, TRUE, FALSE));
    //sides
    int i;
    for (i = 1; i+1 < screenwidth; ++i) {
	obs.append(new Obstacle(&canvas, i*tilesize, 0,
		    TRUE, TRUE, FALSE, FALSE));
	obs.append(new Obstacle(&canvas, i*tilesize, tilesize*screenheight-tilesize,
		    TRUE, TRUE, FALSE, FALSE));
    }
    for (i = 1; i+1 < screenheight; ++i) {
	obs.append(new Obstacle(&canvas, 0, i*tilesize,
		    FALSE, FALSE, TRUE, TRUE));
	obs.append(new Obstacle(&canvas, tilesize*screenwidth-tilesize, i*tilesize,
		    FALSE, FALSE, TRUE, TRUE));
    }

    // now make a couple of med screen obsticals?

    int obwidth = (screenwidth-2) >> 1;
    int obstart = ((screenwidth-2) >> 2) + 1;
    int oboffset = (screenheight-2) / 3;
    for (i = obstart; i < obstart+obwidth; i++) {
	obs.append(new Obstacle(&canvas, i*tilesize, tilesize+oboffset*tilesize,
		    i!=obstart, i!=obstart+obwidth-1, FALSE, FALSE));
	obs.append(new Obstacle(&canvas, i*tilesize, tilesize+2*oboffset*tilesize,
		    i!=obstart, i!=obstart+obwidth-1, FALSE, FALSE));
    }

    //ob_top = new Obstacle(&canvas, tilesize,tilesize, FALSE, FALSE, FALSE, FALSE);
}


void SnakeGame::showScore(int score)
{
#ifndef QTOPIA_PHONE
    scorelabel->setText(tr("     Score :    %1   ").arg(score) );
#else
    Q_UNUSED(score)
#endif
}


void SnakeGame::scoreInc()
{
   showScore( snake->getScore() );
}

void SnakeGame::levelUp()
{
   notargets--;
   if (notargets == 0) {
        stage++;
       if (stage == 3) {
          level++;
          emit moveFaster();
          targetamount++;
          stage = 0;
       }
       createTargets();
   }
}

void SnakeGame::createTargets()
{  
    Target  *foo;
    int	    new_target_count = 0;

    for (int i = 0; i < targetamount; i++) {
	foo = new Target(&canvas);
	if(foo->position() == FALSE) {
	    delete foo;		// ran out of room for targets
	} else {
	    foo->show();
	    new_target_count++;
	}
    }

    notargets = targetamount = new_target_count;
}

void SnakeGame::clear()
{
   delete snake;
   snake = 0;
   QCanvasItemList l = canvas.allItems();
   for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {
        delete *it;
   }  
   gamemessage = 0;
   gamemessagebkg = 0;

   if (obs.count() > 0)
       obs.clear();
}

void SnakeGame::showMessage(const QString &text)
{
#define DEFAULT_TEXT_SIZE 14

   if (!gamemessage) {
       gamemessage = new QCanvasText(&canvas);
       gamemessage->setZ(100);
       //gamemessage->setColor(yellow);
       gamemessage->setColor(palette().color(QPalette::Normal, QColorGroup::HighlightedText));
   }
   if (!gamemessagebkg) {
       gamemessagebkg = new QCanvasRectangle(&canvas);
       gamemessagebkg->setZ(99);
       gamemessagebkg->setBrush(palette().color(QPalette::Normal, QColorGroup::Highlight));
   }
   gamemessage->setText(text);
   
   int size = DEFAULT_TEXT_SIZE;
   QFont fnt( QApplication::font() );
   fnt.setPointSize( size );
   fnt.setBold( TRUE );
   do gamemessage->setFont( fnt );
   while( ( gamemessage->boundingRect().width() > canvas.width() ||
       gamemessage->boundingRect().height() > canvas.height() ) && --size );
       
   int w = gamemessage->boundingRect().width();
   int h = gamemessage->boundingRect().height();
   gamemessage->move(canvas.width()/2 -w/2, canvas.height()/2 -h/2);
   gamemessagebkg->setSize(w+10, h+10);
   gamemessagebkg->move(canvas.width()/2 -w/2 - 5, canvas.height()/2 -h/2 - 5);
   gamemessagebkg->show();
   gamemessage->show();

}
void SnakeGame::gameOver()
{
#ifdef QTOPIA_PHONE
    ContextBar::setLabel( this, Qt::Key_Select, ContextBar::Select );
#endif
   if( snake ) showMessage( tr( "GAME OVER!\nYour Score: %1" ).arg( snake->getScore() ) );
   gamestopped = true;
   waitover = false;
   // timer on waitover?
   QTimer::singleShot(2000, this, SLOT(endWait()));
}

void SnakeGame::endWait()
{
    waitover = true;
    if( snake ) showMessage(tr("GAME OVER!\nYour Score: %1\nPress any\nkey to start\nnew game", "limited space for line length").arg(snake->getScore()));
}

void SnakeGame::keyPressEvent(QKeyEvent* event)
{ 
    switch( event->key() ) {
#ifdef QTOPIA_PHONE
    case Qt::Key_Select:
        if( gamestopped && waitover ) newGame();
        break;
    case Qt::Key_Back:
    case Qt::Key_No:
        QMainWindow::keyPressEvent( event );
        break;
#endif
    default:
        if( gamestopped ) {
            if( waitover ) newGame();
        } else snake->go( event->key() );
        break;
    }
}

