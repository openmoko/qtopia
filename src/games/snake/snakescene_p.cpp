/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "snakescene_p.h"
#include "mouse_p.h"
#include "snake_p.h"
#include "messagebox_p.h"
#include "wall_p.h"
#include <QTime>
#include <QTimer>


// Percentage of (board-width + board-height) that the snake has to fill before the user is proclaimed
// a SuperUser.
const qreal SnakeScene::maxSnakeRatio = 2.0;


/*!
  \internal
  \class SnakeScene

  \brief Scene containing and managing all the snake objects.
*/

/*!
  \internal
  \fn SnakeScene::SnakeScene(SnakeManager *_snakeManager)
*/
SnakeScene::SnakeScene(QWidget *_menuWidget,SnakeManager *_snakeManager)
    : snakeManager(_snakeManager)
    , menuWidget(_menuWidget)
    , snake(0)
    , state(None)
    , score(0)
    , level(1)
    , stage(1)
    , messageBox(0)
    , frozen(false)
#ifdef SNAKEGAME_MOUSE_TERRIFIED
    , terrorTimer(0)
    , tremblingMouse(0)
#endif
{
    setBackgroundBrush(snakeManager->getGroundImage());

    createWalls();

    // Seed the random number generator for randomly positioning the target mice.
    QTime midnight(0, 0, 0);
    srand(midnight.secsTo(QTime::currentTime()) );

    snake = createSnake();
    connect(snake,SIGNAL(snakeDead()),this,SLOT(mournSnake()));
    connect(snake,SIGNAL(mouseEaten(Mouse *)),this,SLOT(mournMouse(Mouse *)));

    // The mouse will be created prior to display, via resize(...).

#ifdef SNAKEGAME_MOUSE_TERRIFIED
    // When the snake gets near, the mouse starts trembling. The terror timer
    // periodically checks to see if the snake is close to the mouse.
    // TODO: It might be better to carry out this check every n moves, e.g. every
    // 4 snake moves, to reduce the overhead of another timer.
    terrorTimer = new QTimer(this);
    connect(terrorTimer, SIGNAL(timeout()), this, SLOT(checkForTerror()));
#endif
}

/*!
  \internal
  \fn SnakeScene::~SnakeScene()
*/
SnakeScene::~SnakeScene()
{
#ifdef SNAKEGAME_MOUSE_TERRIFIED
    delete terrorTimer;
#endif

    for ( QList<SnakeItemInterface *>::iterator control =  items.begin(); control != items.end(); control++ ) {
        // Delete snake and walls, but not mouse -- this is a graphical item and will be deleted by
        // the scene.
        SnakeItemInterface *item = *control;
        if ( item->type() != Mouse::TYPE ) {
            delete item;
        }
    }
}

/*!
  \internal
  \fn QList<SnakeItemInterface *> SnakeScene::getItems() const
  Returns all the items that implement the interface for resizing.
*/

/*!
  \internal
  \fn void SnakeScene::resize(const QRectF &rectangle)
  Called in the event of a resize, and will therefore be called prior to first display.
*/
void SnakeScene::resize(const QRectF &rectangle)
{
    snake->pause();

    QRectF oldGeometry(sceneRect());

    setSceneRect(rectangle);

    // Make sure the message box is always centred.
    if ( messageBox ) {
        messageBox->setPos((width() - messageBox->boundingRect().width())/2,
                           (height() - messageBox->boundingRect().height())/2);
    }

    // Resize all the resizeable items.
    QList<SnakeItemInterface *> items = getItems();
    for ( QList<SnakeItemInterface *>::iterator control = items.begin(); control != items.end(); control++ ) {
        SnakeItemInterface *item = *control;
        item->resize(oldGeometry,rectangle);
    }

    // Set the current region to invalid - it will be calculated again on an as-needed basis.
    availableRegion = QRegion();

    if ( state == None ) {
        // First time display.
        prepareNewGame(false,false,tr("Press Select key to start"));
    } else if ( state == Running ) {
        // Start a new game, but keep the user's score. This sets to Pending.
        pause();
    }
}

