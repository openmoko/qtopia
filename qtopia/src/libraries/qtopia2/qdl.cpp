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

#include <qobjectlist.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <qdatastream.h>
#include <qtimer.h>
#include <qlistbox.h>
#include <qvaluelist.h>
#include <qaction.h>
#include <qlayout.h>

#include <qtopia/qpeapplication.h>
#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#endif
#include <qtopia/resource.h>
#ifndef NO_QT_QCOP
#include <qtopia/qcopenvelope_qws.h>
#include <qcopchannel_qws.h>
#endif
#include <qtopia/global.h>
#include <qtopia/services.h>
#include <qtopia/applnk.h>
#include <qtopia/stringutil.h>
#include <qtopia/quuid.h>

#include "qdl.h"
#include "qdl_p.h"

const QString QDL_KEY_DELIM = "-";

//initalise QDL static constants
const QString QDL::DATA_KEY = "qdl-private-data";

const QCString QDL::CLIENT_CHANNEL = "QPE/QDL";

const ushort QDL::u1 = 0x00B9;
const ushort QDL::u2 = 0x00B2;
const ushort QDL::u3 = 0x00B3;
const ushort QDL::u4 = 0x2074;
const ushort QDL::u5 = 0x2075;
const ushort QDL::u6 = 0x2076;
const ushort QDL::u7 = 0x2077;
const ushort QDL::u8 = 0x2078;
const ushort QDL::u9 = 0x2079;
const ushort QDL::u0 = 0x2070;

/*!
  \class QDL qdl.h
  \brief The QDL class provides general utility functions for use
    with Qtopia Data Linking.

  Qtopia Data Linking (QDL) enables a client to link to data in a source.
  A client may be a stand-alone object (QDLClient) or attached to a
  widget (QDLWidgetClient).
  A source can be any application which acts as a container for useful
  data and provides the QDL Service. 

  This class provides useful functions for loading and saving links
  and dealing with general QDL usage.

  First availability: Qtopia 2.0

  \ingroup qtopiaemb
*/

/*!
  Returns all QDLClient objects that are children of \a parent.
  This is useful for automatically loading links into a bunch of clients eg.

  \code
    QDL::loadLinks( rec, QDL::clients( this ) );
  \endcode
*/
QList<QDLClient> QDL::clients( QObject *parent )
{
    QList<QDLClient> cs;

    if( !parent )
	return cs;

    QObjectList *l = parent->queryList( "QDLClient" );
    if( !l ) return cs;

    QObjectListIt it( *l );

    while( it.current() )
    {
	QDLClient *client = (QDLClient *)it.current();
	cs.append( client );
	++it;
    }

    delete l;

    return cs;
}

/*!
  Sends a heart beat to the client with the specified \a clientID.
  A heart beat is used by a source to let clients know that their request is still
  being actively processed. A client will stop waiting for a response if their request times out
  so this function should be called periodically to keep the request alive.

  Normally you don't want to use this function directly, but the abstraction class QDLHeartBeat.
  
  \sa QDLHeartBeat
*/
void QDL::sendHeartBeat( const QString &clientID )
{
#ifndef QT_NO_QCOP
    QCopEnvelope e( QDL::CLIENT_CHANNEL, "QDLHeartBeat(QString)" );
    e << clientID;
#else
    qDebug("BUG : Using a QDLHeartBeat when QCop not enabled");
#endif
}

/*
   \relates QDL
   Reads \a clientList from the \a stream
*/
QDataStream &operator>>( QDataStream &stream, QList<QDLClient> &clientList )
{
    while( !stream.atEnd() )
    {
	QString clientName;
	uint numLinks = 0;
	stream >> clientName >> numLinks;
	if( clientName.isEmpty() || numLinks == 0 )
	    continue;

	QListIterator<QDLClient> cit( clientList );
	QDLClient *c = 0;
	for( ; *cit ; ++cit )
	{
	    if( QString((*cit)->name()) == clientName )
	    {
		c = *cit;
		break;
	    }
	}

	for( uint i = 0 ; i < numLinks ; ++i )
	{
	    uint lid = 0;
	    QDLLink newLink;
	    stream >> lid >> newLink; 
	    if( c )
	    {
		if( lid == 0 )
		    qDebug("BUG : QDL::loadLinks - Tried to load link from client %s with lid 0", clientName.latin1());
		else
		    c->QDLClient::setLink( lid, newLink );//explicit call to base class version
	    }
	}
    }
    return stream;
}

/*!
  Loads the links stored in \a str into the clients in \a clientList.
  \a str is the base64 encoded binary data of the links.
*/
void QDL::loadLinks( const QString &str, QList<QDLClient> clientList )
{
    // extract all links out of the string
    if( str.isEmpty() )
	return;
    QByteArray data;
    data.duplicate( str.latin1(), str.length() );
    data = Global::decodeBase64( data );
    QDataStream stream( data, IO_ReadOnly );
    stream >> clientList;
}

