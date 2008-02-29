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

#ifndef QABSTRACTIPCINTERFACEGROUPMANAGER_H
#define QABSTRACTIPCINTERFACEGROUPMANAGER_H

#include <qtopiaglobal.h>

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

class QAbstractIpcInterfaceGroupManagerPrivate;

class QTOPIABASE_EXPORT QAbstractIpcInterfaceGroupManager : public QObject
{
    Q_OBJECT
public:
    explicit QAbstractIpcInterfaceGroupManager
        ( const QString& valueSpaceLocation, QObject *parent = 0 );
    ~QAbstractIpcInterfaceGroupManager();

    QStringList groups() const;
    QStringList interfaces( const QString& group ) const;

    template <typename T> QStringList supports() const
        { return supports
            ( reinterpret_cast<T *>(0)->staticMetaObject.className() ); }

    template <typename T> bool supports( const QString& group ) const
        { return supports
            ( group,
              reinterpret_cast<T *>(0)->staticMetaObject.className() ); }

    template <typename T> int priority( const QString& group ) const
        { return priority
            ( group,
              reinterpret_cast<T *>(0)->staticMetaObject.className() ); }

signals:
    void groupsChanged();
    void groupAdded( const QString& group );
    void groupRemoved( const QString& group );

protected:
    void connectNotify( const char *signal );

private slots:
    void groupsChangedInner();

private:
    QAbstractIpcInterfaceGroupManagerPrivate *d;

    QStringList supports( const QString& iface ) const;
    bool supports( const QString& group, const QString& iface ) const;
    int priority( const QString& group, const QString& iface ) const;
};

#endif /* QABSTRACTIPCINTERFACEGROUPMANAGER_H */
