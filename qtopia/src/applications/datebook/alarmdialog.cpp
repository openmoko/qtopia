/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "alarmdialog.h"
#include "eventview.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qtopia/timestring.h>

#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qlayout.h>


AlarmDialog::AlarmDialog( QWidget *parent, const char *name, bool modal, WFlags f )
    : AlarmDialogBase( parent, name, modal, f ),
    mButton( Cancel )
{
    init();
}

void AlarmDialog::init()
{
    eventView->readonly();
    spinSnooze->hide();
    btnSnooze->hide();

    // This is a debug feature (but it must be disabled at runtime)
#ifndef DEBUG
    chkSkipDialogs->hide();
#endif

#ifdef QTOPIA_DESKTOP
    // Details isn't useable until qcop is on the desktop
    btnDetails->hide();
    QPushButton *ok = new QPushButton( tr("Ok"), this, 0 );
    connect( ok, SIGNAL(clicked()), this, SLOT(reject()) );
    layout()->add( ok );
    ok->setDefault( TRUE );
#endif
}

AlarmDialog::Button AlarmDialog::exec(const Occurrence &o)
{
    eventView->init( o );

#ifdef QTOPIA_DESKTOP
    int ret = AlarmDialogBase::exec();
#else
    int ret = QPEApplication::execDialog( this );
#endif

    if ( ret )
	return mButton;
    else
	return Cancel;
}

AlarmDialog::Button AlarmDialog::result()
{
    return mButton;
}

void AlarmDialog::snoozeClicked()
{
    mButton = Snooze;
    accept();
}

void AlarmDialog::detailsClicked()
{
    mButton = Details;
    accept();
}

int AlarmDialog::snoozeLength()
{
    return spinSnooze->value();
}

bool AlarmDialog::getSkipDialogs()
{
    return chkSkipDialogs->isChecked();
}

