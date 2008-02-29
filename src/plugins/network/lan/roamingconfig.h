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

#ifndef ROAMING_PAGE
#define RAOMING_PAGE

#ifndef NO_WIRELESS_LAN
#include <QMultiHash>
#include <QWidget>
#include <QObject>

#include <qtopianetworkinterface.h>

#include "ui_roamingbase.h"

class RoamingPage : public QWidget
{
Q_OBJECT
public:
    RoamingPage( const QtopiaNetworkProperties& cfg, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~RoamingPage();

    QtopiaNetworkProperties properties();
    void setProperties( const QtopiaNetworkProperties& cfg );
private:
    void init( const QtopiaNetworkProperties& cfg );
    void readConfig();
    void saveConfig();

private slots:
    void up();
    void down();
    void reconnectToggled(int newState);

private:
    QMultiHash<QString, QVariant> props;
    Ui::RoamingBase ui;
};

#endif //NO_WIRELESS_LAN
#endif //ROAMING_PAGE
