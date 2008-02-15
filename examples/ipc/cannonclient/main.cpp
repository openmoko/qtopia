/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

class CannonResultListener : public QObject
{
    Q_OBJECT

public:
    CannonResultListener(QObject *parent = 0);

public slots:
    void missed();
    void hit();
};

CannonResultListener::CannonResultListener(QObject *parent)
    : QObject(parent)
{

}

void CannonResultListener::missed()
{
    qDebug() << "Cannon missed.";
    deleteLater();
}

void CannonResultListener::hit()
{
    qDebug() << "Cannon HIT!!!";
    deleteLater();
}

int main( int argc, char **argv)
{
    QtopiaApplication app(argc, argv);

    if (argc != 2) {
        qDebug() << "Please specify the cannonPower argument";
        return 0;
    }

    CannonResultListener *listener = new CannonResultListener;
    QtopiaIpcAdaptor *adaptor = new QtopiaIpcAdaptor("QPE/CannonExample");
    QtopiaIpcAdaptor::connect(adaptor, SIGNAL(missed()), listener, SLOT(missed()));
    QtopiaIpcAdaptor::connect(adaptor, SIGNAL(hit()), listener, SLOT(hit()));

    app.registerRunningTask("CannonResultListener", listener);

    adaptor->send(MESSAGE(shootCannon(int)), QString(argv[1]).toInt());

    app.exec();
    delete adaptor;
}

#include "main.moc"
