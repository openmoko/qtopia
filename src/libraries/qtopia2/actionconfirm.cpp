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

#include <qframe.h>
#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qpainter.h>

#include "actionconfirm.h"

class ActionConfirmDialog : public QFrame 
{
public:
    ActionConfirmDialog( QWidget *parent = 0, const char *name = 0 );

    void show( const QPixmap &pix, const QString &text );
    void hide();

protected:
    void timerEvent( QTimerEvent *e );
    void paintEvent( QPaintEvent *e );

private:
    QLabel *m_pixLabel, *m_textLabel;
    int m_timerId;
};

int mgn = 2;
ActionConfirmDialog::ActionConfirmDialog( QWidget *parent, const char *name )
    : QFrame( parent, name, WStyle_Customize | WStyle_Tool | WType_TopLevel ),
								m_timerId( 0 )
{
    QHBoxLayout *l = new QHBoxLayout( this );
    l->setAutoAdd( TRUE );
    l->setMargin( mgn );
    m_pixLabel = new QLabel( this );
    m_pixLabel->setAlignment( Qt::AlignCenter  );
    m_textLabel = new QLabel( this );
    m_textLabel->setAlignment( Qt::AlignCenter | Qt::WordBreak );
    /*
    QFont f = m_textLabel->font();
    f.setBold( TRUE );
    m_textLabel->setFont( f );
    */
}

void ActionConfirmDialog::show( const QPixmap &pix, const QString &text )
{
    if( m_timerId != 0 )
    {
	killTimer( m_timerId );
	m_timerId = 0;
    }
    m_pixLabel->setPixmap( pix );
    m_textLabel->setText( text );


    int w = m_pixLabel->sizeHint().width() + m_textLabel->sizeHint().width()
	    + (mgn*2),
	h = QMAX(m_pixLabel->sizeHint().height(), 
				    m_textLabel->heightForWidth(w)) + (mgn*2),
	dw = qApp->desktop()->width(),
	dh = qApp->desktop()->height(),
	x = dw / 2 - w / 2,
	y = dh / 2 - h /2;

    setGeometry( x, y, w, h );
    m_timerId = startTimer( 1800 );
    QWidget::show();
}

void ActionConfirmDialog::hide()
{
    killTimer( m_timerId );
    m_timerId = 0;
    QWidget::hide();
}

void ActionConfirmDialog::timerEvent( QTimerEvent *e )
{
    if( m_timerId && e->timerId() == m_timerId )
	hide();
}

void ActionConfirmDialog::paintEvent( QPaintEvent *e )
{
    QFrame::paintEvent( e );
    QPainter p( this );
    p.setPen( colorGroup().text() );
    p.drawRect( 0, 0, width(), height() );
}

ActionConfirmDialog *ActionConfirm::m_dialog = 0;
void ActionConfirm::display( const QPixmap &pix, const QString &text )
{
    if( !m_dialog )
	m_dialog = new ActionConfirmDialog;
    m_dialog->show( pix, text );
}
