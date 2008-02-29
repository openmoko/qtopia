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

