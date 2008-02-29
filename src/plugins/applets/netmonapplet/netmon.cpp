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

#include "netmon.h"
#include "netpw.h"

#include <qpe/resource.h>
#include <qpe/network.h>
#ifdef QWS
#include <qpe/qcopenvelope_qws.h>
#endif

#include <qpainter.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qlineedit.h>

#include <stdio.h>


class NetMonitor : public QVBox {
    Q_OBJECT
public:
    NetMonitor(QWidget* parent=0) :
	QVBox(parent,0,WStyle_StaysOnTop | WType_Popup),
	waiting(FALSE),wantnet(FALSE),havenet(FALSE),
	status(new QLabel(" ",this)),
	services(new QComboBox(this)),
	toggle(new QPushButton(this))
    {
	setFrameStyle(Raised+Panel);
	setMargin(4);
	setSpacing(4);
	status->setAlignment(AlignCenter);
	status->setFixedHeight(toggle->sizeHint().height());
	connect(toggle,SIGNAL(clicked()),this,SLOT(toggleNetwork()));
	setStatus("");
	updateToggle();
    }

    void setHaveNet(bool y)
    {
	if ( havenet != y ) {
	    havenet = y;
	    if ( havenet == wantnet )
		waiting = FALSE;
	    else if ( !waiting )
		wantnet = havenet;
	    if ( !y )
		setStatus("");
	    updateToggle();
	}
    }

    void setStatus(const QString& s)
    {
	if ( s.isEmpty() )
	    status->setText(tr("Network Offline"));
	else
	    status->setText(s);
    }

    void setServices(const QStringList& s)
    {
	service_files = s;
	QString cur = *service_files.at(services->currentItem());
	services->clear();
	int ncur = 0;
	for (QStringList::ConstIterator it=s.begin(); it!=s.end(); ++it) {
	    services->insertItem(Network::serviceName(*it));
	    if ( *it == cur )
		ncur = services->count()-1;
	}
	services->setCurrentItem(ncur);
    }

    void netFailed()
    {
	wantnet = havenet;
	waiting = FALSE;
	updateToggle();
    }


private slots:
    void toggleNetwork()
    {
	if ( !wantnet ) {
	    QString file = *service_files.at(services->currentItem());
	    QString pw;
	    if ( Network::serviceNeedsPassword(file) ) {
		pw = getNetPassword(Network::serviceName(file));
		if ( pw.isNull() )
		    return;
	    }
	    Network::start(file,pw);
	} else {
	    Network::stop();
	}
	wantnet = !wantnet;
	waiting = TRUE;
	updateToggle();
    }

private:
    QString getNetPassword(const QString& sname)
    {
	NetPassword dlg(0,0,TRUE);
	dlg.prompt->setText(dlg.prompt->text().arg(sname));
	if ( dlg.exec() ) {
	    show(); // we would have popped down when the dialog came up [DOESN'T HELP!]
	    return dlg.pw->text();
	} else {
	    return QString::null;
	}
    }

    void updateToggle()
    {
	toggle->setText(havenet ? tr("Disconnect") : tr("Connect"));
	services->setEnabled(!havenet && !waiting);
	toggle->setEnabled(!waiting);
    }

    bool waiting;
    bool wantnet;
    bool havenet;
    QLabel* status;
    QStringList service_files;
    QComboBox* services;
    QPushButton* toggle;
};

#include "netmon.moc"

NetMonitorApplet::NetMonitorApplet( QWidget *parent ) : QWidget( parent )
{
#ifndef QT_NO_COP
    QCopChannel* netChannel = new QCopChannel( "QPE/Network", this );
    connect( netChannel, SIGNAL(received(const QCString &, const QByteArray &)),
             this, SLOT(netMessage( const QCString &, const QByteArray &)) );
#endif

    state = Unavailable;
    netmon = 0;
    demand = FALSE;

    up = Resource::loadPixmap( "netmon/up" );
    down = Resource::loadPixmap( "netmon/down" );
    
    setFixedHeight( 18 );
    setFixedWidth( 14 );
    hide();
}

NetMonitorApplet::~NetMonitorApplet()
{
    delete netmon;
}

void NetMonitorApplet::netMessage( const QCString &msg, const QByteArray &d )
{
    QDataStream stream(d,IO_ReadOnly);
    if ( msg == "up()" ) {
	//qDebug( "QCOP: up()" );
	setNetState(Running);
    } else if ( msg == "down()" ) {
	//qDebug( "QCOP: down()" );
	setNetState(Unknown);
    } else if ( msg == "needed()" ) {
	if ( state != Running ) {
	    ensureMonitor();
	    if ( services.count() ) {
		if ( services.count() == 1 ) {
		    Network::start();
		} else {
		    chooseNet();
		}
	    } else {
		Network::start();
	    }
	}
    } else if ( msg == "available(QStringList)" ) {
	stream >> services;
	//qDebug("QCOP: available, count=%d", services.count() );
	if ( services.count() ) {
	    if ( state == Unavailable )
		setNetState(Available);
	    show();
	    ensureMonitor();
	    netmon->setServices(services);
	} else {
	    setNetState(Unavailable);
	    if ( netmon )
		netmon->setServices(services);
	}
    } else if ( msg == "progress(QString,int)" ) {
	QString p;
	int flag;
	stream >> p >> flag;
	//qDebug("QCOP: progress(%s, %d)", p.latin1(), flag );
	if ( netmon ) {
	    netmon->setStatus(p);
	    if (!(flag&1))
		netmon->netFailed();
	}
	demand = flag&2;
	demand_active = flag&4;
	if ( demand )
	    update();
    } else if ( msg == "failed()" ) {
	//qDebug( "QCOP: failed()" );
	if ( netmon ) {
	    netmon->setStatus("");
	    netmon->netFailed();
	}
    } else if ( msg == "choicesChanged()" ) {
	//qDebug( "QCOP: choicesChanged()" );
	if ( netmon )
	    netmon->setServices(services);
    }
}

void NetMonitorApplet::setNetState(NetState s)
{
    //qDebug("setNetState( %d )", s );
    if ( state != s ) {
	state = s;
	if ( state == Unavailable || services.count() == 0 ) {
	    hide();
	} else {
	    update();
	    show();
	}
	if ( netmon ) {
	    if ( state == Unavailable )
		netmon->hide();
	    netmon->setHaveNet(state == Running);
	    repaint();
	}
    }
}

void NetMonitorApplet::ensureMonitor()
{
    if ( !netmon ) {
	netmon = new NetMonitor;
	netmon->setHaveNet(state == Running);
	netmon->setServices(services);
    }
}

void NetMonitorApplet::mousePressEvent( QMouseEvent * )
{
    chooseNet();
}

void NetMonitorApplet::chooseNet()
{
    ensureMonitor();
    QSize sh = netmon->sizeHint();
    netmon->resize(sh);
    netmon->move(mapToGlobal(QPoint((width()-sh.width())/2,-sh.height())));
    netmon->show();
}


void NetMonitorApplet::paintEvent( QPaintEvent* )
{
    QPainter p(this);
    int x=(width()-up.width())/2;
    int y=(height()-up.height())/2;
    if ( !demand && state == Running || demand && demand_active ) {
	if ( services.count() )
	    p.drawPixmap(x,y,up);
    } else if ( state == Available || state == Unknown || demand && !demand_active ) {
	p.drawPixmap(x,y,down);
    }
}


