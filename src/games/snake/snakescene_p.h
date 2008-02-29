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

#ifndef SNAKESCENE_H
#define SNAKESCENE_H

#include "snakemanager_p.h"
#include <QGraphicsScene>
#include <QList>

#define SNAKEGAME_MOUSE_TERRIFIED

class Snake;
class Mouse;
class Wall;
class MessageBox;
class SnakeItemInterface;
class QWidget;


class SnakeScene : public QGraphicsScene
{
    Q_OBJECT

public:

    enum State {None,Stopped,Paused,Running,Pending};

    explicit SnakeScene(QWidget *menuWidget,SnakeManager *);

    ~SnakeScene();

    QList<SnakeItemInterface *> getItems() const { return items; }

    void resize(const QRectF &);

    void pause();

    void updateGame();

    void turn(SnakeManager::Direction);

signals:

    void newGame();
    void gameOver();

private slots:

    void mournSnake();

    void mournMouse(Mouse *mouse);

    void endWait();

#ifdef SNAKEGAME_MOUSE_TERRIFIED
    void checkForTerror();
#endif

private:

    // The stage increases every time all the mice have been eaten. When we reach stage MAX_STAGE,
    // the level (of difficulty) increases.
    static const int MAX_STAGE = 3;

    static const qreal maxSnakeRatio;

    Snake *createSnake();

    Mouse *createMouse();

    void removeMouse(Mouse *mouse);

    Wall *createWall(uint idx);

    void createWalls();

    void updateMice();

    void repositionMice();

    void prepareNewGame(bool startNow,bool keepScore,const QString &msg);

    void startGame();

    void endGame();

#ifdef SNAKEGAME_MOUSE_TERRIFIED
    void sootheMouse(bool gamePaused);
#endif

    QRegion getAvailableRegion(QRectF);

    int getAvailableWidth() const;
    int getAvailableHeight() const;

    QPointF getRandomPosition(QRectF);

    void showMessage(const QString &txt);

    static const int SCORE_FOR_MOUSE = 5;

    static const int WALL_Z_VALUE = 0;
    static const int CORNER_Z_VALUE = 10;
    static const int MOUSE_Z_VALUE = 20;
    static const int SNAKE_Z_VALUE = 30;
    static const int DISPLAY_Z_VALUE = 40;

    SnakeManager *snakeManager;

    QWidget *menuWidget;

    Snake *snake;

    QList<SnakeItemInterface *> items;

    QList<Mouse *> mice;

    QList<Wall *> innerWalls;

    State state;

    int score;
    int level;
    int stage;

    QRegion availableRegion;

    MessageBox *messageBox;

    // Used when another message is pending.
    bool frozen;

#ifdef SNAKEGAME_MOUSE_TERRIFIED
    QTimer *terrorTimer;
    Mouse *tremblingMouse;
#endif



};

#endif
