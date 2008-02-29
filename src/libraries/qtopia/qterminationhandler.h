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

#ifndef _QTERMINATIONHANDLER_H_
#define _QTERMINATIONHANDLER_H_

#include <QObject>
#include <QtopiaServiceRequest>
#include <QString>

struct QTerminationHandlerPrivate;
struct QTerminationHandlerData;
class QTOPIA_EXPORT QTerminationHandler : public QObject
{
public:
    explicit QTerminationHandler(const QString &text,
                                 const QString &buttonText = QString(),
                                 const QString &icon = QString(),
                                 const QtopiaServiceRequest &action = QtopiaServiceRequest(),
                                 QObject *parent = 0);
    explicit QTerminationHandler(const QtopiaServiceRequest &action,
                                 QObject *parent = 0);
    ~QTerminationHandler();

private:
    void installHandler(const QTerminationHandlerData& data);
    static QTerminationHandlerPrivate* staticData();
};

#endif // _QTERMINATIONHANDLER_H_
