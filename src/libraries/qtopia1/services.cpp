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

#include <qtopia/applnk.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/mimetype.h>

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
  \class Service qtopia1/services.h

  \brief The Service class allows applications to provide services for other
  applications to use.

  \ingroup qtopiaemb

  A Service is a named collection of features that an application
  may choose to provide. For example, web browsers might each provide
  the feature of displaying a web page given a URL.

  Services allow the providers of a service (eg. a web browser) from the
  consumers of the service (eg. a mail reader that wanted the user to
  view a URL).

  \sa ServiceRequest
*/

/*!
  \class ServiceRequest qtopia1/services.h

  \brief The ServiceRequest class allows applications to request services from
  other applications.

  \ingroup qtopiaemb

  A ServiceRequest encapsulates a Service and the message to be sent to
  that service. It is similar to a QCopEnvelope, but uses service names
  rather than direct application names.

  Since ServiceRequest inherits QDataStream, you can write data to the
  request before sending it with send().

  \sa Service
*/


/*!
  Returns a list of available services.
*/
QStringList Service::list()
{
    return serviceList_p(QPEApplication::qpeDir()+"services", ""); // No tr
}

/*!
  Returns the name of the Config file defining the bindings which the
  user has selected for the \a service.
*/
QString Service::binding(const QString& service)
{
    QString svrc = service;
    for (int i=0; i<(int)svrc.length(); i++)
        if ( svrc[i]=='/' ) svrc[i] = '-';
    return "Service-"+svrc; // No tr
}


