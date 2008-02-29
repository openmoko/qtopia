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

#ifndef QABSTRACTIPCINTERFACEGROUP_H
#define QABSTRACTIPCINTERFACEGROUP_H

#include <qabstractipcinterface.h>

class QAbstractIpcInterfaceGroupPrivate;

class QTOPIABASE_EXPORT QAbstractIpcInterfaceGroup : public QObject
{
    Q_OBJECT
public:
    explicit QAbstractIpcInterfaceGroup( const QString& groupName, QObject *parent = 0 );
    ~QAbstractIpcInterfaceGroup();

    QString groupName() const;

    virtual void initialize();

    template <typename T> bool supports() const
        { return _supports
            ( reinterpret_cast<T *>(0)->staticMetaObject.className() ); }
    template <typename T> T *interface() const
        { return qobject_cast<T *>( _interface
            ( reinterpret_cast<T *>(0)->staticMetaObject.className() ) ); }

protected:
    virtual void addInterface( QAbstractIpcInterface *interface );
    template <typename T> void suppressInterface()
        { _suppressInterface
            ( reinterpret_cast<T *>(0)->staticMetaObject.className() ); }

private slots:
    void interfaceDestroyed();

private:
    bool _supports( const char *interfaceName ) const;
    QAbstractIpcInterface *_interface( const char *interfaceName ) const;
    void _suppressInterface( const char *interfaceName );

    QAbstractIpcInterfaceGroupPrivate *d;
};

#endif /* QABSTRACTIPCINTERFACEGROUP_H */
