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

#ifndef _TERMINATIONHANDLER_H_
#define _TERMINATIONHANDLER_H_

#include <QObject>

class QString;
class QPixmap;
class QtopiaServiceRequest;

class QTerminationHandlerProvider : public QObject
{
    Q_OBJECT
public:
    QTerminationHandlerProvider(QObject *parent = 0);
    virtual ~QTerminationHandlerProvider();

signals:
    void applicationTerminated(const QString &name, const QString &text,
                               const QPixmap &icon, const QString &buttonText,
                               QtopiaServiceRequest &buttonAction);
};

#endif
