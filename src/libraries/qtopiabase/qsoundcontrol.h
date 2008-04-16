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

#ifndef QSOUNDCONTROL_H
#define QSOUNDCONTROL_H

#include <QtGui>

// TODO: Messages on QPE/QSound and QPE/MediaServer originate from Qt-Embedded land
//       over QCop.  Need to figure out how to move to DBUS or perhaps keep as is?
#if defined(Q_WS_QWS)
#include <qcopchannel_qws.h>
#elif defined(Q_WS_X11)
#include <qcopchannel_x11.h>
#endif

#include <qtopiaglobal.h>

class QTOPIABASE_EXPORT QSoundControl : public QObject
{
    Q_OBJECT
public:
    explicit QSoundControl( QSound* sound, QObject* parent = 0 );

    void setVolume( int volume );
    int volume() const { return m_volume; }

    enum Priority { Default, RingTone };

    void setPriority( Priority priority );
    Priority priority() const { return m_priority; }

    QSound* sound() const { return m_sound; }

signals:
    // Sound has finished playing
    void done();

#if !defined(QT_NO_COP) || defined(Q_WS_X11)
private slots:
    void processMessage( const QString& msg, const QByteArray& data );
#endif

private:
    QSound *m_sound;
    QUuid m_id;

    int m_volume;
    Priority m_priority;

#if !defined(QT_NO_COP) || defined(Q_WS_X11)
    QCopChannel *m_channel;
#endif
};

#endif // QSOUNDCONTROL_H