/*! \overload

  Loads the links stored in \a str into a single \a client.
*/
void QDL::loadLinks( const QString &str, QDLClient *client )
{
    QList<QDLClient> clients;
    clients.append( client );
    QDL::loadLinks( str, clients );	
}

/*
   \relates QDL
   Writes \a clientList to \a stream
*/
QDataStream &operator<<( QDataStream &stream, const QList<QDLClient> &clientList )
{
    QListIterator<QDLClient> cit( clientList );
    for( ; *cit ; ++cit )
    {
	//verify links for this client
	(*cit)->verifyLinks();

	QMap<uint, QDLLink> links = (*cit)->links();
	QString clientName = (*cit)->name();

	if( clientName.isEmpty() || !links.count() )
	    continue;

	stream << clientName << links.count();

	QMap<uint, QDLLink>::ConstIterator lit;
	for( lit = links.begin() ; lit != links.end() ; ++lit )
	    stream << lit.key() << lit.data();
    }
    return stream;
}

/*!
  Saves the links from \a clientList into \a str.
  \a str will be the base64 encoded binary link data.
*/
void QDL::saveLinks( QString &str, QList<QDLClient> clientList )
{
    QByteArray data;
    QDataStream stream( data, IO_WriteOnly );
    stream << clientList;
    data = Global::encodeBase64( data );
    str = QString( data );
}

/*! \overload
  Saves the links from the single \a client to \a str.
*/
void QDL::saveLinks( QString &str, QDLClient *client )
{
    QList<QDLClient> clients;
    clients.append( client );
    QDL::saveLinks( str, clients );
}


/*!
  Converts the unicode superscript link identifier \a lid into
  an equivalent unsigned integer link identifier.

  \sa lidFromUInt()
*/
uint QDL::lidToUInt( const QString &lid )
{
    const int len = lid.length();
    uint il = 0;
    for( int i = 0 ; i < len ; ++i )
    {
	if( lid[i].isDigit() )
	{
	    il = il * 10 + (uint)lid[i].digitValue();
	    continue;
	}

	ushort ch = lid[i].unicode();
	int v = -1;
	switch( ch )
	{
	    case u1:
		v = 1;
		break;
	    case u2:
		v = 2;
		break;
	    case u3:
		v = 3;
		break;
	    case u4:
		v = 4;
		break;
	    case u5:
		v = 5;
		break;
	    case u6:
		v = 6;
		break;
	    case u7:
		v = 7;
		break;
	    case u8:
		v = 8;
		break;
	    case u9:
		v = 9;
		break;
	    case u0:
		v = 0;
		break;
	}
	if( v != -1 )
	    il = il * 10 + (uint)v;
    }
    return il;
}

/*!
  Converts the unsigned integer link identifier \a lid into an equivalent
  unicode superscript link identifier.
*/
QString QDL::lidFromUInt( uint lid )
{
    QString lidAscStr, lidStr;
    lidAscStr.setNum( lid );
    const int len = lidAscStr.length();

    for( int i = 0 ; i < len ; ++i )
    {
	char ch = lidAscStr[i].latin1();
	switch( ch )
	{
	    case '0':
		lidStr += QChar(u0);
		break;
	    case '1':
		lidStr += QChar(u1);
		break;
	    case '2':
		lidStr += QChar(u2);
		break;
	    case '3':
		lidStr += QChar(u3);
		break;
	    case '4':
		lidStr += QChar(u4);
		break;
	    case '5':
		lidStr += QChar(u5);
		break;
	    case '6':
		lidStr += QChar(u6);
		break;
	    case '7':
		lidStr += QChar(u7);
		break;
	    case '8':
		lidStr += QChar(u8);
		break;
	    case '9':
		lidStr += QChar(u9);
		break;
	}
    }
    return lidStr;
}

/*!
  Activates the QDL link specified by \a ahref. 
  ahref should be in the form \code qdl://<clientName>:<lid> \endcode.
  Rich text links are usually generated by QDL::lidsToAnchors.

  What 'activate' actually means is dependent on the source application.
  As an example, if \c the QDLLink points to a contact, the Contacts 
  application might display that particular contact.

  \sa QDL::lidsToAnchors
*/
void QDL::activateLink( const QString &ahref, const QList<QDLClient> &clientList )
{
    QString text = ahref;
    if( !text.startsWith( "qdl://" ) )
    {
	qWarning("QDL::activateLink - '%s' doesn't appear to be a valid QDL link.", text.latin1() );
	return;
    }
    text = text.mid(6);
    QString clientName;
    uint lid = 0;
    QStringList data;
    data = QStringList::split( ':', text );
    if( data.count() != 2 )
    {
	qWarning("QDL::activateLink called with invalid link data");
	return;
    }
    clientName = QDL::decodeAhref(data[0]);
    lid = QDL::decodeAhref(data[1]).toUInt();
    if( clientName.isEmpty() || lid == 0 )
    {
	qWarning("QDL::activateLink - Skipping activation of empty link.");
	return;
    }

    //find the client
    QDLClient *c = 0;
    QListIterator<QDLClient> it(clientList);
    for( ; *it ; ++it )
    {
	if( QString((*it)->name()) == clientName )
	{
	    c = *it;
	    break;
	}
    }

    if( !c )
    {
	qWarning("QDL::activateLink - Can't activate link for unknown client %s", clientName.latin1());
	return;
    }

    //get the link
    QDLLink al = c->link( lid );
    if( al.isNull() )
    {
	qWarning("QDL::activateLink - Link with lid %d does not exist in client %s", lid, clientName.latin1());
	return;
    }

#ifndef QT_NO_QCOP
    QCString channel;
    channel = "QPE/Application/" +  al.appRef();
    QCopEnvelope e( channel, "QDLActivateLink(QByteArray)" );
    e << al.dataRef();
#endif
}

