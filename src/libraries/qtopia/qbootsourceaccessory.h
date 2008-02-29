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

#ifndef QBOOTSOURCEACCESSORY_H
#define QBOOTSOURCEACCESSORY_H

// Local includes
#include "qhardwareinterface.h"

// ============================================================================
//
// QBootSourceAccessory
//
// ============================================================================

class QTOPIA_EXPORT QBootSourceAccessory : public QHardwareInterface
{
    Q_OBJECT
public:
    enum Source
    {
        Unknown = 0,
        PowerKey = 1,
        Charger = 2,
        Alarm = 3,
        Watchdog = 4,
        Software = 5
    };

    explicit QBootSourceAccessory( const QString& id = QString(), QObject *parent = 0,
                                   QAbstractIpcInterface::Mode mode = Client );
    ~QBootSourceAccessory();

    QBootSourceAccessory::Source bootSource() const;

signals:
    void bootSourceModified();
};

// ============================================================================
//
// QBootSourceAccessoryProvider
//
// ============================================================================

class QTOPIA_EXPORT QBootSourceAccessoryProvider : public QBootSourceAccessory
{
    Q_OBJECT
public:
    explicit QBootSourceAccessoryProvider( const QString& id, QObject *parent = 0 );
    ~QBootSourceAccessoryProvider();

public slots:
    void setBootSource( QBootSourceAccessory::Source source );
};

#endif //QBOOTSOURCEACCESSORY_H
