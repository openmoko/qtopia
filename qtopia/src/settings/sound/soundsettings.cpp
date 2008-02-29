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
#include "soundsettings.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qtopia/custom.h>

#include <qslider.h>
#include <qcheckbox.h>

SoundSettings::SoundSettings( QWidget* parent,  const char* name, WFlags fl )
    : SoundSettingsBase( parent, name, TRUE, fl )
{
    Config config( "Sound" );
    config.setGroup( "System" );
    origVol = 100-config.readNumEntry("Volume");
    volume->setValue(origVol);
    touchsound->setChecked(config.readBoolEntry("Touch"));
    keysound->setChecked(config.readBoolEntry("Key"));

    connect(volume, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));
    connect(qApp, SIGNAL(volumeChanged(bool)), this, SLOT(volumeChanged(bool)));

#ifndef CUSTOM_SOUND_TOUCH
    touchsound->hide();
#endif
#ifndef CUSTOM_SOUND_KEYCLICK
    keysound->hide();
#endif
}

void SoundSettings::reject()
{
    setVolume(origVol);
    QDialog::reject();
}

void SoundSettings::accept()
{
    QDialog::accept();
    {
	Config config( "Sound" );
	config.setGroup( "System" );
	config.writeEntry("Volume",100-volume->value());
	config.writeEntry("Touch",touchsound->isChecked());
	config.writeEntry("Key",keysound->isChecked());
    }
    setVolume(volume->value());
}

void SoundSettings::done(int r) { 
  QDialog::done(r);
  close();
}

void SoundSettings::setVolume(int v)
{
    Config config( "Sound" );
    config.setGroup( "System" );
    config.writeEntry("Volume",100-v);
#ifndef QT_NO_COP
    QCopEnvelope( "QPE/System", "setVolume(int,int)" ) << 0 << 100-v;
#endif
}

void SoundSettings::volumeChanged( bool )
{
    Config config( "Sound" );
    config.setGroup( "System" );
    volume->setValue(100-config.readNumEntry("Volume"));
}

