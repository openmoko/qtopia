/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#ifndef __BTAUDIOVOLUMEMANAGER_P_H__
#define __BTAUDIOVOLUMEMANAGER_P_H__

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>

class BluetoothAudioVolumeControl;
class QBluetoothAddress;

class BtAudioVolumeManager : public QObject
{
    Q_OBJECT

public:
    BtAudioVolumeManager(const QString &service, QObject *parent = 0);
    ~BtAudioVolumeManager();

private slots:
    void serviceAdded(const QString &service);
    void serviceRemoved(const QString &service);

    void audioGatewayConnected(bool success, const QString &msg);
    void audioGatewayDisconnected();
    void audioDeviceConnected(const QBluetoothAddress &addr);
    void audioStateChanged();

private:
    void createVolumeControl();
    void removeVolumeControl();

    QString m_service;
    BluetoothAudioVolumeControl *m_volumeControl;
};


#endif
