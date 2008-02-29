/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "mindbreaker.h"

#include <qtopia/resource.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/qpetoolbar.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

static int pegRTTI = 3393393;

static int adjusted_panel_height;
static int adjusted_panel_width;

static int adjusted_bin_margin;
static int adjusted_peg_size;
static int adjusted_answerpeg_size;

static int adjusted_title_height;
static int adjusted_title_width;

static int adjusted_first_peg_x_diff;
static int adjusted_first_peg_y_diff;
static int adjusted_peg_spacing;

static int adjusted_answerpegx;
static int adjusted_answerpegy;
static int adjusted_answerpeg_xdiff;
static int adjusted_answerpeg_ydiff;

static int adjusted_board_height;
static int adjusted_board_width;

static void setupBoardSize(int w, int h)
{
    adjusted_panel_width = w * 3/4;
    adjusted_title_width = w * 3/4;

    adjusted_title_height = h/10;
    adjusted_panel_height = (h-adjusted_title_height)/9;

    adjusted_bin_margin = w * 10/240;
    adjusted_peg_size = adjusted_panel_height*3/4;
    adjusted_answerpeg_size = QMIN(adjusted_panel_width*15/180,adjusted_panel_height*15/25);
    
    // looks a bit dodgy on larger sizes
    if ( adjusted_peg_size > 40 )
	adjusted_peg_size = 40;
    
    adjusted_first_peg_x_diff = w * 31/240-adjusted_peg_size/2;
    adjusted_first_peg_y_diff = (adjusted_panel_height - adjusted_peg_size)/2;
    adjusted_peg_spacing = w * 30/240;

    // looks a bit dodgy on larger sizes (still does though, but not as much...)
    if ( adjusted_answerpeg_size > 22 )
	adjusted_answerpeg_size = 22;
    
    adjusted_answerpegx = adjusted_panel_width * 159/180 - adjusted_answerpeg_size/2;
    adjusted_answerpegy = adjusted_panel_height/3 - adjusted_answerpeg_size/2;
    adjusted_answerpeg_xdiff = adjusted_panel_width * 10/180;
    adjusted_answerpeg_ydiff = adjusted_panel_height * 9/25;
    
    adjusted_board_height = adjusted_title_height + (adjusted_panel_height * 9);
    adjusted_board_width = adjusted_panel_width + (adjusted_bin_margin * 2) + adjusted_peg_size;

//    qDebug("Adjusted width %d height %d", adjusted_board_width, adjusted_board_height);
}


/* helper class,  */
class Peg : public QCanvasRectangle
{
public:
    Peg(QCanvas *canvas, int type, int go = -1, int pos = -1);
    int rtti() const {return pegRTTI; }
    void advance(int phase);

    bool hit( const QPoint &) const;

/* a placed peg is one that has been set down on the board correctly and 
   should not be moved, only copied */
    bool placed() const;
    void setPlaced(bool);

    int pegGo() const;
    int pegPos() const;
    void setPegPos(int);

    int type() const;

    static void buildImages();
    static QImage imageForType(int t);

    static int eggLevel;

protected:
    void drawShape(QPainter &);
private:
    static QVector<QImage> normalPegs;
    static QVector<QImage> specialPegs;

    bool isplaced;
    int pegtype;
    int peg_go;
    int peg_pos;

    int aniStep;
};
int Peg::eggLevel = 0;
QVector<QImage> Peg::normalPegs;
QVector<QImage> Peg::specialPegs;