// Sets the given text in the message box, and makes it visible.
void SnakeScene::showMessage(const QString &txt)
{
    if ( !messageBox ) {
        messageBox = new MessageBox;
        messageBox->setZValue(DISPLAY_Z_VALUE);
        addItem(messageBox);

    } else {
        // The message box is already in existence - only have to reset the text and position it.
        messageBox->setVisible(true);
    }

    messageBox->setMessage(txt,getAvailableWidth());

    messageBox->setPos((width() - messageBox->boundingRect().width())/2,
                       (height() - messageBox->boundingRect().height())/2);
}

// Signal handler for Snake's mouseEaten(Mouse *) signal. Removes and destroys the mouse.
// If there are no mice left, increases the stage and possibly the level, and calls
// updateMice(). Otherwise, just respositions the existing mice on the board.
// Since the snake grows when it eats a mouse, also checks the length of the snake - may
// have to shrink the snake and call prepareNewGame(...).
void SnakeScene::mournMouse(Mouse *mouse)
{
    score += SCORE_FOR_MOUSE;

    removeMouse(mouse);

    // If there are no mice left, replace the mice (number of mice equals the level number)
    // and go up a stage. When the stage equals three, go up a level and increase the speed.
    if ( mice.count() == 0 ) {
        stage++;
        if ( stage > MAX_STAGE ) {
            level++;
            snake->increaseSpeed();
            stage = 1;
        }
        // Make sure number of mice == level (create, delete, whatever) and reposition all
        // the mice on the board.
        updateMice();
    } else {
        // Just make sure the mice we've got are repositioned on the board.
        repositionMice();
    }

    // Check the snake's pending size (the snake has a tendency to grow as it eats mice) -
    // if the snake's length in pixels is getting too large for the available space, it will be
    // impossible for the user to continue. Therefore, congratulate the user on their prowess,
    // restart the game at a higher level of difficulty but keep the score.
    qreal allowedLength = (width() + height()) * maxSnakeRatio;
    if ( snake->getPendingSize() > allowedLength ) {
        prepareNewGame(false,true,tr("Current score: %1.").arg(score) + tr("Press Select key to resume."));
    }
}

// Signal handler for Snake's snakeDead() signal. Stops the game.
void SnakeScene::mournSnake()
{
    endGame();
}

/*!
  \internal
  \fn void SnakeScene::pause()
   If the game is running, causes it to temporarily halt. Otherwise, does nothing.
*/
void SnakeScene::pause()
{
    // Only valid if state is Running.
    if ( state == Running ) {
        updateGame();
    }
}

// Updates all the game counters, calls updateMice(), starts the game if startNow is true,
// otherwise gets the game ready to start.
// Changes state to Pending (ready to start) or Running.
// Emits the signal newGame.
void SnakeScene::prepareNewGame(bool startNow,bool keepScore,const QString &msg)
{
#ifdef SNAKEGAME_MOUSE_TERRIFIED
    // Stop any trembling mice, and stop the timer.
    sootheMouse(true);
#endif

    if ( !keepScore ) {
        score = 0;
    }
    level = 1;
    stage = 1;

    // Reposition the snake at the start, and the mice. The parameter to reset
    // indicates that the snake's current speed should be maintained if the game is
    // continuing (i.e. the user is allowed to keep their score) - otherwise, the speed
    // is set to the slower starting speed.
    snake->reset(!keepScore);
    updateMice();

    emit newGame();

    if ( startNow ) {
        startGame();
    } else {
        snake->pause();
        showMessage(msg);
        state = Pending;
    }
}

// Switches off all messages and gets the snake running.
void SnakeScene::startGame()
{
    // Start the game - get rid of any messages.
    if ( messageBox ) {
        messageBox->setVisible(false);
    }

    // Get the snake going.
    state = Running;
#ifdef SNAKEGAME_MOUSE_TERRIFIED
    terrorTimer->start(snake->getSpeed() * 4);
#endif

    snake->goGirl();
}

