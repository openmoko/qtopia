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

#ifndef _QDEVICEINDICATORS_H_
#define _QDEVICEINDICATORS_H_

#include <QObject>
#include <qtopiaglobal.h>

class QDeviceIndicatorsPrivate;
class QTOPIA_EXPORT QDeviceIndicators : public QObject
{
Q_OBJECT
public:
    explicit QDeviceIndicators(QObject *parent = 0);
    virtual ~QDeviceIndicators();

    enum IndicatorState { Off, On };

    bool isIndicatorSupported(const QString &);
    bool isIndicatorStateSupported(const QString &, IndicatorState);

    IndicatorState indicatorState(const QString &);
    void setIndicatorState(const QString &name, IndicatorState state);
    QStringList supportedIndicators() const;

signals:
    void indicatorStateChanged(const QString &name, IndicatorState newState);

private:
    QDeviceIndicatorsPrivate *d;
};

#endif // _QDEVICELEDS_H_