void Peg::buildImages()
{
    QImage pegs = Resource::loadImage("mindbreaker/pegs");
    int x = 0; 
    int y = 0;
    int i;
    eggLevel = 0;
    normalPegs.resize(10);
    for (i = 0; i < 6; i++) {
        normalPegs.insert(i, new QImage(pegs.copy(x, y, peg_size, peg_size).
				smoothScale(adjusted_peg_size, adjusted_peg_size) ));
        x += peg_size;
    }
    specialPegs.resize(5);
    for (i = 0; i < 5; i++) {
        specialPegs.insert(i, new QImage(pegs.copy(x,y,peg_size, peg_size).
				smoothScale(adjusted_peg_size, adjusted_peg_size) ));
        x += peg_size;
    }

    QImage image = Resource::loadImage("mindbreaker/mindbreaker");
    /* copy from master image to functional images */
    x = 0;
    y = panel_height;
    normalPegs.insert(8, 
            new QImage( image.copy(x, y, panel_width, panel_height).
			smoothScale( adjusted_panel_width, adjusted_panel_height)
		      ));
    y += panel_height;
    y += title_height;
    normalPegs.insert(9, 
            new QImage(image.copy(x, y, title_width, title_height).
			smoothScale( adjusted_title_width, adjusted_title_height)
		    ));
    y += title_height;

    x = 6 * peg_size;
    normalPegs.insert(6, 
            new QImage(image.copy(x, y, answerpeg_size, answerpeg_size).
			smoothScale( adjusted_answerpeg_size, adjusted_answerpeg_size) ));
    x += answerpeg_size;
    normalPegs.insert(7, 
            new QImage(image.copy(x, y, answerpeg_size, answerpeg_size).
			smoothScale( adjusted_answerpeg_size, adjusted_answerpeg_size) ));
}

QImage Peg::imageForType(int t) 
{
    if (eggLevel > t ) {
        if( t < 5) {
            return *specialPegs[t];
        } else {
            return *normalPegs[rand() % 6];
        }
    }
    return *normalPegs[t];
}

Peg::Peg(QCanvas *canvas , int t, int g, int p) 
        : QCanvasRectangle(canvas)
{
    setSize(normalPegs[t]->width(), normalPegs[t]->height() );
    pegtype = t;
    isplaced = FALSE;
    peg_pos = p;
    peg_go = g;
    aniStep = rand() % 6;
    setAnimated(TRUE);
}

void Peg::advance(int phase) {
    if (phase == 0)
        aniStep = (++aniStep) % 6;
    else {
        hide();
        show();
    }
}

void Peg::drawShape(QPainter &p )
{
    if ((pegtype == 5) && eggLevel > 5) {
        p.drawImage(int(x()), int(y()), *normalPegs[aniStep]);
    } else 
        p.drawImage(int(x()), int(y()), imageForType(pegtype));
}

bool Peg::hit( const QPoint &p ) const
{
    int ix = p.x() - int(x());
    int iy = p.y() - int(y());
    if (!normalPegs[pegtype]->valid(ix, iy))
        return FALSE;
    QRgb pixel = normalPegs[pegtype]->pixel(ix, iy);
    return (qAlpha(pixel ) != 0);
}

inline bool Peg::placed() const
{
    return isplaced;
}

inline int Peg::pegGo() const
{
    return peg_go;
}

inline int Peg::pegPos() const
{
    return peg_pos;
}

inline void Peg::setPegPos(int p) 
{
    peg_pos = p;
}

inline void Peg::setPlaced(bool p) 
{
    isplaced = p;
}

inline int Peg::type() const 
{
    return pegtype;
}

/* Load the main image, copy from it the pegs, the board, and the answer image 
 * and use these to create the tray, answer and board
 */
MindBreaker::MindBreaker( QWidget *parent, const char *name, int wFlags )
   : QMainWindow(parent, name, wFlags)
{
    setMinimumSize(160,210);

    QWidget *w = new QWidget( this );
    w->setBackgroundColor( black );
    QHBoxLayout *hb = new QHBoxLayout( w );
    hb->addStretch();
    board = new MindBreakerBoard(w);
    hb->addWidget( board, 100 );
    hb->addStretch();

    setCentralWidget(w);
    
    setToolBarsMovable( FALSE );

    QPEToolBar *tb = new QPEToolBar(this);
    tb->setHorizontalStretchable( TRUE );

    QIconSet newicon = Resource::loadIconSet("new");
    new QToolButton(newicon, tr("New Game"), 0, 
                                       board, SLOT(clear()), tb, "NewGame");

    score = new QToolButton(tb);
    score->setText("");
    score->setMaximumHeight(20);
    score->setUsesTextLabel(TRUE);
    tb->setStretchableWidget(score);

    connect(board, SIGNAL(scoreChanged(int, int)), this, SLOT(setScore(int, int)));
    connect(score, SIGNAL(clicked()), board, SLOT(resetScore()));

    int a, b;
    board->getScore(&a, &b);
    setScore(a,b);
    
    layout()->setResizeMode(QLayout::FreeResize);
}

