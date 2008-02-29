/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "snakeview.h"
#include "snakemanager_p.h"
#include "snakescene_p.h"
#include <QResizeEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#ifdef QTOPIA_PHONE
# include <qtopia/qsoftmenubar.h>
#endif

/*!
  \class SnakeView

  \brief   Top-level class for the snake game.

  \mainclass The snake is placed in an environment consisting
  of walls (which the snake must avoid) and one or more mice (which it hunts). The user acts on
  behalf of the snake, and drives its direction via the arrow keys.

  The rules of the game are as follows:
  \list
  \o The game is completely restarted and the score set to 0 when the snake dies (i.e. hits a wall or itself).
  \o The user's score is increased by SnakeScene::SCORE_FOR_MOUSE whenever the snake eats a mouse.
  \o When a mouse is eaten, the snake grows in size. If there are no mice left, we replace the mice (number
      of mice equals the level number) and go up a stage. When the stage equals three, we go up a level
      and increase the speed.
  \o Mice are placed randomly on the board when they are created and when one of their brethren has been consumed.
   \o If the game loses focus and is running, we pause the game. The user must then explicitly start it again.
   \o If the game resizes and is running, we pause the game. The user must then explicitly start it again.
   \o If the snake becomes too long, the game is restarted, but the user's score is maintained.
   \endlist
*/


const QString SnakeView::imageFileName = ":image/snake/smallsnake";


/*!
  \fn SnakeView::SnakeView(QWidget *parent = 0)
  The SnakeView is the one and only view of the SnakeScene; as such, it creates the SnakeScene
  itself, and then sets itself as the view onto that scene. Creation and display of the SnakeView
  is all that is needed to start the snake game.
  Note that the flags are ignored.
*/
SnakeView::SnakeView(QWidget *parent, Qt::WFlags)
    : QGraphicsView(parent)
    , snakeScene(0)
    , snakeManager(0)
{
    setWindowTitle(tr("Snake"));
    // !!!!!!!!!!!!!HACK (workaround) FOR QT BUG
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    snakeManager = new SnakeManager(QImage(imageFileName));
    snakeScene = new SnakeScene(this,snakeManager);
    setScene(snakeScene);

    // Need to react to a new game, and game ending, in terms of the soft menu bar.
    connect(snakeScene,SIGNAL(newGame()),this,SLOT(handleNewGame()));
    connect(snakeScene,SIGNAL(gameOver()),this,SLOT(handleGameOver()));

    // Need to set the focus policy to something other than NoFocus to ensure that we get
    // the focusOutEvent(...).
    setFocusPolicy(Qt::StrongFocus);

#ifdef QTOPIA_PHONE
    (void)QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::Select );
#endif
}

/*!
  \fn SnakeView::~SnakeView()
*/
SnakeView::~SnakeView()
{
    delete snakeScene;
    delete snakeManager;
}

/*!
  \fn void SnakeView::keyPressEvent(QKeyEvent* event)
  Changes the status of the game if any of the keys Select, Enter or Return are activated.
  Allows the user to change the direction of the running snake if any of the arrow keys are
  activated. Numerical keys are also mapped to direction turning.
 */
void SnakeView::keyPressEvent(QKeyEvent* event)
{
    switch( event->key() ) {
    case Qt::Key_Select:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        // This will pause the game if it's running, continue if it was previously paused,
        // or start a new game if one is not currently running.
        snakeScene->updateGame();
        break;

#ifdef QTOPIA_PHONE
    case Qt::Key_6:
        // FALL THROUGH
#endif
    case Qt::Key_Right:
        snakeScene->turn(SnakeManager::Right);
        break;
#ifdef QTOPIA_PHONE
    case Qt::Key_4:
        // FALL THROUGH
#endif
    case Qt::Key_Left:
        snakeScene->turn(SnakeManager::Left);
        break;
#ifdef QTOPIA_PHONE
    case Qt::Key_2:
        // FALL THROUGH
#endif
    case Qt::Key_Up:
        snakeScene->turn(SnakeManager::Up);
        break;
#ifdef QTOPIA_PHONE
    case Qt::Key_8:
        // FALL THROUGH
#endif
    case Qt::Key_Down:
        snakeScene->turn(SnakeManager::Down);
        break;
        /*
#ifdef QTOPIA_PHONE
    case Qt::Key_1:
        snakeScene->turn(SnakeManager::Up | SnakeManager::Left);
            break;
    case Qt::Key_3:
        // Up or right, depending on direction.
        snakeScene->turn(SnakeManager::Up | SnakeManager::Right);
        break;
    case Qt::Key_7:
        snakeScene->turn(SnakeManager::Down | SnakeManager::Left);
        break;
    case Qt::Key_9:
        snakeScene->turn(SnakeManager::Down | SnakeManager::Right);
        break;
#endif
        */
    default:
        // We don't want this event.
        event->ignore();
    }
}

/*!
  \fn void SnakeView::focusOutEvent(QFocusEvent *event)
  When focus is removed from the SnakeView, the SnakeScene pauses (if it is running).
  The user must then explicitly restart the game by pressing Select/Enter/Return.
 */
void SnakeView::focusOutEvent(QFocusEvent *event)
{
    snakeScene->pause();

    QGraphicsView::focusOutEvent(event);
}

/*!
  \fn void SnakeView::resizeEvent (QResizeEvent *event)
  When the SnakeView resizes, the scene is resized so that it always fits inside the view.
 */
void SnakeView::resizeEvent (QResizeEvent *event)
{
    snakeScene->resize(QRectF(0,0,width()-1,height()-1));

    QGraphicsView::resizeEvent(event);
}

void SnakeView::handleNewGame()
{
// TODO: Show pause/play icons instead of Select
#ifdef QTOPIA_PHONE
    QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::Select );
#endif
}

void SnakeView::handleGameOver()
{
#ifdef QTOPIA_PHONE
    QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::Select );
#endif
}
