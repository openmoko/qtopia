/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/


#include <qdialog.h>

#include "qpedialog.h"
#include "qpeapplication.h"

// Allow access to nornally protected accept and reject functions
class HackedPrivateQDialog : public QDialog{
public:
    void accept() { QDialog::accept();}
    void reject() { QDialog::reject();}
};

QPEDialogListener::QPEDialogListener(QDialog *di ) : QObject(di)
{
    dialog = di;
    connect(qApp, SIGNAL(appMessage(const QCString&, const QByteArray&)),
	    this, SLOT(appMessage(const QCString&, const QByteArray&)) );
}

QPEDialogListener::~QPEDialogListener() {}

void QPEDialogListener::appMessage( const QCString &msg, const QByteArray & )
{
    if (!dialog) 
	return;

    HackedPrivateQDialog *hackedDialog = (HackedPrivateQDialog*)dialog;

    if (msg == "accept()") {
	hackedDialog->accept();
    } else if (msg == "reject()") {
	hackedDialog->reject();
    }
}