void MindBreaker::setScore(int turns, int games)
{
    double average;
    double total_turns = turns;
    double total_games = games;

    if(total_games > 0) 
        average = total_turns / total_games;
    else 
        average = 0.0;

    score->setText(tr("win avg: %1 turns (%2 games)").arg(average).arg(games));
}

void MindBreaker::resizeEvent( QResizeEvent *e )
{
    board->fixSize();
    QMainWindow::resizeEvent( e );
}


MindBreakerBoard::MindBreakerBoard( QWidget *parent, 
                                    const char *name, int wFlags )
           : QCanvasView(0, parent, name, wFlags), 
	    moving(0), game_over(FALSE), total_turns(0), total_games(0)
{
    setFrameStyle( NoFrame );
    setupBoardSize(qApp->desktop()->width(),qApp->desktop()->height());
    cnv.resize(100,100);
    setCanvas(&cnv);
    setBackgroundColor( black );
    
    struct timeval tv;

    current_go = 0;
    gettimeofday(&tv, 0);
    srand(tv.tv_usec);

    canvas()->setAdvancePeriod(500);
    current_highlight = 0;

    widthTimer = new QTimer( this );
    connect(widthTimer, SIGNAL(timeout()), this, SLOT(doFixSize()) );

    setMaximumWidth( QMIN(qApp->desktop()->height(),qApp->desktop()->width()) );
    //doFixSize(); // build images... needs to be done before reading config.
    //readConfig(); // first read... to ensure initial labels and side look right.
}

void MindBreakerBoard::readConfig()
{
    Config c("MindBreaker", Config::User);
    c.setGroup("Board");
    game_over = FALSE;
    int i;
    if (c.readNumEntry("Answer0") < 0) {
        for (i = 0; i < 4; i++) {
            answer[i] = rand() % 6;
            current_guess[i] = 6;
        }
        total_turns = 0;
        total_games = 0;
    } else {
        int j;
        c.setGroup("Score");
        total_turns = c.readNumEntry("Turns");
        total_games = c.readNumEntry("Games");
        if(total_turns < 0)
            total_turns = 0;
        if(total_games < 0)
            total_games = 0;


        checkScores();
        c.setGroup("Board");
        for(i = 0; i < 4; i++) 
            answer[i] = c.readNumEntry(QString("Answer%1").arg(i));
        /* read, and parse past guesses */
        current_go = 0;
        for(j=0; j < 9; j++) {
            current_guess[0] = c.readNumEntry(QString("Go%1p0").arg(j));
            if (current_guess[0] < 0)
                break;
            placeGuessPeg(0, current_guess[0]);
            current_guess[1] = c.readNumEntry(QString("Go%1p1").arg(j));
            placeGuessPeg(1, current_guess[1]);
            current_guess[2] = c.readNumEntry(QString("Go%1p2").arg(j));
            placeGuessPeg(2, current_guess[2]);
            current_guess[3] = c.readNumEntry(QString("Go%1p3").arg(j));
            placeGuessPeg(3, current_guess[3]);
            checkGuess();
        }
        for(i = 0; i < 4; i++) {
            current_guess[i] = c.readNumEntry(QString("CurrentGo%1").arg(i));
            if (current_guess[i] != 6) 
                placeGuessPeg(i, current_guess[i]);
        }
    }
}

