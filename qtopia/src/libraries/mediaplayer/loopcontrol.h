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
#ifndef LOOP_CONTROL_H
#define LOOP_CONTROL_H


#include <qdatetime.h>
#include "loopcontrolbase.h"
#include "loopcontextbase.h"


class AudioDevice;


class LoopControl : public LoopControlBase {
    Q_OBJECT
public:
    LoopControl( QObject *parent, const char *name, AudioDevice *ad );
    ~LoopControl();
    bool init( const QString& filename, const QString& mimetype = QString::null, bool isURL = false );
    void start();
    bool hasVideo() const { return video->enabled; }
    long totalPlaytime() const;

public slots:
    void audioDeviceReady();
    void audioDeviceError();
    void audioDeviceErrorHandler();

    void setPaused( bool );
    void setPosition( long );

protected:
    void timerEvent(QTimerEvent*);

private:
    void stop();
    void startTimers();
    void killTimers();

    QTime playtime;
    int sliderId;
    LoopContextBase *video;
    LoopContextBase *audio;
    AudioDevice *audioDevice;
    bool audioDeviceIsReady;
    bool startWhenAudioDeviceReady;
};


#endif // LOOP_CONTROL_H

