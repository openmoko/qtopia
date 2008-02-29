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

#include "phonethemeview.h"
#include "homescreen.h"
#include <qtopiaservices.h>
#include <qtopianamespace.h>
#include <qvaluespace.h>
#include <qtopiaipcenvelope.h>

QSet<PhoneThemedView *> PhoneThemedView::m_themedViews;

// declare PhoneThemedViewMonitor
class PhoneThemedViewMonitor : public QObject
{
Q_OBJECT
public:
    PhoneThemedViewMonitor();
    void add(PhoneThemedView *);

private slots:

    void sysMessage(const QString& message, const QByteArray&);

private:
    void setThemeText(const QString &name, const QString &text, const QString &shortText=QString());
    void setThemeStatus(const QString &name, bool enable);
    void setThemeItemActive(const QString &name, bool active);

    QtopiaChannel sysChannel;
};
Q_GLOBAL_STATIC(PhoneThemedViewMonitor, phoneThemedViewMonitor);

/*
  \class PhoneThemedView
  \brief The PhoneThemedView class provides a ThemedView supporting all the
         common Qtopia UI theme tags.
  \ingroup QtopiaServer::PhoneUI

  Deriving from this class gives your themed view the following value and
  interactive item support.

  Hopefully this class will be replaced by themes sourcing data from the
  valuespace.

  \section1 Common value items

  \section1 Interactive items

  \table
  \header
    \o Item
    \o Action
  \row
    \o messages
    \o Launch message viewer
  \row
    \o calls
    \o Shows call history
  \row
    \o roaming
    \o Starts call networks
  \row
    \o signal
    \o Starts call networks
  \row
    \o calldivert
    \o Starts call forwarding
  \row
    \o alarm
    \o Edits alarm
  \row
    \o battery
    \o Starts light-and-power
  \row
    \o time
    \o Edits time
  \row
    \o date
    \o Edits date
  \row
    \o profile
    \o Starts ring profile
  \row
    \o dialer
    \o Shows dialer
  \endtable

 */

struct PhoneKeyDescription
{
    const char *name;
    int key;
    int ascii;
    const char *text;
};

PhoneKeyDescription keyMap[] = {
    {"zero", Qt::Key_0, '0', "0"}, // no tr
    {"one", Qt::Key_1, '1', "1"}, // no tr
    {"two", Qt::Key_2, '2', "2"}, // no tr
    {"three", Qt::Key_3, '3', "3"}, // no tr
    {"four", Qt::Key_4, '4', "4"}, // no tr
    {"five", Qt::Key_5, '5', "5"}, // no tr
    {"six", Qt::Key_6, '6', "6"}, // no tr
    {"seven", Qt::Key_7, '7', "7"}, // no tr
    {"eight", Qt::Key_8, '8', "8"}, // no tr
    {"nine", Qt::Key_9, '9', "9"}, // no tr
    {"hash", Qt::Key_NumberSign, '#', "#"}, // no tr
    {"star", Qt::Key_Asterisk, '*', "*"}, // no tr
    {0, 0, 0, 0}
};

PhoneThemedView::PhoneThemedView(QWidget *parent, Qt::WFlags f)
: ThemedView(parent, f)
{
    QObject::connect(this, SIGNAL(itemClicked(ThemeItem *)),
                     this, SLOT(myItemClicked(ThemeItem *)));
    m_themedViews.insert(this);
    phoneThemedViewMonitor()->add(this);
}

PhoneThemedView::~PhoneThemedView()
{
    m_themedViews.remove(this);
}

