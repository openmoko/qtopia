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

#include "e1_telephony.h"
#include <QVBoxLayout>
#include "e1_callscreen.h"
#include "e1_error.h"
#include "e1_dialer.h"
#include "e1_bar.h"
#include "phone/themecontrol.h"
#include <qcopchannel_qws.h>
#include <QTimerEvent>
#include <QDebug>
#include "phone/dialerservice.h"
#include <QString>

class E1DialerServiceProxy : public DialerService
{
Q_OBJECT
public:
    E1DialerServiceProxy(QObject *parent)
        : DialerService(parent)
    {}

signals:
    void doShowDialer(const QString &);

protected:
    virtual void dialVoiceMail() {}
    virtual void dial( const QString&, const QString& number ) { emit doShowDialer(number); }
    virtual void dial( const QString& number, const QUniqueId&) { emit doShowDialer(number); }
    virtual void showDialer( const QString& digits ) { emit doShowDialer(digits); }

};

E1Telephony::E1Telephony(QWidget *parent)
: QWidget(parent), m_state(Dialer), m_scrollStep(0)
{
    m_tbar = new E1PhoneTelephonyBar(this);

    m_bar = new E1Bar(this);
    E1CloseButton *but = new E1CloseButton;
    but->setMargin(3);
    m_bar->addItem(but);
    m_bar->addSeparator();
    E1Button *middleButton = new E1Button;
    middleButton->setMargin(3);
    m_bar->addItem(middleButton);
    middleButton->setFlag(E1Button::Expanding);
    m_bar->addSeparator();
    E1Menu *context = new E1Menu;
    context->setMargin(3);
    context->setPixmap(QPixmap(":image/samples/e1_context"));
    m_bar->addItem(context);

    m_callscreen = new E1Callscreen(middleButton, this);
    m_callscreen->hide();
    QObject::connect(m_callscreen, SIGNAL(toDialer()),
                     this, SLOT(slideToDialer()));
    QObject::connect(m_callscreen, SIGNAL(closeMe()),
                     this, SLOT(slideToDialer()));
    QObject::connect(m_callscreen, SIGNAL(showMe()),
                     this, SLOT(popupCallscreen()));

    m_dialer = new E1Dialer(middleButton, this);
    m_dialer->hide();
    QObject::connect(m_dialer, SIGNAL(toCallScreen()),
                     this, SLOT(slideToCallscreen()));
//    ThemeControl::instance()->registerThemedView( m_dialer, "Dialer" );

    QObject::connect(m_dialer, SIGNAL(sendNumber(const QString &)),
                     m_callscreen, SLOT(sendNumber(const QString &)));

    // Listen to header channel
    QCopChannel* channel = new QCopChannel( "QPE/E1", this );
    connect( channel, SIGNAL(received(const QString&,const QByteArray&)),
             this, SLOT(message(const QString&,const QByteArray&)) );

    E1DialerServiceProxy *proxy = new E1DialerServiceProxy(this);
    QObject::connect(proxy, SIGNAL(doShowDialer(const QString &)),
                     this, SLOT(doShowDialer(const QString &)));
}

void E1Telephony::showEvent(QShowEvent *)
{
    doLayout();
}

void E1Telephony::doLayout()
{
    m_tbar->setGeometry(0, 0, width(), 26);

    QRect geom(0, 27, width(), height() - 26 - 32 - 1);

    if(m_scrollStep == 0) {
        m_dialer->setGeometry(geom);
        m_dialer->show();
        m_callscreen->hide();
        m_dialer->setActive();
    } else if(m_scrollStep == m_scrollSteps) {
        m_callscreen->setGeometry(geom);
        m_dialer->hide();
        m_callscreen->show();
        m_callscreen->setActive();
    } else {
        // Somewhere in the middle
        QRect dialerGeom(-1 *((m_scrollStep * width()) / m_scrollSteps),
                         27, width(), height() - 26 - 32 - 1);
        QRect callGeom(width() + -1 *((m_scrollStep * width()) / m_scrollSteps),
                         27, width(), height() - 26 - 32 - 1);

        m_dialer->setGeometry(dialerGeom);
        m_callscreen->setGeometry(callGeom);
        m_dialer->show();
        m_callscreen->show();
    }

    m_bar->setGeometry(0, height() - 32, width(), 32);
}

void E1Telephony::timerEvent(QTimerEvent *e)
{
    if(m_state == Dialer) {

        if(m_scrollStep > 0)
            --m_scrollStep;
        if(!m_scrollStep)
            killTimer(e->timerId());
        doLayout();

    } else if(m_state == CallScreen) {

        if(m_scrollStep < m_scrollSteps)
            ++m_scrollStep;
        if(m_scrollStep == m_scrollSteps)
            killTimer(e->timerId());
        doLayout();

    }
}

void E1Telephony::message(const QString &message, const QByteArray &)
{
    if(message == "showTelephony()")
        display();
    else if(message == "showCallscreen()")
        popupCallscreen();
    else if(message == "showDialer()")
        popupDialer();
    else if(message == "slideToDialer()")
        slideToDialer();
    else if(message == "slideToCallscreen()")
        slideToCallscreen();
    else if(message == "error()") {
        E1Error::error("Hello world!");
    }

}

void E1Telephony::display()
{
    showMaximized();
    raise();
    if(m_state == Dialer)
        m_dialer->setActive();
    else
        m_callscreen->setActive();
}

void E1Telephony::slideToDialer()
{
    m_state = Dialer;
    m_scrollStep = m_scrollSteps;
    startTimer(m_scrollStepTime);
    doLayout();
    m_dialer->setActive();
}

void E1Telephony::slideToCallscreen()
{
    m_state = CallScreen;
    m_scrollStep = 0;
    startTimer(m_scrollStepTime);
    doLayout();
    m_callscreen->setActive();
}

void E1Telephony::popupDialer()
{
    m_state = Dialer;
    m_scrollStep = 0;
    doLayout();
    m_dialer->setActive();
}

void E1Telephony::popupCallscreen()
{
    m_state = CallScreen;
    m_scrollStep = m_scrollSteps;
    doLayout();
    m_callscreen->setActive();
}

void E1Telephony::doShowDialer(const QString &num)
{
    popupDialer();
    display();
    m_dialer->setNumber(num);
}

#include "e1_telephony.moc"
