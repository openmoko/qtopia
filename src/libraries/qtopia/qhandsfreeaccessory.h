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

#ifndef QHANDSFREEACCESSORY_H
#define QHANDSFREEACCESSORY_H

// Local includes
#include "qhardwareinterface.h"

// ============================================================================
//
// QHandsfreeAccessory
//
// ============================================================================

class QTOPIA_EXPORT QHandsfreeAccessory : public QHardwareInterface
{
    Q_OBJECT

public:
    explicit QHandsfreeAccessory( const QString& id = QString(),
                                  QObject *parent = 0,
                                  QAbstractIpcInterface::Mode mode = Client );
    ~QHandsfreeAccessory();

    enum Mode { NotSupported, Portable, Vehicle };

    Mode mode() const;
};

// ============================================================================
//
// QHandsfreeAccessoryProvider
//
// ============================================================================

class QTOPIA_EXPORT QHandsfreeAccessoryProvider
:   public QHandsfreeAccessory
{
    Q_OBJECT

public:
    explicit QHandsfreeAccessoryProvider( const QString& id, QObject *parent = 0 );
    ~QHandsfreeAccessoryProvider();

public slots:
    void setMode( const Mode mode );
};

#endif //QHANDSFREEACCESSORY_H
