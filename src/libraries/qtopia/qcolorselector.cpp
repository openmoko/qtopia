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
#include "qcolorselector.h"
#include <qpainter.h>
#include <qmenu.h>
#include <qpushbutton.h>
#include <qstyle.h>
#include <qlayout.h>
#include <qevent.h>
#include <qframe.h>
#include <qtopiaapplication.h>

#include <QStyleOptionButton>
#include <QDesktopWidget>

class QColorSelectorPrivate
{
public:
    QRgb palette[256];
    QColor col,defCol;
    int highlighted;
    bool pressed;
};

class QColorSelectorDialogPrivate
{
public:
    QColorSelector *picker;
};

QColorSelectorDialog::QColorSelectorDialog( const QColor &c, QWidget *parent,
                                        Qt::WFlags f )
    : QDialog( parent, f )
{
    init();
    setDefaultColor( c );
}

QColorSelectorDialog::QColorSelectorDialog( QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f )
{
    init();
}

QColor QColorSelectorDialog::color() const
{
    return d->picker->color();
}

void QColorSelectorDialog::setDefaultColor( const QColor &c )
{
    d->picker->setDefaultColor( c );
}

const QColor &QColorSelectorDialog::defaultColor() const
{
    return d->picker->defaultColor();
}

QColor QColorSelectorDialog::getColor( const QColor &c, QWidget *parent ) // static
{
    QColor fetchedColor;
    QColorSelectorDialog *dialog = new QColorSelectorDialog( c, parent );
    dialog->setModal(true);
    dialog->setWindowTitle( tr("Select color") );
    if( QtopiaApplication::execDialog( dialog ) == QDialog::Accepted )
        fetchedColor = dialog->color();
    delete dialog;
    return fetchedColor;
}

void QColorSelectorDialog::setColor( const QColor &c )
{
    d->picker->setColor( c );
}

void QColorSelectorDialog::colorSelected( const QColor &c )
{
    if( isModal() )
        accept();
    else
        hide();
    emit selected( c );
}

void QColorSelectorDialog::init()
{
    d = new QColorSelectorDialogPrivate;
    d->picker = new QColorSelector( this );
    connect( d->picker, SIGNAL(selected(const QColor&)),
                                     this, SLOT(colorSelected(const QColor&)) );
    QVBoxLayout *l = new QVBoxLayout( this );
    l->addWidget( d->picker );
#ifdef QTOPIA_PHONE
    QtopiaApplication::setMenuLike( this, true );
#endif
}

QColorSelector::QColorSelector( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    d = new QColorSelectorPrivate;
    d->highlighted = -1;
    d->pressed = false;
    int idx = 0;
    for( int ir = 0x0; ir <= 0xff; ir+=0x55 ) {
        for( int ig = 0x0; ig <= 0xff; ig+=0x55 ) {
            for( int ib = 0x0; ib <= 0xff; ib+=0x55 ) {
                d->palette[idx]=qRgb( ir, ig, ib );
                idx++;
            }
        }
    }

    bool changed = true;
    while ( changed ) {
        changed = false;
        int i = 0;
        QColor col( d->palette[i] );
        while ( i < idx-1 ) {
            QColor ncol( d->palette[i+1] );
            int h1, s1, v1;
            int h2, s2, v2;
            col.getHsv( &h1, &s1, &v1 );
            ncol.getHsv( &h2, &s2, &v2 );
            if ( h1*255+v1 > h2*255+v2 ) {
                QRgb tmp = d->palette[i];
                d->palette[i] = d->palette[i+1];
                d->palette[i+1] = tmp;
                changed = true;
            }
            col = ncol;
            i++;
        }
    }
    setMinimumSize( 65, 65 );
    setFocus();
}

QColorSelector::~QColorSelector()
{
    delete d;
}

QSize QColorSelector::sizeHint() const
{
    int s = 12*8+1;
    if( parent() && parent()->isWidgetType() )
        s = qMax( s, ((QWidget *)parent())->width() );
    return QSize( s, s + (d->defCol.isValid()?QApplication::globalStrut().height():0) );
}

void QColorSelector::setDefaultColor( const QColor &c )
{
    d->defCol = c;
    if ( !d->col.isValid() && d->defCol.isValid() )
        d->highlighted = -1;
    update();
}

const QColor &QColorSelector::defaultColor() const
{
    return d->defCol;
}

QColor QColorSelector::color() const
{
    return d->col;
}

