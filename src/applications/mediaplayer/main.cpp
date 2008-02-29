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
#include <qpe/qpeapplication.h>
#include "mediaplayerstate.h"
#include "playlistwidget.h"
#include "audiowidget.h"
#include "videowidget.h"
#include "loopcontrol.h"
#include "mediaplayer.h"


MediaPlayerState *mediaPlayerState = 0;
PlayListWidget *playList = 0;
LoopControl *loopControl = 0;


int main(int argc, char **argv) {
    QPEApplication a(argc,argv);

    MediaPlayerState st( 0, "mediaPlayerState" );
    mediaPlayerState = &st;

    PlayListWidget pl( 0, "playList" );
    playList = &pl;
    pl.setCaption( MediaPlayer::tr("Media Player") );
    pl.showMaximized(); // show First
    a.processEvents(); // (shows up quicker)
    
    st.loadPlugins(); // Load plugins and
    LoopControl lc( 0, "loopControl" );
    loopControl = &lc;
    MediaPlayer mp( 0, "mediaPlayer" );
    pl.loadFiles(); // Load files after getting the main window up

    // Set for recieving setDocument
    a.showMainDocumentWidget(&pl);

    return a.exec();
}


