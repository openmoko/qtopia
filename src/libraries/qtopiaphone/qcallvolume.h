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

#ifndef QCALLVOLUME_H
#define QCALLVOLUME_H

#include <qcomminterface.h>

class QTOPIAPHONE_EXPORT QCallVolume : public QCommInterface
{
    Q_OBJECT
    Q_PROPERTY(int speakerVolume READ speakerVolume WRITE setSpeakerVolume);
    Q_PROPERTY(int minimumSpeakerVolume READ minimumSpeakerVolume);
    Q_PROPERTY(int maximumSpeakerVolume READ maximumSpeakerVolume);
    Q_PROPERTY(int microphoneVolume READ microphoneVolume WRITE setMicrophoneVolume);
    Q_PROPERTY(int minimumMicrophoneVolume READ minimumMicrophoneVolume);
    Q_PROPERTY(int maximumMicrophoneVolume READ maximumMicrophoneVolume);
public:
    explicit QCallVolume( const QString& service = QString(),
                          QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QCallVolume();

    int speakerVolume() const;
    int minimumSpeakerVolume() const;
    int maximumSpeakerVolume() const;

    int microphoneVolume() const;
    int minimumMicrophoneVolume() const;
    int maximumMicrophoneVolume() const;

signals:
    void speakerVolumeChanged(int volume);
    void microphoneVolumeChanged(int volume);

public slots:
    virtual void setSpeakerVolume( int volume );
    virtual void setMicrophoneVolume( int volume );
};

#endif /* QCALLVOLUME_H */
