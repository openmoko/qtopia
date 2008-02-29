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

#ifndef __QTOPIA_SERVER_AUDIOVOLUMEMANAGERTASK_H
#define __QTOPIA_SERVER_AUDIOVOLUMEMANAGERTASK_H

#include <qstring.h>
#include <qlist.h>
#include <qmap.h>
#include <qtopiaipcadaptor.h>

class AudioVolumeManager : public QtopiaIpcAdaptor
{
    Q_OBJECT
    typedef QMap<QString, QString>  VolumeServiceProviders;
    typedef QList<QString>          VolumeDomains;

public:
    AudioVolumeManager();
    ~AudioVolumeManager();

    bool canManageVolume() const;

public slots:
    void setVolume(int volume);
    void increaseVolume(int increment);
    void decreaseVolume(int decrement);
    void setMuted(bool mute);

    void registerHandler(QString const& domain, QString const& channel);
    void unregisterHandler(QString const& domain, QString const& channel);
    void setActiveDomain(QString const& domain);
    void resetActiveDomain(QString const& domain);

private:
    QString findProvider() const;

    VolumeDomains           m_domains;
    VolumeServiceProviders  m_vsps;
};

#endif  // __QTOPIA_SERVER_AUDIOVOLUMEMANAGERTASk_H
