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

#ifndef CONTENTSERVER_H
#define CONTENTSERVER_H

#include <QThread>
#include <QStringList>
#include <QSet>
#include <QTimer>

#include <qcontent.h>
#include <qcontentset.h>
#include <qtopiaipcadaptor.h>
#include "qtopiaserverapplication.h"
#include <qtopia/private/contentserverinterface_p.h>

class ServerInterface;
class AppLoaderPrivate;
class QValueSpaceObject;

class ContentServer : public ContentServerInterface
{
    Q_OBJECT
public:
    static ContentServer *getInstance();

    ContentServer( QObject *parent = 0 );
    ~ContentServer();
    virtual void run();

    void removeContent(QContentIdList cids);
    void addContent(const QFileInfo &fi);

private slots:
    void initDocScan();
    void processContent();

private:
    QContentIdList removeList;
    QList<QFileInfo> addList;
    QTimer contentTimer;
    QValueSpaceObject *scannerVSObject;
    friend class ContentProcessor;
    friend class DirectoryScannerManager;
};

/*!
  \internal
  \class RequestQueue
  Models all registered QContentSet objects and on receipt of requests to
  update the database from new path information, forwards updates to those
  objects which are affected.
*/
class RequestQueue : public QtopiaIpcAdaptor
{
    Q_OBJECT
public:
    RequestQueue();
    ~RequestQueue();
public slots:
    //void newRequest( const QString &msg, const QByteArray &data );
    void receiveUpdate();
    void reflectMirror( QContentId cid, QContent::ChangeType ctype );
    void scanPath(const QString &newPath, int priority);
    void registerCLS(const QString& CLSid, const QContentFilter& filters, int index);
    void unregisterCLS(const QString& CLSid, int index);
private:
    friend class ContentServer;
    QContentSet *cls;
    QHash<QString, QContentSet *> mirrors;
    QHash<QContentSet *, QString> mirrorsByPtr;
};

// declare ContentServerTask
class ContentServerTask : public SystemShutdownHandler
{
Q_OBJECT
public:
    ContentServerTask();

    virtual bool systemRestart();
    virtual bool systemShutdown();

private slots:
    void finished();

private:
    void doShutdown();

    bool m_finished;
};

#endif