MindBreakerBoard::~MindBreakerBoard()
{
    int i;
    if (game_over) {
        current_go = 0;
        /* clear the answer, clear the guess */
        for (i = 0; i < 4; i++) {
            answer[i] = rand() % 6;
            current_guess[i] = 6;
        }
    }
    writeConfig();
}

void MindBreakerBoard::writeConfig()
{
    Config c("MindBreaker", Config::User);
    c.setGroup("Board");
    c.clearGroup();
    /* write the board */
    int i,j;
    for (i = 0; i < current_go; i++) {
        for(j = 0; j < 4; j++)
            c.writeEntry(QString("Go%1p%2").arg(i).arg(j), past_guesses[4*i+j]);
    }
    for(j = 0; j < 4; j++) 
        c.writeEntry(QString("CurrentGo%1").arg(j), current_guess[j]);
    for(j = 0; j < 4; j++) 
        c.writeEntry(QString("Answer%1").arg(j), answer[j]);

    c.setGroup("Score");
    /* write the score */

    c.writeEntry("Turns", total_turns);
    c.writeEntry("Games", total_games);
}

void MindBreakerBoard::getScore(int *a, int *b)
{
    *a = total_turns;
    *b = total_games;
    return;
}

void MindBreakerBoard::fixSize()
{
    hide();
    setMaximumWidth( parentWidget()->height() );
    widthTimer->start( 20, TRUE );
}

void MindBreakerBoard::doFixSize()
{
    QSize s = size();
    int fw = frameWidth();
    s.setWidth(s.width() - fw);
    s.setHeight(s.height() - fw);

    /* min size is 200 x 260 */
/*
    if (s.width() < adjusted_board_width)
        s.setWidth(adjusted_board_width);

    if (s.height() < adjusted_board_height)
        s.setHeight(adjusted_board_height);
*/

    if ( current_highlight ) // non-first resize
	writeConfig();

    setupBoardSize(s.width() - fw, s.height() - fw);
    canvas()->resize(s.width() - fw, s.height() - fw);
    Peg::buildImages(); // must be done BEFORE any pegs are made

    QImage image = Resource::loadImage("mindbreaker/mindbreaker");

    /* copy from master image to functional images */
    int x = 0;
    int y = 0;
    panelImage = image.copy(x, y,  panel_width, panel_height).
		smoothScale( adjusted_panel_width, adjusted_panel_height);
    
    y += panel_height;
    y += panel_height;

    titleImage = image.copy(x, y, title_width, title_height).
		smoothScale( adjusted_title_width, adjusted_title_height);
    show();

    delete current_highlight;
    current_highlight = new Peg(canvas(), 8);
    current_highlight->setPlaced(TRUE);
    current_highlight->setX(0);
    current_highlight->setY(adjusted_board_height - ((current_go + 1) * adjusted_panel_height));
    current_highlight->setZ(0);
    current_highlight->show();

    /* set up the game */
    //readConfig();

    /* draw initial screen */
    //drawBackground();
    //canvas()->update();
    clear();

    readConfig();
}

void MindBreakerBoard::placeGuessPeg(int pos, int pegId)
{
        int x = adjusted_first_peg_x_diff + (pos * adjusted_peg_spacing);
        int y = adjusted_board_height - ((current_go + 1) * adjusted_panel_height) 
                + adjusted_first_peg_y_diff;

        Peg *peg = new Peg(canvas(), pegId, current_go, pos);
        peg->setPegPos(pos);
        peg->setPlaced(TRUE);
        peg->setX(x);
        peg->setY(y);
        peg->setZ(2);
        peg->show();
}

