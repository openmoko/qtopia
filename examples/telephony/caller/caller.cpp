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

#include "caller.h"
#include <QtopiaApplication>

Caller::Caller(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    setWindowTitle(tr("Caller"));
    setupUi(this);

    QtopiaApplication::setInputMethodHint(numberToDial, QtopiaApplication::PhoneNumber);
    connect(dialButton, SIGNAL(clicked()), this, SLOT(dial()));
    connect(acceptButton, SIGNAL(clicked()), this, SLOT(accept()));

// Left-aligned for correct indentation in docs.
mgr = new QPhoneCallManager(this);
connect(mgr, SIGNAL(newCall(QPhoneCall)), this, SLOT(newCall(QPhoneCall)));

// END-MGR-CREATE

    dataDevice = 0;
}

void Caller::dial()
{
    QPhoneCall call = mgr->create("Voice");

    QDialOptions dialOptions;
    dialOptions.setNumber(numberToDial->text());

    call.dial(dialOptions);

    call.connectStateChanged(this, SLOT(stateChanged(QPhoneCall)));
}

void Caller::dialData()
{
    QPhoneCall call = mgr->create("Data");

    QDialOptions dialOptions;
    dialOptions.setNumber(numberToDial->text());
    dialOptions.setSpeed(9600);
    dialOptions.setTransparentMode(QDialOptions::NonTransparent);

    call.dial(dialOptions);

    call.connectStateChanged(this, SLOT(dataStateChanged(QPhoneCall)));
}

void Caller::accept()
{
    QList<QPhoneCall> calls = mgr->calls();
    foreach (QPhoneCall call, calls) {
        if (call.state() == QPhoneCall::Incoming) {
            call.accept();
            break;
        }
    }
}

void Caller::stateChanged(const QPhoneCall& call)
{
    if (call.state() == QPhoneCall::Connected)
        status->setText("Call " + call.number() + " has connected");
    else if ( call.dropped() )
        status->setText("Call " + call.number() + " has ended");
}

void Caller::newCall(const QPhoneCall& call)
{
    if ( call.state() == QPhoneCall::Incoming &&
         call.callType() == "Voice" ) {
        status->setText("Incoming call from " + call.number());
    }
}
// END-NEWCALL-DOC

void Caller::dataStateChanged(const QPhoneCall& call)
{
    if (call.callType() == "Data" && call.state() == QPhoneCall::Connected) {
        dataDevice = call.device();
        connect(dataDevice, SIGNAL(readyRead()), this, SLOT(dataReady()));
        connect(dataDevice, SIGNAL(closed()), this, SLOT(dataClosed()));
    }
}

void Caller::dataReady()
{
    char buffer[1024];
    qint64 len;
    while ( ( len = dataDevice->read( buffer, sizeof(buffer) ) ) > 0 ) {
        // There is new data ready to process.
        // ...
    }
}

void Caller::dataClosed()
{
    // The data call has ended - the device is now closed.
    dataDevice = 0;
}

void Caller::dataWrite( const char *buf, int len )
{
    // Send the specified data on the data call if it is still active.
    if (dataDevice)
        dataDevice->write(buf, len);
}
// END-DATA-DOC
