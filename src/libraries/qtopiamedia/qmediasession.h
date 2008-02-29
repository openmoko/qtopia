/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef __QTOPIAMEDIA_MEDIASESSION_H
#define __QTOPIAMEDIA_MEDIASESSION_H

#include "qmediacontrol.h"
#include "qmediadeviceinfo.h"
#include "qmediadevicerep.h"
#include "qmediacodecinfo.h"
#include "qmediacodecrep.h"

class QMediaSessionPrivate;

class QMediaSession : public QObject
{
    Q_OBJECT

    friend class QMediaSessionPrivate;

public:
    explicit QMediaSession(QString const& domain = QLatin1String("default"), QObject* parent = NULL);
    ~QMediaSession();

    QMediaCodecInfoList availableEncoders() const;
    QMediaCodecInfoList availableDecoders() const;
    QMediaDeviceInfoList availableMediaDevices() const;

    QMediaCodecInfo defaultEncoder() const;
    QMediaDeviceInfo defaultOuptutDevice() const;
    QMediaDeviceInfo defaultInputDevice() const;

    int preparePlay(QString const& url,
                        QMediaCodecRep const& codec,
                        QMediaDeviceRep const& device);

    int prepareRecord(QString const& url,
                        QMediaCodecRep const& codec,
                        QMediaDeviceRep const& device);

    int preparePlayAndRecord(QMediaCodecRep const& codec,
                                QMediaDeviceRep const& inputSink,
                                QMediaDeviceRep const& outputSink,
                                QMediaDeviceRep const& inputSource,
                                QMediaDeviceRep const& outputSource);

signals:
    void mediaReady(int key, QMediaControl* mediaControl);
    void mediaError(int key, QString error);

private:
    QMediaSessionPrivate*   d;
};

#endif  //__QTOPIAMEDIA_MEDIASESSION_H
