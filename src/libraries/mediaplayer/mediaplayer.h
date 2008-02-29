/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H


#include <qwidgetstack.h>
#include "mediaplayerstate.h"


class DocLnk;
class ControlWidgetBase;
class LoopControlBase;
class MediaSelectorBase;
class AudioDevice;


class MediaPlayer : public QWidgetStack
{
    Q_OBJECT
public:
    MediaPlayer( QWidget *parent, const char *name = 0, WFlags fl = 0 );
    ~MediaPlayer();

    virtual ControlWidgetBase *createAudioUI() { return 0; }
    virtual ControlWidgetBase *createVideoUI() { return 0; }
    virtual LoopControlBase *createLoopControl() { return 0; }
    virtual MediaSelectorBase *createMediaSelector() { return 0; }
    virtual AudioDevice *createAudioDevice() { return 0; }

    // Exposed public functionality 
    void playDocLnk(const DocLnk *lnk);
    void playURL(const QString &url, const QString &mimetype);

public slots:
    ControlWidgetBase *audioUI();
    ControlWidgetBase *videoUI();
    AudioDevice *audioDevice();

    void setView( View );
    void createMediaPlayer();
    virtual void setDocument( const QString& fileref );
    void appMessage( const QCString &msg, const QByteArray &data );

    void setPlaying( bool );
    void pauseCheck( bool );

    void startDecreasingVolume();
    void startIncreasingVolume();
    void startScanningBackward();
    void startScanningForward();

    void stopDecreasingVolume();
    void stopIncreasingVolume();
    void stopScanningBackward();
    void stopScanningForward();

protected:
    void timerEvent( QTimerEvent *e );
    void closeEvent( QCloseEvent *ce );
    bool eventFilter( QObject *o, QEvent *e );

    void setTickerText(QString file);
    void error(const QString& error, const QString& errorMsg); 
    
    void setPlaying(const QString& URL);
    void playFile(const QString &file, const QString &name);
    void play();
    void stop(bool);

    enum ScanDirection { Left, Right };
    void internalScan(ScanDirection scanDirection);

private:
    int scanSpeed;
    int upTimerId;
    int dnTimerId;
    int rtTimerId;
    int ltTimerId;
    bool scanning;
    ControlWidgetBase *aw;
    ControlWidgetBase *vw;
    LoopControlBase *loopControlBase;
    AudioDevice *ad;
    MediaSelectorBase *list;
    DocLnk *docLnk;
    QString docString;
    QString url, mimetype;
    bool disabledSuspendScreenSaver;
    bool previousSuspendMode;
};


#endif // MEDIA_PLAYER_H

