/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#include "outlooksync.h"
#include "outlookthread.h"

#include <trace.h>
#include <qcopenvelope_qd.h>
#include <desktopsettings.h>
#include <center.h>

#include <QBuffer>
#include <QApplication>
#include <QXmlStreamReader>
#include <QTimer>

OutlookSyncPlugin::OutlookSyncPlugin( QObject *parent )
    : QDServerSyncPlugin( parent ), thread( 0 )
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::OutlookSyncPlugin";
}

OutlookSyncPlugin::~OutlookSyncPlugin()
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::~OutlookSyncPlugin";
}

QString OutlookSyncPlugin::id()
{
    return QString("com.trolltech.plugin.outlook.sync.%1").arg(dataset());
}

void OutlookSyncPlugin::prepareForSync()
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::prepareForSync";
    Q_ASSERT(!thread);

    thread = OutlookThread::getInstance( centerInterface()->syncObject() );

    so = new OTSyncObject();
    so->q = this;
    so->o = thread->o;
    so->moveToThread( thread );

    // connect up the sync messages to the thread
    connect( this, SIGNAL(t_fetchChangesSince(QDateTime)), so, SLOT(fetchChangesSince(QDateTime)) );
    connect( this, SIGNAL(t_updateClientRecord(QByteArray)), so, SLOT(updateClientRecord(QByteArray)), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL(t_removeClientRecord(QString)), so, SLOT(removeClientRecord(QString)), Qt::BlockingQueuedConnection );
    // connect up the thread's signals to ours
    connect( so, SIGNAL(mappedId(QString,QString)), this, SIGNAL(mappedId(QString,QString)) );
    connect( so, SIGNAL(createServerRecord(QByteArray)), this, SIGNAL(createServerRecord(QByteArray)) );
    connect( so, SIGNAL(replaceServerRecord(QByteArray)), this, SIGNAL(replaceServerRecord(QByteArray)) );
    connect( so, SIGNAL(removeServerRecord(QString)), this, SIGNAL(removeServerRecord(QString)) );
    connect( so, SIGNAL(serverChangesCompleted()), this, SIGNAL(serverChangesCompleted()) );

    connect( so, SIGNAL(logonDone(bool)), this, SLOT(logonDone(bool)) );
    QTimer::singleShot( 0, so, SLOT(logon()) );
}

void OutlookSyncPlugin::finishSync()
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::finishSync";
    delete so;
    so = 0;
    thread = 0; // cleanup up by centerInterface()->syncObject()
    emit finishedSync();
}

void OutlookSyncPlugin::fetchChangesSince(const QDateTime &timestamp)
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::fetchChangesSince";
    // returns immediately
    emit t_fetchChangesSince( timestamp );
}

void OutlookSyncPlugin::logonDone( bool ok )
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::logonDone";
    emit readyForSync( ok );
}

void OutlookSyncPlugin::createClientRecord(const QByteArray &record)
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::createClientRecord";
    // blocks until complete
    emit t_updateClientRecord( record );
}

void OutlookSyncPlugin::replaceClientRecord(const QByteArray &record)
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::replaceClientRecord";
    // blocks until complete
    emit t_updateClientRecord( record );
}

void OutlookSyncPlugin::removeClientRecord(const QString &identifier)
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::removeClientRecord";
    // blocks until complete
    emit t_removeClientRecord( identifier );
}

QString OutlookSyncPlugin::dateToString( const QDate &date )
{
    return date.toString( "yyyy-MM-dd" );
}

QDate OutlookSyncPlugin::stringToDate( const QString &string )
{
    return QDate::fromString( string, "yyyy-MM-dd" );
}

QString OutlookSyncPlugin::dateTimeToString( const QDateTime &datetime, bool utc )
{
    if ( utc )
        return datetime.toString( "yyyy-MM-ddThh:mm:ssZ" );
    else
        return datetime.toString( "yyyy-MM-ddThh:mm:ss" );
}

QDateTime OutlookSyncPlugin::stringToDateTime( const QString &string, bool utc )
{
    if ( utc )
        return QDateTime::fromString( string, "yyyy-MM-ddThh:mm:ssZ" );
    else
        return QDateTime::fromString( string, "yyyy-MM-ddThh:mm:ss" );
}

