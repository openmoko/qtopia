/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef QSOUNDCONTROL_H
#define QSOUNDCONTROL_H

#include <QtGui>

// TODO: Messages on QPE/QSound and QPE/MediaServer originate from Qt-Embedded land
//       over QCop.  Need to figure out how to move to DBUS or perhaps keep as is?
#ifndef QT_NO_COP
#include <qcopchannel_qws.h>
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

#ifndef QT_NO_COP
private slots:
    void processMessage( const QString& msg, const QByteArray& data );
#endif

private:
    QSound *m_sound;
    QUuid m_id;

    int m_volume;
    Priority m_priority;

#ifndef QT_NO_COP
    QCopChannel *m_channel;
#endif
};

#endif // QSOUNDCONTROL_H