void QColorSelector::setColor( const QColor &c )
{
    d->col = c;
    if ( QColor(d->defCol) == c ) {
        d->highlighted = -1;
        update();
        return;
    }

    int r = c.red();
    int g = c.green();
    int b = c.blue();
    int rd = (QColor(d->palette[0]).red() - r);
    int gd = (QColor(d->palette[0]).green() - g);
    int bd = (QColor(d->palette[0]).blue() - b);
    int bestCol = 0;
    int bestDiff = rd*rd + gd*gd + bd*bd;
    for ( int i = 0; i < 8*8; i++ ) {
        if ( QColor(d->palette[i]) == c ) {
            d->highlighted = i;
            update();
            return;
        } else {
            rd = (QColor(d->palette[i]).red() - r);
            gd = (QColor(d->palette[i]).green() - g);
            bd = (QColor(d->palette[i]).blue() - b);
            int diff = rd*rd + gd*gd + bd*bd;
            if ( diff < bestDiff ) {
                bestDiff = diff;
                bestCol = i;
            }
        }
    }

    // We don't have the exact color specified, so we'll pick the closest.
    // Not really optimal, but better than settling for the default.
    d->highlighted = bestCol;
    update();
}

void QColorSelector::paintEvent( QPaintEvent * )
{
    QPainter p( this );

    int cw = (width()-1)/8;
    int ch = (height()-1-(d->defCol.isValid()?16:0))/8;
    int idx = 0;
    for ( int y = 0; y < 8; y++ ) {
        for ( int x = 0; x < 8; x++ ) {
            p.fillRect( x*cw+1, y*ch+1, cw-1, ch-1, QColor(d->palette[idx]) );
            if ( idx == d->highlighted )
                p.drawRect( x*cw, y*ch, cw+1, ch+1 );
            idx++;
        }
    }
    if ( d->defCol.isValid() ) {
        p.fillRect( 1, 8*ch+1, width()-2, height()-(8*ch+1), d->defCol );
        if ( d->highlighted == -1 )
            p.drawRect( 0, 8*ch, width(), height()-(8*ch) );
        if ( qGray(d->defCol.rgb()) < 128 )
            p.setPen( Qt::white );
        p.drawText( 0, 8*ch+1, width(), height()-(8*ch+1), Qt::AlignCenter, tr("Default") );
    }
}

void QColorSelector::mousePressEvent( QMouseEvent *me )
{
    d->pressed = true;
    mouseMoveEvent( me );
}

void QColorSelector::mouseMoveEvent( QMouseEvent *me )
{
    int cw = (width()-1)/8;
    int ch = (height()-1-(d->defCol.isValid()?16:0))/8;
    int row = (me->pos().y()-1)/ch;
    int col = (me->pos().x()-1)/cw;
    int old = d->highlighted;
    if ( row >=0 && row < 8 && col >= 0 && col < 8 )
        d->highlighted = row*8+col;
    else //if ( d->defCol.isValid() && row >=8 && me->pos().y() < height() )
        d->highlighted = -1;
    /*else
        d->highlighted = -2;
        */

    if ( old != d->highlighted ) {
        repaint( rectOfColor( old ) );
        repaint( rectOfColor( d->highlighted ) );
    }
}

void QColorSelector::mouseReleaseEvent( QMouseEvent * )
{
    if ( !d->pressed )
        return;
    if ( d->highlighted >= 0 ) {
        d->col = QColor(d->palette[d->highlighted]);
        emit selected( d->col );
    } else if ( d->highlighted == -1 ) {
        d->col = d->defCol;
        emit selected( d->col );
    }
}

void QColorSelector::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
        case Qt::Key_Left:
        {
            int old = d->highlighted;
            if( d->highlighted == -1  )
                d->highlighted = 63;
            else
                d->highlighted--;
            repaint( rectOfColor( old ) );
            repaint( rectOfColor( d->highlighted ) );
            break;
        }
        case Qt::Key_Right:
        {
            int old = d->highlighted;
            d->highlighted++;
            if ( d->highlighted >= 64 )
                d->highlighted = -1;
            repaint( rectOfColor( old ) );
            repaint( rectOfColor( d->highlighted ) );
            break;
        }
        case Qt::Key_Up:
        {
            int old = d->highlighted;
            if( d->highlighted <= 7 && d->highlighted >= 0 )
                d->highlighted = -1;
            else if ( d->highlighted == -1 )
                d->highlighted = 63;
            else
                d->highlighted-=8;
            repaint( rectOfColor( old ) );
            repaint( rectOfColor( d->highlighted ) );
            break;
        }
        case Qt::Key_Down:
        {
            int old = d->highlighted;
            if( d->highlighted == -1 )
                d->highlighted = 0;
            else
                d->highlighted+=8;
            if ( d->highlighted > 63 )
                d->highlighted = -1;
            repaint( rectOfColor( old ) );
            repaint( rectOfColor( d->highlighted ) );
            break;
        }