/*!
  Converts the unicode superscript link identifiers in \a lidText to rich text anchors 
  based on the links in \a client.

  If \a withIcons is TRUE (the default), icons are included as part of the rich text links.
*/
QString QDL::lidsToAnchors( const QString &lidText, const QDLClient *client, bool withIcons )
{
    QMap<uint, QDLLink> links = client->links();
    QString t = lidText;
    QMap<uint, QDLLink>::ConstIterator it;
    for( it = links.begin() ; it != links.end() ; ++it )
    {
	QString lid = QDL::lidFromUInt( it.key() );
	QString lt = (*it).description()+lid;
	int f = t.find( lt ); 
	bool haveDesc = TRUE;
	if( f == -1 )
	{
	    lt = lid;
	    f = t.find( lid );
	    if( f != -1 )
		haveDesc = FALSE;
	}
	if( f != -1 )
	{
	    t = t.remove( f, lt.length() );
	    QString ahref = "<a href=\"qdl://%1:%2\">%3%4</a>";
	    ahref = ahref.arg( QDL::encodeAhref(client->name()) ).arg( it.key() );
	    if( haveDesc )
		ahref = ahref.arg( Qtopia::escapeString((*it).description()) );
	    else
		ahref = ahref.arg( "" );
	    /*
	     if they've changed the description of the link, override withIcons 
	     and always include an icon.  // otherwise, what would reference the link?
	     */
	    if( withIcons || !haveDesc ) 
	    {
		QString iFileName = (*it).icon();
		if( iFileName.isEmpty() )
		    iFileName = "qdllink";
		/* FIXME
	    should the user be able to set the size of the icon? eg font().pointSize()
		*/
		QString img = "<img width=\"12\" height=\"12\" src=\"%1\">";
		img = img.arg( Qtopia::escapeString(iFileName) );
		ahref = ahref.arg( img );
	    }
	    else
	    {
		ahref = ahref.arg(QString::null);
	    }
	    t = t.insert( f, ahref );
	}
    }
    return t;
}

/*!
  Removes the unicode superscript link identifiers in \a lidText.
*/
QString QDL::removeLids( const QString &lidText )
{
    QString t;
    for (uint i = 0; i < lidText.length(); i++) {
	switch(lidText[i].unicode()) {
	    case u0:
	    case u1:
	    case u2:
	    case u3:
	    case u4:
	    case u5:
	    case u6:
	    case u7:
	    case u8:
	    case u9:
		break; // do nothing
	    default:
		t += lidText[i];
	}
    }
    return t;
}

/*!
  Encodes the string \a ahref so that it may be included as part of a QDL link.
*/
QString QDL::encodeAhref( const QString &ahref )
{
    QString encAhref = ahref;
    return encAhref.replace( QRegExp(":"), "%3A" );
}

/*!
  Decodes the string \a ahref that was encoded as part of a QDL link.
*/
QString QDL::decodeAhref( const QString &ahref )
{
    QString decAhref = ahref;
    return decAhref.replace( QRegExp("%3A"), ":" );
}


/*!
  \class QDLLink qdl.h

  \brief The QDLLink class encapsulates the data of a link.

  The QDLLink class encapsulates the data of a link.

  A link is made up of four elements. They are:
  <ol>
    <li>Application Reference (QCString)- A reference to the application which contains the data to be linked.
    This is the application's filename (eg. addressbook)
    <li>Data Reference (QByteArray)- Binary data that the source uses which uniquely identifies the data to be linked.
    <li>Description (QString)- A description of the link to display to the user.
    <li>Icon (QCString) The name of an icon which qualifies the link in some way
	    (eg. as a link to a contact, an event etc). 
	    Passed to Resource::loadPixmap()
  </ol>

  \ingroup qtopiaemb
*/

