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

#include "qtopiaservices.h"

#include <qdir.h>
#include <qfile.h>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <qsettings.h>

#if defined(QTOPIA_DBUS_IPC)
#include <qtdbus/qdbusmessage.h>
#include <qtdbus/qdbusconnection.h>
#include <qtdbus/qdbusargument.h>
#include <qtdbus/qdbusinterface.h>
#include "dbusipccommon_p.h"
#endif

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
  \class QtopiaService

  \brief The QtopiaService class allows applications to provide services for use
  by other applications.

  \ingroup qtopiaemb

  A QtopiaService is a named collection of features that an application
  may choose to provide. For example, web browsers providing
  the feature of displaying a web page given a URL.

  \sa {Services}, QtopiaServiceRequest
*/

/*!
  \class QtopiaServiceRequest

  \brief The QtopiaServiceRequest class allows applications to request services from
  other applications.

  \ingroup qtopiaemb

  A \c QtopiaServiceRequest encapsulates a \c QtopiaService and the message to be sent to
  that service. It is similar to a \c QtopiaIpcEnvelope, but uses service names
  rather than direct application names.

  Since \c QtopiaServiceRequest inherits \c QDataStream, data may be written to the
  request prior to sending it with \c send().

  \sa {Services}, QtopiaService
*/


/*!
  Returns a list of available services.
*/
QStringList QtopiaService::list()
{
    return serviceList_p(Qtopia::qtopiaDir()+"services", ""); // No tr
}

/*!
  Returns the name of the \c QSettings file defining the bindings which the
  user has selected for the \a service.
*/
QString QtopiaService::binding(const QString& service)
{
    QString svrc = service;
    for (int i=0; i<(int)svrc.length(); i++)
        if ( svrc[i]=='/' ) svrc[i] = '-';
    return "Service-"+svrc; // No tr
}


/*!
  Returns the name of the \c QSettings file defining the facilities available from
  the applications providing the \a service, or
  a null string if there is no such service.

  Applications providing the service may have additional configuration
  defined in the \c appConfig() file.
*/
QString QtopiaService::config(const QString& service)
{
    return Qtopia::qtopiaDir()+"services/"+service+".service";
}


/*!
  Returns the \c QSettings file defining the facilities available from
  the applications providing the \a service, or
  a null string if there is no such service.

  The \c QSettings can be used to extract more detailed application-specific
  information about the application providing the service.

  Note: The application always provides at least the services defined
  in the \c config() file - the information in the \c appConfig()
  is application-specific.

  If \a appname is provided, a specific binding to that application may
  be used rather than the global binding.
*/
QString QtopiaService::appConfig(const QString& service, const QString& appname)
{
    QString a = app(service,appname);
    if ( a.isNull() )
        return a;
    return Qtopia::qtopiaDir()+"services/"+service+"/"+a;
}


/*!
  Returns all applications that offer the \a service, or
  a null string if there is no such service offered.
*/
QStringList QtopiaService::apps(const QString& service)
{
    QStringList all;

    QDir dir(Qtopia::qtopiaDir()+"services/"+service, "*",
      QDir::Unsorted, QDir::Files );

    all = dir.entryList();

    return all;
}

/*!
  Returns the application providing the \a service or
  a null string if there is no such service.

  If \a appname is provided, a specific binding to that application may
  be used rather than the global binding.
*/
QString QtopiaService::app(const QString& service, const QString& appname)
{
    QSettings cfg("Trolltech",binding(service));
    QString r;
    if ( cfg.status()==QSettings::NoError ) {
        cfg.beginGroup("Service");
        if  (!appname.isEmpty())
            r = cfg.value(appname).toString();
        if ( r.isNull() )
            r = cfg.value("default").toString();
    }
    if ( r.isNull() ) {
        QDir dir(Qtopia::qtopiaDir()+"services/"+service, QString("*"),
          QDir::Unsorted, QDir::Files );

        if ( dir.count() )
        {
            r = dir[0]; //use any as a default
            for( unsigned int i = 0 ; i < dir.count() ; ++i )
                if( dir[i] == appname )
                    r = dir[i]; //but use specific if it exists
        }
    }
    return r;
}


/*!
  Returns the QCop channels for applications that offer
  \a service.
*/
QStringList QtopiaService::channels(const QString& service)
{
    QStringList r;
    QStringList rs = apps(service);
    for (QStringList::ConstIterator it = rs.begin(); it!=rs.end(); ++it)
        r.append( QString("QPE/Application/") + (*it).toLatin1() );
    return r;
}

/*!
  Returns the \c QCop channel for the given \a service, or
  a null string if there is no such service.

  If \a appname is provided, a specific binding to that application may
  be used rather than the global binding.
*/
QString QtopiaService::channel(const QString& service, const QString& appname)
{
    QString r = app(service,appname).toLatin1();
    return r.isEmpty() ? QString() : "QPE/Application/"+r;
}

#include <qtopiaipcenvelope.h>

/*!
  Construct a null service request.
  \c setService() and\c  setMessage() must be called before \c send(), but the
  service may be written prior to the calls.
 */
QtopiaServiceRequest::QtopiaServiceRequest()
{
}

/*!
  Construct a service request that will send \a message to
  a \a service when \c send() is called. The service may be written
  prior to the calls.
*/
QtopiaServiceRequest::QtopiaServiceRequest(const QString& service, const QString& message)
    : m_Service(service), m_Message(message)
{
}

/*!
  Copy constructor. Any data previously written to the \a orig
  service will be in the copy.
*/
QtopiaServiceRequest::QtopiaServiceRequest(const QtopiaServiceRequest& orig)
{
    m_Service = orig.m_Service;
    m_Message = orig.m_Message;
    m_arguments = orig.m_arguments;
}

