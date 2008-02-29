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

#ifndef QINFRAREDACCESSORY_H
#define QINFRAREDACCESSORY_H

// Local includes
#include "qhardwareinterface.h"

// ============================================================================
//
// QInfraredAccessory
//
// ============================================================================

class QTOPIA_EXPORT QInfraredAccessory : public QHardwareInterface
{
    Q_OBJECT

public:
    explicit QInfraredAccessory( const QString& id = QString(),
                                 QObject *parent = 0,
                                 QAbstractIpcInterface::Mode mode = Client );
    ~QInfraredAccessory();

    enum Mode { Internal, External };

    Mode mode() const;
};

// ============================================================================
//
// QInfraredAccessoryProvider
//
// ============================================================================

class QTOPIA_EXPORT QInfraredAccessoryProvider
:   public QInfraredAccessory
{
    Q_OBJECT

public:
    explicit QInfraredAccessoryProvider( const QString& id, QObject *parent = 0 );
    ~QInfraredAccessoryProvider();

public slots:
    void setMode( const Mode mode );
};

#endif //QINFRAREDACCESSORY_H
