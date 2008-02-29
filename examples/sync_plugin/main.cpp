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

#include <qtopiaglobal.h>
#include <qtopia4sync.h>

class MySyncPlugin : public Qtopia4SyncPlugin
{
    Q_OBJECT
public:
    MySyncPlugin( QObject *parent = 0 )
        : Qtopia4SyncPlugin( parent )
    {
    }

    ~MySyncPlugin()
    {
    }

    QString dataset() { return "mydataset"; }

    void fetchChangesSince( const QDateTime & /*timestamp*/ )
    {
        // emit createClientRecord(QByteArray)
        // emit replaceClientRecord(QByteArray)
        // emit deleteClientRecord(QString)

        // emit clientChangesCompleted()
    }

    void beginTransaction( const QDateTime & /*timestamp*/ )
    {
        // set a point for roll back
    }

    void createServerRecord( const QByteArray & /*record*/ )
    {
        // store record
        // emit mappedId(serverId,localId)
    }

    void replaceServerRecord( const QByteArray & /*record*/ )
    {
        // store record
    }

    void removeServerRecord( const QString & /*localId*/ )
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

QTOPIA_EXPORT_PLUGIN(MySyncPlugin)