void PhoneThemedView::myItemClicked(ThemeItem *item)
{
    if( !item->isInteractive() )
        return;

    QString in = item->itemName();
    if( in == "messages" ) {
        ///TODO : should open inbox?
        QtopiaServiceRequest e( "SMS", "viewSms()" );
        e.send();
    } else if( in == "calls" ) {
#ifdef QTOPIA_PHONEUI
        HomeScreen::getInstancePtr()->showCallHistory(true, QString());
#endif
    } else if( in == "roaming" || in == "signal" ) {
        QContent app( QtopiaService::app( "CallNetworks" ));
        if( app.isValid() )
            app.execute();
    } else if( in == "calldivert" ) {
        QContent app( QtopiaService::app( "CallForwarding" ));
        if( app.isValid() )
            app.execute();
    } else if( in == "alarm" ) {
        QtopiaServiceRequest env( "Alarm", "editAlarm()" );
        env.send();
    } else if( in == "battery" ) {
        // TODO: show more battery info, as per PDA plugin
        Qtopia::execute("light-and-power");
    } else if( in == "time" ) {
        QtopiaServiceRequest env( "Time", "editTime()" );
        env.send();
    } else if( in == "date" ) {
        QtopiaServiceRequest env( "Date", "editDate()" );
        env.send();
    } else if( in == "profile" ) {
        QtopiaServiceRequest e( "RingProfiles", "showRingProfiles()" );
        e.send();
    } else if( in == "dialer" ) {
        QtopiaServiceRequest e( "Dialer", "showDialer(QString)" );
        e << QString();
        e.send();
    } else if( in == "mainmenu" ) {
#ifdef QTOPIA4_TODO
        QtopiaApplication::sendEvent( this, new QKeyEvent( QEvent::KeyPress, Qt::Key_Select, Qt::Key_Select, Qt::NoButton ) );
#endif
    }

#if 0
    int i = 0;
    while (keyMap[i].name != 0) {
        if (in == keyMap[i].name) {
            PhoneLock::instance()->preProcessLocked(keyMap[i].key, keyMap[i].ascii, true);
            break;
        }
        ++i;
    }
#endif
}

QSet<PhoneThemedView *> PhoneThemedView::themedViews()
{
    return m_themedViews;
}

// define PhoneThemedViewMonitor
PhoneThemedViewMonitor::PhoneThemedViewMonitor()
: QObject(0),
  sysChannel("QPE/System")
{
    QObject::connect(&sysChannel, SIGNAL(received(const QString&,const QByteArray&)),
                     this, SLOT(sysMessage(const QString&,const QByteArray&)));
}

void PhoneThemedViewMonitor::add(PhoneThemedView *)
{
}

void PhoneThemedViewMonitor::sysMessage(const QString& message, const QByteArray&)
{
    if ( message == "clearCellLocation()" ) {
        setThemeItemActive("location", false);
    } else if ( message == "displayCellLocation()" ) {
        setThemeItemActive("location", true);
    }
}

void PhoneThemedViewMonitor::setThemeText(const QString &name, const QString &text, const QString &shortText)
{
    foreach (ThemedView *view, PhoneThemedView::themedViews()) {
        QList<ThemeItem*> list = view->findItems(name, ThemedView::Text);
        QList<ThemeItem*>::Iterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            ThemeTextItem *textItem = (ThemeTextItem *)(*it);
            if (textItem) {
                if (textItem->shortLabel() && !shortText.isEmpty())
                    textItem->setText(shortText);
                else
                    textItem->setText(text);
            }
        }
    }
}

void PhoneThemedViewMonitor::setThemeStatus(const QString &name, bool enable)
{
    foreach (ThemedView *view, PhoneThemedView::themedViews()) {
        QList<ThemeItem*> list = view->findItems(name, ThemedView::Status);
        QList<ThemeItem*>::Iterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            ThemeStatusItem *status = (ThemeStatusItem *)(*it);
            if (status)
                status->setOn(enable);
        }
    }
    setThemeItemActive(name, enable);
}

void PhoneThemedViewMonitor::setThemeItemActive(const QString &name, bool active)
{
    foreach (ThemedView *view, PhoneThemedView::themedViews()) {
        QList<ThemeItem*> list = view->findItems(name, ThemedView::Item);
        QList<ThemeItem*>::Iterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            ThemeItem *item = *it;
            if (item && item->rtti() != ThemedView::Status)
                item->setActive(active);
        }
    }
}

#include "phonethemeview.moc"

