/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "senddialog.h"
#include "qirserver.h"

#include <qlabel.h>
#include <qpushbutton.h>

SendDialog::SendDialog( QIrServer *irserver, const QString &desc, QWidget *parent, const char * name )
    : SendDialogBase( parent, name, FALSE) //, WStyle_StaysOnTop )
{
    server = irserver;

    setCaption( tr("Infrared send") );
    QString str = tr("<p>Sending %1").arg(desc);
    infoLabel->setText( str );
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( canceled() ) );
}

void SendDialog::canceled()
{
    server->cancel();
    delete this;
}

void SendDialog::keyPressEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_Escape) {
	canceled();
    }
}

void SendDialog::closeEvent(QCloseEvent *)
{
    canceled();
}
