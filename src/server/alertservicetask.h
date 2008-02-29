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

#ifndef _ALERTSERVICETASK_H_
#define _ALERTSERVICETASK_H_

#include <qtopiaabstractservice.h>

class AlertService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    AlertService( QObject *parent );
    ~AlertService();

public slots:
    virtual void soundAlert() = 0;
};

class AlertServiceTask : public AlertService
{
Q_OBJECT
public:
    AlertServiceTask();
    virtual void soundAlert();

private slots:

    void playDone();
};

#endif // _ALERTSERVICETASK_H_