QString OutlookSyncPlugin::escape( const QString &string )
{
    QString ret = string;
    ret.replace(QRegExp("&"), "&amp;");
    ret.replace(QRegExp(">"), "&gt;");
    ret.replace(QRegExp("<"), "&lt;");
    ret.replace(QRegExp("\r\n"), "\n"); // convert to Unix line endings
    ret.replace(QRegExp("\n$"), ""); // remove the trailing newline
    return ret;
}

QString OutlookSyncPlugin::unescape( const QString &string )
{
    QString ret = string;
    ret.replace(QRegExp("&lt;"), "<");
    ret.replace(QRegExp("&gt;"), ">");
    ret.replace(QRegExp("&amp;"), "&");
    return ret;
}

bool OutlookSyncPlugin::getIdentifier( const QByteArray &record, QString &id, bool &local )
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::getIdentifier";
    QXmlStreamReader reader(record);
    bool isIdent = false;
    bool leave = false;
    while (!reader.atEnd()) {
        switch(reader.readNext()) {
            case QXmlStreamReader::NoToken:
            case QXmlStreamReader::Invalid:
                return false;
            case QXmlStreamReader::StartElement:
                if (reader.qualifiedName() == "Identifier") {
                    isIdent = true;
                    QXmlStreamAttributes identAttr = reader.attributes();
                    QStringRef v = identAttr.value("localIdentifier");
                    if ( v.isNull() || v == "true" || v == "1" )
                        local = true;
                    else
                        local = false;
                }
                break;
            case QXmlStreamReader::EndElement:
                if (isIdent)
                    leave = true;
                break;
            case QXmlStreamReader::Characters:
                if (isIdent)
                    id = unescape(reader.text().toString());
                break;
            default:
                break;
        }
        if ( leave )
            break;
    }
    return true;
}

void OutlookSyncPlugin::init_item( IDispatchPtr dispatch )
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::init_item";
}

// =====================================================================

OTSyncObject::OTSyncObject()
    : QObject(), q(0), o(0), mapi(0), abort( false )
{
}

OTSyncObject::~OTSyncObject()
{
    TRACE(OutlookSyncPlugin) << "OTSyncObject::~OTSyncObject";
    LOG() << "mapi" << mapi;
    if ( mapi ) {
        delete mapi;
        mapi = 0;
    }

    DesktopSettings settings(q->id());
    LOG() << "SAVING rememberedIds" << rememberedIds;
    settings.setValue( "rememberedIds", rememberedIds );
}

void OTSyncObject::logon()
{
    TRACE(OutlookSyncPlugin) << "OTSyncObject::logon";
    bool ok = false;

    if ( !q ) {
        WARNING() << "BUG: Cannot call OTSyncObject::logon() while q is 0!";
        return;
    }

    LOG() << "q" << q;
    LOG() << "q->id()" << q->id();
    LOG() << "DesktopSettings settings(q->id())";
    DesktopSettings settings(q->id());
    LOG() << "settings.value(\"rememberedIds\")";
    rememberedIds = settings.value( "rememberedIds" ).toStringList();
    LOG() << rememberedIds;

    if ( o->logon() ) {
        LOG() << "mapi = new QMAPI::Session()";
        mapi = new QMAPI::Session();
        LOG() << "mapi" << mapi << "connected" << mapi->connected();
        if ( !mapi->connected() ) {
            delete mapi;
            mapi = 0;
            LOG() << "Can't connect to MAPI";
            // TODO there should be a non-connection-specific hint system to plug into
            QCopEnvelope e( "QD/Connection", "setHint(QString,QString)" );
            e << tr("Can't connect to MAPI") << (QString)QLatin1String("mapi");
        }
        ok = true;
    }

    LOG() << "emit logonDone()";
    emit logonDone( ok );
}

void OTSyncObject::waitForAbort()
{
    TRACE(OutlookSyncPlugin) << "OTSyncObject::abort";
}

