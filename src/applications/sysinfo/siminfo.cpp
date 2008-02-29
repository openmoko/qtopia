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

#include "siminfo.h"
#include "graph.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

SimInfo::SimInfo( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    QTimer::singleShot(70, this, SLOT(init()));
}

SimInfo::~SimInfo()
{
    delete smsData;
    delete pbData;
}

void SimInfo::init()
{
    sms = new QSMSReader( QString(), this );
    pb = new QPhoneBook( QString(), this );
    pbused = -1;
    pbtotal = -1;

    connect( sms, SIGNAL(unreadCountChanged()), this, SLOT(updateData()) );
    connect( sms, SIGNAL(messageCount(int)), this, SLOT(updateData()) );

    connect( pb, SIGNAL(limits(QString,QPhoneBookLimits)),
             this, SLOT(limits(QString,QPhoneBookLimits)) );
    pb->requestLimits();

    QVBoxLayout *vb = new QVBoxLayout( this );

    smsData = new GraphData();
    smsGraph = new BarGraph( this );
    smsGraph->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    vb->addWidget( smsGraph, 1 );
    smsGraph->setData( smsData );
    smsLegend = new GraphLegend( this );
    vb->addWidget( smsLegend );
    smsLegend->setData( smsData );

    pbData = new GraphData();
    pbGraph = new BarGraph( this );
    pbGraph->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    vb->addWidget( pbGraph, 1 );
    pbGraph->setData( pbData );
    pbLegend = new GraphLegend( this );
    vb->addWidget( pbLegend );
    pbLegend->setData( pbData );

    vb->addStretch(100);

    updateData();
}

void SimInfo::updateData()
{
    // Update the sms information.
    int smsUsed = sms->usedMessages();
    int smsTotal = sms->totalMessages();
    smsData->clear();
    if ( smsUsed != -1 && smsTotal != -1 ) {
        smsData->addItem( tr("SMS Used: %1", "%1=number").arg(smsUsed), smsUsed );
        smsData->addItem( tr("SMS Free: %1", "%1=number").arg(smsTotal - smsUsed),
                          smsTotal - smsUsed );
    } else {
        smsData->addItem( tr("SMS Used: Please wait"), 0 );
        smsData->addItem( tr("SMS Free: Please wait"), 1 );
    }
    smsGraph->repaint();
    smsLegend->update();
    smsGraph->show();
    smsLegend->show();

    // Update the phone book information.
    pbData->clear();
    if ( pbused != -1 && pbtotal != -1 ) {
        pbData->addItem( tr("Contacts Used: %1", "%1=number").arg(pbused), pbused );
        pbData->addItem( tr("Contacts Free: %1", "%1=number").arg(pbtotal - pbused),
                          pbtotal - pbused );
    } else {
        pbData->addItem( tr("Contacts Used: Please wait"), 0 );
        pbData->addItem( tr("Contacts Free: Please wait"), 1 );
    }
    pbGraph->repaint();
    pbLegend->update();
    pbGraph->show();
    pbLegend->show();
}

void SimInfo::limits( const QString& store, const QPhoneBookLimits& value )
{
    if ( store == "SM" ) {    // No tr
        pbused = (int)( value.used() );
        pbtotal = (int)( value.lastIndex() - value.firstIndex() + 1 );
        if ( pbtotal <= 1 ) {
            // No information available.
            pbused = -1;
            pbtotal = -1;
        }
        updateData();
    }
}
