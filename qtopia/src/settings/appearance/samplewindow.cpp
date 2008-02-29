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

#include "samplewindow.h"
#include <qtopia/global.h>
#include <qtopia/fontdatabase.h>
#include <qtopia/config.h>
#include <qtopia/applnk.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/pluginloader.h>
#include <qtopia/styleinterface.h>
#include <qtopia/windowdecorationinterface.h>
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#endif
#ifdef QTOPIA_PHONE
# include <qtopia/private/phonedecoration_p.h>
# include <qtopia/contextbar.h>
# include <qtopia/phonestyle.h>
#else
# include <qtopia/qpestyle.h>
#endif

#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qslider.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qdir.h>
#include <qgroupbox.h>
#include <qwindowsstyle.h>
#include <qobjectlist.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qmenubar.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qpixmapcache.h>
#if QT_VERSION >= 0x030000
#include <qstylefactory.h>
#endif
#include <qaccel.h>

#include <stdlib.h>

#define SAMPLE_HEIGHT	115

class SampleText : public QWidget
{
public:
    SampleText( const QString &t, bool h, QWidget *parent )
	: QWidget( parent ), hl(h), text(t)
    {
	if ( hl )
	    setBackgroundMode( PaletteHighlight );
	else
	    setBackgroundMode( PaletteBase );
    }

    QSize sizeHint() const
    {
	QFontMetrics fm(font());
	return QSize( fm.width(text)+10, fm.height()+4 );
    }

    void paintEvent( QPaintEvent * )
    {
	QPainter p(this);
	if ( hl )
	    p.setPen( colorGroup().highlightedText() );
	else
	    p.setPen( colorGroup().text() );
	p.drawText( rect(), AlignLeft | AlignVCenter, text );
    }

private:
    bool hl;
    QString text;
};

//-------------------------------------------------------------------------

SampleWindow::SampleWindow( QWidget *parent )
    : QWidget(parent), iface(0), popup(0)
{
    setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum) );
    init();
}

QSize SampleWindow::sizeHint() const
{
    int w = QMIN(container->sizeHint().width() + 10, qApp->desktop()->width()-10);
    int h = container->sizeHint().height() + 30;
    return QSize( w, QMAX(SAMPLE_HEIGHT,h) );
}

void SampleWindow::setFont( const QFont &f )
{
    QWidget::setFont( f );
    if (popup)
	popup->setFont( f );
}

void SampleWindow::setDecoration( WindowDecorationInterface *i )
{
    iface = i;
    wd.rect = QRect( 0, 0, 150, 75 );
    wd.caption = tr("Sample");
    wd.palette = palette();
    wd.flags = WindowDecorationInterface::WindowData::Dialog |
	       WindowDecorationInterface::WindowData::Active;
    wd.reserved = 1;

    th = iface->metric(WindowDecorationInterface::TitleHeight, &wd);
    tb = iface->metric(WindowDecorationInterface::TopBorder, &wd);
    lb = iface->metric(WindowDecorationInterface::LeftBorder, &wd);
    rb = iface->metric(WindowDecorationInterface::RightBorder, &wd);
    bb = iface->metric(WindowDecorationInterface::BottomBorder, &wd);

    int yoff = th + tb;
    int xoff = lb;

    wd.rect.setX( 0 );
    wd.rect.setWidth( width() - lb - rb );
    wd.rect.setY( 0 );
    wd.rect.setHeight( height() - yoff - bb );

    container->setGeometry( xoff, yoff, wd.rect.width(), wd.rect.height() );
    setMinimumSize( QMIN(container->sizeHint().width()+lb+rb, qApp->desktop()->width()-10),
		    QMAX(SAMPLE_HEIGHT,container->sizeHint().height()+tb+th+bb) );
}

