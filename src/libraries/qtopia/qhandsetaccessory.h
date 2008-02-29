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

#ifndef QHANDSETACCESSORY_H
#define QHANDSETACCESSORY_H

// Local includes
#include "qhardwareinterface.h"

// ============================================================================
//
// QHandsetAccessory
//
// ============================================================================

class QTOPIA_EXPORT QHandsetAccessory : public QHardwareInterface
{
    Q_OBJECT

public:
    explicit QHandsetAccessory( const QString& id = QString(), QObject *parent = 0,
                                QAbstractIpcInterface::Mode mode = Client );
    ~QHandsetAccessory();

    enum Mode { Internal, External };

    Mode mode() const;
    bool speakerPhone() const;
    bool tty() const;
};

// ============================================================================
//
// QHandsetAccessoryProvider
//
// ============================================================================

class QTOPIA_EXPORT QHandsetAccessoryProvider : public QHandsetAccessory
{
    Q_OBJECT
public:
    explicit QHandsetAccessoryProvider( const QString& id, QObject *parent = 0 );
    ~QHandsetAccessoryProvider();

public slots:
    void setMode( const Mode mode );
    void setSpeakerPhone( const bool speakerPhone );
    void setTty( const bool tty );

};

#endif //QHANDSETACCESSORY_H