/*
   \relates QDLLink
   Reads \a link from the \a stream
*/
QDataStream &operator>>( QDataStream &stream, QDLLink &link )
{
    QCString appRef, icon;
    QByteArray dataRef;
    QString desc;
    stream >> appRef >> dataRef >> desc >> icon;
    link.setAppRef( appRef );
    link.setDataRef( dataRef );
    link.setDescription( desc );
    link.setIcon( icon );
    return stream;
}

/*
   \relates QDLLink
   Writes \a link to the \a stream
*/
QDataStream &operator<<( QDataStream &stream, const QDLLink &link )
{
    stream << link.appRef() << link.dataRef() << link.description() << link.icon();
    return stream;
}

/*!
   Constructs a null link.
*/
QDLLink::QDLLink()
{
}

/*!
  Constructs a link from
   the application reference \a appRef, the data reference \a dataRef,
   the description \a desc and the icon \a icon.
*/
QDLLink::QDLLink( const QCString &appRef, const QByteArray &dataRef,
    const QString &desc, const QCString &icon ) 
{
    mAppRef = appRef;
    mDataRef = dataRef;
    mDescription = desc;
    mIcon = icon;
}

/*!
  Copy constructor. Performs a deep copy of the link specified by \a other.
*/
QDLLink::QDLLink( const QDLLink &other )
    : QObject()
{
    mAppRef = other.mAppRef;
    mDataRef = other.mDataRef;
    mDescription = other.mDescription;
    mIcon = other.mIcon;
}


/*!
  Deep copies the link specified by \a other.
*/
QDLLink &QDLLink::operator=( const QDLLink &other )
{
    mAppRef = other.mAppRef;
    mDataRef = other.mDataRef;
    mDescription = other.mDescription;
    mIcon = other.mIcon;
    return *this;
}

/*!
  Returns TRUE if the link is null, otherwise returns FALSE.
  A null link has no application or data reference.
*/
bool QDLLink::isNull() const
{
    return mAppRef.isEmpty() || mDataRef.isEmpty();
}


/*!
  Sets the application reference of the link to \a ref.
*/
void QDLLink::setAppRef( const QCString &ref )
{
    mAppRef = ref;
}

/*!
  Returns the application reference of the link.
*/
QCString QDLLink::appRef() const
{
    return mAppRef;
}

/*!
  Sets the data reference of the link to \a ref.
*/
void QDLLink::setDataRef( const QByteArray &ref )
{
    mDataRef = ref;
}

/*!
  Returns the data reference of the link.
*/
QByteArray QDLLink::dataRef() const
{
    return mDataRef;
}

/*!
  Sets the description of the link to \a desc.
*/
void QDLLink::setDescription( const QString &desc )
{
    mDescription = desc;
}

/*!
  Returns the description of the link.
*/
QString QDLLink::description() const
{
    return mDescription;
}

/*!
  Sets the icon of the link to \a icon.
*/
void QDLLink::setIcon( const QCString &icon )
{
    mIcon = icon;
}

/*!
  Returns the icon of the link.
*/
QCString QDLLink::icon() const
{
    return mIcon;
}


class QDLHeartBeatPrivate
{
public:
    QDLHeartBeatPrivate( const QString &cid )
	: clientID( cid )
    {
	timer = new QTimer();
    }
    ~QDLHeartBeatPrivate()
    {
	delete timer;
    }
    QString clientID;
    QTimer *timer;
};
/*!
   \class QDLHeartBeat qdl.h
   \brief The QDLHeartBeat class automates the sending of a heart beat to clients. 

   QDLHeartBeat automates the sending of a heart beat to
   a QDLClient. It begins sending a heart beat when constructed
   and terminates when it is destructed (eg goes out of scope).

   This is used by a source application to let a client know that
   the request it made is being actively processed.

   \sa QDLClient

  \ingroup qtopiaemb
*/

/*!
  Constructs a QDLHeartBeat and begins sending heart beats to the 
  client specified by \a clientID immediately.
*/
QDLHeartBeat::QDLHeartBeat( const QString &clientID )
{
    d = new QDLHeartBeatPrivate( clientID );
    connect( d->timer, SIGNAL(timeout()), this, SLOT(beat()) );
    d->timer->start( 3000, FALSE );
    beat(); //force an update right now
}

/*!
  Destructs the QDLHeartBeat so that it no longer sends heart beats.
*/
QDLHeartBeat::~QDLHeartBeat()
{
    delete d;
}

/*!
  Sends a heart beat. Simply calls QDL::sendHeartBeat() with the id of the client.
*/
void QDLHeartBeat::beat()
{
    QDL::sendHeartBeat( d->clientID );
}

class QDLClientPrivate
{
public:
    QDLClientPrivate()
	: timeout( 5000 )
    {
	lc = 0;
	timer = new QTimer();

	uid = Global::generateUuid();
    }

    ~QDLClientPrivate()
    {
	delete timer;
    }

    QCopChannel *lc;

    QString hint;

    QTimer *timer;
    const uint timeout;

    QUuid uid;

