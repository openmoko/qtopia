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
#include "minefield.h"

#include <qsettings.h>

#include <qtopiaapplication.h>

#include <qpainter.h>
#include <qdrawutil.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qtimer.h>
#include <qstyle.h>
#include <qevent.h>

#include <stdlib.h>

static const int maxGrid = 28;
static const int minGrid = 16;
static const int preferredGrid = 22;

class Mine //: public Qt
{
public:
    enum MineState {
        Hidden = 0,
        Empty,
        Mined,
        Flagged,
#ifdef MARK_UNSURE
        Unsure,
#endif
        Exploded,
        Wrong
    };

    Mine( MineField* );
    void paint( QPainter * p, const QPalette & cg, const QRect & cr );

    QSize sizeHint() const { return QSize( maxGrid, maxGrid ); }

    void activate( bool sure = true );
    void setHint( int );

    void setState( MineState );
    MineState state() const { return st; }

    bool isMined() const { return mined; }
    void setMined( bool m ) { mined = m; }

#ifdef QTOPIA_PHONE
    bool selected() const { return mSelected; }
    void setSelected( bool f ) { mSelected = f; }
#endif

    static void paletteChange();


private:
#ifdef QTOPIA_PHONE
    bool mSelected;
#endif

    bool mined;
    int hint;

    MineState st;
    MineField *field;

    static QPixmap* knownField;
    static QPixmap* unknownField;
    static QPixmap* flag_pix;
    static QPixmap* mine_pix;
};

QPixmap* Mine::knownField = 0;
QPixmap* Mine::unknownField = 0;
QPixmap* Mine::flag_pix = 0;
QPixmap* Mine::mine_pix = 0;

Mine::Mine( MineField *f )
{
#ifdef QTOPIA_PHONE
    mSelected = false;
#endif
    mined = false;
    st = Hidden;
    hint = 0;
    field = f;
}

void Mine::activate( bool sure )
// sure is true if the mine is being triggered (as in left-clicked),
// and false if it is being flagged/deflagged (as in right-clicked)
{
    if ( !sure ) {
        switch ( st ) {
        case Hidden:
            setState( Flagged );
            break;
        case Flagged:
#ifdef MARK_UNSURE
            setState( Unsure );
            break;
        case Unsure:
#endif
            setState( Hidden );
        default:
            break;
        }
    } else if ( st == Flagged ) {
        return;
    } else {
        if ( mined ) {
            setState( Exploded );
        } else {
            setState( Empty );
        }
    }
}

void Mine::setState( MineState s )
{
    st = s;
}

void Mine::setHint( int h )
{
    hint = h;
}

void Mine::paletteChange()
{
    delete knownField;
    knownField = 0;
    delete unknownField;
    unknownField = 0;
    delete mine_pix;
    mine_pix = 0;
    delete flag_pix;
    flag_pix = 0;
}

