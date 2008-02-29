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
#ifndef SOUND_H
#define SOUND_H

#include <qtopia/qpeglobal.h>

class QString;
class SoundData;

class QTOPIA_EXPORT Sound {
public:
    Sound(const QString& name);
    ~Sound();

    void play();
    void playLoop();
    void stop();

    static void soundAlarm();
    //static void soundClick();
    //static void soundTap();
    
private:
    SoundData* d;
};

#endif
