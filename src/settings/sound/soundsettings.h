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
#ifndef SOUNDSETTINGS_H
#define SOUNDSETTINGS_H


#include "soundsettingsbase.h"


class SoundSettings : public SoundSettingsBase
{ 
    Q_OBJECT

public:
    SoundSettings( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );

protected:
    void accept();
    void reject();
    void done(int r);

private slots:
    void setVolume(int);
};


#endif // SOUNDSETTINGS_H

