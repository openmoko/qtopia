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
#ifndef IPCONFIGIMPL_H
#define IPCONFIGIMPL_H

#include <QWidget>
#include <qtopianetworkinterface.h>
#include <qtopiaglobal.h>

class QCheckBox;
class QLabel;
class QLineEdit;
class QGroupBox;

class QTOPIACOMM_EXPORT IPPage : public QWidget
{
    Q_OBJECT
public:
    explicit IPPage( const QtopiaNetworkProperties& cfg, QWidget* parent = 0, Qt::WFlags flags = 0 );
    virtual ~IPPage();

    QtopiaNetworkProperties properties();
private slots:
    void connectWdgts();

private:
    void init();

    void readConfig( const QtopiaNetworkProperties& prop);

private:
    QCheckBox* autoIp;
    QGroupBox* dhcpGroup;
    QLabel* ipLabel;
    QLineEdit* ipAddress;

    QLabel* dnsLabel1, *dnsLabel2;
    QLineEdit* dnsAddress1, *dnsAddress2;

    QLabel* broadcastLabel;
    QLineEdit* broadcast;
    QLabel* gatewayLabel;
    QLineEdit* gateway;
    QLabel* subnetLabel;
    QLineEdit* subnet;


};
#endif // IPCONFIGIMPL_H

