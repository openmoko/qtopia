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
#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H


#include <qobject.h>


class AudioDevicePrivate;


class AudioDevice : public QObject {
    Q_OBJECT
public:
    AudioDevice(QObject *parent=0, const char *name=0);
    ~AudioDevice();

    void open(unsigned int freq = 44000, unsigned int channels = 2, unsigned int bytesPerSample = 2, bool needCompletedSignal = true);
    void close();

    unsigned int canWrite() const;
    bool write( char *buffer, unsigned int length );
    int bytesWritten();

    unsigned int channels() const;
    unsigned int frequency() const;
    unsigned int bytesPerSample() const;
    unsigned int bufferSize() const;

    // Each volume level is from 0 to 0xFFFF
    static void volume(int &left, int &right);
    static void setVolume(int left, int right);

    static bool muted();
    static void setMuted(bool);

    static void increaseVolume() { int l,r; volume(l,r); setVolume(l+1968,r+1968); }
    static void decreaseVolume() { int l,r; volume(l,r); setVolume(l-1966,r-1966); }

public slots:
    void volumeChanged( bool muted );

signals:
    void completedIO();
    void deviceReady();
    void deviceOpenError();

private:
    AudioDevicePrivate *d;
};


#endif // AUDIODEVICE_H

