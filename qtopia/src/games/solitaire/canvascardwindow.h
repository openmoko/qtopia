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
#ifndef CANVAS_CARD_WINDOW_H 
#define CANVAS_CARD_WINDOW_H


#include <qmainwindow.h>
#include <qcanvas.h>
#include <qtimer.h>


class CanvasCardGame;
class CanvasCardView;
class ZoomView;
class QPopupMenu;
class ContextMenu;
class QAction;


class CanvasCardWindow : public QMainWindow {
    Q_OBJECT

public:
    CanvasCardWindow(QWidget* parent=0, const char* name=0, WFlags f=0);
    virtual ~CanvasCardWindow();

public slots:
    void setCardBacks();
    void doResize();
    void initFreecell();
    void initPatience();
    void doSettings();

protected:
    virtual void showEvent( QShowEvent *e );
    virtual void resizeEvent( QResizeEvent *e );
    void updateDraw();

private:
    void initGame( bool newGame, int type );
    void initGame();
    void closeGame();

    QTimer resizeTimeout;
    
    QCanvas canvas;
    CanvasCardView *canvasView;
    bool casinoRules;
    bool zoomOn;
    bool snapOn;
    bool drawThree;
    int cardBack;
    int drawId;
    int gameType;
    bool resizing;
    CanvasCardGame *cardGame;
    int timeout;

    QAction *newPatGame;
    QAction *newFreGame;
    QAction *settingsAct;
};


class CanvasCardView : public QCanvasView
{
    Q_OBJECT
public:
    CanvasCardView(CanvasCardWindow *w, QCanvas *c, QWidget *parent);
    void setCardGame(CanvasCardGame *game);
    void setZoomViewEnabled(bool b);
    void updateZoomPos();
    void showWinText(bool);
    CanvasCardWindow *window() const { return canvasWindow; }

public slots:
    void updateZoomView();
protected:
    void keyPressEvent(QKeyEvent *ke);
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);
    void contentsMouseDoubleClickEvent(QMouseEvent *e);
private:
    CanvasCardWindow *canvasWindow;
    CanvasCardGame *cardGame;
    ZoomView *zoomView;

    QCanvasText *wonText1, *wonText2;
};

#endif

