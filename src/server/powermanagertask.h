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
#ifndef HAVE_POWERMANAGERTASK_H
#define HAVE_POWERMANAGERTASK_H

#include <qtopiaapplication.h>
#include <qpowerstatus.h>

class PowerManagerTask : public QObject
{
    Q_OBJECT
public:
    PowerManagerTask(QObject *parent = 0);

protected slots:
    virtual void powerStatusChanged(const QPowerStatus &);
};

#endif