void OTSyncObject::fetchChangesSince( const QDateTime &timestamp )
{
    TRACE(OutlookSyncPlugin) << "OTSyncObject::fetchChangesSince" << "timestamp" << timestamp;
    QStringList leftover = rememberedIds;
    Outlook::MAPIFolderPtr folder = o->ns->GetDefaultFolder(q->folderEnum());
    Outlook::_ItemsPtr items = folder->GetItems();
    int max = items->GetCount();
    LOG() << "iterating over" << max << "items";
    QStringList seenIds;
    if ( abort ) return;
    for ( int i = 0; i < max; i++ )  {
        QString id;
        QDateTime lm;
        long item_to_get = i+1;
        q->getProperties( items->Item(item_to_get), id, lm );
        LOG() << "I've seen id" << id << "last modified" << lm;
        seenIds << id;
        if ( lm >= timestamp ) {
            QBuffer buffer;
            buffer.open( QIODevice::WriteOnly );
            {
                // scoped to ensure everything gets flushed out
                QTextStream stream( &buffer );
                q->dump_item(items->Item(item_to_get), stream);
            }
            LOG() << "remembered?" << (bool)rememberedIds.contains(id);
            if ( timestamp.isNull() || !rememberedIds.contains(id) ) {
                emit createServerRecord( buffer.buffer() );
            } else {
                emit replaceServerRecord( buffer.buffer() );
            }
            if ( rememberedIds.contains(id) )
                leftover.removeAt( leftover.indexOf(id) );
        }
        if ( abort ) return;
    }
    if ( !timestamp.isNull() ) {
        foreach ( const QString &id, leftover ) {
            emit removeServerRecord( id );
            if ( abort ) return;
        }
    }
    rememberedIds = seenIds;
    LOG() << "rememberedIds" << rememberedIds;
    emit serverChangesCompleted();
}

void OTSyncObject::updateClientRecord(const QByteArray &record)
{
    TRACE(OutlookSyncPlugin) << "OTSyncObject::updateClientRecord";
    LOG() << "record" << record;

    QString clientid;
    bool local;
    bool ok = q->getIdentifier( record, clientid, local );
    LOG() << "getIdentifier() returned" << "clientid" << clientid << "local" << local;
    Q_ASSERT( ok );

    QString entryid;

    if ( local ) {
        entryid = clientid;
        LOG() << "This is a known record with entryid" << entryid;
    } else {
        LOG() << "This is an unknown record with clientid" << clientid;
    }

    IDispatchPtr dispatch = findItem( entryid );

    entryid = q->read_item( dispatch, record );
    LOG() << "read_item() returned entryid" << entryid;
    if ( entryid.isEmpty() ) {
        WARNING() << "BUG: Could not add/update item!!!";
        return;
    }

    if ( !local ) {
        rememberedIds << entryid;
        LOG() << "rememberedIds" << rememberedIds;
        emit mappedId( entryid, clientid );
    }
}

void OTSyncObject::removeClientRecord(const QString &identifier)
{
    TRACE(OutlookSyncPlugin) << "OTSyncObject::removeClientRecord" << "identifier" << identifier;
    Outlook::MAPIFolderPtr folder = o->ns->GetDefaultFolder(q->folderEnum());
    Outlook::_ItemsPtr items = folder->GetItems();
    int max = items->GetCount();
    bool found = false;
    for ( int i = 0; i < max; ++i ) {
        QString id;
        QDateTime lm;
        long item_to_get = i+1;
        q->getProperties( items->Item(item_to_get), id, lm );
        if ( id == identifier ) {
            found = true;
            q->delete_item( items->Item(item_to_get) );
            rememberedIds.removeAt( rememberedIds.indexOf(id) );
            LOG() << "rememberedIds" << rememberedIds;
            break;
        }
    }
    if ( !found )
        LOG() << "Could not find item" << identifier << "to remove!";
}

IDispatchPtr OTSyncObject::findItem( const QString &entryid )
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::findItem" << "entryid" << entryid;

    IDispatchPtr disp;

    if ( !entryid.isEmpty() ) {
        Outlook::MAPIFolderPtr folder = o->ns->GetDefaultFolder(q->folderEnum());
        Outlook::_ItemsPtr items = folder->GetItems();
        int max = items->GetCount();
        for ( int i = 0; i < max; i++ )  {
            QString id;
            QDateTime lm;
            long item_to_get = i+1;
            q->getProperties( items->Item(item_to_get), id, lm );
            if ( entryid == id ) {
                disp = items->Item(item_to_get);
                break;
            }
        }
    }

    if ( disp ); else {
        LOG() << "Item not found... creating a new item";
        LOG() << "disp = o->ap->CreateItem(" << q->itemEnum() << ")";
        disp = o->ap->CreateItem(q->itemEnum());
        LOG() << "init_item( disp )";
        q->init_item( disp );
    }

    return disp;
}

// Transaction support is not implemented
void OutlookSyncPlugin::beginTransaction(const QDateTime & /*timestamp*/)
{
}

void OutlookSyncPlugin::abortTransaction()
{
}

void OutlookSyncPlugin::commitTransaction()
{
}


