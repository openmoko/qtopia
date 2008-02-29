/****************************************************************************
**
** Copyright (C) 2007-2007 TROLLTECH ASA. All rights reserved.
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

#include <QtopiaApplication>
#include <QDebug>
#include <QtopiaIpcAdaptor>
#include <QString>
#include <QtGlobal>
#include <cstdlib>

class CannonListener : public QtopiaIpcAdaptor
{
    Q_OBJECT

public:
    CannonListener(QObject *parent = 0);

public slots:
    void shootCannon(int);

signals:
    void missed();
    void hit();
};

CannonListener::CannonListener(QObject *parent)
    : QtopiaIpcAdaptor("QPE/CannonExample", parent)
{
    publishAll(QtopiaIpcAdaptor::SignalsAndSlots);
}

void CannonListener::shootCannon(int power)
{
    int pwr = power % 100;
    int roll = qrand() % 100;

    if (pwr >= roll)
        emit hit();
    else
        emit missed();

    deleteLater();
}

int main( int argc, char **argv)
{
    QtopiaApplication app(argc, argv);

    CannonListener *listener = new CannonListener;
    app.registerRunningTask("CannonListener", listener);

    app.exec();
}

#include "main.moc"