/*!
  Returns the name of the Config file defining the facilities that are provided by
  all applications that provide the \a service, or
  a null string if there is no such service.

  Applications providing the service may have additional configuration
  defined in the appConfig()) file.
*/
QString Service::config(const QString& service)
{
    return QPEApplication::qpeDir()+"services/"+service+".service";
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
  searched for a service provider. The search of application executables is solely
  for backward compatibility.
*/
QString Service::appConfig(const QString& service, const QString& appname, bool automatics)
{
    QString a = app(service,appname,automatics);
    if ( a.isNull() )
	return a;
    return QPEApplication::qpeDir()+"services/"+service+"/"+a;
}


/*!
  Returns all applications that offer the \a service, or
  a null string if there is no such service offered.

  If \a automatics is TRUE (the default),
  the MimeType database and application executables are also
  searched for service providers. The search of application executables is solely
  for backward compatibility.
*/
QStringList Service::apps(const QString& service, bool automatics)
{
    QStringList all;

    QDir dir(QPEApplication::qpeDir()+"services/"+service, QString::null,
      QDir::Unsorted, QDir::Files );
    all = dir.entryList();
    if ( automatics ) {
	QString svmaj = service.left(5);
	if ( svmaj == "View/" || svmaj == "Open/" ) {
	    // Automatic MIME-type specification services
	    MimeType m(service.mid(5));
	    /* Sorry, not available in 1.5.0
	    QList<AppLnk> l = m.applications();
	    for ( QListIterator<AppLnk> i(l); i.current(); ++i) {
		const AppLnk* a = *i;
	    */{
		const AppLnk* a = m.application();
		if ( a )
		    all.append(a->exec());
	    }
	} else if ( QFile::exists(QPEApplication::qpeDir()+"bin/"+service ) ) {
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
  searched for a service provider. The search of application executables is solely
  for backward compatibility.
*/
QString Service::app(const QString& service, const QString& appname, bool automatics)
{
    Config cfg(binding(service));
    QString r;
    if ( cfg.isValid() ) {
	cfg.setGroup("Service");
	r = cfg.readEntry(appname);
	if ( r.isNull() )
	    r = cfg.readEntry("default");
    }
    if ( r.isNull() ) {
	// None defined, use any...
	QDir dir(QPEApplication::qpeDir()+"services/"+service, QString::null,
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
	    } else if ( QFile::exists(QPEApplication::qpeDir()+"bin/"+service ) ) {
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
  searched for a service provider. The search of application executables is solely
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
  searched for a service provider. The search of application executables is solely
  for backward compatibility.
*/
QCString Service::channel(const QString& service, const QString& appname, bool automatics)
{
    QCString r = app(service,appname,automatics).latin1();
    return r.isNull() ? r : "QPE/Application/"+r;
}

/*!
  Returns an AppLnk to an application providing the \a service.
  If \a appname is provided, a specific binding to that application may
  be used rather than the global binding.
  
  If \a automatics is TRUE (the default),
  the MimeType database and application executables are also
  searched for service providers. The search of application executables is solely
  for backward compatibility.
*/
AppLnk Service::appLnk( const QString& service, const QString& appname, bool automatics)
{
    QString r = app(service,appname,automatics);
    if ( r.isEmpty() )
	return AppLnk();
    AppLnkSet s(QPEApplication::qpeDir()+"apps");
    return *s.findExec(r);
}


#include <qtopia/qcopenvelope_qws.h>

/*!
  Construct a null service request. You will need to call
  setService() and setMessage() before send(), but you
  may write to the service beforehand.
*/
ServiceRequest::ServiceRequest() : QDataStream(new QBuffer())
{
    device()->open(IO_WriteOnly);
}

/*!
  Construct a service request that will send \a message to
  a \a service when you call send(). You
  may write to the service beforehand.
*/
ServiceRequest::ServiceRequest(const QString& service, const QCString& message) 
  : QDataStream(new QBuffer()), m_Service(service), m_Message(message)
{
    device()->open(IO_WriteOnly);
}

/*!
  Copy constructor. Any data previously written to the \a orig
  service will be in the copy.
*/
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

/*!
  Returns TRUE if either the service or message is not set.
*/
bool ServiceRequest::isNull() const
{
    return m_Service.isNull() || m_Message.isNull();
}

/*!
  Assignment operator.
  Any data previously written to the \a orig
  service will be in the copy.
*/
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

/*!
  Returns the current stored data. Before you modify the returned value,
  you must call QArray::detach().
*/
const QByteArray ServiceRequest::data() const
{
    return ((QBuffer*)device())->buffer();
}

/*!
  Destructs the service request. Unlike QCopEnvelope, the
  request is NOT automatically sent.
*/
ServiceRequest::~ServiceRequest()
{
    // If we still have our QBuffer, clean it up...
    if (device())
        delete device();
    unsetDevice();
}

/*!
  Sends the request. Returns FALSE if the request was null.
*/
bool ServiceRequest::send() const
{
    if ( isNull() )
	return FALSE;

#ifndef QT_NO_COP
    QCString ch = channel();
    if ( !ch.isNull() ) {
	QCopEnvelope e(ch, message());

	QByteArray array(data());
	array.detach();
  
	delete e.device();
	e.setDevice(new QBuffer(array));
	return TRUE;
    } else
#endif
	return FALSE;
}
 
/*!
  Sets the service to which the request will be sent.
*/
void ServiceRequest::setService(const QString& service)
{
    m_Service = service;
}

/*!
  Sets the message to be sent to the service.
*/
void ServiceRequest::setMessage(const QCString& message)
{
    m_Message = message;
}

/*!
  See Service::channels().
*/
QStrList ServiceRequest::channels(bool automatics) const
{
    return Service::channels(m_Service, automatics);
}

/*!
  See Service::channel().
*/
QCString ServiceRequest::channel(const QString& appname, bool automatics) const
{
    return Service::channel(m_Service, appname, automatics);
}

/*!
  See Service::apps().
*/
QStringList ServiceRequest::apps(bool automatics) const
{
    return Service::apps(m_Service, automatics);
}

/*!
  See Service::app().
*/
QString ServiceRequest::app(const QString& appname, bool automatics) const
{
    return Service::app(m_Service, appname, automatics);
}
