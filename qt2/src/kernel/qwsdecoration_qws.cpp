/****************************************************************************
** $Id: qt/src/kernel/qwsdecoration_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of Qt/Embedded window manager decorations
**
** Created : 000101
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "qwsdecoration_qws.h"

#ifndef QT_NO_QWS_MANAGER

#include "qdrawutil.h"
#include "qpopupmenu.h"
#include "qpainter.h"
#include "qregion.h"



/*!
  \class QWSDecoration qwsdecoration_qws.h
  \brief The QWSDecoration class allows the appearance of the Qt/Embedded Window
  Manager to be customized.

  Qt/Embedded provides window management to top level windows.  The
  appearance of the borders and buttons (the decoration) around the
  managed windows can be customized by creating your own class derived
  from QWSDecoration and overriding a few methods.

  This class is non-portable.  It is available \e only in Qt/Embedded.

  \sa QApplication::qwsSetDecoration()
*/

/*!
  \fn QWSDecoration::QWSDecoration()

  Constructs a decorator.
*/

/*!
  \fn QWSDecoration::~QWSDecoration()

  Destructs a decorator.
*/

/*!
  \enum QWSDecoration::Region

  This enum describes the regions in the window decorations.

  <ul>
  <li> \c None - used internally.
  <li> \c All - the entire region used by the window decoration.
  <li> \c Title - Displays the window title and allows the window to be
	  moved by dragging.
  <li> \c Top - allows the top of the window to be resized.
  <li> \c Bottom - allows the bottom of the window to be resized.
  <li> \c Left - allows the left edge of the window to be resized.
  <li> \c Right - allows the right edge of the window to be resized.
  <li> \c TopLeft - allows the top-left of the window to be resized.
  <li> \c TopRight - allows the top-right of the window to be resized.
  <li> \c BottomLeft - allows the bottom-left of the window to be resized.
  <li> \c BottomRight - allows the bottom-right of the window to be resized.
  <li> \c Close - clicking in this region closes the window.
  <li> \c Minimize - clicking in this region minimizes the window.
  <li> \c Maximize - clicking in this region maximizes the window.
  <li> \c Normalize - returns a maximized window to previous size.
  <li> \c Menu - clicking in this region opens the window operations menu.
  </ul>
*/

/*!
  \fn QRegion QWSDecoration::region( const QWidget *widget, const QRect &rect, Region type )

  Returns the requested region \a type which will contain \a widget
  with geometry \a rect.
*/

/*!
  Called when the user clicks in the \c Close region.

  \a widget is the QWidget to be closed.

  The default behaviour is to close the widget.
*/
void QWSDecoration::close( QWidget *widget )
{
    widget->close(FALSE);
}


#include <qdialog.h>

/*

#include <qbitmap.h>

class MinimisedWindow : public QWidget
{
public:
    MinimisedWindow( QWidget *restore ) : 
	QWidget( (QWidget *)restore->parent(), restore->caption(), WStyle_Customize | WStyle_NoBorder ),
	w(restore)
    {
	w->hide();
	QPixmap p( "../pics/tux.png" );
	setBackgroundPixmap( p );
	setFixedSize( p.size() );
	setMask( p.createHeuristicMask() );
	show();
    }
 
    void mouseDoubleClickEvent( QMouseEvent * ) { w->show(); delete this; }
    void mousePressEvent( QMouseEvent *e ) { clickPos = e->pos(); }
    void mouseMoveEvent( QMouseEvent *e ) { move( e->globalPos() - clickPos ); }

    QWidget *w;
    QPoint clickPos;
};

*/


/*!
  Called when the user clicks in the \c Minimize region.

  \a widget is the QWidget to be minimized.

  The default behaviour is to ignore this action.
*/
void QWSDecoration::minimize( QWidget * )
{
//      new MinimisedWindow( w );
    
    //    qDebug("No minimize functionality provided");
}


/*!
  Called when the user clicks in the \c Maximize region.

  \a widget is the QWidget to be maximized.

  The default behaviour is to resize the widget to be full-screen.
  This method can be overridden to, e.g. avoid launch panels.
*/
void QWSDecoration::maximize( QWidget *widget )
{
    QRect nr;

    // find out how much space the decoration needs
    extern Q_EXPORT QRect qt_maxWindowRect;
    QRect desk = qt_maxWindowRect;

/*
#ifdef QPE_WM_LOOK_AND_FEEL
    if (wmStyle == QtEmbedded_WMStyle) {
        QRect dummy( 0, 0, desk.width(), 1 );
	QRegion r = region(widget, dummy, Title);
        QRect rect = r.boundingRect();
        nr = QRect(desk.x(), desk.y()-rect.y(),
            desk.width(), desk.height() - rect.height());
    } else
#endif
*/
    {
        QRect dummy( 0, 0, 1, 1);
        QRegion r = region(widget, dummy);
	if (r.isEmpty()) {
	    nr = desk;
	} else {
	    QRect rect = r.boundingRect();
	    nr = QRect(desk.x()-rect.x(), desk.y()-rect.y(),
		    desk.width() - (rect.width()==1 ? 0 : rect.width()), // ==1 -> dummy
		    desk.height() - (rect.height()==1 ? 0 : rect.height()));
	}
    }
    widget->setGeometry(nr);
}

/*!
  Called to create a QPopupMenu containing the valid menu operations.

  The default implementation adds all possible window operations.
*/

#ifndef QT_NO_POPUPMENU
QPopupMenu *QWSDecoration::menu(const QWidget *, const QPoint &)
{
    QPopupMenu *m = new QPopupMenu();

    m->insertItem(QObject::tr("&Restore"), (int)Normalize);
    m->insertItem(QObject::tr("&Move"), (int)Title);
    m->insertItem(QObject::tr("&Size"), (int)BottomRight);
    m->insertItem(QObject::tr("Mi&nimize"), (int)Minimize);
    m->insertItem(QObject::tr("Ma&ximize"), (int)Maximize);
    m->insertSeparator();
    m->insertItem(QObject::tr("Close"), (int)Close);

    return m;
}
#endif

/*!
  \fn void QWSDecoration::paint( QPainter *painter, const QWidget *widget )

  Override to paint the border and title decoration around \a widget using
  \a painter.

*/

/*!
  \fn void QWSDecoration::paintButton( QPainter *painter, const QWidget *widget, Region type, int state )

  Override to paint a button \a type using \a painter.

  \a widget is the widget whose button is to be drawn.
  \a state is the state of the button.  It can be a combination of the
  following ORed together:
  <ul>
  <li> \c QWSButton::MouseOver
  <li> \c QWSButton::Clicked
  <li> \c QWSButton::On
  </ul>
*/


#endif