void MindBreakerBoard::drawBackground()
{
    int i, j, x, y, x_gap, y_gap;
    QPixmap background  = QPixmap(canvas()->width(), canvas()->height());

    QPainter painter(&background);

    painter.fillRect(0, 0, canvas()->width(), canvas()->height(), QColor(0,0,0));
    /* very first thing is to draw the bins, as everything else needs
     * to be drawn over them */

    QPen pen(QColor(85, 45, 27), 4);
    painter.setPen(pen);
    x_gap = canvas()->width() - (adjusted_panel_width + (2 * adjusted_bin_margin));
    //x_gap += peg_size >> 1;
    if (x_gap < 1) 
        x_gap = 1;

    y_gap = adjusted_board_height / 6;
    y_gap -= (2 * adjusted_bin_margin);
    //y_gap += peg_size >> 1;
    if (y_gap < 1) 
        y_gap = 1;
    x = adjusted_panel_width + adjusted_bin_margin - (adjusted_peg_size >> 1);
    y = adjusted_bin_margin - (adjusted_peg_size >> 1) + 2;

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 10; j++) {
            int rx = x + (rand() % x_gap);
            int ry = y + (rand() % y_gap);
            painter.drawImage(rx,ry, Peg::imageForType(i));
        }
        y += adjusted_board_height / 6;
    }
    /* now draw the surrounding boxes */
    x_gap = canvas()->width() - adjusted_panel_width;
    if (x_gap < 1) x_gap = 1;
    y_gap = adjusted_board_height / 6;
    x = adjusted_panel_width;
    y = 1;

    for (i = 0; i < 6; i++) {
	painter.drawRect(x, y, x_gap, y_gap);
        y += y_gap;
    }

    x = 0;
    y = 0;

    painter.drawImage(x,y, titleImage);
    y = adjusted_title_height;
    /* now nine gues panels */
    for (i = 0; i < 9; i ++) {
        painter.drawImage(x, y, panelImage);
        y += adjusted_panel_height;
    }

    painter.flush();
    canvas()->setBackgroundPixmap(background);
}

void MindBreakerBoard::checkGuess()
{
    int i,j;
    int num_white = 0;
    int num_black = 0;
    int copy_answer[4];
    int copy_guess[4];

    for(i = 0; i < 4; i++) {
        copy_answer[i] = answer[i];
        copy_guess[i] = current_guess[i];
        if (current_guess[i] == 6)
            return;
        if (answer[i] == current_guess[i]) {
            num_black++;
            copy_answer[i] = 6;
            copy_guess[i] = 7; 
        }
    }

    /* now sure that user has completed a 'guess' */
    for (i = 0; i < 4; i++) {
        if (copy_guess[i] == 7)
            continue; // already marked for a black
        for (j = 0; j < 4; j++) {
            if(copy_guess[i] == copy_answer[j]) {
                copy_answer[j] = 6;
                num_white++;
                break;
            }
        }
    }

    int x = adjusted_answerpegx;
    int y = (adjusted_board_height - ((current_go + 1) * adjusted_panel_height)) + adjusted_answerpegy;
  
    if (num_black == 4)
        game_over = TRUE;

    while(num_black > 0) {
        Peg *p = new Peg(canvas(), 7);
        p->setPlaced(TRUE);
        p->setX(x);
        p->setY(y);
        p->setZ(1);
        p->show();
        num_black--;
        
        if (x == adjusted_answerpegx)
            x = adjusted_answerpegx + adjusted_answerpeg_xdiff;
        else  {
            x = adjusted_answerpegx;
            y += adjusted_answerpeg_ydiff;
        }
    }
    while(num_white > 0){
        Peg *p = new Peg(canvas(), 6);
        p->setPlaced(TRUE);
        p->setX(x);
        p->setY(y);
        p->setZ(1);
        p->show();
        num_white--;
        
        if (x == adjusted_answerpegx)
            x = adjusted_answerpegx + adjusted_answerpeg_xdiff;
        else  {
            x = adjusted_answerpegx;
            y += adjusted_answerpeg_ydiff;
        }
    }
    /* move to next go */
    for(i = 0; i < 4; i++) {
        past_guesses[4*current_go+i] = current_guess[i];
        current_guess[i] = 6;
    }

    current_go++;
    if((current_go > 8) || game_over) {
        total_games++;
        if(!game_over)
            total_turns += 10;
        else 
            total_turns += current_go;

        emit scoreChanged(total_turns, total_games);
        Peg *p = new Peg(canvas(), 9);
        game_over = TRUE;
        p->setPlaced(TRUE);
        p->setX(0);
        p->setY(0);
        p->setZ(0);
        p->show();
        
        for (i = 0; i < 4; i++) {
            p = new Peg(canvas(), answer[i], -1);
            p->setX(adjusted_first_peg_x_diff + (i * adjusted_peg_spacing));
            p->setY(adjusted_first_peg_y_diff);
            p->setZ(3);
            p->show();
        }
    } else {
       current_highlight->setY(adjusted_board_height - ((current_go + 1) * adjusted_panel_height));
    }
    canvas()->update();
}

