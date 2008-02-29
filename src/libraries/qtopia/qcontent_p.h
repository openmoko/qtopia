/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef QCONTENT_P_H__
#define QCONTENT_P_H__

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
#include <QObject>
#include <QTimer>
#include <QMutex>
#include <qatomic.h>
#include <qcontent.h>

class QValueSpaceObject;

class QValueSpaceProxyObject : public QObject
{
    Q_OBJECT
    public:
        explicit QValueSpaceProxyObject( const QString & objectPath, QObject * parent = 0 );
        virtual ~QValueSpaceProxyObject();
        QString objectPath () const;
        static void sync ();
        void setAttribute ( const QByteArray & attribute, const QVariant & data );
        void setAttribute ( const char * attribute, const QVariant & data );
        void setAttribute ( const QString & attribute, const QVariant & data );
        void removeAttribute ( const QString & attribute );

    signals:
        void itemRemove ( const QByteArray & attribute );
        void itemSetValue ( const QByteArray & attribute, const QVariant & value );
        void doInit(const QString & objectPath);
        void doInternalSetAttribute(const QString& attribute, const QVariant &data);
        void doInternalremoveAttribute ( const QString & attribute );

    private slots:
        void init( const QString & objectPath );

    private:
        QValueSpaceObject *d;
        QString path;
};

class QContentUpdateManager : public QObject
{
    Q_OBJECT
    public:
        QContentUpdateManager(QObject *parent=NULL);
        void addUpdated(QContentId id, QContent::ChangeType c);
        void requestRefresh();

        static QContentUpdateManager *instance();
        
    signals:
        void refreshRequested();

    public slots:
        void beginInstall();
        void endInstall();
        void beginSendingUpdates();
        void endSendingUpdates();
        void sendUpdate();

    private:

        QList<QPair<QContentId,QContent::ChangeType> > updateList;
        QTimer updateTimer;
        QMutex mutex;
#if QT_VERSION < 0x040400
        QAtomic installAtom;
        QAtomic updateAtom;
#else
        QAtomicInt installAtom;
        QAtomicInt updateAtom;
#endif
        QValueSpaceProxyObject *vsoDocuments;
};

#endif
