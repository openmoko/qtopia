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
#ifndef PROXIESSETTINGS_H
#define PROXIESSETTINGS_H

#include <QFrame>
#include <qtopianetworkinterface.h>
#include <qtopiaglobal.h>

class ProxiesPageBaseContainer;

class QTOPIACOMM_EXPORT ProxiesPage : public QWidget {
    Q_OBJECT

public:
    explicit ProxiesPage( const QtopiaNetworkProperties& cfg,
                          QWidget* parent = 0, Qt::WFlags flags = 0 );
    virtual ~ProxiesPage();

    QtopiaNetworkProperties properties();

private slots:
    void typeChanged(int);
private:
    void readConfig( const QtopiaNetworkProperties& prop);

private:
    ProxiesPageBaseContainer* d;
};

#endif

