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

#define QTOPIA_INTERNAL_MIMEEXT

#include "services.h"

#include <qdir.h>
#include <qfile.h>

#include <qpe/applnk.h>
#include <qpe/config.h>
#include <qpe/qpeapplication.h>
#include <qpe/mimetype.h>

static QStringList serviceList_p(const QString& d, const QString n)
{
    QStringList r;
    QDir dir(d, "???*", QDir::Unsorted, QDir::Dirs );
    QStringList dl=dir.entryList();
    for (QStringList::ConstIterator it=dl.begin(); it!=dl.end(); ++it) {
	r.append(n+*it);
	r += serviceList_p(d+"/"+*it,n+*it+"/");
    }
    return r;
}


/*!
  Returns a list of available services.
*/
QStringList Service::list()
{
    return serviceList_p(QPEApplication::qpeDir()+"/services", "");
}

/*!
  Returns the Config file defining the bindings which the user has
  selected for the \a service.
*/
QString Service::binding(const QString& service)
{
    QString svrc = service;
    for (int i=0; i<(int)svrc.length(); i++)
        if ( svrc[i]=='/' ) svrc[i] = '-';
    return "Service-"+svrc;
}


/*!
  Returns the Config file defining the facilities that are provided by
  all applications that provide the \a service, or
  a null string if there is no such service.

  Applications providing the service may have additional configuration
  defined in the appConfig()) file.
*/
QString Service::config(const QString& service)
{
    return QPEApplication::qpeDir()+"/services/"+service+".service";
}


/*!
  Returns the Config file defining the facilities provided by application that
  provides the \a service, or
  a null string if there is no such service.

  The Config can be used to extract more detailed application-specific
  information about the application providing the service.

  Note that the application always provides at least the services defined
  in the config() file - the information in the appConfig()
  is application-specific.

  If \a appname is provided, a specific binding to that application may
  be used rather than the global binding.

  If \a automatics is TRUE (the default), and the service is not otherwise
  defined, the MimeType database and application executables are also
  searched for a service provider. The application executables is solely
  for backward compatibility.
*/
QString Service::appConfig(const QString& service, const QString& appname, bool automatics)
{
    QString a = app(service,appname,automatics);
    if ( a.isNull() )
	return a;
    return QPEApplication::qpeDir()+"/services/"+service+"/"+a;
}


/*!
  Returns all applications that offer the \a service, or
  a null string if there is no such service offered.

  If \a automatics is TRUE (the default),
  the MimeType database and application executables are also
  searched for service providers. The application executables is solely
  for backward compatibility.
*/
QStringList Service::apps(const QString& service, bool automatics)
{
    QStringList all;

    QDir dir(QPEApplication::qpeDir()+"/services/"+service, QString::null,
      QDir::Unsorted, QDir::Files );
    all = dir.entryList();
    if ( automatics ) {
	QString svmaj = service.left(5);
	if ( svmaj == "View/" || svmaj == "Open/" ) {
	    // Automatic MIME-type specification services
	    MimeType m(service.mid(5));
	    QList<AppLnk> l = m.applications();
	    for ( QListIterator<AppLnk> i(l); i.current(); ++i) {
		const AppLnk* a = *i;
		if ( a )
		    all.append(a->exec());
	    }
	} else if ( QFile::exists(QPEApplication::qpeDir()+"/bin/"+service ) ) {
	    // Automatic application service (LAST RESORT!)
	    all.append(service);
	}
    }

    return all;
}

/*!
  Returns the application that provides the \a service, or
  a null string if there is no such service.

  If \a appname is provided, a specific binding to that application may
  be used rather than the global binding.

  If \a automatics is TRUE (the default), and the service is not otherwise
  defined, the MimeType database and application executables are also
  searched for a service provider. The application executables is solely
  for backward compatibility.
*/
QString Service::app(const QString& service, const QString& appname, bool automatics)
{
    Config cfg(binding(service));
    cfg.setGroup("Service");
    QString r = cfg.readEntry(appname);
    if ( r.isNull() )
	r = cfg.readEntry("default");
    if ( r.isNull() ) {
	// None defined, use any...
	QDir dir(QPEApplication::qpeDir()+"/services/"+service, QString::null,
          QDir::Unsorted, QDir::Files );
	r = dir[0];
	if ( automatics && r.isNull() ) {
	    QString svmaj = service.left(5);
	    if ( svmaj == "View/" || svmaj == "Open/" ) {
		// Automatic MIME-type specification services
		MimeType m(service.mid(5));
		const AppLnk* a = m.application();
		if ( a )
		    r = a->exec();
	    } else if ( QFile::exists(QPEApplication::qpeDir()+"/bin/"+service ) ) {
		// Automatic application service (LAST RESORT!)
		r = service;
	    }
	}
    }
    return r;
}


