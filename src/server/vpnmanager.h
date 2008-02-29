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

#ifndef VPN_MANAGER_H
#define VPN_MANAGER_H

#ifdef QTOPIA_VPN
#include <QObject>

class QVpnManager;
class QtopiaVpnManager : public QObject
{
public:
    QtopiaVpnManager(QObject * parent = 0);
    ~QtopiaVpnManager();

private:
    QVpnManager* manager;
};
#endif //QTOPIA_VPN

#endif //VPN_MANAGER_H
