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
#ifndef __REMOTEDEVICESDIALOG_H__
#define __REMOTEDEVICESDIALOG_H__

#include <qbluetoothsdpquery.h>

#include <QDialog>

class QContent;
class QAction;
class QDocumentSelector;
class QFile;
class MyDeviceDialog;

class RemoteDevicesWindow : public QDialog
{
    Q_OBJECT

public:
    RemoteDevicesWindow(QWidget *parent = 0, Qt::WFlags fl=0);
    ~RemoteDevicesWindow();

public slots:
    void start();
    void sendVCard();
    void sendFile();

private:
    MyDeviceDialog *m_deviceDialog;
    QBluetoothSdpQuery m_vcardSDAP;            // SDAP for querying the m_vcardDevice
};

#endif
