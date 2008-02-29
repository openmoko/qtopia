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

#include <qexportedbackground.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QCopChannel>
#include <QCloseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QShowEvent>
#include <QMouseEvent>
#include <QLabel>
#include "e1_launcher.h"
#include "e1_header.h"
#include "e1_phonebrowser.h"
#include "e1_dialer.h"
#include "e1_dialog.h"
#include "phone/themecontrol.h"
#include "phone/ringcontrol.h"
#include "phone/dialer.h"
#include <qtopiaipcenvelope.h>
#include "phone/dialercontrol.h"
#include "e1_popup.h"
#include "e1_telephony.h"
#include <QListWidget>
#include <QDebug>
#include "e1_incoming.h"
#include <qtopiaipcenvelope.h>
#include "qtopiaserverapplication.h"
#include "qabstractserverinterface.h"

E1ServerInterface::E1ServerInterface(QWidget *parent, Qt::WFlags flags)
: QAbstractServerInterface(parent, flags), m_dialer(0), m_newMessages("/Communications/Messages/NewMessages")
{
    m_eb = new QExportedBackground(0, this);
    QObject::connect(m_eb, SIGNAL(wallpaperChanged()),
                     this, SLOT(wallpaperChanged()));

    wallpaperChanged();

    setGeometry(qApp->desktop()->rect());

    ThemeControl::instance();

    // Listen for showDialer message
    QCopChannel* dialerChannel = new QCopChannel( "QPE/Application/qpe", this );
    connect( dialerChannel, SIGNAL(received(const QString&,const QByteArray&)), this, SLOT(showDialer(const QString&, const QByteArray&)) );

    m_header = new E1Header(0, Qt::FramelessWindowHint |
                                Qt::Tool |
                                Qt::WindowStaysOnTopHint);

    DialerControl::instance();

    m_browser = new E1PhoneBrowser;

    E1Telephony *telephony = new E1Telephony(0);


    E1Incoming * incoming = new E1Incoming();
    QObject::connect(incoming, SIGNAL(showCallscreen()),
                     telephony, SLOT(popupCallscreen()));
    QObject::connect(incoming, SIGNAL(showCallscreen()),
                     telephony, SLOT(display()));

    connect( &m_newMessages, SIGNAL(contentsChanged()), this, SLOT(messageCountChanged()) );
        
    QtopiaIpcEnvelope env("QPE/E1", "showHome()");
}

void E1ServerInterface::messageCountChanged()
{
    if( m_newMessages.value().toInt() != 0 ) {
        E1Dialog* dialog = new E1Dialog( this, E1Dialog::NewMessage );
        if( dialog->exec() == QDialog::Accepted )
            QtopiaIpcEnvelope env("QPE/Application/qtmail", "raise()");
        delete dialog;
    }
}

void E1ServerInterface::wallpaperChanged()
{
    m_wallpaper = m_eb->wallpaper();
    update();
}

void E1ServerInterface::paintEvent(QPaintEvent *)
{
    if(m_wallpaper.isNull())
        return;

    QPainter p(this);
    p.drawTiledPixmap(rect(), m_wallpaper);
}

void E1ServerInterface::keyReleaseEvent(QKeyEvent *e)
{
    e->accept();
}

void E1ServerInterface::closeEvent(QCloseEvent *e)
{
    e->ignore();
}

void E1ServerInterface::keyPressEvent(QKeyEvent *e)
{
    e->accept();
}

void E1ServerInterface::showEvent(QShowEvent *e)
{
    m_header->show();
    QWidget::showEvent(e);
}

void E1ServerInterface::mousePressEvent(QMouseEvent *e)
{
    if(m_header->isVisible())
        m_header->hide();
    else
        m_header->show();
    e->accept();
}

void E1ServerInterface::showDialer( const QString& msg, const QByteArray& )
{
    if( msg == "Dialer::showDialer()" ) {
#if 0
        if( !m_dialer ) {
            m_dialer = new E1Dialer( 0 );
            m_dialer->setObjectName("Dialer");
            /* Not connected to anything
            connect(m_dialer, SIGNAL(dial(const QString&, const QUniqueId&)),
                    this, SLOT(dialNumber(const QString&, const QUniqueId&)) );
                    */
        }
        m_dialer->showMaximized();
        m_dialer->raise();
#endif
    }
}

QTOPIA_REPLACE_WIDGET(QAbstractServerInterface, E1ServerInterface);
