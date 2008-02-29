/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef SNAKEGAME_H
#define SNAKEGAME_H

#include <QGraphicsView>

class Snake;
class QLabel;
class QGraphicsSimpleTextItem;
class QGraphicsRectItem;

class SnakeGame : public QGraphicsView
{
    Q_OBJECT

public:
    SnakeGame(QWidget* parent=0, Qt::WFlags f=0);
    ~SnakeGame();

protected:
    virtual void keyPressEvent(QKeyEvent*);
    virtual void resizeEvent(QResizeEvent*);
    virtual void focusOutEvent(QFocusEvent*);
    virtual void mousePressEvent(QMouseEvent*);

private slots:
    void newGame(bool start=true);
    void gameOver();
    void endWait();
    void levelUp();

private:
    void clear();
    void createWalls();
    void createMice();
    void showMessage(const QString &);
    void clearMessage();
    void pause();
    void resume();

    QGraphicsScene *scene;
    Snake* snake;
    int screenwidth;
    int screenheight;
    int level;
    int stage;
    int mice;
    bool waitover;
    bool gamestopped;
    bool paused;

    QGraphicsSimpleTextItem *gamemessage;
    QGraphicsRectItem *gamemessagebkg;
};

#endif
