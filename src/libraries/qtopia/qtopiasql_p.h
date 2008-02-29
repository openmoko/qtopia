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

#ifndef _QPESQL_P_H_
#define _QPESQL_P_H_

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
#include <QThread>
#include <QMutex>
#include <QHash>
#include <QMap>
#include <QStorageMetaInfo>
#include <QtopiaSql>
#include <QHash>

class QtopiaSqlPrivate : public QObject {
    Q_OBJECT
    public:
        QtopiaSqlPrivate();
        ~QtopiaSqlPrivate() {}

        static QtopiaSqlPrivate *instance();

        QString databaseFile( const QString &path );
#ifndef QTOPIA_HOST
        void connectDiskChannel();
#endif

    private:
        QString type;
        QString name;
        QString user;
        QString password;
        QString hostname;
        QSqlDatabase *defaultConn;
        QHash<QtopiaDatabaseId, QSqlDatabase> masterAttachedConns;
        QHash<QtopiaDatabaseId, QString> dbPaths;
        QMap<Qt::HANDLE, QHash<QtopiaDatabaseId, QSqlDatabase> > dbs;
        QMap<Qt::HANDLE, QHash<QtopiaDatabaseId, QString> > connectionNames;
        QMutex guardMutex;
        quint32 conId;
        QSqlDatabase nullDatabase;
        void installSorting( QSqlDatabase &db);
        friend class QtopiaSql;
    private slots:
        void disksChanged ();
        void threadTerminated();
        void cardMessage(const QString &message,const QByteArray &data);
        void systemMessage(const QString &message,const QByteArray &data);
};

#endif
