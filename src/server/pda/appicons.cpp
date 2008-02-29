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


#include "appicons.h"

#include <qtopiaipcenvelope.h>

#include <qpixmap.h>
#include <qevent.h>
#include <qlayout.h>


AppIcons::AppIcons( QWidget *parent ) :
    QWidget(parent)
{
    hbLayout = new QHBoxLayout(this);
    hbLayout->setMargin(0);
    hbLayout->setSpacing(0);
    QtopiaChannel* channel = new QtopiaChannel("Tray", this);
    connect(channel, SIGNAL(received(const QString&,const QByteArray&)),
            this, SLOT(receive(const QString&,const QByteArray&)));
}

AppIcons::~AppIcons()
{
    qDeleteAll(buttons);
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
        emit clicked(e->pos(),e->button(),true);
    }
    void mouseReleaseEvent(QMouseEvent* e)
    {
        if ( rect().contains(e->pos()) )
            emit clicked(e->pos(),e->button(),false);
    }

signals:
    void clicked(const QPoint&, int, bool);
};

QLabel* AppIcons::button(int id)
{
    QLabel* f = 0;
    QMap<int,QLabel*>::const_iterator it = buttons.find(id);
    if ( it == buttons.end() ) {
        buttons.insert(id,f=new FlatButton(this));
        hbLayout->addWidget(f);
        connect(f,SIGNAL(clicked(const QPoint&,int,bool)),this,SLOT(clicked(const QPoint&,int,bool)));
        f->show();
    } else {
        f = *it;
    }
    return f;
}

int AppIcons::findId(QLabel* b)
{
    QMap<int,QLabel*>::const_iterator it;
    for (it = buttons.begin(); it != buttons.end(); ++it) {
        if (*it == b)
            return it.key();
    }

    return -1;
}

void AppIcons::clicked(const QPoint& relpos, int button, bool dbl)
{
    QLabel* s = (QLabel*)sender();
    if ( button == Qt::RightButton ) {
        QtopiaIpcEnvelope("Tray","popup(int,QPoint)")
            << findId(s) << s->mapToGlobal(QPoint(0,0));
    } else {
        QtopiaIpcEnvelope("Tray",
                dbl ? "doubleClicked(int,QPoint)" : "clicked(int,QPoint)")
            << findId(s) << relpos;
    }
}

void AppIcons::remove(int id)
{
    buttons.remove(id);
}

void AppIcons::receive( const QString &msg, const QByteArray &data )
{
    QDataStream stream( data );
    if ( msg == "remove(int)" ) {
        int id;
        stream >> id;
        remove(id);
    } else if ( msg == "setIcon(int,QPixmap)" ) {
        int id;
        QPixmap pm;
        stream >> id >> pm;
        setIcon(id,pm);
    }
}

#include "appicons.moc"
