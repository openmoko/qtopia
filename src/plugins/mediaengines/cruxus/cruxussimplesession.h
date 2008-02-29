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

#ifndef __CRUXUS_SIMPLESESSION_H
#define __CRUXUS_SIMPLESESSION_H

#include <QMediaHandle>
#include <QMediaServerSession>


class QMediaDevice;
class QMediaDecoder;

namespace cruxus
{

class SimpleSessionPrivate;

class SimpleSession : public QMediaServerSession
{
public:
    SimpleSession(QMediaHandle const& handle,
                  QMediaDevice* source,
                  QMediaDecoder* coder,
                  QMediaDevice* sink);
    ~SimpleSession();

    void start();
    void pause();
    void stop();

    void suspend();
    void resume();

    void seek(quint32 ms);
    quint32 length();

    void setVolume(int volume);
    int volume() const;

    void setMuted(bool mute);
    bool isMuted() const;

    QtopiaMedia::State playerState() const;

    QString errorString();

    void setDomain(QString const& domain);
    QString domain() const;

    QStringList interfaces();

    QString id() const;
    QString reportData() const;

private:
    SimpleSessionPrivate* d;
};

}   // ns cruxus

#endif  // __CRUXUS_SIMPLESESSION_H
