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

#ifndef _VIDEORINGTONE_H_
#define _VIDEORINGTONE_H_

#include <QObject>


class QWidget;

class VideoRingtonePrivate;

class VideoRingtone : public QObject
{
    Q_OBJECT

public:
    ~VideoRingtone();

    void playVideo(const QString& fileName);
    void stopVideo();
    QWidget* videoWidget();

    static VideoRingtone* instance();

signals:
    void videoRingtoneFailed();
    void videoWidgetReady();
    void videoRingtoneStopped();

private:
    VideoRingtone();

    VideoRingtonePrivate *d;
};

#endif // _VIDEORINGTONE_H_
