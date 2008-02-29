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

#ifndef __QAUDIOSTATEMANAGER_H__
#define __QAUDIOSTATEMANAGER_H__

#include <QObject>
#include <qtopiaglobal.h>
#include <qaudionamespace.h>

class QAudioStateInfo;
class QAudioStateManagerPrivate;

class QTOPIAAUDIO_EXPORT QAudioStateManager : public QObject
{
    Q_OBJECT

public:
    QAudioStateManager(QObject *parent = 0);
    ~QAudioStateManager();

public:
    bool setState(const QAudioStateInfo &state, QAudio::AudioCapability capability);

    bool setProfile(const QByteArray &profile);
    bool setDomain(const QByteArray &domain, QAudio::AudioCapability capability);

private:
    QAudioStateManagerPrivate *m_data;
    Q_DISABLE_COPY(QAudioStateManager)
};

#endif