    QMap<uint, QDLLink> links;
};

/*!
   \class QDLClient qdl.h
   \brief The QDLClient class provides a set of links and communication with a data source.
    
   A QDLClient contains a set of links and handles communication with a data source.

   A single client can have an unlimited number of links. To load and save links to and from a client
   you can use the utility functions QDL::loadLinks() and QDL::saveLinks() respectively.

   This class operates independently of any other object. The subclass QDLWidgetClient extends 
   this class to operate on a widget and its text dependently.

   \sa QDLWidgetClient

  \ingroup qtopiaemb
*/

/*!
  Constructs a QDLClient. \a parent and \a name are passed on to QObject.
*/
QDLClient::QDLClient( QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QDLClientPrivate();
    connect( d->timer, SIGNAL(timeout()), this, SLOT(requestTimeout()) );
}

/*!
  Clears all links in the client.
*/
void QDLClient::clear()
{
    d->links.clear();
}

/*!
   Adds \a newLink to the client. A new link identifier is generated.
*/
void QDLClient::addLink( const QDLLink &newLink )
{
    if( newLink.isNull() ) 
	return;
    setLink( nextLID(), newLink );
}


/*! 
   Sets the link specified by the link identifier \a lid to \a newLink.
   If no link for \a lid currently exists, it is created.
*/
void QDLClient::setLink( uint lid, const QDLLink &newLink )
{
    if( newLink.isNull() ) 
	return;
    d->links[lid] = newLink;
}

/*! 
   Removes the link specified by the link identifier \a lid.
*/
void QDLClient::removeLink( uint lid )
{
    if( d->links.contains( lid ) )
	d->links.remove( lid );
}

/*!
  Returns TRUE if a request for links is active, otherwise returns FALSE.
*/
bool QDLClient::isRequestActive() const
{
    if( d->lc )
	return TRUE;
    return FALSE;
}


/*!
  Returns the link specified by the link identifier \a lid.
  If no such link exists a null link is returned. 
*/
QDLLink QDLClient::link( uint lid ) const
{
    if( !d->links.contains( lid ) )
	return QDLLink();
    return d->links[lid];
}

/*!
  Returns a link identifier to QDLLink map of all links in the client.
*/
QMap<uint, QDLLink> QDLClient::links() const
{
    return d->links;
}

/*!
  Returns the number of links in the client.
*/
uint QDLClient::count() const
{
    return d->links.count();
}

/*!
  Sets the request hint for the client to \a hint.
  A hint is sent to a data source as part of the request to help it determine 
  pertinent data.

  A hint can only be used with a QDLClient by calling setHint() explicitly.

  \sa QDLWidgetClient
*/
void QDLClient::setHint( const QString &hint )
{
    d->hint = hint;
}

/*!
  Returns the request hint for this client.
  
  Subclasses may override this function if they wish to dynamically determine a hint.

  \sa QDLWidgetClient::hint()
*/
QString QDLClient::hint() const
{
    return d->hint;
}

/*!
  Sends a request for links to the data source listening on \a channel.
*/
void QDLClient::requestLink( const QString &channel )
{
#ifndef QT_NO_QCOP
    if( isRequestActive() )
	return;

    QString h = hint();
    QCopEnvelope e( channel.latin1(), "QDLRequestLink(QString,QString)" );
    e << d->uid.toString() << h;
    d->lc = new QCopChannel( QDL::CLIENT_CHANNEL, this );
    connect( d->lc, SIGNAL(received(const QCString&, const QByteArray&)), this, 
	    SLOT(receiveLinks(const QCString&, const QByteArray&)) );

    d->timer->start( d->timeout, TRUE );
#else
    qDebug("QDLClient - Can't request link because QCop not enabled.");
#endif
}

/*! \overload
  Prompts the user to select a data source application, determines source
  application's channel
  based on the user's selection, and then passes it to the above function.
  \a parent is the parent of the dialog used.
*/
void QDLClient::requestLink( QWidget *parent )
{
    QDLSourceSelector *s = new QDLSourceSelector( parent, "sourceSelector" );
#ifndef QTOPIA_DESKTOP
    if( QPEApplication::execDialog( s ) == QDialog::Accepted )
#else
    if( d->exec() == QDialog::Accepted )
#endif
    {
	QMap<QString, QString> sources = s->selected();
	QMap<QString, QString>::ConstIterator sit;
	for( sit = sources.begin() ; sit != sources.end() ; ++sit )
	    requestLink( sit.data() );
    }
    delete s;
}


