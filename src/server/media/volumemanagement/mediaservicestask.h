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

#ifndef __QTOPIA_SERVER_MEDIA_MEDIASERVICETASK_H
#define __QTOPIA_SERVER_MEDIA_MEDIASERVICETASK_H

#include "qtopiaserverapplication.h"

class AudioVolumeManager;
class MediaKeyService;

class MediaServicesTask : public QObject
{
    Q_OBJECT

public:
    MediaServicesTask();
    ~MediaServicesTask();
    
public slots:
    void setVolume(bool up);

signals:
    void volumeChanged(bool up);

private:
    AudioVolumeManager* m_avm;
    MediaKeyService*    m_mks;
};
QTOPIA_TASK_INTERFACE(MediaServicesTask);


#endif  // __QTOPIA_SERVER_MEDIA_MEDIASERVICETASK_H