void MindBreakerBoard::clear()
{
    if(!game_over) {
        total_games++;
        total_turns += 10;
        emit scoreChanged(total_turns, total_games);
    }
    int i;
    /* reset the game board */
    game_over = FALSE;
    /* clear the answer, clear the guess */
    for (i = 0; i < 4; i++) {
        answer[i] = rand() % 6;
        current_guess[i] = 6;
    }
    current_go = 0;

    QCanvasItemList list = canvas()->allItems();
    QCanvasItemList::Iterator it = list.begin();
    for (; it != list.end(); ++it) {
        if (*it == current_highlight)
            continue;
        if (*it)
            delete *it;
    }

    current_highlight->setY(adjusted_board_height - ((current_go + 1) * adjusted_panel_height));
    checkScores();
    drawBackground();
    canvas()->update();
}

void MindBreakerBoard::resetScore() 
{
    /* are u sure */

    if (QMessageBox::information(this, tr( "Reset Statistics" ), 
            tr( "Reset the win ratio?" ),
            tr( "OK" ), tr( "Cancel" ) ) == 0) {
        total_turns = 0;
        total_games = 0;
        Peg::eggLevel = 0;
        drawBackground();
        canvas()->update();
        emit scoreChanged(total_turns, total_games);
    }
}

/* EVENTS */

void MindBreakerBoard::contentsMousePressEvent(QMouseEvent *e)
{
    if (game_over) {
	null_press = TRUE;
	null_point = e->pos();
	moving = 0;
	return;
    }

    copy_press = FALSE;
    null_press = FALSE;
    /* ok, first work out if it is one of the bins that
       got clicked */
    if (e->x() > adjusted_panel_width) {
        /* its a bin, but which bin */
        int bin = (e->y() + 2) / (adjusted_board_height / 6);
	if (bin > 5)
            return; // missed everything

        /* make new peg... set it moving */
        moving_pos = e->pos();
        moving = new Peg(canvas(), bin, current_go);
        moving->setX(e->x() - (adjusted_peg_size >> 1));
        moving->setY(e->y() - (adjusted_peg_size >> 1));
        moving->setZ(5);
        moving->show();
        canvas()->update();
        return;
    }

    QCanvasItemList l = canvas()->collisions(e->pos());
    for (QCanvasItemList::Iterator it=l.begin(); it !=l.end(); ++it) {
        if ( (*it)->rtti() == pegRTTI ) {
            Peg *item = (Peg *)(*it);
            if (!item->hit(e->pos()))
                continue;
            if (item->type() > 5) {
                null_press = TRUE;
                null_point = e->pos();
                continue; /* not a color peg */
            }
            if (item->placed()) {
                /* copy */
                if(item->pegGo() == -1)
                    return;
                if(item->pegGo() == current_go) {
                    copy_press = TRUE;
                    copy_peg = item;
                }
                moving = new Peg(canvas(), 
                                 item->type(), current_go);
                moving->setX(e->x() - (adjusted_peg_size >> 1));
                moving->setY(e->y() - (adjusted_peg_size >> 1));
                moving->setZ(5);
                moving->show();
                moving_pos = QPoint(e->x(), e->y());
                canvas()->update();
                return;
            }
            moving = (Peg *)*it;
            moving_pos = e->pos();
            canvas()->update();
            return;
        }
    }
    null_press = TRUE;
    null_point = e->pos();
    moving = 0;
}

