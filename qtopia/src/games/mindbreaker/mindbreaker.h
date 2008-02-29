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

#ifndef MINDBREAKER_H
#define MINDBREAKER_H

#include <qwidget.h>
#include <qmainwindow.h>
#include <qimage.h>
#include <qvector.h>
#include <qcanvas.h>
#include <qlabel.h>

static const int panel_height = 26;
static const int panel_width = 180;

static const int title_height = 25;
static const int title_width = 180;

static const int bin_margin = 10;
static const int peg_size = 20;
static const int answerpeg_size = 13;

static const int first_peg_x_diff = 21;
static const int first_peg_y_diff = ((panel_height - peg_size) >> 1);
static const int peg_spacing = 30;

static const int answerpegx = 152;
static const int answerpegy = 2;
static const int answerpeg_diff = 9;

static const int board_height = (title_height + (panel_height * 9));
static const int board_width = (panel_width + (bin_margin * 2) + peg_size);

class Peg;
class QToolButton;
class QTimer;

class MindBreakerBoard : public QCanvasView // QWidget
{
    Q_OBJECT
public:
    MindBreakerBoard(QWidget *parent=0, const char *name=0, int wFlags=0 );
    ~MindBreakerBoard();

    void getScore(int *, int *);

    void resizeEvent(QResizeEvent*);
    void fixSize();

signals:
    void scoreChanged(int, int);

public slots:
    void clear();
    void resetScore();

private slots:
    void doFixSize();

protected:
    void contentsMousePressEvent(QMouseEvent *);
    void contentsMouseMoveEvent(QMouseEvent *);
    void contentsMouseReleaseEvent(QMouseEvent *);

private:
    QCanvas cnv;

    void readConfig();
    void writeConfig();

    void drawBackground();
    void checkGuess();
    void checkScores();
    void placeGuessPeg(int pos, int pegId);

    QImage panelImage;
    QImage titleImage;

    Peg *moving;
    Peg *current_highlight;
    QPoint moving_pos;

    // the game stuff 
    int answer[4];
    int current_guess[4];
    int past_guesses[4*9];
    int current_go;

    int null_press;
    QPoint null_point;
    bool copy_press;
    Peg *copy_peg;
    bool game_over;

    int total_turns;
    int total_games;

    QTimer *widthTimer;
};

class MindBreaker : public QMainWindow // QWidget
{
    Q_OBJECT
public:
    MindBreaker(QWidget *parent=0, const char *name=0, WFlags f=0 );

public slots:
    void setScore(int, int);

protected:
    void resizeEvent( QResizeEvent * );

private:
    QCanvas canvas;
    MindBreakerBoard *board;
    QToolButton *score;

};


#endif 
