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

#include "exceptiondialog.h"

#include <qtopiaapplication.h>

/* TODO may want to add code that prevents all three checkboxes being
   deselected */
ExceptionDialog::ExceptionDialog( QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f )
{
    setupUi( this );
}

int ExceptionDialog::exec(bool editMode)
{
    int series = 0;
    checkEarlier->setChecked(false);
    checkSelected->setChecked(true);
    checkLater->setChecked(true);

    if (editMode) {
        lblMessage->setText(tr("<qt>This appointment is part of a series. Select the part of the series you want to change below.</qt>"));
    } else {
        lblMessage->setText(tr("<qt>This appointment is part of a series. Select the part of the series you want to delete below.</qt>"));
    }
    if (!QtopiaApplication::execDialog( this ))
        return 0;

    if (checkSelected->isChecked())
        series |= Selected;
    if (checkLater->isChecked())
        series |= Later;
    if (checkEarlier->isChecked())
        series |= Earlier;
    return series;
}

int ExceptionDialog::result() const
{
    int series = 0;
    if (checkSelected->isChecked())
        series |= Selected;
    if (checkLater->isChecked())
        series |= Later;
    if (checkEarlier->isChecked())
        series |= Earlier;
    return series;
}
