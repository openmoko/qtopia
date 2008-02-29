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

#ifndef SNAKEVIEW_H
#define SNAKEVIEW_H

#include <QGraphicsView>

class SnakeManager;
class SnakeScene;
class QResizeEvent;
class QKeyEvent;
class QFocusEvent;


class SnakeView : public QGraphicsView
{
    Q_OBJECT

public:

    explicit SnakeView(QWidget *parent = 0, Qt::WFlags f=0);

    ~SnakeView();

protected:

    void resizeEvent(QResizeEvent *);

    void keyPressEvent(QKeyEvent *);

    void focusOutEvent(QFocusEvent *);

private slots:

    void handleNewGame();
    void handleGameOver();

private:

    static const QString imageFileName;

    // The scene that this object views.
    SnakeScene *snakeScene;

    // Storage area for pixmaps, constants etc.
    SnakeManager *snakeManager;
};


#endif
