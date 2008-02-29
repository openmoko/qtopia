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
#include "connectionstatuswidget.h"

#include <qdplugin.h>
#include <qcopchannel_qd.h>
#include <qcopenvelope_qd.h>
#include <qtopiadesktoplog.h>

#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>

ConnectionStatusWidget::ConnectionStatusWidget( QWidget *parent )
    : QWidget( parent )
{
    QHBoxLayout *hbox = new QHBoxLayout( this );
    hbox->setMargin( 0 );
    hbox->setSpacing( 0 );

    statusBtn = new QPushButton;
    statusBtn->setFlat( true );
    statusBtn->setFocusPolicy( Qt::NoFocus );
    connect( statusBtn, SIGNAL(clicked()), this, SLOT(statusClicked()) );

    hintBtn = new QPushButton;
    hintBtn->setFlat( true );
    hintBtn->setFocusPolicy( Qt::NoFocus );
    hintBtn->setIcon( QPixmap(":image/hint") );
    connect( hintBtn, SIGNAL(clicked()), this, SLOT(hintClicked()) );

    hbox->addWidget( statusBtn );
    hbox->addWidget( hintBtn );

    setState( QDConPlugin::Disconnected );
    connect( qApp, SIGNAL(setConnectionState(int)), this, SLOT(setState(int)) );

    QCopChannel *sys = new QCopChannel( "QD/Connection", this );
    connect( sys, SIGNAL(received(QString,QByteArray)),
            this, SLOT(connectionMessage(QString,QByteArray)) );
}

ConnectionStatusWidget::~ConnectionStatusWidget()
{
}

void ConnectionStatusWidget::setState( int state )
{
    // This comes out once per main window
    //TRACE(UI) << "ConnectionStatusWidget::setState" << "state" << state;
    mState = state;
    switch ( mState ) {
        case QDConPlugin::Connected:
            statusBtn->setIcon( QPixmap(":image/connected") );
            statusBtn->setToolTip( tr("Connected") );
            hintBtn->hide();
            break;
        case QDConPlugin::Disconnected:
            statusBtn->setIcon( QPixmap(":image/disconnected") );
            statusBtn->setToolTip( tr("Disconnected") );
            break;
        case QDConPlugin::Connecting:
            statusBtn->setIcon( QPixmap(":image/connecting") );
            statusBtn->setToolTip( tr("Connecting") );
            break;
    }
}

void ConnectionStatusWidget::connectionMessage( const QString &message, const QByteArray &data )
{
    QDataStream stream( data );
    if ( message == "setHint(QString,QString)" ) {
        QString hint;
        QString section;
        stream >> hint >> section;
        // if ( pasthints.contains(hint) ) return;
        hintBtn->show();
        hintBtn->setToolTip( hint );
    }
    else if ( message == "clearHint()" ) {
        hintBtn->hide();
    }
    else if ( message == "setBusy()" ) {
        Q_ASSERT( mState == QDConPlugin::Connected );
        statusBtn->setIcon( QPixmap(":image/busy") );
    }
    else if ( message == "clearBusy()" ) {
        setState( mState );
    }
}

void ConnectionStatusWidget::statusClicked()
{
    TRACE(UI) << "ConnectionStatusWidget::statusClicked";
    QCopEnvelope e( "QD/Connection", "statusButtonClicked()" );
}

void ConnectionStatusWidget::hintClicked()
{
    TRACE(UI) << "ConnectionStatusWidget::hintClicked";
    hintBtn->hide();
    LOG() << "launch manual section" << section;
}

void ConnectionStatusWidget::showEvent( QShowEvent * /*e*/ )
{
    hintBtn->hide();
}