// Shows appropriate messages to inform the user that the game is lost.
// Emits the signal gameOver
void SnakeScene::endGame()
{
#ifdef SNAKEGAME_MOUSE_TERRIFIED
    // Stop any trembling mouse, and stop the timer.
    sootheMouse(true);
#endif

    showMessage(tr("GAME OVER!\nYour score: %1").arg(score));

    emit gameOver();

    // Start a timer to freeze the application
    frozen = true;
    state = Stopped;
    QTimer::singleShot(2000, this, SLOT(endWait()));
}

/*!
  \internal
  \fn void SnakeScene::updateGame()
 This will pause the game if it's running, continue if it was previously paused,
 start the game if it is pending or stopped..
*/
void SnakeScene::updateGame()
{
    if ( !snake ) {
        return;
    }
    if ( frozen ) {
        // We're waiting for the user message to finish. Don't allow the user to hit
        // keys etc in the meantime.
        return;
    }

    switch ( state ) {
    case Pending:
        startGame();
        break;
    case Stopped:
        // Start the game - 1st parameter means immediate start, second means don't keep the score.
        prepareNewGame(true,false,tr("Press Select key to start"));
        break;
    case Running:
        // Pause the game.
#ifdef SNAKEGAME_MOUSE_TERRIFIED
        // Stop any tremlbing mouse, and stop the timer.
        sootheMouse(true);
#endif
        snake->pause();
        showMessage(tr("Game paused. Press Select key to resume."));
        state = Paused;
        break;
    case Paused:
        if ( messageBox ) {
            messageBox->setVisible(false);
        }

        state = Running;
#ifdef SNAKEGAME_MOUSE_TERRIFIED
        terrorTimer->start(snake->getSpeed() * 4);
#endif
        snake->goGirl();
        break;
    default:
        qWarning("SnakeScene::updateGame(): Invalid state %d",state);
    }
}

void SnakeScene::endWait()
{
    frozen = false;

    showMessage(tr("GAME OVER!\nYour score: %1").arg(score) + QLatin1String("\n") + tr("Press Select key to start new game."));
}

// Removes and destroys the mouse.
void SnakeScene::removeMouse(Mouse *mouse)
{
    // Retrieve the mouse object, and remove it from mice list.
    if ( mouse ) {
        // We already have the mouse - find it in the mice list.
        int listIndex = mice.indexOf(mouse);
        if ( listIndex > -1 ) {
            mice.removeAt(listIndex);
        } else {
            qWarning("SnakeScene::removeMouse(): !!!!!!!!Could not find the mouse in the mouse list.");
            // Don't try to go any further.
#ifdef SNAKEGAME_MOUSE_TERRIFIED
            if ( mouse == tremblingMouse )
                tremblingMouse = 0;
#endif
            delete mouse; // at least avoid a memory leak
            return;
        }
    } else {
        // Assume we want to remove the last mouse - remove it from mice.
        mouse = mice.takeLast();
    }

    if ( !mouse ) {
        qWarning("SnakeScene::removeMouse() - Unable to remove a mouse.");
        return;
    }

    // Also remove it from items
    int listIndex = items.indexOf(mouse);
    if ( listIndex > -1 ) {
        items.removeAt(listIndex);
    } else {
        qWarning("SnakeScene::updateMice() - no mouse in items!!");
    }

    // Remove mouse from the scene as well.
    removeItem(mouse);
    update(mouse->sceneBoundingRect());

#ifdef SNAKEGAME_MOUSE_TERRIFIED
    if ( mouse == tremblingMouse )
        tremblingMouse = 0;
#endif
    delete mouse;
}

// Places each existing mouse at a new random position on the board.
void SnakeScene::repositionMice()
{
#ifdef SNAKEGAME_MOUSE_TERRIFIED
    // Stop any trembling mouse (but don't stop the timer).
    sootheMouse(false);
#endif

    for ( QList<Mouse *>::iterator control = mice.begin(); control != mice.end(); control++ ) {
        Mouse *mouse = *control;

        // Position the mouse randomly on the board.
        QPointF randomPosition = getRandomPosition(mouse->boundingRect());
        mouse->setPos(randomPosition);
    }
}

