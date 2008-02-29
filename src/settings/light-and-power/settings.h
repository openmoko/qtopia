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
#ifndef SETTINGS_H
#define SETTINGS_H


#include <qstrlist.h> 
#include <qasciidict.h>

#include <qpe/power.h>

#include "lightsettingsbase.h"


class Config;

class LightSettings : public LightSettingsBase
{ 
    Q_OBJECT

public:
    LightSettings( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~LightSettings();

protected:
    void accept();
    void reject();

private slots:
    void applyBrightness();
    void applyMode();
    void powerTypeClicked(int);

private:
    struct PowerMode {
	PowerMode()
	{
	    intervalDim = 20;
	    intervalLightOff = 30;
	    intervalSuspend = 60;
	    initbright = 255;

	    dim = TRUE;
	    lightoff = TRUE;
	    suspend = TRUE;
	    networkedsuspend = FALSE;
	}
	
	bool dim;
	bool lightoff;
	bool suspend;
	bool networkedsuspend;
	int intervalDim;
	int intervalLightOff;
	int intervalSuspend;
	int initbright;
    };

    void writeMode(Config &config, PowerMode *mode);

private:
    int initbright;

    PowerMode batteryMode, externalMode;
    PowerMode *currentMode;

    PowerStatus powerStatus;

    bool horizontalized;
};


#endif // SETTINGS_H

