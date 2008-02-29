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

#include "settingsdialog.h"
#include <qspinbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcombobox.h>

SettingsDialog::SettingsDialog( QWidget *parent, const char *name, bool modal, WFlags f )
    : SettingsDialogBase( parent, name, modal, f )
{
}

void SettingsDialog::setDelay(int d)
{
    secsDelay->setValue(d);
}

int SettingsDialog::delay() const
{
    return secsDelay->value();
}

void SettingsDialog::setRepeat( bool r )
{
    repeatCheck->setChecked( r );
}

bool SettingsDialog::repeat() const
{
    return repeatCheck->isChecked();
}

void SettingsDialog::setReverse(bool r)
{
    reverseCheck->setChecked(r);
}

bool SettingsDialog::reverse() const
{
    return reverseCheck->isChecked();
}

void SettingsDialog::setRotation(RotationDirection r)
{
    if (r == rotate_clockwise) {
	rotationClockwise->setChecked(TRUE);
    } else if (r == rotate_counterclockwise) {
	rotationCounterclockwise->setChecked(TRUE);
    } else {
	rotationNone->setChecked(TRUE);
    }
}

RotationDirection SettingsDialog::rotation(void) const
{
    RotationDirection	r = rotate_none;

    if (rotationClockwise->isChecked()) {
	r = rotate_clockwise;
    } else if (rotationCounterclockwise->isChecked()) {
	r = rotate_counterclockwise;
    }

    return r;
}

void SettingsDialog::setFastLoad(bool f)
{
    fastLoadCheck->setChecked(f);
}

bool SettingsDialog::fastLoad() const
{
    return fastLoadCheck->isChecked();
}

void SettingsDialog::setSmallScale(bool s)
{
    smallScaleCheck->setChecked(s);
}

bool SettingsDialog::smallScale() const
{
    return smallScaleCheck->isChecked();
}
