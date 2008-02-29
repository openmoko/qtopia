/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H


#include <qmainwindow.h>
#include <qframe.h>
#include <qtopia/qlibrary.h>
#include <qtopia/mediaplayerplugininterface.h>


class DocLnk;
class AudioWidget;


class MediaPlayer : public QObject {
    Q_OBJECT
public:
    MediaPlayer( QObject *parent, const char *name );
    ~MediaPlayer();

private slots:
    void setPlaying( bool );
    void pauseCheck( bool );
    void play();
    void next();
    void prev();

    void startDecreasingVolume();
    void startIncreasingVolume();
    void startScanningBackward();
    void startScanningForward();

    void stopDecreasingVolume();
    void stopIncreasingVolume();
    void stopScanningBackward();
    void stopScanningForward();

    void updateOnscreenDisplay();
    void hideOnscreenDisplay();

public slots:
    void openURL( const QString &url, const QString &mimetype );
    bool eventFilter( QObject *o, QEvent *e );
    bool keyPressEvent( QKeyEvent *e );
    bool keyReleaseEvent( QKeyEvent *e );

protected:
    void timerEvent( QTimerEvent *e );
    void setPlaying( const QString& URL );

private:
    void error( const QString& error, const QString& errorMsg ); 
    const DocLnk *currentFile;
    int upTimerId;
    int dnTimerId;
    int rtTimerId;
    int ltTimerId;
    bool scanning;
};


#endif // MEDIA_PLAYER_H

