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

#include <qtopia/private/qcontent_p.h>
#include <qtopiaipcenvelope.h>
#include <QDebug>
#include <qtopialog.h>
#include <QValueSpaceObject>
#include <QCoreApplication>
#include <QThread>

QContentUpdateManager *QContentUpdateManager::manager = 0;

QContentUpdateManager::QContentUpdateManager(QObject *parent)
    : QObject(parent), mutex(QMutex::Recursive)
{
    updateTimer.setSingleShot(true);
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(sendUpdate()));
    updateTimer.setSingleShot(true);
    updateTimer.setInterval(50);
    vsoDocuments = new QValueSpaceProxyObject("/Documents", this);
}

void QContentUpdateManager::sendUpdate()
{
    QMutexLocker lock(&mutex);
    qLog(DocAPI) << "updating" << updateList.count() << "content objects";
    if (updateList.count()) {
        QContentIdList ids;
        QContent::ChangeType ct = updateList[0].second;
        QPair<QContentId,QContent::ChangeType> item;
        foreach (item, updateList) {
            if (item.second == ct) {
                ids += item.first;
            } else {
                QtopiaIpcEnvelope se("QPE/System", "contentChanged(QContentIdList,QContent::ChangeType)");
                se << ids;
                se << ct;
                ids.clear();
                ct = item.second;
                ids += item.first;
                qLog(DocAPI) << "contentChanged(QContentIdList,QContent::ChangeType)" << ids << ct;
            }
        }
        QtopiaIpcEnvelope se("QPE/System", "contentChanged(QContentIdList,QContent::ChangeType)");
        se << ids;
        se << ct;
        qLog(DocAPI) << "contentChanged(QContentIdList,QContent::ChangeType)" << ids << ct;
        updateList.clear();
    }
    qLog(DocAPI) << "updated, now has: " << updateList.count() << "content objects";
}

QContentUpdateManager *QContentUpdateManager::instance()
{
    Q_ASSERT(manager != NULL);
    return manager;
}

void QContentUpdateManager::addUpdated(QContentId id, QContent::ChangeType c)
{
    QMutexLocker lock(&mutex);
    if (id == QContent::InvalidId)
    {
        qWarning() << "Attempting to add an invalid ID to the contentset update notification list";
        return;
    }
    updateList.append(QPair<QContentId,QContent::ChangeType>(id,c));
    if (updateList.count() == 1)
    {
        QMetaObject::invokeMethod(&updateTimer, "stop");
        QMetaObject::invokeMethod(&updateTimer, "start");
    }
}

void QContentUpdateManager::requestRefresh()
{
    emit refreshRequested();
}

void QContentUpdateManager::beginInstall()
{
    if((int)installAtom==0)
        vsoDocuments->setAttribute("Installing", true);
    installAtom.ref();
}

void QContentUpdateManager::endInstall()
{
    installAtom.deref();
    if((int)installAtom==0)
        vsoDocuments->setAttribute("Installing", false);
}

void QContentUpdateManager::beginSendingUpdates()
{
    if((int)updateAtom==0)
        vsoDocuments->setAttribute("Updating", true);
    updateAtom.ref();
}

void QContentUpdateManager::endSendingUpdates()
{
    updateAtom.deref();
    if((int)updateAtom==0)
        vsoDocuments->setAttribute("Updating", false);
}

QValueSpaceProxyObject::QValueSpaceProxyObject( const QString & objectPath, QObject * parent )
    : QObject(parent), d(NULL), path(objectPath)
{
    qRegisterMetaType<QVariant>("QVariant");
    connect(this, SIGNAL(doInit(const QString &)), this, SLOT(init(const QString&)), Qt::QueuedConnection);
}

QValueSpaceProxyObject::~QValueSpaceProxyObject()
{
    if(d)
        delete d;
}

QString QValueSpaceProxyObject::objectPath () const
{
    return d ? d->objectPath() : QString();
}

void QValueSpaceProxyObject::sync ()
{
    QValueSpaceObject::sync();
}

void QValueSpaceProxyObject::init( const QString & objectPath )
{
    if(!d)
    {
        d=new QValueSpaceObject(objectPath, this);
        connect(d, SIGNAL(itemRemove(const QByteArray &)), this, SIGNAL(itemRemove(const QByteArray &)), Qt::QueuedConnection);
        connect(d, SIGNAL(itemSetValue (const QByteArray &, const QVariant &)), this, SIGNAL(itemSetValue (const QByteArray &, const QVariant &)), Qt::QueuedConnection);
        connect(this, SIGNAL(doInternalSetAttribute(const QString &, const QVariant &)), d, SLOT(setAttribute(const QString&, const QVariant&)), Qt::QueuedConnection);
        connect(this, SIGNAL(doInternalremoveAttribute(const QString &)), d, SLOT(removeAttribute(const QString&)), Qt::QueuedConnection);
    }
}


void QValueSpaceProxyObject::setAttribute ( const QByteArray & attribute, const QVariant & data )
{
    if(!d)
        emit doInit(path);
    emit doInternalSetAttribute(attribute, data);
}

void QValueSpaceProxyObject::setAttribute ( const char * attribute, const QVariant & data )
{
    if(!d)
        emit doInit(path);
    emit doInternalSetAttribute(attribute, data);
}

void QValueSpaceProxyObject::setAttribute ( const QString & attribute, const QVariant & data )
{
    if(!d)
        emit doInit(path);
    emit doInternalSetAttribute(attribute, data);
}

void QValueSpaceProxyObject::removeAttribute ( const QString & attribute )
{
    if(!d)
        emit doInit(path);
    emit doInternalremoveAttribute(attribute);
}
