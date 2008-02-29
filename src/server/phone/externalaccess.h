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

#ifndef EXTERNALACCESS_H
#define EXTERNALACCESS_H

#include <QObject>
#include "qtopiaserverapplication.h"

class QCommServiceManager;

class ExternalAccess : public QObject
{
    Q_OBJECT
public:
    ExternalAccess( QObject *parent = 0 );
    ~ExternalAccess();

private slots:
    void servicesChanged();
    void start();

private:
    QCommServiceManager *manager;
    bool started;
};

QTOPIA_TASK_INTERFACE(ExternalAccess);

#endif // EXTERNALACCESS_H
