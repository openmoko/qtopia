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

#include <qtopia/comm/qbluetoothdeviceselector.h>
#include <qtopia/comm/qsdap.h>

#include <QDialog>

class QContent;
class QAction;
class QDocumentSelector;
class QBluetoothDeviceSelector;
class QFile;


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
    void sendFileRequest(const QContent &doc);

private:
    QBluetoothDeviceSelector *m_deviceSelector;

    QDialog *m_fileSelector;
    QSDAP m_vcardSDAP;            // SDAP for querying the m_vcardDevice
    QDocumentSelector *m_docSelector;
};

#endif
