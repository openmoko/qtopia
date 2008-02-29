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

#ifndef _DEVICELEDS_H_
#define _DEVICELEDS_H_

#include <QObject>
#include <QDeviceIndicators>

class QDeviceIndicatorsProviderPrivate;
class QDeviceIndicatorsProvider : public QObject
{
Q_OBJECT
public:
    QDeviceIndicatorsProvider(QObject *parent = 0);

protected:
    void setSupportedIndicators(const QStringList &);
    void setIndicatorState(const QString &, QDeviceIndicators::IndicatorState);

    virtual void changeIndicatorState(const QString &,
                                      QDeviceIndicators::IndicatorState) = 0;

private slots:
    void itemSetValue(const QString &attribute, const QVariant &data);

private:
    QDeviceIndicatorsProviderPrivate *d;
};

#endif // _DEVICELEDS_H_
