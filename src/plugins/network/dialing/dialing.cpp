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

#include "dialing.h"

#include <qtopiaapplication.h>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif
#include <ipvalidator.h>


DialingPage::DialingPage( const QtopiaNetworkProperties prop, QWidget* parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
    ui.setupUi( this );
    init();
    readConfig( prop );

#ifdef QTOPIA_PHONE
    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this , true );
#endif
    setObjectName("dialing");
}

DialingPage::~DialingPage()
{
}

void DialingPage::init()
{
    IPValidator * val = new IPValidator( this );
    ui.dns1->setValidator( val );
    ui.dns2->setValidator( val );

#ifdef QTOPIA_PHONE
    QtopiaApplication::setInputMethodHint(ui.dns1, "netmask");
    QtopiaApplication::setInputMethodHint(ui.dns2, "netmask");
#endif

    connect( ui.timeout_box, SIGNAL(stateChanged(int)),
                this, SLOT(timeoutSelected(int)));
    connect( ui.usepeerdns, SIGNAL(stateChanged(int)),
           this, SLOT(manualDNS(int)));
}

void DialingPage::readConfig( const QtopiaNetworkProperties& prop )
{
    QVariant v = prop.value("Serial/Timeout");
    if (!v.isValid()) {
        ui.timeout_box->setCheckState(Qt::Checked);
    } else if (v.toString() == "none") {
        ui.timeout_box->setCheckState(Qt::Unchecked);
        ui.timeout->setValue( 120 );
    } else if (v.canConvert(QVariant::Int)) {
        ui.timeout_box->setCheckState(Qt::Checked);
        ui.timeout->setValue( v.toInt() );
    }
    timeoutSelected(ui.timeout_box->checkState());

    v = prop.value("Serial/UsePeerDNS");
    if ( v.toString() != "n" )
        ui.usepeerdns->setCheckState(Qt::Checked);
    else
        ui.usepeerdns->setCheckState(Qt::Unchecked);
    manualDNS( ui.usepeerdns->checkState() );

    ui.dns1->setText( prop.value("Properties/DNS_1").toString() );
    ui.dns2->setText( prop.value("Properties/DNS_2").toString() );

}

QtopiaNetworkProperties DialingPage::properties()
{
    QtopiaNetworkProperties props;
    if (ui.timeout_box->checkState() == Qt::Checked) {
        props.insert("Serial/Timeout", ui.timeout->value());
    } else {
        props.insert("Serial/Timeout", QString("none"));
    }

    props.insert("Serial/UsePeerDNS",
        ui.usepeerdns->checkState()==Qt::Checked ? QString("y"): QString("n"));
    props.insert("Properties/DNS_1", ui.dns1->text());
    props.insert("Properties/DNS_2", ui.dns2->text());

    return props;
}

void DialingPage::timeoutSelected(int state) {
    if ( state == Qt::Checked )
        ui.timeout->setEnabled( true );
    else {
        ui.timeout->setEnabled( false );
    }
}

void DialingPage::manualDNS(int state) {
    if ( state == Qt::Checked )
        ui.dns_box->setEnabled( false );
    else {
        ui.dns_box->setEnabled( true );
    }
}
