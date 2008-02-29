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

#ifndef QHARDWAREMANAGER_H
#define QHARDWAREMANAGER_H

// Local includes
#include "qabstractipcinterfacegroupmanager.h"

// ============================================================================
//
// QHardwareManager
//
// ============================================================================

class QTOPIA_EXPORT QHardwareManager : public QAbstractIpcInterfaceGroupManager
{
    Q_OBJECT
public:
    explicit QHardwareManager( QObject *parent = 0 );
    ~QHardwareManager();

    QStringList accessoryIds() const;
    QStringList accessoryTypes( const QString& id ) const;

signals:
    void accessoriesChanged();
    void accessoryAdded( const QString& id );
    void accessoryRemoved( const QString& id );
};

#endif //QHARDWAREMANAGER_H