/*!
  Assignment operator.
  Any data previously written to the \a orig
  service will be in the copy.
*/
QtopiaServiceRequest& QtopiaServiceRequest::operator=(const QtopiaServiceRequest& orig)
{
    if( &orig == this )
        return *this;

    m_Service = orig.m_Service;
    m_Message = orig.m_Message;
    m_arguments = orig.m_arguments;

    return *this;
}

/*!
  Destructs the service request. Unlike \c QtopiaIpcEnvelope, the
  request is NOT automatically sent.
*/
QtopiaServiceRequest::~QtopiaServiceRequest()
{
}

/*!
  Returns true if either the service or message is not set.
 */
bool QtopiaServiceRequest::isNull() const
{
    return m_Service.isNull() || m_Message.isNull();
}

/*!
  Sends the request. Returns false if the request was null.
*/
bool QtopiaServiceRequest::send() const
{
    if (isNull())
        return false;

#if defined(QTOPIA_DBUS_IPC)
    QDBusConnection dbc = QDBus::sessionBus();
    QString service = dbusInterface;
    service.append(".");
    service.append(m_Service);
    QString path = dbusPathBase;
    path.append(m_Service);

    qLog(Services) << "Sending service request on" << service << path << dbusInterface;
    /*
    QDBusInterface *iface = new QDBusInterface(service,
                                               path,
                                               dbusInterface, dbc);
    if (!iface) {
        qWarning("No interface registered!!");
        return false;
    }
    */

    int idx = m_Message.indexOf('(');
    QString dbusMethod = m_Message.left(idx);

    qLog(DBUS) << "Calling method" << dbusMethod;

    QDBusMessage msg = QDBusMessage::methodCall(service,
            path, dbusInterface, dbusMethod, dbc);
    msg.setArguments(m_arguments);
    dbc.call(msg, QDBus::NoBlock);

    /*
    iface->callWithArgumentList(QDBus::NoBlock,
                                dbusMethod, m_arguments);
    */
    qLog(DBUS) << "Calling method finished...";

    return true;
#elif !defined(QT_NO_COP)
    QString ch = QtopiaService::channel(m_Service, QString());

    if (ch.isNull())
        return false;

    QBuffer *buffer = new QBuffer();
    QDataStream stream(buffer);
    stream.device()->open(QIODevice::WriteOnly);

    foreach(QVariant item, m_arguments) {
        QtopiaIpcAdaptorVariant copy( item );
        copy.save( stream );
    }

    QString msg = message();

    int indexOfCC = msg.indexOf("::");
    if (indexOfCC != -1) {
        int indexOfParen = msg.indexOf("(");
        if (indexOfCC > indexOfParen) {
            msg = service() + "::" + msg;
        }
    }
    else {
        msg = service() + "::" + msg;
    }

    QtopiaIpcEnvelope e(ch, msg);
    delete e.device();
    e.setDevice(buffer);

    return true;
#else
    return false;
#endif
}

/*!
  Sets the \a service to which the request will be sent.
 */
void QtopiaServiceRequest::setService(const QString& service)
{
    m_Service = service;
}

/*!
  \fn QString QtopiaServiceRequest::service() const

  Returns the service to which this request will be sent.

  \sa setService()
*/

/*!
    Sets the \a message to be sent to the service.
*/
void QtopiaServiceRequest::setMessage(const QString& message)
{
    m_Message = message;
}

/*!
  \fn QString QtopiaServiceRequest::message() const

  Returns the message of the request.

  \sa setMessage()
*/

/*!
    \fn const QList<QVariant> &QtopiaServiceRequest::arguments() const

    Get the complete list of arguments for this service request.
*/

/*!
    \fn void QtopiaServiceRequest::setArguments(const QList<QVariant> &arguments)

    Set the complete list of \a arguments for this service request.
*/

/*!
    \fn QtopiaServiceRequest &QtopiaServiceRequest::operator<< (const T &var)

    Add \a var to the list of arguments for this service request.
*/

/*!
    \fn QtopiaServiceRequest &QtopiaServiceRequest::operator<< (const char *var)

    Add \a var to the list of arguments for this service request.
*/

/*!
    \fn void QtopiaServiceRequest::addVariantArg(const QVariant& var)

    Adds the variant \a var to the list of arguments, so that the variant's
    value is serialized in send() rather than the variant itself.
*/

/*!
    \internal
*/
QByteArray QtopiaServiceRequest::serializeArguments(const QtopiaServiceRequest &action)
{
    QByteArray ret;
    QBuffer *buffer = new QBuffer(&ret);
    buffer->open(QIODevice::WriteOnly);
    QDataStream stream(buffer);
    stream << action.m_arguments;

    delete buffer;
    return ret;
}

/*!
    \internal
*/
void QtopiaServiceRequest::deserializeArguments(QtopiaServiceRequest &action,
        const QByteArray &data)
{
    QDataStream stream(data);
    stream >> action.m_arguments;
}

/*!
    \internal
    \fn void QtopiaServiceRequest::serialize(Stream &stream) const
*/
template <typename Stream> void QtopiaServiceRequest::serialize(Stream &stream) const
{
    stream << m_arguments;
    stream << m_Service;
    stream << m_Message;
}

/*!
    \internal
    \fn void QtopiaServiceRequest::deserialize(Stream &stream)
*/
template <typename Stream> void QtopiaServiceRequest::deserialize(Stream &stream)
{
    stream >> m_arguments;
    stream >> m_Service;
    stream >> m_Message;
}

Q_IMPLEMENT_USER_METATYPE(QtopiaServiceRequest)
