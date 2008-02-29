/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef _QFILEMONITOR_H_
#define _QFILEMONITOR_H_

#include "qtopiailglobal.h"
#include <QObject>

class QFileMonitorPrivate;
class QTOPIAIL_EXPORT QFileMonitor : public QObject
{
Q_OBJECT
public:
    enum Strategy { Auto, DNotify, INotify, Poll, None };

    explicit QFileMonitor(QObject * = 0);
    explicit QFileMonitor(const QString &, Strategy = Auto, QObject * = 0);
    virtual ~QFileMonitor();

    bool isValid() const;
    QString fileName() const;
    Strategy strategy() const;

signals:
    void fileChanged(const QString &);

private:
    friend class QFileMonitorPrivate;
    QFileMonitorPrivate * d;
};

#endif // _QFILEMONITOR_H_