// Called when starting a new game, when the level changes, etc. Gets rid of excess existing mice,
// creates enough mice to equal the level number, repositions all mice randomly on the board.
void SnakeScene::updateMice()
{
    // Get rid of any excess mice.
    for ( int i = mice.count(); i > level; i-- ) {
        // Remove the last mouse from the mice list & all other cleaning up.
        removeMouse(0);
    }
#ifdef SNAKEGAME_MOUSE_TERRIFIED
    tremblingMouse = 0;
#endif

    // Create randomly placed mice equal to the level number.
    for ( int i = mice.count(); i < level; i++ ) {
        createMouse(); // also appends it to 'mice', scene etc
    }

    // Go through all the mice (whether new or otherwise) and give them random positions on
    // the board.
    repositionMice();
}

/*!
  \internal
  \fn void SnakeScene::turn(SnakeManager::Direction direction)
*/
void SnakeScene::turn(SnakeManager::Direction direction)
{
    snake->turn(direction);
}

// Creates snake and adds it to the scene.
Snake *SnakeScene::createSnake()
{
    Snake *snake = new Snake(this,snakeManager);
    items.append(snake);
    snake->setZValue(SNAKE_Z_VALUE);
    return snake;
}

// Creates mouse and adds it to the scene.
Mouse *SnakeScene::createMouse()
{
    QPixmap pixmap = snakeManager->getMouseImage();
    Mouse *mouse = new Mouse(this,pixmap);
    items.append(mouse);
    mice.append(mouse);
    mouse->setZValue(MOUSE_Z_VALUE);
    return mouse;
}

// Creates a wall and adds it to the scene.
Wall *SnakeScene::createWall(uint idx)
{
    QPixmap pixmap = snakeManager->getWallImage(idx);
    Wall *wall = new Wall(this,pixmap,idx);

    // Store the wall in the list of items.
    items.append(wall);
    // If it's an inner wall, store it separately so that getAvailableRegion() has access to it.
    if ( wall->isBorder() ) {
        innerWalls.append(wall);
    }

    int zVal;
    // Establish whether it's a corner, ie LU, RU, LD, RD.
    if ( ((idx & SnakeManager::Left) ^ (idx & SnakeManager::Right)) && ((idx & SnakeManager::Up) ^ (idx & SnakeManager::Down)) ) {
        zVal = CORNER_Z_VALUE;
    } else {
        zVal = WALL_Z_VALUE;
    }

    wall->setZValue(zVal);

    return wall;
}

// Called by ctor to create all the walls.
void SnakeScene::createWalls()
{
    //int tileSize = SnakeManager::tileSize;

    //int right = tileSize*screenwidth-tileSize;
    //int bottom = tileSize*screenheight-tileSize;
    Wall *wall = 0;
    // Create corners. Each corner object will be created based on left + right + up + down.
    wall = createWall(SnakeManager::Right | SnakeManager::Down);
    wall = createWall(SnakeManager::Left | SnakeManager::Up);
    wall = createWall(SnakeManager::Left | SnakeManager::Down);
    wall = createWall(SnakeManager::Right | SnakeManager::Up);

    // Create top and bottom sides.
    wall = createWall(SnakeManager::Left | SnakeManager::Right | SnakeManager::Up);
    wall = createWall(SnakeManager::Left | SnakeManager::Right | SnakeManager::Down);

    // Create left and right sides.
    wall = createWall(SnakeManager::Up | SnakeManager::Down | SnakeManager::Left);
    wall = createWall(SnakeManager::Up | SnakeManager::Down | SnakeManager::Right);

    // Create mid-screen walls.
    wall = createWall(SnakeManager::Up);
    wall = createWall(SnakeManager::Down);

    // Create ends for the mid-screen walls.


}

// Returns the width of the scene inside the border walls, in pixels.
int SnakeScene::getAvailableWidth() const
{
    return qRound(width() - (SnakeManager::BORDER * 2) - (SnakeManager::TILE_SIZE * 2) - 2);
}

