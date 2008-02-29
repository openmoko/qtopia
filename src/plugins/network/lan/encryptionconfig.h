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

#ifndef WIRELESS_ENCRYPTION_CONFIG
#define WIRELESS_ENCRYPTION_CONFIG

#include <custom.h>

#ifndef NO_WIRELESS_LAN

#include <QWidget>
#include <qtopianetworkinterface.h>

#include "ui_wirelessencryptbase.h"

class WirelessEncryptionPage : public QWidget
{
    Q_OBJECT
public:
    WirelessEncryptionPage( const QtopiaNetworkProperties& cfg, QWidget* parent = 0, Qt::WFlags flags = 0 );
    ~WirelessEncryptionPage();
    QtopiaNetworkProperties properties();

    void setProperties( const QtopiaNetworkProperties& cfg );

private:
    void init( const QtopiaNetworkProperties& cfg );
    void readConfig();
    void saveConfig();

private slots:
    void newNetSelected(int idx);
    void selectEncryptAlgorithm( int index );
    void selectEncryptType( int index );
    void checkPassword();
    void wpaEnterpriseChanged(int index);

private:
    Ui::WirelessEncryptionBase ui;
    QtopiaNetworkProperties props;
    int lastIndex;
};

#endif // NO_WIRELESS_LAN
#endif //WIRELESS_ENCRYPTION_CONFIG
