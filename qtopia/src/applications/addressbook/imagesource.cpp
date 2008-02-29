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


#include "imagesource.h"

#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/services.h>
#include <qtopia/global.h>
#include <qtopia/mimetype.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/resource.h>
#include <qtopia/qcopenvelope_qws.h>

#include <qimage.h>
#include <qsignalmapper.h>
#include <qguardedptr.h>

class ValueServiceChannel : public QCopChannel {
    Q_OBJECT

public:
    ValueServiceChannel(QObject* parent) :
	QCopChannel("QPE/QDS",parent)
    {
	//allapps=new AppLnkSet(MimeType::appsFolderName());
    }

    //AppLnkSet *allapps;

signals:
    void valueSupplied(const QString&,QDataStream&);

private:
    void receive(const QCString& msg,const QByteArray& data)
    {
	if ( msg.left(22) == "valueSupplied(QString," ) {
	    QDataStream ds(data,IO_ReadOnly);
	    QString id;
	    ds >> id;
	    emit valueSupplied(id,ds);
	}
    }
};

class ValueServiceData {
public:
    ValueServiceData(ValueService::Type st, const char* mimetype, const char* params, const char* returns) 
	: rettypes( returns ), args(params)
    {
	/*
	get all the servers for this service
	a server is an action of an application, not just the application itself
	*/
	QStringList apps;
	const QString serviceName = ServiceNames[(uint)st]+QCString("/")+mimetype+QCString("/");
	apps = Service::apps( serviceName );
	for( QStringList::Iterator it = apps.begin() ; it != apps.end() ; ++it )
	{
	    QString config = 
	    Service::appConfig( serviceName, *it );
	    Config cfg( config, Config::File );
	    cfg.setGroup("Standard");
	    QStringList actions = QStringList::split( ";", 
				cfg.readEntry("Actions").stripWhiteSpace() );
	    for( QStringList::Iterator ait = actions.begin() ; 
						ait != actions.end() ; ++ait )
	    {
		cfg.setGroup( *ait );
               
		QCString actionName = (*ait).left( (*ait).find( '(' ) ).latin1();
		ValueServiceServer src( cfg.readEntry("Name").stripWhiteSpace(), 
                    QCString("QPE/Application/")+(*it).latin1(), actionName,
                    Resource::loadIconSet( cfg.readEntry("Icon").stripWhiteSpace() )
                    .pixmap( QIconSet::Small, TRUE ), (*ait).contains(',') + 1 );
                qDebug("*m* ValueService->paramCount = %d", src.paramCount());
                servers += src;
	    }
	}
	id = Global::generateUuid().toString();
	sm = 0;
    }

    void changeArgs( const char* params )
    {
        args = params;
    }

    QCString requestMessage( int i )
    {
	if( i > (int)servers.count() || i < 0 )
	{
	    return 0;
	}
	return servers[i].actionName()+QCString("(QCString,QString,")+args+")";
    }

    QString rettypes;
    QValueList<ValueServiceServer> servers;
    QString id;
    QSignalMapper *sm;
    static QGuardedPtr<ValueServiceChannel> *channel;
private:
    QCString args;
    static const char *ServiceNames[];
};

//service names corresponding to the values of ValueService::Type
const char *ValueServiceData::ServiceNames[] = { "GetValue", "SetValue" };
QGuardedPtr<ValueServiceChannel> *ValueServiceData::channel = 0;

ValueServiceServer::ValueServiceServer()
{
}

ValueServiceServer::ValueServiceServer( const QString &n, const QCString &c, const QCString &a, const QPixmap &p, const int paramCount )
    : mName( n ), mChannel( c ), mAction( a ), mPix( p ), mParamCount( paramCount )
{
}

bool ValueServiceServer::isNull() const
{
    return (mName.isNull() && mChannel.isNull() && mAction.isNull() && mPix.isNull());
}

