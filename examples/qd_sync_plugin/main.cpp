/****************************************************************************
 **
 ** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#include <qdplugin.h>

class MySyncPlugin : public QDServerSyncPlugin
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(MySyncPlugin,QDServerSyncPlugin)
public:
    QString id() { return "com.myorg.mysyncplugin"; }
    QString displayName() { return tr("MySyncPlugin"); }

    QString dataset() { return "mydataset"; }
    //QByteArray referenceSchema() { return QByteArray(); }
    void prepareForSync()
    {
        emit readyForSync( true );
    }

    void finishSync()
    {
        emit finishedSync();
    }

    void fetchChangesSince( const QDateTime & /*timestamp*/ )
    {
        // emit createServerRecord(QByteArray)
        // emit replaceServerRecord(QByteArray)
        // emit deleteServerRecord(QString)

        // emit serverChangesCompleted()
    }

    void beginTransaction( const QDateTime & /*timestamp*/ )
    {
        // set a point for roll back
    }

    void createClientRecord( const QByteArray & /*record*/ )
    {
        // store record
        // emit mappedId(localId,clientId)
    }

    void replaceClientRecord( const QByteArray & /*record*/ )
    {
        // store record
    }

    void removeClientRecord( const QString & /*localId*/ )
    {
        // remove record
    }

    void abortTransaction()
    {
        // revert any create/replace/delete events since beginTransaction()
    }

    void commitTransaction()
    {
        // commit any create/replace/delete events since beginTransaction()
    }
};

QD_REGISTER_PLUGIN(MySyncPlugin)
