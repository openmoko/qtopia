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

#ifndef SCREENSAVERSERVICE_H
#define SCREENSAVERSERVICE_H

#include <qtopiaabstractservice.h>
#include "qtopiapowermanager.h"

class QtopiaPowerManager;

class QtopiaPowerManagerService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    QtopiaPowerManagerService( QtopiaPowerManager *manager, QObject *parent );
    ~QtopiaPowerManagerService();

public slots:
    void setIntervals( int dim, int lightOff, int suspend );
    void setDefaultIntervals();
    void setBacklight( int brightness );
    void setConstraint( int hint, QString);
    void setActive( bool on);

private:
    QtopiaPowerManager *m_powerManager;
};

#endif
