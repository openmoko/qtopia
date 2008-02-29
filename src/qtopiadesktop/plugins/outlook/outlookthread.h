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
#ifndef OUTLOOKTHREAD_H
#define OUTLOOKTHREAD_H

#include "qoutlook.h"

#include <QDThread>
#include <QStringList>

class OutlookSyncPlugin;
namespace QMAPI {
    class Session;
};
class QDateTime;
class OutlookThreadObject;

// =====================================================================

class OutlookThread : public QDThread
{
    Q_OBJECT
private:
    OutlookThread( QObject *parent = 0 );
    ~OutlookThread();

    void t_init();
    void t_quit();

public:
    static OutlookThread *getInstance( QObject *syncObject );
    OutlookThreadObject *o;
};

// =====================================================================

// This object lives on the Outlook thread
class OutlookThreadObject : public QObject
{
    Q_OBJECT
public:
    OutlookThreadObject();
    ~OutlookThreadObject();

    bool logon();

public:
    Outlook::_ApplicationPtr ap;
    Outlook::_NameSpacePtr ns;
};

#endif
