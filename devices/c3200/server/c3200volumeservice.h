/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __QTOPIA_MEDIA_DEFAULTVOLUMEPROVIDER_H
#define __QTOPIA_MEDIA_DEFAULTVOLUMEPROVIDER_H

#include <qtopiaipcadaptor.h>

class C3200VolumeService : public QtopiaIpcAdaptor
{
    Q_OBJECT
    enum AdjustType { Relative, Absolute };

public:
    C3200VolumeService();
    ~C3200VolumeService();

public slots:
    void setVolume(int volume);
    void setVolume(int leftChannel, int rightChannel);
    void increaseVolume(int increment);
    void decreaseVolume(int decrement);
    void setMute(bool mute);

private slots:
    void registerService();
    void setCallDomain();

private:
    void adjustVolume(int leftChannel, int rightChannel, AdjustType);

    int m_leftChannelVolume;
    int m_rightChannelVolume;
};


#endif  // __QTOPIA_MEDIA_DEFAULTVOLUMEPROVIDER_H
