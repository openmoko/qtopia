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

#ifndef MEDIABROWSER_H
#define MEDIABROWSER_H

#include "playercontrol.h"

#include <qmediacontent.h>
#include <requesthandler.h>
#include <menumodel.h>
#include <menuview.h>

class TitleBar;
class PlaylistMenuModel;
class ItemDelegate;
class PaletteBlend;
class IndexHistory;
class ActionGroup;

class MediaBrowser : public QWidget
{
    Q_OBJECT
public:
    MediaBrowser( PlayerControl* control, RequestHandler* handler = 0, QWidget* parent = 0 );
    ~MediaBrowser();

    void setCurrentPlaylist( Playlist* playlist );

    bool hasBack() const;
    void goBack();

public slots:
    void setMediaContent( QMediaContent* content );

private slots:
    void executeSelectedAction( const QModelIndex& index );
    void executeHeldAction( const QModelIndex& index );

    void removePlaylistItem();
    void clearPlaylist();

    void savePlaylist();

private:
    RequestHandler *m_requesthandler;

    MenuView *m_view;
    MenuStack *m_stack;

    ItemDelegate *m_delegate;
    PaletteBlend *m_paletteblend;
    QTimeLine *m_timeline;

    IndexHistory *m_history;

    SimpleMenuModel *m_mainmenu;
    PlaylistMenuModel *m_currentplaylistmenu;

    ActionGroup *m_removegroup;
    ActionGroup *m_savegroup;

    TitleBar *m_titlebar;
};

#endif // MEDIABROWSER_H