/*!
  Returns the QCop channels for all applications that offer
  \a service.

  If \a automatics is TRUE (the default), and the service is not otherwise
  defined, the MimeType database and application executables are also
  searched for a service provider. The application executables is solely
  for backward compatibility.
*/
QStrList Service::channels(const QString& service, bool automatics)
{
    QStrList r;
    QStringList rs = apps(service,automatics);
    for (QStringList::ConstIterator it = rs.begin(); it!=rs.end(); ++it)
	r.append( QCString("QPE/Application/") + (*it).latin1() );
    return r;
}

/*!
  Returns the QCop channel for the given \a service, or
  a null string if there is no such service.

  If \a appname is provided, a specific binding to that application may
  be used rather than the global binding.

  If \a automatics is TRUE (the default), and the service is not otherwise
  defined, the MimeType database and application executables are also
  searched for a service provider. The application executables is solely
  for backward compatibility.
*/
QCString Service::channel(const QString& service, const QString& appname, bool automatics)
{
    QCString r = app(service,appname,automatics).latin1();
    return r.isNull() ? r : "QPE/Application/"+r;
}

AppLnk Service::appLnk( const QString& service, const QString& appname, bool automatics)
{
    QString r = app(service,appname,automatics);
    if ( r.isEmpty() )
	return AppLnk();
    AppLnkSet s(QPEApplication::qpeDir()+"/apps");
    return *s.findExec(r);
}


#include <qtopia/qcopenvelope_qws.h>

ServiceRequest::ServiceRequest() : QDataStream(new QBuffer())
{
    device()->open(IO_WriteOnly);
}

ServiceRequest::ServiceRequest(const QString& service, const QCString& message) 
  : QDataStream(new QBuffer()), m_Service(service), m_Message(message)
{
    device()->open(IO_WriteOnly);
}

ServiceRequest::ServiceRequest(const ServiceRequest& orig) : QDataStream()
{
    // The QBuffer is going to share the byte array, so it will keep the
    // data pointer even when this one goes out of scope.
    QByteArray array(((QBuffer*)orig.device())->buffer());
    array.detach();
    setDevice(new QBuffer(array));
    device()->open(IO_WriteOnly|IO_Append);

    m_Service = orig.service();
    m_Message = orig.message();
}

bool ServiceRequest::isNull() const
{
    return m_Service.isNull() || m_Message.isNull();
}

ServiceRequest& ServiceRequest::operator=(const ServiceRequest& orig)
{
    if (device()) {
        delete device();
        unsetDevice();
    }

    // The QBuffer is going to share the byte array, so it will keep the
    // data pointer even when this one goes out of scope.
    QByteArray array(((QBuffer*)orig.device())->buffer());
    array.detach();
    setDevice(new QBuffer(array));
    device()->open(IO_WriteOnly|IO_Append);

    m_Service = orig.service();
    m_Message = orig.message();  

    return *this;
}

const QByteArray ServiceRequest::data() const
{
    return ((QBuffer*)device())->buffer();
}

ServiceRequest::~ServiceRequest()
{
    // If we still have our QBuffer, clean it up...
    if (device())
        delete device();
    unsetDevice();
}

bool ServiceRequest::send() const
{
    QCString ch = channel();
    if ( !ch.isNull() ) {
	QCopEnvelope e(ch, message());

	QByteArray array(data());
	array.detach();
  
	delete e.device();
	e.setDevice(new QBuffer(array));
	return TRUE;
    } else
	return FALSE;
}
 
void ServiceRequest::setService(const QString& service)
{
    m_Service = service;
}

void ServiceRequest::setMessage(const QCString& message)
{
    m_Message = message;
}

// convenience function
QStrList ServiceRequest::channels(bool automatics) const
{
    return Service::channels(m_Service, automatics);
}

// convenience function
QCString ServiceRequest::channel(const QString& appname, bool automatics) const
{
    return Service::channel(m_Service, appname, automatics);
}

// convenience function
QStringList ServiceRequest::apps(bool automatics) const
{
    return Service::apps(m_Service, automatics);
}

// convenience function
QString ServiceRequest::app(const QString& appname, bool automatics) const
{
    return Service::app(m_Service, appname, automatics);
}

QDataStream& operator<<( QDataStream& s, const ServiceRequest &r )
{
    return s << r.service() << r.message();
}

QDataStream& operator>>( QDataStream& s, ServiceRequest &r )
{
    QString sr; QCString m;
    s >> sr >> m;
    r.setService(sr);
    r.setMessage(m);
    return s;
}

