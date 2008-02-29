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

#include "alarmdialog.h"

#include <qtopiaapplication.h>
#include <qtimestring.h>

#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qlayout.h>


AlarmDialog::AlarmDialog( QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f ),
    mButton( Cancel )
{
    setupUi( this );
    init();
}

void AlarmDialog::init()
{
    spinSnooze->hide();
    btnSnooze->hide();
    pmIcon->setPixmap( QPixmap(":image/alarmbell"));
#ifdef QTOPIA_DESKTOP
    // Details isn't useable until qcop is on the desktop
    btnDetails->hide();
    chkSkipDialogs->hide();
    QPushButton *ok = new QPushButton( tr("Ok"), this, 0 );
    connect( ok, SIGNAL(clicked()), this, SLOT(reject()) );
    layout()->add( ok );
    ok->setDefault( true );
#endif

    connect(btnDetails, SIGNAL(clicked()), this, SLOT(detailsClicked()));
}

AlarmDialog::Button AlarmDialog::exec(const QOccurrence &e)
{
    lblDescription->setText( e.appointment().description() );
    lblLocation->setText( e.appointment().location() );
    lblApptTime->setText( QTimeString::localHM( e.start().time() ) );
    lblApptDate->setText( QTimeString::localYMD( e.start().date() ) );

/*#ifdef QTOPIA_PHONE
    int ret = QtopiaApplication::execDialog(this);
#else*/
    showMaximized();
    int ret = QDialog::exec();
//#endif

    if (ret)
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

