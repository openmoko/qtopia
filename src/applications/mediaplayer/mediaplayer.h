/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include "playlist.h"

#include <QtGui>

class PlayerControl;
class PlayerWidget;
class MediaBrowser;
class RequestHandler;

class MediaPlayer : public QWidget
{
    Q_OBJECT
public:
    MediaPlayer( QWidget* parent = 0, Qt::WFlags f = 0 );

    void setPlayerVisible( bool visible );

    // Open playlist in player
    void setPlaylist( Playlist* playlist );
    Playlist* playlist() const { return m_playlist; }

    // Open url in player
    void openUrl( const QString& url );

public slots:
    // Load and parse playlist from file
    void setDocument( const QString& doc );

private slots:
    void playingChanged( const QModelIndex& index );

protected:
    void closeEvent( QCloseEvent* e );

private:
    QLayout *m_layout;
    PlayerControl *m_playercontrol;
    PlayerWidget *m_playerwidget;
    bool m_closeonback;

    RequestHandler *m_requesthandler;
    MediaBrowser *m_mediabrowser;
    Playlist *m_playlist;
};

#endif // MEDIAPLAYER_H
