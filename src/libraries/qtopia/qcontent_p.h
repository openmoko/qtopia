/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef QCONTENT_P_H__
#define QCONTENT_P_H__
#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QAtomic>
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

class QContentUpdateManagerPrivate;
class QContentUpdateManager : public QObject
{
    Q_OBJECT
    public:
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

    private slots:
        void sendUpdate();

    private:
        QContentUpdateManager(QObject *parent=NULL);
        friend class QContentUpdateManagerPrivate;
        QList<QPair<QContentId,QContent::ChangeType> > updateList;
        QTimer updateTimer;
        QMutex mutex;
        QAtomic installAtom;
        QAtomic updateAtom;
        QValueSpaceProxyObject *vsoDocuments;
};

#endif
