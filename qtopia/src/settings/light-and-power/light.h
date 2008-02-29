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
#ifndef LAPSETTINGS_H
#define LAPSETTINGS_H


#include <qstrlist.h> 
#include <qasciidict.h>
#include <qdialog.h>

#include <qtopia/power.h>

#include "lightsettingsbase.h"

#ifdef QTOPIA_PHONE
#include "displaysettingsbase.h"
#endif


class Config;

class LightSettings : public QDialog
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
    void sysMessage(const QCString&, const QByteArray&);

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
    LightSettingsBase *b;
#ifdef QTOPIA_PHONE
    DisplaySettingsBase *d;
    bool showHomeScreen;
    int intervalShowHomeScreen;
    QString keyLock;
#endif
};


#endif // LAPSETTINGS_H

