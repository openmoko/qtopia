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

#ifndef MEDIABROWSER_H
#define MEDIABROWSER_H

#include "playercontrol.h"

#include <qmediacontent.h>
#include <private/requesthandler_p.h>
#include <private/menumodel_p.h>
#include <private/menuview_p.h>
#include <private/activitymonitor_p.h>

class TitleBar;
class PlaylistMenuModel;
class ItemDelegate;
class CustomView;
class AnimationDirector;
class HelpDirector;
class IndexHistory;
class ActionGroup;
class Playlist;

class MediaBrowser : public QWidget
{
    Q_OBJECT
public:
    MediaBrowser( PlayerControl* control, RequestHandler* handler = 0, QWidget* parent = 0 );
    ~MediaBrowser();

    void setCurrentPlaylist( Playlist* playlist );

    bool hasBack() const;
    void goBack();

    // QWidget
    bool eventFilter( QObject* o, QEvent* e );

public slots:
    void setMediaContent( QMediaContent* content );

private slots:
    void executeSelectedAction( const QModelIndex& index );
    void executeHeldAction( const QModelIndex& index );
    void executeHeldLongAction( const QModelIndex& index );

    void removePlaylistItem();
    void clearPlaylist();

    void savePlaylist();

    void generateMyShuffle();
    void resetMyShuffle();

    // ### HACK
    void enableNowPlaying();

    void directCurrentChange();
    void directSelectRelease();

    void executeShowPlayerRequest();

private:
    RequestHandler *m_requesthandler;

    CustomView *m_view;
    MenuStack *m_stack;

    AnimationDirector *m_director;
    HelpDirector *m_helpdirector;

    ItemDelegate *m_delegate;

    IndexHistory *m_history;

    SimpleMenuModel *m_mainmenu;
    PlaylistMenuModel *m_currentplaylistmenu;

    ActionGroup *m_removegroup;
    ActionGroup *m_savegroup;
    ActionGroup *m_resetgroup;

    TitleBar *m_titlebar;

    Playlist *m_playlist;
    bool m_hasnowplaying;

    ActivityMonitor *m_browsermonitor;
    bool m_focused;
};

#endif // MEDIABROWSER_H
