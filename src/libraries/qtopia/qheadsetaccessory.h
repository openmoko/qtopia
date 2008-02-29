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

#ifndef QHEADSETACCESSORY_H
#define QHEADSETACCESSORY_H

// Local includes
#include "qhardwareinterface.h"

// ============================================================================
//
// QHeadsetAccessory
//
// ============================================================================

class QTOPIA_EXPORT QHeadsetAccessory : public QHardwareInterface
{
    Q_OBJECT

public:
    explicit QHeadsetAccessory( const QString& id = QString(), QObject *parent = 0,
                                QAbstractIpcInterface::Mode mode = Client );
    ~QHeadsetAccessory();

    enum ConnectionType { Internal, Wired, Bluetooth };

    int channels() const;
    bool microphonePresent() const;
    int microphoneVolume() const;
    bool speakerPresent() const;
    int speakerVolume() const;
    ConnectionType connectionType() const;

public slots:
    virtual void setMicrophoneVolume( const int volume );
    virtual void setSpeakerVolume( const int volume );

signals:
    void microphoneVolumeModified();
    void speakerVolumeModified();
};

// ============================================================================
//
// QHeadsetAccessoryProvider
//
// ============================================================================

class QTOPIA_EXPORT QHeadsetAccessoryProvider : public QHeadsetAccessory
{
    Q_OBJECT
public:
    explicit QHeadsetAccessoryProvider( const QString& id, QObject *parent = 0 );
    ~QHeadsetAccessoryProvider();

public slots:
    void setChannels( const int channels );
    void setMicrophonePresent( const bool present );
    void setSpeakerPresent( const bool present );
    void setConnectionType( const ConnectionType connection );
    void setMicrophoneVolume( const int volume );
    void setSpeakerVolume( const int volume );
};

#endif //QHEADSETACCESSORY_H