/*! \internal
    Processes messages from a data source.
*/
void QDLClient::receiveLinks( const QCString &sig, const QByteArray &data )
{
    if( !isRequestActive() )
	return;

    QDataStream s( data, IO_ReadOnly );	    
    //first argument is already the id
    QString id;
    s >> id;
    if( d->uid.toString() != id )
	return; // message isn't for this client

    if( sig == "QDLProvideLink(QString,int,...)" )
    {
	uint numLinks = 0;

	s >> numLinks;
	for( uint i = 0 ; i < numLinks ; ++i )
	{
	    QDLLink newLink;
	    s >> newLink;
	    addLink( newLink );
	}
	d->timer->stop();
	delete d->lc;
	d->lc = 0;
    }
    else if( sig == "QDLHeartBeat(QString)" )
    {
	//reset next timeout time
	d->timer->stop();
	d->timer->start( d->timeout, TRUE );
    }
}

/*!
  Verifies that all links in the client are valid. The default implementation does nothing,
  This function is called from QDL::saveLinks().

  Subclasses that wish to perform any verification processing should override this function.
*/
void QDLClient::verifyLinks()
{
}

/*! \internal
  Kills an active request.
*/
void QDLClient::requestTimeout()
{
    if( !isRequestActive() )
	return;

    delete d->lc;
    d->lc = 0;

    d->timer->stop();
}

/*! \internal
  Generates a new link identifier
*/
uint QDLClient::nextLID()
{
    uint numLids = d->links.count();

    for( uint i = 1 ; i <= numLids ; ++i )
	if( !(d->links.contains( i )) )
	    return i;
    return numLids+1;
}
    
#include <qlineedit.h>
#include <qmultilineedit.h>
//add more widgets here
union QDLClientWidgets
{
    QLineEdit *le;
    QMultiLineEdit *me;
    //add more widgets here
};

class QDLWidgetClientPrivate
{
public:
    QDLClientWidgets widget;
    QString widgetType;
};

/*!
   \class QDLWidgetClient qdl.h
   \brief The QDLWidgetClient class is a QDLClient that interfaces with a widget.

   A QDLWidgetClient has all the capabilities of a QDLClient, but also
   depends on a widget and interfaces with its text.

   A QDLWidgetClient can currently interface with the following widgets.
   <ul>
    <li>QLineEdit
    <li>QMultiLineEdit
   </ul>
   Other widgets may be supported in later releases of Qtopia.

  \ingroup qtopiaemb
*/

/*!
  Constructs a QDLWidgetClient with \a w as the parent and
  the monitored widget. \a name is passed on to QObject.
*/
QDLWidgetClient::QDLWidgetClient( QWidget *w, const char *name )
    : QDLClient( w, name )
{
    d = new QDLWidgetClientPrivate();

    setWidget( w );
}

/*! \internal
  Sets the widget for this client to \a w.
*/
void QDLWidgetClient::setWidget( QWidget *w )
{

    if( !w )
	setWidgetType( QString::null );
    else if( w->inherits( "QLineEdit" ) )
    {
	d->widget.le = (QLineEdit *)w;
	setWidgetType( "QLineEdit" );
    }
    else if( w->inherits( "QMultiLineEdit" ) )
    {
	d->widget.me = (QMultiLineEdit *)w;
	setWidgetType( "QMultiLineEdit" );
    }
    //add more widgets here
}

/*! \internal
  Returns the widget for this client.
  Returns 0 if an unsupported widget was passed to the constructor.

  \sa QDLWidgetClient::QDLWidgetClient()
*/
QWidget *QDLWidgetClient::widget() const
{
    QString wt = widgetType();

    if( wt == "QLineEdit" )
	return d->widget.le;
    else if( wt == "QMultiLineEdit" )
	return d->widget.me;
    //add more widgets here
    else
	return 0;
}

/*! \internal
  Sets the type of the widget for the client to \a t. The type
  is the class name of the widget, eg QLineEdit
*/
void QDLWidgetClient::setWidgetType( const QString &t )
{
    d->widgetType = t;
}

/*! \internal
  Returns the type of the widget for the client.
*/
QString QDLWidgetClient::widgetType() const
{
    return d->widgetType;
}

/*!
  Returns the text of the client's widget.
*/
QString QDLWidgetClient::text() const
{
    QString wt = widgetType();

    if( wt == "QLineEdit" )
	return d->widget.le->text();
    else if( wt == "QMultiLineEdit" )
	return d->widget.me->text();
    //add more widgets here
    else
	return QString::null;
}

/*!
  Adds \a newLink to the client. A new link identifier is generated.
  The link is also added into the widget's text.
*/
void QDLWidgetClient::addLink( const QDLLink &newLink )
{
    uint nl = nextLID();
    int p = textPos();
    QString t = text();
    QString space;
    if( !t.isEmpty() && p )
	if( !t[p-1].isSpace() )
	    space = " ";

    t = t.insert( p, space+newLink.description()+QDL::lidFromUInt(nl) );
    setText( t );

    QDLClient::setLink( nl, newLink );
}

