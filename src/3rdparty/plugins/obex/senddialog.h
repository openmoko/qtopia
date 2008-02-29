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
#ifndef SENDDIALOG_H
#define SENDDIALOG_H

#include "senddialogbase.h"

class QIrServer;

class SendDialog : public SendDialogBase
{
    Q_OBJECT
public:
    SendDialog( QIrServer *server, const QString &desc, QWidget *parent = 0, const char * name = 0 );

protected:
    void keyPressEvent(QKeyEvent*);
    void closeEvent(QCloseEvent*);

public slots:
    void canceled();

private:
    QIrServer *server;
};

#endif