#ifdef QTOPIA_PHONE
        case Qt::Key_Select:
#endif
        case Qt::Key_Space:
        case Qt::Key_Return:
        {
            if ( d->highlighted >= 0 ) {
                d->col = QColor(d->palette[d->highlighted]);
                emit selected( d->col );
            } else if ( d->highlighted == -1 ) {
                d->col = d->defCol;
                emit selected( d->col );
            }
            topLevelWidget()->hide();
            break;
        }
#ifdef QTOPIA_PHONE
        case Qt::Key_Back:
        case Qt::Key_No:
#endif
        case Qt::Key_Escape:
        {
            topLevelWidget()->hide();
        }
            break;
        default:
            QWidget::keyPressEvent( e );

    }
}

void QColorSelector::showEvent( QShowEvent *e )
{
    QWidget::showEvent(e);
    d->pressed = false;
}

QRect QColorSelector::rectOfColor( int idx ) const
{
    QRect r;
    int cw = (width()-1)/8;
    int ch = (height()-1-(d->defCol.isValid()?16:0))/8;
    if ( idx >= 0 ) {
        int row = idx/8;
        int col = idx%8;
        r = QRect( col*cw, row*ch, cw+1, ch+1 );
    } else if ( idx == -1 ) {
        r = QRect( 0, ch*8, width(), height()-ch*8 );
    }

    return r;
}

//===========================================================================

class QColorButtonPrivate
{
public:
    QFrame *popup;
    QColorSelector *picker;
    QColor col;
};

QColorButton::QColorButton( QWidget *parent )
    : QPushButton( parent )
{
    init();
}

QColorButton::QColorButton( const QColor &c, QWidget *parent )
    : QPushButton(parent)
{
    init();
    d->col = c;
}

QColorButton::~QColorButton()
{
    delete d;
}

QColor QColorButton::color() const
{
    return d->col;
}

void QColorButton::init()
{
    d = new QColorButtonPrivate;
    // d->popup = new QMenu( this );
    d->popup = new QFrame( this, Qt::Popup );
    d->popup->setAttribute(Qt::WA_ShowModal, true);
    d->popup->setFrameStyle( QFrame::Box | QFrame::Plain );
    QVBoxLayout *l = new QVBoxLayout( d->popup );
    l->setMargin( 0 );
    d->picker = new QColorSelector( d->popup );
    l->addWidget( d->picker );
    // d->popup->insertItem( d->picker );
    // setPopup( d->popup );
    connect( d->picker, SIGNAL(selected(const QColor&)),
            this, SIGNAL(selected(const QColor&)) );
    connect( d->picker, SIGNAL(selected(const QColor&)),
            this, SLOT(colorSelected(const QColor&)) );
    connect( this, SIGNAL( clicked() ),
            this, SLOT( showSelector() ));
}

void QColorButton::showSelector()
{
    d->popup->move( mapToGlobal( QPoint( 0, height() )));
    d->popup->show();
}

void QColorButton::setColor( const QColor &c )
{
    d->col = c;
    d->picker->setColor( c );
    update();
}

void QColorButton::setDefaultColor( const QColor &c )
{
    d->picker->setDefaultColor( c );
}

const QColor &QColorButton::defaultColor() const
{
     return d->picker->defaultColor();
}

void QColorButton::colorSelected( const QColor &c )
{
    d->popup->hide();
    d->col = c;
}

void QColorButton::paintEvent( QPaintEvent *e )
{
    QPushButton::paintEvent( e );
    QPainter p( this );
    drawButtonLabel( &p );
}

void QColorButton::drawButtonLabel( QPainter *p )
{
    QStyleOptionButton sob;
    sob.init( this );
    QRect r = style()->subElementRect( QStyle::SE_PushButtonContents, &sob );
    if ( isDown() || isChecked() ){
        int sx = style()->pixelMetric( QStyle::PM_ButtonShiftHorizontal, &sob );
        int sy = style()->pixelMetric( QStyle::PM_ButtonShiftVertical, &sob );
        r.moveTo( r.topLeft() + QPoint( sx, sy ) );
    }
    int x, y, w, h;
    r.getRect( &x, &y, &w, &h );
    int dx = style()->pixelMetric( QStyle::PM_MenuButtonIndicator, &sob );
    QStyleOptionButton arrowStyle = sob;
    arrowStyle.rect.setLeft( x + (w - h) );
    style()->drawPrimitive( QStyle::PE_IndicatorArrowDown, &arrowStyle, p );
    w -= dx;
    if ( d->col.isValid() )
        p->fillRect( x+2, y+2, w-6, h-4 , d->col );
    else if ( defaultColor().isValid() )
        p->fillRect( x+2, y+2, w-6, h-4 , defaultColor() );
}