/*!
   Sets the link specified by the link identifier \a lid to \a newLink both in QDLClient
   and in the widget's text. If no link for \a lid currently exists, it is created
   and added to the widget's text.
*/
void QDLWidgetClient::setLink( uint lid, const QDLLink &newLink )
{
    QDLLink l = link( lid );

    QString t = text();
    //first try and find the description and lid. failing that, just the lid
    QString lt = l.description()+QDL::lidFromUInt(lid);
    int f = t.find( lt );
    if( f != -1 )
    {
	t = t.remove( f, lt.length() );
    }
    else
    {
	QString textLID = QDL::lidFromUInt(lid);
	f = t.find( textLID );
	if( f != -1 )
	{
	    t = t.remove( f, textLID.length() );
	}
	else
	{
	    f = 0;
	}
    }

    t = t.insert( f, newLink.description()+lid );
    setText( t );

    QDLClient::setLink( lid, newLink );
}

/*!
  Removes the link specified by the link identifier \a lid.
  It is removed from QDLClient and in the widget's text.
*/
void QDLWidgetClient::removeLink( uint lid )
{
    if( links().contains( lid ) )
    {
	QDLLink l = link( lid );
	QString lt = l.description()+QDL::lidFromUInt(lid);

	//first try and find the description and lid. failing that, just the lid
	QString t = text();
	int f = t.find( lt );
	if( f != -1 )
	{
	    t = t.remove( f, lt.length() );
	    setText( t );
	}
	else
	{
	    QString textLID = QDL::lidFromUInt( lid );
	    f = t.find( textLID );
	    if( f != -1 )
	    {
		t = t.remove( f, textLID.length() );
	    }
	    setText( t );
	}

	QDLClient::removeLink( lid );
    }
}

#ifdef QTOPIA_PHONE 
/*!
  Automatically creates a standard 'Insert Link' action in the ContextMenu 
  specified by \a context. If \a context is 0, a new ContextMenu is created.
  
  The action is connected to QDLWidgetClient::requestLink().
*/
ContextMenu *QDLWidgetClient::setupStandardContextMenu( ContextMenu *context )
// TODO : How does this work on the PDA?
{
    QWidget *w = widget();
    if( !w ) {
	qWarning("BUG : trying to setup a context menu on a null widget client");
	return 0;
    }
    if ( !context )
	context = new ContextMenu( w, 0, ContextMenu::Modal );
    QAction *action = new QAction( tr( "Insert Link" ), Resource::loadIconSet( "qdllink" ),
	                           QString::null, 0, w, 0 );
    QObject::connect( action, SIGNAL(activated()), this, SLOT(requestLink()) );
    action->addTo( context );
    context->insertItem(Resource::loadIconSet("cut"), tr("Cut"), w, SLOT(cut()));
    context->insertItem(Resource::loadIconSet("copy"), tr("Copy"), w, SLOT(copy()));
    context->insertItem(Resource::loadIconSet("paste"), tr("Paste"), w, SLOT(paste()));
    return context;
}
#endif

/*! \internal
  Sets the text of the widget to \a txt.
*/
void QDLWidgetClient::setText( const QString &txt )
{
    QWidget *w = widget();
    if( !w )
	return;
    w->setFocus();
#ifdef QTOPIA_PHONE
    if( !Global::mousePreferred() ) 
    {
	if( w->isModalEditing() )
	    w->setModalEditing( TRUE );
    }
#endif
    QString wt = widgetType();
    if( wt == "QLineEdit" )
	d->widget.le->setText( txt );
    else if( wt == "QMultiLineEdit" )
	d->widget.me->setText( txt );
    //add more widgets here

}


/*!
  Convenience function. Simply calls QDLClient::requestLink() with
  the client's widget as parent.
*/
void QDLWidgetClient::requestLink()
{
    QDLClient::requestLink( widget() );
}

/*!
  Returns the request hint for this widget.
  If a hint has not been explicitly set with QDLClient::setHint(),
  then this function tries to dynamically determine the hint from 
  the text of the client's widget, otherwise it just returns QDLClient::hint() 
*/
QString QDLWidgetClient::hint() const
{
    //if the user has manually set a hint use that otherwise determine a hint(selection)
    QString h = QDLClient::hint();
    if( !h.isEmpty() )
	return h;
    return determineHint();
}

/*! \internal
  Dynamically determines a hint from the widget's text.
  This is currently the selected text of the widget, and therefore
  doesn't work on Qtopia Phone Edition.
*/
QString QDLWidgetClient::determineHint() const
{
    QString wt = widgetType();
    if( wt == "QLineEdit" )
	return d->widget.le->markedText();
    else if( wt == "QMultiLineEdit" )
	/*FIXME return d->widget.me->markedText()*/;
    //add more widgets here

    return QString::null;
}