void Mine::paint( QPainter* p, const QPalette &cg, const QRect& cr )
{
    int x = cr.x();
    int y = cr.y();

    QPalette scg;
    scg = cg;
#ifdef QTOPIA_PHONE
    if( selected()  && !Qtopia::mousePreferred())
    {
        scg.setBrush( QPalette::Base, cg.brush( QPalette::Highlight ) );
        scg.setBrush( QPalette::Button, cg.brush( QPalette::Highlight ) );
    }
#endif

    const int pmmarg=cr.width()/5;

#ifndef QTOPIA_PHONE
    if ( !unknownField || unknownField->width() != cr.width() ||
         unknownField->height() != cr.height() ) {
        delete unknownField;
        unknownField = new QPixmap( cr.width(), cr.height() );
        QPainter pp( unknownField );
        QBrush br( scg.button() );
        qDrawWinButton( &pp, QRect( 0, 0, cr.width(), cr.height() ), scg, false, &br );
    }
#else
    QPainter pixp;

    if( unknownField )
        delete unknownField;
    unknownField = new QPixmap( cr.width(), cr.height() );
    unknownField->fill();
    pixp.begin( unknownField );
    const QBrush ubr( scg.button() );
    qDrawWinButton( &pixp, QRect(0, 0, cr.width(), cr.height()), scg, false, &ubr );
    pixp.end();
#endif

#ifndef QTOPIA_PHONE
    if ( !knownField || knownField->width() != cr.width() ||
         knownField->height() != cr.height() ) {
        delete knownField;
        knownField = new QPixmap( cr.width(), cr.height() );
        QPainter pp( knownField );
        QBrush br( scg.button().color().dark(115) );
        qDrawWinButton( &pp, QRect( 0, 0, cr.width(), cr.height() ), scg, true, &br );
    }
#else
// nb: knownField is currently being redrawn every single time Mine::paint() is called on a phone.  This does not seem ideal
    if(knownField)
        delete knownField;
    knownField = new QPixmap( cr.width(), cr.height() );
    knownField->fill();
    pixp.begin( knownField );
    const QBrush kbr( scg.button().color().dark(115) );
    qDrawWinButton( &pixp, QRect(0, 0, cr.width(), cr.height()), scg, true, &kbr );
    pixp.end();

#endif

    if ( !flag_pix || flag_pix->width() != cr.width()-pmmarg*2 ||
         flag_pix->height() != cr.height()-pmmarg*2 ) {
        if( flag_pix )
            delete flag_pix;
//      flag_pix = new QPixmap( cr.width()-pmmarg*2, cr.height()-pmmarg*2 );
        flag_pix = new QPixmap(QPixmap(":image/flag").scaled(cr.width()-pmmarg*2, cr.height()-pmmarg*2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }

    if ( !mine_pix || mine_pix->width() != cr.width()-pmmarg*2 ||
         mine_pix->height() != cr.height()-pmmarg*2 ) {
        if( mine_pix )
            delete mine_pix;
        mine_pix = new QPixmap((QPixmap(":image/mine").scaled(cr.width()-pmmarg*2, cr.height()-pmmarg*2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation) ));
    }

    p->save();

    switch(st) {
    case Hidden:
        p->drawPixmap( x, y, *unknownField );
#ifdef MINE_SWEEP_CHEAT
        if (isMined()) p->drawPixmap( x+pmmarg, y+pmmarg, *mine_pix );
#endif
    break;
    case Empty:
        p->drawPixmap( x, y, *knownField );
        if ( hint > 0 ) {
            switch( hint ) {
            case 1:
                p->setPen( Qt::blue );
                break;
            case 2:
                p->setPen( QColor( Qt::green ).dark() );
                break;
            case 3:
                p->setPen( Qt::red );
                break;
            case 4:
                p->setPen( QColor( Qt::darkYellow ) );
                break;
            case 5:
                p->setPen( Qt::darkMagenta );
                break;
            case 6:
                p->setPen( Qt::darkRed );
                break;
            default:
                p->setPen( Qt::black );
                break;
            }
            p->drawText( cr, Qt::AlignHCenter | Qt::AlignVCenter, QString::number( hint ) );
        }
        break;
    case Mined:
        p->drawPixmap( x, y, *knownField );
        p->drawPixmap( x+pmmarg, y+pmmarg, *mine_pix );
        break;
    case Exploded:
        p->drawPixmap( x, y, *knownField );
        p->drawPixmap( x+pmmarg, y+pmmarg, *mine_pix );
        p->setPen( Qt::red );
        p->drawText( cr, Qt::AlignHCenter | Qt::AlignVCenter, "X" );
        break;
    case Flagged:
        p->drawPixmap( x, y, *unknownField );
        p->drawPixmap( x+pmmarg, y+pmmarg, *flag_pix );
        break;
#ifdef MARK_UNSURE
    case Unsure:
        p->drawPixmap( x, y, *unknownField );
        p->drawText( cr, Qt::AlignHCenter | Qt::AlignVCenter, "?" );
        break;
#endif
    case Wrong:
        p->drawPixmap( x, y, *unknownField );
        p->drawPixmap( x+pmmarg, y+pmmarg, *flag_pix );
        p->setPen( Qt::red );
        p->drawText( cr, Qt::AlignHCenter | Qt::AlignVCenter, "X" );
        break;
    }

    p->restore();
}

/*
  MineField implementation
*/

MineField::MineField( QWidget* parent )
: QFrame( parent )
{
    topMargin = 0;
    leftMargin = 0;

    setState( GameOver );
    setFrameStyle(NoFrame);
    setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    setFocusPolicy( Qt::NoFocus );

    holdTimer = new QTimer( this );
    connect( holdTimer, SIGNAL( timeout() ), this, SLOT( held() ) );

    flagAction = NoAction;
    ignoreClick = false;
    currRow = currCol = -1;
    pressed = false;
    minecount=0;
    mineguess=0;
    nonminecount=0;
    cellSize = -1;
#ifdef QTOPIA_PHONE
    mAlreadyHeld = false;
#endif

    numRows = 0;
    numCols = 0;
    mines = NULL;
}

MineField::~MineField()
{
    for ( int i = 0; i < numCols*numRows; i++ )
        delete mines[i];
    delete[] mines;
}

void MineField::setState( State st )
{
    stat = st;
}

void MineField::setup( int level )
{
#ifdef QTOPIA_PHONE
    currRow = 0; currCol = 0;
#endif
    lev = level;
    setState( Waiting );

    int i;
    for ( i = 0; i < numCols*numRows; i++ )
        delete mines[i];
    delete[] mines;
    switch( lev ) {
    case 1:
        numRows = 9 ;
        numCols = 9 ;
        minecount = 12;
        break;
    case 2:
        numRows = 13;
        numCols = 13;
        minecount = 33;
        break;
    case 3:
        numCols = 18;
        numRows = 18;
        minecount = 66 ;
        break;
    }
    mines = new Mine* [numRows*numCols];
    for ( i = 0; i < numCols*numRows; i++ )
        mines[i] = new Mine( this );

#ifdef QTOPIA_PHONE
    if( mines[0] )
        mines[0]->setSelected( true );
#endif

    nonminecount = numRows*numCols - minecount;
    mineguess = minecount;
    emit mineCount( mineguess );
    Mine::paletteChange();

    setCellSize(findCellSize());

    update( 0, 0, numCols*cellSize, numRows*cellSize) ;
    updateGeometry();
}

void MineField::paintEvent(QPaintEvent* event)
{
    QRect eventRect = event->rect();

    int clipx = eventRect.left();
    int clipy = eventRect.top();
    int clipw = eventRect.width();
    int cliph = eventRect.height();

    QPainter p(this);

    int c1 = clipx / cellSize;
    int c2 = ( clipx + clipw - 1 ) / cellSize;
    int r1 = clipy / cellSize;
    int r2 = ( clipy + cliph - 1 ) / cellSize;

    for ( int c = c1; c <= c2 ; c++ ) {
        for ( int r = r1; r <= r2 ; r++ ) {
            int x = c * cellSize;
            int y = r * cellSize;
            Mine *m = mine( r, c );
            if ( m )
            {
#ifdef QTOPIA_PHONE
                if( r == currRow && c == currCol )
                    m->setSelected( true );
                else
                    m->setSelected( false );
#endif
                m->paint( &p, QPalette(), QRect(x, y, cellSize, cellSize ) );
            };
        }
    }
}


void MineField::setAvailableRect( const QRect &r )
{
    availableRect = r;
    int newCellSize = findCellSize();


    if ( newCellSize == cellSize ) {
        setCellSize( cellSize );
    } else {
        setCellSize( newCellSize );
    }
}

int MineField::findCellSize()
{
    int w = availableRect.width() - 2;
    int h = availableRect.height() - 2;
    int cellsize;

    cellsize = qMin( w/numCols, h/numRows );
    if(cellsize < minGrid) cellsize = minGrid;
    if(cellsize > maxGrid) cellsize = maxGrid;
    return cellsize;
}

QSize MineField::sizeHint() const {
    return QSize(cellSize*numCols, cellSize*numRows);
}

void MineField::setCellSize( int cellsize )
{

    int w2 = cellsize*numCols;
    int h2 = cellsize*numRows;

    // Don't rely on the change in cellsize to force a resize,
    // as it's possible to have the same size cells when going
    // from a large play area to a small one.

    resize(w2, h2);

    setGeometry(0, 0, w2, h2);
    updateGeometry();
    cellSize = cellsize;
}

QSize MineField::minimumSize() const
{
    return QSize(cellSize*numCols, cellSize*numRows);
}

void MineField::placeMines()
{
    int mines = minecount;
    while ( mines ) {
        int col = int((double(rand()) / double(RAND_MAX)) * numCols);
        int row = int((double(rand()) / double(RAND_MAX)) * numRows);

        Mine* m = mine( row, col );

        if ( m && !m->isMined() && m->state() == Mine::Hidden ) {
            m->setMined( true );
            mines--;
        }
    }
}


void MineField::updateCell( int r, int c )
{
    update( c*cellSize, r*cellSize, cellSize, cellSize );
}




/*
 state == Waiting means no "hold"


*/
void MineField::cellPressed( int row, int col )
{
    if ( state() == GameOver )
        return;
    currRow = row;
    currCol = col;
    if ( state() == Playing )
        holdTimer->setSingleShot( true);
        holdTimer->start( 150 );
}

void MineField::held()
{
#ifndef QTOPIA_PHONE
    flagAction = FlagNext;
    updateMine( currRow, currCol );
    ignoreClick = true;
#else
    if( !mAlreadyHeld )
    {
        flagAction = FlagNext;
        updateMine( currRow, currCol );
        ignoreClick = true;
        mAlreadyHeld = true;
    }
#endif
}



void MineField::mousePressEvent( QMouseEvent* e )
{
    int c = e->pos().x() / cellSize;
    int r = e->pos().y() / cellSize;
    if ( onBoard( r, c ) ){
            updateCell( currRow, currCol ); //Counting on the update event to land after currRow and currCol have changed to de-highlight the old selected square
            cellPressed( r, c );
            updateCell( r, c );
    }
    else
        currCol = currRow = -1;
    pressed = true;
        holdTimer->setSingleShot( true);
        holdTimer->start( 150 );

}

void MineField::keyPressEvent( QKeyEvent* e )
{
#ifndef QTOPIA_PHONE
#if defined(Q_WS_QWS) || defined(Q_WS_QWS)
    flagAction = ( e->key() == Qt::Key_Up ) ? FlagOn : NoAction;
#else
    flagAction = ( ( e->state() & ShiftButton ) ==  ShiftButton ) ? FlagOn : NoAction;
#endif
#endif

#ifdef QTOPIA_PHONE
    if (e->isAutoRepeat()) {
        QFrame::keyPressEvent( e );
        return;
    }

    int row = currRow, col = currCol;
    int key = e->key();

    if ( key == Qt::Key_Select ) {
        cellPressed( currRow, currCol );
        pressed = true;
    } else if( key == Qt::Key_Back || key == Qt::Key_No ) {
        QFrame::keyPressEvent( e );
    } else {
        if( state() == GameOver )
            return;

        switch( key )
        {
            case Qt::Key_Up:
                --row;
                break;
            case Qt::Key_Down:
                ++row;
                break;
            case Qt::Key_Left:
                --col;
                break;
            case Qt::Key_Right:
                ++col;
                break;
            default:
                QFrame::keyPressEvent( e );
                break;
        }

        if( (currRow != row || currCol != col) && onBoard( row, col ) )
        {
            //update affected mines
            updateCell( currRow, currCol );
            currRow = row;
            currCol = col;
            updateCell( currRow, currCol );
            emit currentPointChanged(QPoint(currCol*cellSize, currRow*cellSize));
        }
    }
#endif
}

void MineField::mouseReleaseEvent( QMouseEvent* e )
{
    if( state() == GameOver )
        emit newGameSelected();
    int c = e->pos().x() / cellSize;
    int r = e->pos().y() / cellSize;
    if ( onBoard( r, c ) && c == currCol && r == currRow )

        emit currentPointChanged(QPoint(currCol*cellSize, currRow*cellSize));
// Mouse button debug stuff
        switch (e->button())
        {
            case Qt::NoButton:
                break;
            case Qt::LeftButton:
                cellClicked( r, c ); //temp - this may belong in left button, it may not.
                break;
            case Qt::RightButton:
                flagAction=FlagNext;
                updateMine(r,c);
                break;
            default:
                break;
        }


    if ( flagAction == FlagNext ) {
        flagAction = NoAction;
    }
    pressed = false;
}


void MineField::keyReleaseEvent( QKeyEvent *e )
{
    flagAction = NoAction;

#ifdef QTOPIA_PHONE
    if (e->isAutoRepeat()) {
        QFrame::keyPressEvent( e );
        return;
    }
    int key = e->key();

    if ( key == Qt::Key_Select && pressed ) {
        mAlreadyHeld = false;
        if( state() == GameOver )
            emit newGameSelected();
        else
            cellClicked( currRow, currCol );
    } else {
        QFrame::keyReleaseEvent( e );
    }

    pressed = false;
#else
    Q_UNUSED( e );
#endif

}

int MineField::getHint( int row, int col )
{
    int hint = 0;
    for ( int c = col-1; c <= col+1; c++ )
        for ( int r = row-1; r <= row+1; r++ ) {
            Mine* m = mine( r, c );
            if ( m && m->isMined() )
                hint++;
        }

    return hint;
}

void MineField::setHint( int row, int col )
{
    Mine *m = mine( row, col );
    if ( !m )
        return;

    int hint = getHint( row, col );

    if ( !hint ) {
        for ( int c = col-1; c <= col+1; c++ )
            for ( int r = row-1; r <= row+1; r++ ) {
                Mine* m = mine( r, c );
                if ( m && m->state() == Mine::Hidden ) {
                    m->activate( true );
                    nonminecount--;
                    setHint( r, c );
                    updateCell( r, c );
                }
            }
    }

    m->setHint( hint );
    updateCell( row, col );
}

/*
  Only place mines after first click, since it is pointless to
  kill the player before the game has started.
*/

void MineField::cellClicked( int row, int col )
{
    if ( state() == GameOver )
        return;
    if ( state() == Waiting ) {
        Mine* m = mine( row, col );
        if ( !m )
            return;
        m->setState( Mine::Empty );
        nonminecount--;
        placeMines();
        setState( Playing );
        emit gameStarted();
        updateMine( row, col );
    } else { // state() == Playing
        holdTimer->stop();
        if ( ignoreClick )
            ignoreClick = false;
        else
            updateMine( row, col );
    }
}

void MineField::updateMine( int row, int col )
{
    Mine* m = mine( row, col );
    if ( !m )
        return;

    bool wasFlagged = m->state() == Mine::Flagged;
    bool wasEmpty =  m->state() == Mine::Empty;

    m->activate( flagAction == NoAction );

    if ( m->state() == Mine::Exploded ) {
        emit gameOver( false );
        setState( GameOver );
        return;
    } else if ( m->state() == Mine::Empty ) {
        setHint( row, col );
        if ( !wasEmpty )
            nonminecount--;
    }

    if ( flagAction != NoAction ) {
        if ( m->state() == Mine::Flagged ) {
            if (/*mineguess > 0*/ true) {  //TODO: Address the implications of this change, especially as relates to negative numbers displayed on the LCD display on a PDA
                --mineguess;
                emit mineCount( mineguess );
                if ( m->isMined() )
                    --minecount;
            } else {
                m->setState(Mine::Hidden);
            }
        } else if ( wasFlagged ) {
            ++mineguess;
            emit mineCount( mineguess );
            if ( m->isMined() )
                ++minecount;
        }
    }

    updateCell( row, col );

    if ( !nonminecount ) {
        emit gameOver( true );
        setState( GameOver );
    }
}

void MineField::showMines()
{
    for ( int c = 0; c < numCols; c++ )
        for ( int r = 0; r < numRows; r++ ) {
            Mine* m = mine( r, c );
            if ( !m )
                continue;
            if ( m->isMined() && m->state() == Mine::Hidden )
                m->setState( Mine::Mined );
            if ( !m->isMined() && m->state() == Mine::Flagged )
                m->setState( Mine::Wrong );

            updateCell( r, c );
        }
}

void MineField::paletteChange( const QPalette &o )
{
    Mine::paletteChange();
    QFrame::paletteChange( o );
}

void MineField::writeConfig(QSettings& cfg) const
{
    cfg.beginGroup("Field");
    cfg.setValue("Level",lev);
    QString grid="";
    if ( stat == Playing ) {
        for ( int x = 0; x < numCols; x++ )
            for ( int y = 0; y < numRows; y++ ) {
                char code='A'+(x*17+y*101)%21; // Reduce the urge to cheat
                const Mine* m = mine( y, x );
                int st = (int)m->state(); if ( m->isMined() ) st+=5;
                grid += code + st;
            }
    }
    cfg.setValue("Grid",grid);
    cfg.endGroup();
}

void MineField::readConfig(QSettings& cfg)
{
    cfg.beginGroup("Field");
    lev = cfg.value("Level",1).toInt();
    setup(lev);
    flagAction = NoAction;
    ignoreClick = false;
    currRow = currCol = 0;
    QString grid = cfg.value("Grid").toString();
    int x;
    if ( !grid.isEmpty() ) {
        int i=0;
        minecount=0;
        mineguess=0;
        for ( x = 0; x < numCols; x++ ) {
            for ( int y = 0; y < numRows; y++ ) {
                char code='A'+(x*17+y*101)%21; // Reduce the urge to cheat
                int st = (char)grid[i++].unicode()-code;
                Mine* m = mine( y, x );
                if ( st >= 5 ) {
                    st-=5;
                    m->setMined(true);
                    minecount++;
                    mineguess++;
                }
                    m->setState((Mine::MineState)st);
                switch ( m->state() ) {
                  case Mine::Flagged:
                    if (m->isMined())
                        minecount--;
                    mineguess--;
                    break;
                  case Mine::Empty:
                    --nonminecount;
                    break;
                default:
                    break;
                }
            }
        }
        for ( x = 0; x < numCols; x++ ) {
            for ( int y = 0; y < numRows; y++ ) {
                Mine* m = mine( y, x );
                if ( m->state() == Mine::Empty )
                    m->setHint(getHint(y,x));
            }
        }
    }
    setState( Playing );
    cfg.endGroup();
    emit mineCount( mineguess );
}

