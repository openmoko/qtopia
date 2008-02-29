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

#ifndef __QAUDIOSTATECONFIGURATION_H__
#define __QAUDIOSTATECONFIGURATION_H__

#include <QObject>
#include <qtopiaglobal.h>
#include <qaudionamespace.h>

class QByteArray;
class QAudioStateInfo;

class QAudioStateConfigurationPrivate;
class QTOPIAAUDIO_EXPORT QAudioStateConfiguration : public QObject
{
    Q_OBJECT

    friend class QAudioStateConfigurationPrivate;

public:
    explicit QAudioStateConfiguration(QObject *parent = 0);
    ~QAudioStateConfiguration();

    bool isInitialized() const;

    QSet<QByteArray> domains() const;
    QSet<QAudioStateInfo> states() const;
    QSet<QAudioStateInfo> states(const QByteArray &domain) const;

    bool isStateAvailable(const QAudioStateInfo &state) const;
    QAudio::AudioCapabilities availableCapabilities(const QAudioStateInfo &state) const;

    QAudioStateInfo currentState() const;
    QAudio::AudioCapability currentCapability() const;

signals:
    void configurationInitialized();
    void availabilityChanged();
    void currentStateChanged(const QAudioStateInfo &state, QAudio::AudioCapability capability);

private:
    QAudioStateConfigurationPrivate *m_data;
};


#endif