void SampleWindow::paintEvent( QPaintEvent * )
{
    if ( !iface )
	return;

    QPainter p( this );

    p.translate( lb, th+tb );

    iface->drawArea(WindowDecorationInterface::Border, &p, &wd);
    iface->drawArea(WindowDecorationInterface::Title, &p, &wd);

    p.setPen(palette().active().color(QColorGroup::HighlightedText));
    QFont f( font() );
    f.setWeight( QFont::Bold );
    p.setFont(f);
    iface->drawArea(WindowDecorationInterface::TitleText, &p, &wd);

    QRect brect( 0, -th, iface->metric(WindowDecorationInterface::HelpWidth,&wd), th );
    iface->drawButton( WindowDecorationInterface::Help, &p, &wd,
	brect.x(), brect.y(), brect.width(), brect.height(), (QWSButton::State)0 );
    brect.moveBy( wd.rect.width() -
	iface->metric(WindowDecorationInterface::OKWidth,&wd) -
	iface->metric(WindowDecorationInterface::CloseWidth,&wd), 0 );
    iface->drawButton( WindowDecorationInterface::Close, &p, &wd,
	brect.x(), brect.y(), brect.width(), brect.height(), (QWSButton::State)0 );
    brect.moveBy( iface->metric(WindowDecorationInterface::CloseWidth,&wd), 0 );
    iface->drawButton( WindowDecorationInterface::OK, &p, &wd,
	brect.x(), brect.y(), brect.width(), brect.height(), (QWSButton::State)0 );
}

void SampleWindow::init()
{
    int dheight = QApplication::desktop()->height();
    int dwidth = QApplication::desktop()->width();
    bool wide = ( dheight < 300 && dwidth > dheight );

    container = new QVBox( this );
    popup = new QPopupMenu( container );
    popup->insertItem( tr("Normal Item"), 1 );
    popup->insertItem( tr("Disabled Item"), 2 );
    popup->setItemEnabled(2, FALSE);
    QMenuBar *mb = new QMenuBar( container );
    mb->insertItem( tr("Menu"), popup );
    QHBox *hb = new QHBox( container );
    QWidget *w = new QWidget( hb );
    (void)new QScrollBar( 0, 0, 0, 0, 0, Vertical, hb );

    int m = 4;
    if (qApp->desktop()->width() < 200)
	m = 2;

    QGridLayout *gl;
    if ( wide )
	gl = new QGridLayout( w, 4, 1, m );
    else
	gl = new QGridLayout( w, 2, 2, m );

    SampleText *t1 = new SampleText( tr("Normal Text"), FALSE, w );

    SampleText *t2 = new SampleText( tr("Highlighted Text"), TRUE, w );

    QPushButton *pb = new QPushButton( tr("Button"), w );
    pb->setFocusPolicy( NoFocus );

    QCheckBox *cb = new QCheckBox( tr("Check Box"), w );
    cb->setFocusPolicy( NoFocus );
    cb->setChecked( TRUE );

    if ( wide ) {
	gl->addWidget( t1, 0, 0 );
	gl->addWidget( t2, 1, 0 );
	gl->addWidget( pb, 2, 0 );
	gl->addWidget( cb, 3, 0 );
    } else {
	gl->addWidget( t1, 0, 0 );
	gl->addWidget( t2, 1, 0 );
	gl->addWidget( pb, 0, 1 );
	gl->addWidget( cb, 1, 1 );
    }

    QWhatsThis::add( this, tr("Sample window using the selected settings.") );
}

bool SampleWindow::eventFilter( QObject *, QEvent *e )
{
    switch ( e->type() ) {
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::MouseMove:
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	    return TRUE;
	default:
	    break;
    }

    return FALSE;
}

void SampleWindow::paletteChange( const QPalette &old )
{
    QWidget::paletteChange( old );
    wd.palette = palette();
    if (popup)
	popup->setPalette( palette() );
}

void SampleWindow::resizeEvent( QResizeEvent *re )
{
    wd.rect.setX( 0 );
    wd.rect.setWidth( width() - lb - rb );
    wd.rect.setY( 0 );
    wd.rect.setHeight( height() - th - tb - bb );

    container->setGeometry( lb, th+tb, wd.rect.width(), wd.rect.height() );
    QWidget::resizeEvent( re );
}

void SampleWindow::setUpdatesEnabled( bool e )
{
    QWidget::setUpdatesEnabled( e );
    const QObjectList *ol = children();
    if ( ol) {
	QObjectListIt it( *ol );
	for ( ; it.current(); ++it ) {
	    QObject *o = *it;
	    if( o->isWidgetType() ) {
		((QWidget *)o)->setUpdatesEnabled( e );
	    }
	}
    }
}

void SampleWindow::fixGeometry()
{
    int w = QMIN(container->sizeHint().width()+lb+rb, qApp->desktop()->width()-10);
    int h = container->sizeHint().height()+tb+th+bb;

    setMinimumSize( w, QMAX(SAMPLE_HEIGHT,h) );
}