void MindBreakerBoard::contentsMouseMoveEvent(QMouseEvent* e)
{
    if (moving ) {
        moving->moveBy(e->pos().x() - moving_pos.x(),
                       e->pos().y() - moving_pos.y());
        moving_pos = e->pos();
        canvas()->update();
        return;
    } 
}

void MindBreakerBoard::contentsMouseReleaseEvent(QMouseEvent* e)
{
    /* time to put down the peg */
    if(moving) {
        if(copy_press) {
            /* check if collided with original. if so, delete both */
            copy_press = FALSE;
            QCanvasItemList l = canvas()->collisions(e->pos());
            for (QCanvasItemList::Iterator it=l.begin(); it !=l.end(); ++it) {
                if (*it == copy_peg) 
                    copy_press = TRUE;
            }
            if (copy_press) {
                current_guess[copy_peg->pegPos()] = 6;
                delete copy_peg;
                delete moving;
                copy_press = FALSE;
                moving = 0;
                copy_peg = 0;
                canvas()->update();
                return;
            }
        }
            
        /* first work out if in y */
        if (e->y() > (adjusted_board_height - (current_go * adjusted_panel_height))) {
            delete moving;
            moving = 0;
            canvas()->update();
            return;
        }
        if (e->y() < (adjusted_board_height - ((current_go + 1) * adjusted_panel_height))) {
            delete moving;
            moving = 0;
            canvas()->update();
            return;
        }
        /* ok, a valid go, but which peg */
        int x_bar = adjusted_first_peg_x_diff - (adjusted_peg_size >> 1);
        x_bar += adjusted_peg_spacing;
        int pos = 0;
        if (e->x() > x_bar) 
            pos = 1;
        x_bar += adjusted_peg_spacing;
        if (e->x() > x_bar)
            pos = 2;
        x_bar += adjusted_peg_spacing;
        if (e->x() > x_bar)
            pos = 3;
        x_bar += adjusted_peg_spacing;

        if (e->x() > x_bar) {
            /* invalid x */
            delete moving;
            moving = 0;
            canvas()->update();
            return;
        }

        int x = adjusted_first_peg_x_diff + (pos * adjusted_peg_spacing);
        int y = adjusted_board_height - ((current_go + 1) * adjusted_panel_height) 
                + adjusted_first_peg_y_diff;
        moving->setPegPos(pos);
        moving->setX(x);
        moving->setY(y);
        moving->setZ(2);

        /* remove all other pegs from this position */
        QCanvasItemList l = canvas()->collisions(QPoint(x,y));
        for (QCanvasItemList::Iterator it=l.begin(); it !=l.end(); ++it) {
            if ( (*it)->rtti() == pegRTTI ) {
                Peg *item = (Peg *)(*it);
                if ((item != moving) && (item != current_highlight))
                    delete item;
            }
        }
        current_guess[pos] = ((Peg *)moving)->type();

        ((Peg *)moving)->setPlaced(true);
        canvas()->update();
        return;
    }
    moving = 0;
    null_point -= e->pos();
    if(null_point.manhattanLength() < 6) {
        if (game_over) 
            clear();
        else 
            checkGuess();
    }
}

void MindBreakerBoard::resizeEvent(QResizeEvent *e)
{
    QCanvasView::resizeEvent(e);
    fixSize();
}


/* Easter egg function... beat the clock */
void MindBreakerBoard::checkScores()
{
    double games = total_games;
    double turns = total_turns;
    double g = games / 10.0;
    Peg::eggLevel = 0;

    double break_even = 5.0;
    if (g < 1.0) 
        return;
    double avg = turns / games;
    g--;
    while (break_even >= 0.0) {
        if (avg >= (break_even + g)) 
            return;
        // score a peg.
        break_even -= 1.0;
        Peg::eggLevel = int(5.0 - break_even);
    }
}