// Returns the height of the scene inside the border walls, in pixels.
int SnakeScene::getAvailableHeight() const
{
    return qRound(height() - (SnakeManager::BORDER * 2) - (SnakeManager::TILE_SIZE * 2) - 2);
}

// Returns all the available areas that are unoccupied by walls or snake.
QRegion SnakeScene::getAvailableRegion(QRectF bounds)
{
    if ( availableRegion.isEmpty() ) {
        // Has been re-set or else not needed before - figure it out now.
        // Set the initial region to the area inside the border walls.
        QRect rectangle(SnakeManager::BORDER + SnakeManager::TILE_SIZE + 1,
                        SnakeManager::BORDER + SnakeManager::TILE_SIZE + 1,
                        qRound(getAvailableWidth() - bounds.width()),
                        qRound(getAvailableHeight() - bounds.height()));
        availableRegion |= rectangle;

        // Also need the inner walls.
        for ( QList<Wall *>::const_iterator control = innerWalls.constBegin(); control != innerWalls.constEnd(); control++ ) {
            Wall *wall = *control;
            // Remove the region for this wall.
            QRect boundingRect = wall->sceneBoundingRect(); //.toRect();
            // Warning - don't use setPos() here, since it may change the width and height (Qt is full
            // of side-effects!!!)
            boundingRect = QRect(qRound(boundingRect.x() - bounds.width()),
                                 qRound(boundingRect.y() - bounds.height()),
                                 qRound(boundingRect.width() + bounds.width()),
                                 qRound(boundingRect.height() + bounds.height()));
            availableRegion -= QRegion(boundingRect);
        }
    }

    // Also subtract the snake's current area from the available region. Don't subtract this from
    // 'availableRegion', since the snake is forever moving around, while the walls are static.
    QRegion region = availableRegion - snake->getRegion();

    return region;
}

// Gets a random position capable of holding an object that has the bounding rectangle of 'bounds',
// by calling getAvailableRegion(...)
QPointF SnakeScene::getRandomPosition(QRectF bounds)
{
    // We need a random point within the walls. The available region gives us all the areas that are unoccupied
    // by walls or snake.
    QRegion region = getAvailableRegion(bounds);
    QVector<QRect> areas = region.rects();

    // Choose one of the rectangles randomly.
    QRect area = areas.at(rand() % areas.count());

    // Choose a random x and a random y within that area.
    int x = area.x() + (rand() % area.width());
    int y = area.y() + (rand() % area.height());

    return QPointF(x,y);
}

#ifdef SNAKEGAME_MOUSE_TERRIFIED
void SnakeScene::checkForTerror()
{
    tremblingMouse = 0;

    // Go through all of our mice, find out who is close (and is the closest to) the snake.
    QPointF snakePos = snake->pos();
    int dist = -1;
    for ( QList<Mouse *>::const_iterator control = mice.constBegin(); control != mice.constEnd(); control++ ) {
        Mouse *nextMouse = *control;
        // Find out what the distance is between the snake and this next mouse. This is a pretty
        // primitive distance finding mechanism (x-distance + y-distance), but it will do for
        // this purpose.
        int nextDist = qAbs(qRound(snakePos.x() - nextMouse->pos().x())) + qAbs(qRound(snakePos.y() - nextMouse->pos().y()));
        if ( nextDist < 5 * SnakeManager::TILE_SIZE ) {
            // This mouse is close to the snake. If it's nearer to the snake than the
            // one we've got, use this mouse.
            if ( (dist == -1) || (nextDist < dist) ) {
                tremblingMouse = nextMouse;
                dist = nextDist;
            }
        }
    }

    if ( tremblingMouse ) {
        // This mouse should feel terrified.
        tremblingMouse->changeFrightStatus();
    }
}
#endif

#ifdef SNAKEGAME_MOUSE_TERRIFIED
void SnakeScene::sootheMouse(bool gamePaused)
{
    if ( gamePaused ) {
        terrorTimer->stop();
    }

    if ( tremblingMouse ) {
        tremblingMouse->changeFrightStatus(false);
        tremblingMouse = 0;
    }
}
#endif

