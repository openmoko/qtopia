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

#include "finddialog.h"
#include "findwidget_p.h"

#include <qlayout.h>
#include <qpushbutton.h>

FindDialog::FindDialog( const QString &appName, QWidget *parent )
    : QDialog( parent )
{
    setWindowTitle( tr("Find") );
    QVBoxLayout *vb = new QVBoxLayout(this);
    fw = new FindWidget( appName, this );
    vb->addWidget(fw);
    QObject::connect( fw, SIGNAL(signalFindClicked(const QString&,bool,bool,int)),
                      this, SIGNAL(signalFindClicked(const QString&,bool,bool,int)) );
    QObject::connect( fw, SIGNAL(signalFindClicked(const QString&,const QDate&,bool,bool,int)),
                      this, SIGNAL(signalFindClicked(const QString&,const QDate&,bool,bool,int)) );
    d = 0;
}

FindDialog::~FindDialog()
{
}

QString FindDialog::findText() const
{
    return fw->findText();
}

void FindDialog::setUseDate( bool show )
{
    fw->setUseDate( show );
}

void FindDialog::setDate( const QDate &dt )
{
    fw->setDate( dt );
}

void FindDialog::slotNotFound()
{
    fw->slotNotFound();
}

void FindDialog::slotWrapAround()
{
    fw->slotWrapAround();
}