/*! \internal
    Returns the current position within text() (eg cursor position)
*/
int QDLWidgetClient::textPos() const
{
    QString type = widgetType();

    if( type == "QLineEdit" )
    {
	return d->widget.le->cursorPosition();
    }
    else if( type == "QMultiLineEdit" )
    {
	QString t = text();
	const int len = t.length();
	int i = 0;
	int r = 0, c = 0;
	int rr = 0, rc = 0;
	//work out how many characters there are up to row,col
	d->widget.me->getCursorPosition( &rr, &rc );
	while( i < len )
	{
	    if( t[i] == '\n' )
	    {
		++r;
		c = 0;
	    }
	    else
	    {
		++c;
	    }
	    ++i;
	    if( r == rr && c == rc )
		break;
	}
	return i;
    }
    //add more widgets here
    return 0;
}

/*!
  Verifies that all internal links of QDLClient exist in the widget's text.
  Any internal link that doesn't exist in the widget's text is removed.
*/
void QDLWidgetClient::verifyLinks()
{
    QValueList<uint> invalidLids;

    QMap<uint, QDLLink>::ConstIterator it;
    QMap<uint, QDLLink> lnks = links();
    for( it = lnks.begin() ; it != lnks.end() ; ++it )
    {
	QString t = text();
	int p = t.find( QDL::lidFromUInt(it.key()) );
	if( p == -1 )
	    invalidLids.append( it.key() );
    }

    QValueList<uint>::ConstIterator iit; 
    for( iit = invalidLids.begin() ; iit != invalidLids.end() ; ++iit )
	QDLClient::removeLink( *iit );
}

class QDLSourceSelectorApp
{
public:
    QDLSourceSelectorApp()
    {
    }

    QDLSourceSelectorApp( const QDLSourceSelectorApp &c )
    {
	name = c.name;
	channel = c.channel;
	icon = c.icon;
    }

    QDLSourceSelectorApp( QString n, QString c, QPixmap i )
	: name( n ), channel( c ), icon( i )
    {
        name = Qtopia::dehyphenate(name);
    }

    QDLSourceSelectorApp &operator=( const QDLSourceSelectorApp &c )
    {
	name = c.name;
	channel = c.channel;
	icon = c.icon;
	return *this;
    }

    QString name, channel;
    QPixmap icon;
};

class QDLSourceSelectorPrivate
{
public:
    QDLSourceSelectorPrivate()
	: appList( 0 )
    {
    }

    QListBox *appList;
    QValueList<QDLSourceSelectorApp> apps;
    QMap<QListBoxItem *, int> itemToAppIndex;
};

QDLSourceSelector::QDLSourceSelector( QWidget *parent, const char *name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    d = new QDLSourceSelectorPrivate();

    QVBoxLayout *l = new QVBoxLayout( this );

    d->appList = new QListBox( this, "appList" );
    l->addWidget( d->appList );
    d->appList->setFocus();

    setCaption( tr("Select Source") );

    //get all applications that are qdl sources
    QStringList qdlApps = Service::apps( "qdl" );
    if( qdlApps.count() )
    {
	QStringList::Iterator it = qdlApps.begin();
	AppLnkSet alList( QPEApplication::qpeDir() + "apps" );
	for( ; it != qdlApps.end() ; ++it )
	{
	    AppLnk l = *alList.findExec( *it );
	    if( !l.isValid() )
	    {
		qWarning("QDLSourceSelector: Can't get a valid AppLnk for '%s'", (*it).latin1());
	    }
	    else
	    {
		QIconSet is( Resource::loadPixmap( l.icon() ) );

		QDLSourceSelectorApp a( l.name(), "QPE/Application/" + (*it), 
						is.pixmap( QIconSet::Small, TRUE ) );
		d->apps.append( a );
	    }
	}
    }
    
    QValueList<QDLSourceSelectorApp> &a = d->apps;

    const int len = a.count();
    for( int i = 0 ; i < len ; ++i ) {
	d->appList->insertItem( a[i].icon, a[i].name, i );
	d->itemToAppIndex[ d->appList->item( i ) ] = i;
    }

    connect( d->appList, SIGNAL(selected(int)), this, SLOT(accept()) );

    // and, since we are MenuLike...
    connect( d->appList, SIGNAL(pressed(QListBoxItem*)), this, SLOT(accept()) );

    d->appList->sort();

    if ( len && !d->appList->isSelected( d->appList->currentItem() ) ) // XXX should use most-recent-for-this-app
	d->appList->setSelected( 0, TRUE );
    QPEApplication::setMenuLike( this, TRUE );
}

QMap<QString, QString> QDLSourceSelector::selected() const
{
    QListBox *l = d->appList;
    QMap<QString, QString> selections;


    const unsigned int numApps = l->count();
    for( int i = 0 ; i < (int)numApps ; ++i )
    {
	if( l->isSelected( i ) )
	{
	    int j = d->itemToAppIndex[l->item( i )];
	    selections[d->apps[j].name] = d->apps[j].channel;
	}
    }

    return selections;
}

QSize QDLSourceSelector::sizeHint() const
{
    return QSize( width(), qApp->desktop()->height() );
}

void QDLSourceSelector::accept()
{
    QDialog::accept();
    emit selected( selected() );
}

