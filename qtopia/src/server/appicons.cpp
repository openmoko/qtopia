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


#include "appicons.h"

#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qtooltip.h>
#include <qpixmap.h>


AppIcons::AppIcons( QWidget *parent ) :
    QHBox(parent)
{
    buttons.setAutoDelete(TRUE);

#ifndef QT_NO_COP
    QCopChannel* channel = new QCopChannel("Qt/Tray", this);
    connect(channel, SIGNAL(received(const QCString&,const QByteArray&)),
	    this, SLOT(receive(const QCString&,const QByteArray&)));
#endif
}

void AppIcons::setIcon(int id, const QPixmap& pm)
{
    button(id)->setPixmap(pm);
}

class FlatButton : public QLabel {
    Q_OBJECT
public:
    FlatButton(QWidget* parent) : QLabel(parent) { }

    void mouseDoubleClickEvent(QMouseEvent* e)
    {
	emit clicked(e->pos(),e->button(),TRUE);
    }
    void mouseReleaseEvent(QMouseEvent* e)
    {
	if ( rect().contains(e->pos()) )
	    emit clicked(e->pos(),e->button(),FALSE);
    }

signals:
    void clicked(const QPoint&, int, bool);
};

QLabel* AppIcons::button(int id)
{
    QLabel* f = buttons.find(id);
    if ( !f ) {
	buttons.insert(id,f=new FlatButton(this));
	connect(f,SIGNAL(clicked(const QPoint&,int,bool)),this,SLOT(clicked(const QPoint&,int,bool)));
	f->show();
    }
    return f;
}

int AppIcons::findId(QLabel* b)
{
    QIntDictIterator<QLabel> it(buttons);
    for ( ; ; ++it )
	if ( it.current() == b ) return it.currentKey();
}

void AppIcons::clicked(const QPoint& relpos, int button, bool dbl)
{
#ifndef QT_NO_COP
    QLabel* s = (QLabel*)sender();
    if ( button == RightButton ) {
	QCopEnvelope("Qt/Tray","popup(int,QPoint)")
	    << findId(s) << s->mapToGlobal(QPoint(0,0));
    } else {
	QCopEnvelope("Qt/Tray",
		dbl ? "doubleClicked(int,QPoint)" : "clicked(int,QPoint)")
	    << findId(s) << relpos;
    }
#endif
}

void AppIcons::setToolTip(int id, const QString& tip)
{
    QToolTip::add(button(id),tip);
}

void AppIcons::remove(int id)
{
    buttons.remove(id);
}

void AppIcons::receive( const QCString &msg, const QByteArray &data )
{
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "remove(int)" ) {
	int id;
	stream >> id;
	remove(id);
    } else if ( msg == "setIcon(int,QPixmap)" ) {
	int id;
	QPixmap pm;
	stream >> id >> pm;
	setIcon(id,pm);
    } else if ( msg == "setToolTip(int,QString)" ) {
	int id;
	QString s;
	stream >> id >> s;
	setToolTip(id,s);
    }
}

#include "appicons.moc"