ValueServiceServer::ValueServiceServer( const ValueServiceServer &copy )
{
    mName = copy.mName;
    mChannel = copy.mChannel;
    mAction = copy.mAction;
    mPix = copy.mPix;
    mParamCount = copy.mParamCount;
}

ValueServiceServer &ValueServiceServer::operator=( const ValueServiceServer 
									&copy )
{
    mName = copy.mName;
    mChannel = copy.mChannel;
    mAction = copy.mAction;
    mPix = copy.mPix;
    mParamCount = copy.mParamCount;
    return *this;
}

QString ValueServiceServer::name() const
{
    return mName;
}

QCString ValueServiceServer::channel() const
{
    return mChannel;
}

QCString ValueServiceServer::actionName() const
{
    return mAction;
}

QPixmap ValueServiceServer::pixmap() const
{
    return mPix;
}

int ValueServiceServer::paramCount() const
{
    return mParamCount;
}

ValueService::ValueService(QObject* parent, ValueService::Type st, const char* type, const char* params, const char* returns) :
    d(new ValueServiceData(st,type,params,returns))
{
    if ( !d->channel )
	d->channel = new QGuardedPtr<ValueServiceChannel>();

    if ( !(*(d->channel)) )
	*(d->channel) = new ValueServiceChannel(parent);
    connect(*(d->channel), SIGNAL(valueSupplied(const QString&,QDataStream&)),
	this, SLOT(valueSupplied(const QString&,QDataStream&)));
}

ValueService::~ValueService()
{
    delete d;
}

void ValueService::sendParameters( QCopEnvelope &)
{
    qWarning("ValueService::sendParameters(ServiceRequest&) must be reimplemented " );
}

void ValueService::valueSupplied(const QString& id, QDataStream& ds)
{
    if ( id == d->id ) // Could also check rettypes (need to then be given msg)
	valueSupplied(ds);
}

void ValueService::changeArgs( const char* params )
{
    d->changeArgs( params );
}

int ValueService::serverCount() const
{
    return d->servers.count();
}

ValueServiceServer ValueService::server(int serverindex) const
{
    return (serverindex >= 0 && serverindex < serverCount()) ? 
	    d->servers[serverindex] : ValueServiceServer();
}

void ValueService::connectToServer(QObject* sender, const char* signl, int serverindex)
{
    if ( !d->sm ) {
	d->sm = new QSignalMapper(this);
	connect(d->sm, SIGNAL(mapped(int)), this, SLOT(request(int)));
    }
    d->sm->setMapping(sender,serverindex);
    connect(sender, signl, d->sm, SLOT(map()));
}

void ValueService::request(int serverindex)
{
    /*
    don't just send a service request. we've already looked up
    the service information, now send a message directly to
    the server
    */
    QCopEnvelope e( d->servers[serverindex].channel(), d->requestMessage( serverindex ) );
    e << QCString("QPE/QDS") << d->id;
    sendParameters( e );
}


ImageServerManager::ImageServerManager(QObject* parent) :
    ValueService(parent,ValueService::Get,"image","int,int,QImage","QImage"), // No tr
    maxw(-1),maxh(-1)
{
}

ImageServerManager::~ImageServerManager()
{
}

void ImageServerManager::valueSupplied(QDataStream& ds)
{
    QImage img;
    ds >> img;
    emit imageSupplied(img);
    // Optimize case of no connections, no need to make pixmap
    if ( receivers( SIGNAL(pixmapSupplied(const QPixmap&)) ) ) {
	QPixmap pm;
	pm.convertFromImage(img);
	emit pixmapSupplied(pm);
    }
}

void ImageServerManager::setMaximumSize(int w,int h)
{
    maxw = w;
    maxh = h;
}

void ImageServerManager::setDefaultImage( const QImage &img )
{
    defaultImage = img;
}

void ImageServerManager::sendParameters( QCopEnvelope &sreq)
{
    sreq << maxw << maxh << defaultImage;
}

#include "imagesource.moc"
