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

#include "e1_incoming.h"
#include "dialercontrol.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "e1_bar.h"
#include <QPixmap>
#include <QString>
#include <qtopia/pim/qcontactmodel.h>
#include <qtopia/pim/qcontact.h>

E1Incoming::E1Incoming()
: E1Dialog(0, Generic)
{
    QObject::connect(DialerControl::instance(),
                     SIGNAL(callIncoming(QPhoneCall)),
                     this,
                     SLOT(callIncoming(QPhoneCall)));
    QObject::connect(DialerControl::instance(),
                        SIGNAL(callMissed(QPhoneCall)),
                        this,
                        SLOT(missedIncomingCall(QPhoneCall)));


    E1Button * but = new E1Button;
    but->setMargin(3);
    but->setFlag(E1Button::Expanding);
    but->setText("Ignore");
    QObject::connect(but, SIGNAL(clicked()), this, SLOT(ignore()));
    bar()->addItem(but);
    bar()->addSeparator();

    but = new E1Button;
    but->setMargin(3);
    but->setFlag(E1Button::Expanding);
    but->setText("Busy");
    QObject::connect(but, SIGNAL(clicked()), this, SLOT(busy()));
    bar()->addItem(but);
    bar()->addSeparator();

    but = new E1Button;
    but->setMargin(3);
    but->setFlag(E1Button::Expanding);
    but->setText("Answer");
    QObject::connect(but, SIGNAL(clicked()), this, SLOT(answer()));
    bar()->addItem(but);

    QWidget * wid = new QWidget(this);
    QVBoxLayout *l = new QVBoxLayout(wid);
    QLabel * incoming = new QLabel(wid);
    incoming->setText("Incoming Call...");
    l->addWidget(incoming);

    QHBoxLayout *h = new QHBoxLayout;
    l->addLayout(h);
    image = new QLabel(wid);
    h->addWidget(image);
    name = new QLabel(wid);
    h->addWidget(name);

    setContentsWidget(wid);
    setFixedWidth(180);
}

void E1Incoming::callIncoming( const QPhoneCall &call )
{
    if(call.identifier() == m_lastId)
        return;

    if(DialerControl::instance()->hasActiveCalls() ||
       DialerControl::instance()->hasCallsOnHold()) {
        DialerControl::instance()->sendBusy();
        return;
    }

    m_lastId = call.identifier();
    currentCall = call;
    updateLabels();
    exec();
}

void E1Incoming::missedIncomingCall( const QPhoneCall& call )
{
    if( call == currentCall )
        accept();
}

void E1Incoming::ignore()
{
    accept();
}

void E1Incoming::busy()
{
    DialerControl::instance()->sendBusy();
    accept();
}

void E1Incoming::answer()
{
    DialerControl::instance()->accept();
    emit showCallscreen();
    accept();
}

void E1Incoming::updateLabels()
{
    // Get the number or name to display in the text area.
    QString m_name = currentCall.number();

    QContact cnt;
    if (!currentCall.contact().isNull()) {
        QContactModel m;
        cnt = m.contact(currentCall.contact());
    } else if (!m_name.isEmpty()) {
        QString name;
        QContactModel m;
        cnt = m.matchPhoneNumber(m_name);
    }

    QPixmap m_image;

    if (!cnt.uid().isNull()) 
        m_name = cnt.label();

    m_image = cnt.thumbnail();

    name->setText(m_name);
    image->setPixmap(m_image);
}

