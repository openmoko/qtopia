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

#ifndef CONTENTSERVERINTERFACE_P_H
#define CONTENTSERVERINTERFACE_P_H

#include <qcontentset.h>

#include <QThread>
#include <QMutex>

/*!
  \internal
  \class ContentServerInterface
  Pure virtual base class to provide interface for calling server.
*/
class QTOPIA_EXPORT ContentServerInterface : public QThread
{
public:
    explicit ContentServerInterface( QObject *parent = 0 );
    virtual ~ContentServerInterface();
    // implementations must reimplement QThread::run()

    static bool inServerProcess();
    static ContentServerInterface *server();
    static void addCLS( QContentSet * );
    static void removeCLS( QContentSet * );

    static QList<QContentSet *> &sets();
protected:
    static ContentServerInterface *serverInstance;
    static QMutex listMutex;
    static QList<QContentSet *> linkSets;
};

#endif
