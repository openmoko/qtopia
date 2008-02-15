/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef __QTOPIA_CRUXUS_WAVDECODER_H
#define __QTOPIA_CRUXUS_WAVDECODER_H


#include <qtopiamedia/media.h>
#include <QMediaDecoder>


class QMediaDevice;

class WavDecoderPrivate;

class WavDecoder : public QMediaDecoder
{
    Q_OBJECT

public:
    WavDecoder();
    ~WavDecoder();

    QMediaDevice::Info const& dataType() const;

    bool connectToInput(QMediaDevice* input);
    void disconnectFromInput(QMediaDevice* input);

    void start();
    void stop();
    void pause();

    quint64 length();
    bool seek(qint64 ms);

    void setVolume(int volume);
    int volume();

    void setMuted(bool mute);
    bool isMuted();

private:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

    WavDecoderPrivate* d;
};


#endif  // __QTOPIA_CRUXUS_WAVDECODER_H
