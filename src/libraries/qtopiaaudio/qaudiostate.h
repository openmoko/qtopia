/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __QAUDIOSTATE_H__
#define __QAUDIOSTATE_H__

#include <qaudionamespace.h>
#include <QObject>

class QAudioStateInfo;

class QTOPIAAUDIO_EXPORT QAudioState : public QObject
{
    Q_OBJECT

public:
    explicit QAudioState(QObject *parent = 0);
    virtual ~QAudioState();

    virtual QAudioStateInfo info() const = 0;
    virtual QAudio::AudioCapabilities capabilities() const = 0;

    virtual bool isAvailable() const = 0;
    virtual bool enter(QAudio::AudioCapability capability) = 0;
    virtual bool leave() = 0;

signals:
    void availabilityChanged(bool available);
    void doNotUseHint();
    void useHint();
};

#endif
