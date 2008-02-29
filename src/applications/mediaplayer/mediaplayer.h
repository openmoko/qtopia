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

#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include "playlist.h"

#include <QtGui>

class PlayerControl;
class PlayerWidget;
class MediaBrowser;
class RequestHandler;
class QMediaContentContext;

class MediaPlayer : public QWidget
{
    Q_OBJECT
public:
    MediaPlayer( QWidget* parent = 0, Qt::WFlags f = 0 );
    ~MediaPlayer();

    bool isPlayerVisible() const;
    void setPlayerVisible( bool visible );

    // Open playlist in player
    void setPlaylist( QExplicitlySharedDataPointer<Playlist> playlist );
    QExplicitlySharedDataPointer<Playlist> playlist() const { return m_playlist; }

    // Open url in player
    void openUrl( const QString& url );

    static MediaPlayer *instance();

public slots:
    // Load and parse playlist from file
    void setDocument( const QString& doc );

private slots:
    void playingChanged( const QModelIndex& index );

protected:
    void keyPressEvent( QKeyEvent* e );
    void closeEvent( QCloseEvent* e );

private:
    QLayout *m_layout;
    PlayerControl *m_playercontrol;
    PlayerWidget *m_playerwidget;
    bool m_closeonback, m_acceptclose;
    QMediaContentContext *context;

    RequestHandler *m_requesthandler;
    MediaBrowser *m_mediabrowser;
    QExplicitlySharedDataPointer< Playlist > m_playlist;
};

#endif // MEDIAPLAYER_H
