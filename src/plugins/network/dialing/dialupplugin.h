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

#ifndef DIALUPPLUGIN_H
#define DIALUPPLUGIN_H

#include <qtopianetworkinterface.h>
#include <qtopiaglobal.h>

#include <QList>
#include <QPointer>

#include "dialup.h"

class QTOPIA_PLUGIN_EXPORT DialupPlugin : public QtopiaNetworkPlugin
{
public:
    DialupPlugin();
    virtual ~DialupPlugin();

    virtual QPointer<QtopiaNetworkInterface> network( const QString& confFile);
    virtual QtopiaNetwork::Type type() const;
private:
    QList<QPointer<QtopiaNetworkInterface> > instances;
};

#endif //DialupPlugin_H
