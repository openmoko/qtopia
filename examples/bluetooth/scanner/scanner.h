/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef __SCANNER_H__
#define __SCANNER_H__

#include <QMainWindow>
#include <QString>
#include <QBluetoothRemoteDevice>

class QAction;
class QBluetoothLocalDevice;
class QBluetoothRemoteDevice;
class QListWidget;
class QListWidgetItem;

class Scanner : public QMainWindow
{
    Q_OBJECT

public:
    Scanner(QWidget *parent = 0, Qt::WFlags f = 0);
    ~Scanner();

private slots:
    void discoveryStarted();
    void discoveryComplete();
    void remoteDeviceFound(const QBluetoothRemoteDevice &dev);
    void itemActivated(QListWidgetItem *item);

private:
    QAction *startScan;
    QBluetoothLocalDevice *btDevice;
    QListWidget *deviceList;
    QMap<QString, QBluetoothRemoteDevice> deviceInfo;
};

#endif
