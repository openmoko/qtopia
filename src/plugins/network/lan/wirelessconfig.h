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
#ifndef WIRELESSIMPL_H
#define WIRELESSIMPL_H

#include <custom.h>

#ifndef NO_WIRELESS_LAN

#include <QWidget>
#include <qtopianetworkinterface.h>

#include "ui_wirelessbase.h"

class WirelessPage : public QWidget
{
    Q_OBJECT
public:
    WirelessPage( const QtopiaNetworkProperties& cfg, QWidget* parent = 0, Qt::WFlags flags = 0 );
    ~WirelessPage();

    QtopiaNetworkProperties properties();
    void setProperties( const QtopiaNetworkProperties& cfg );

private slots:
    void changeChannelMode( int index );
    void newNetSelected( int index );
    void setNewNetworkTitle( const QString& );

    void removeWLAN();
    void addWLAN();

private:
    void init();
    void initNetSelector( const QtopiaNetworkProperties& prop );
    void readConfig( );
    void saveConfig( );

private:
    Ui::WirelessBase ui;
    QtopiaNetworkProperties changedSettings;
    int lastIndex;

};
#endif //NO_WIRELESS_LAN

#endif // WIRELESSIMPL_H

