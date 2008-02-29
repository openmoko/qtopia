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

#include <qtopiaservices.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaapplication.h>

#include "savetocontacts.h"

SavePhoneNumberDialog::SavePhoneNumberDialog(QWidget *parent)
    : PhoneMessageBox(parent)
{
    setTitle( tr("Save to Contacts") );
    setText( "<qt>" + tr("Create a new contact?") + "</qt>" );
    setIcon( PhoneMessageBox::Warning );
    setButtons( PhoneMessageBox::Yes, PhoneMessageBox::No );
}

SavePhoneNumberDialog::~SavePhoneNumberDialog()
{}

void SavePhoneNumberDialog::savePhoneNumber(const QString &number)
{
    SavePhoneNumberDialog diag;
    if (QtopiaApplication::execDialog(&diag) == QAbstractMessageBox::Yes) {
        QtopiaServiceRequest req( "Contacts", "createNewContact(QString)" );
        req << number;
        req.send();
    } else {
        QtopiaServiceRequest req( "Contacts", "addPhoneNumberToContact(QString)" );
        req << number;
        req.send();
    }
}
