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
#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H


#include <qobject.h>


class AudioInputPrivate;


class AudioInput : public QObject {
    Q_OBJECT
public:
    AudioInput( unsigned int frequency = 44000, unsigned int channels = 2 );
    ~AudioInput();

    int read( short *buffer, unsigned int length );

    unsigned int channels() const;
    unsigned int frequency() const;
    unsigned int bufferSize() const;

    bool isActive() const;

    // Each volume level is from 0 to 0xFFFF
    static void getVolume( unsigned int& left, unsigned int& right );
    static void setVolume( unsigned int  left, unsigned int  right );

    static unsigned int leftVolume()  { unsigned int l, r; getVolume( l, r ); return l;     }
    static unsigned int rightVolume() { unsigned int l, r; getVolume( l, r ); return r;     }

    static void increaseVolume() { setVolume( leftVolume() + 1968, rightVolume() + 1968 ); }
    static void decreaseVolume() { setVolume( leftVolume() - 1966, rightVolume() - 1966 ); }

    long position() const;

public slots:
    // Convenience functions derived from the above functions.
    void setVolume( unsigned int level ) { setVolume( level, level ); }

    void start();
    void stop();

signals:
    void dataAvailable();

private:
    AudioInputPrivate *d;
};


#endif // AUDIOINPUT_H

