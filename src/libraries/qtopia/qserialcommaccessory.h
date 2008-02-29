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

#ifndef QSERIALCOMMACCESSORY_H
#define QSERIALCOMMACCESSORY_H

// Local includes
#include "qhardwareinterface.h"

// ============================================================================
//
// QSerialCommAccessory
//
// ============================================================================

class QTOPIA_EXPORT QSerialCommAccessory : public QHardwareInterface
{
    Q_OBJECT

public:
    explicit QSerialCommAccessory( const QString& id = QString(),
                                   QObject *parent = 0,
                                   QAbstractIpcInterface::Mode mode = Client );
    ~QSerialCommAccessory();

    enum ConnectionType { Invalid, RS232, USB, FireWire };

    ConnectionType connectionType() const;
};

// ============================================================================
//
// QSerialCommAccessoryProvider
//
// ============================================================================

class QTOPIA_EXPORT QSerialCommAccessoryProvider
:   public QSerialCommAccessory
{
    Q_OBJECT

public:
    explicit QSerialCommAccessoryProvider( const QString& id, QObject *parent = 0 );
    ~QSerialCommAccessoryProvider();

public slots:
    void setConnectionType( const ConnectionType connectionType );
};

#endif //QSERIALCOMMACCESSORY_H
