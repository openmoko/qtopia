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

#ifndef PLAYERCONTROL_H
#define PLAYERCONTROL_H

#include <qmediatools.h>
#include <qmediacontent.h>
#include <qmediacontrol.h>

#include <QtCore>

class PlayerControl : public QObject
{
    Q_OBJECT
public:
    PlayerControl( QObject* parent = 0 );
    ~PlayerControl();

    enum State { Playing, Paused, Stopped };

    void open( const QString& url );
    void open( const QContent& content );

    State state() const { return m_state; }
    void setState( State state );

    int volume() const { return m_volume; }

signals:
    void contentChanged( QMediaContent* content );
    void stateChanged( PlayerControl::State state );

public slots:
    void setVolume( int volume );
    void setMute( bool mute );

private slots:
    void activate();

private:
    void setMediaContent( QMediaContent* content );

    QMediaControlNotifier *m_notifier;
    QMediaContent *m_content;
    QMediaControl *m_control;
    State m_state;
    int m_volume;
    bool m_ismute;
};

#endif // PLAYERCONTROL_H
