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

#ifndef __HEADSET_H__
#define __HEADSET_H__

#include <QDialog>
#include <qtopia/comm/qbluetoothnamespace.h>
#include <qtopia/comm/qbluetoothaddress.h>
#include "ui_myheadset.h"

class QBluetoothAudioGateway;
class QWaitWidget;
class QBluetoothSdpQuery;
class QBluetoothSdpQueryResult;

class HeadsetDialog : public QDialog
{
    Q_OBJECT

public:
    HeadsetDialog(QBluetooth::SDPProfile profile, QWidget *parent = 0, Qt::WFlags fl = 0);
    ~HeadsetDialog();

    void start();

private slots:
    void speakerVolumeChanged();
    void microphoneVolumeChanged();
    void audioStateChanged();

    void searchComplete(const QBluetoothSdpQueryResult &result);

    void doDisconnect();
    void disconnected();
    void doConnect();
    void connected(bool success, const QString &msg);

    void newConnection(const QBluetoothAddress &addr);

    void changeAudio(int);

private:
    Ui::MyHeadset *m_ui;
    QBluetoothAudioGateway *m_ag;
    QWaitWidget *m_waitWidget;
    QBluetoothSdpQuery *m_sdap;
    QBluetooth::SDPProfile m_profile;
    QBluetoothAddress m_addr;
};

#endif
