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

#ifndef BOOKMARKSUI_H
#define BOOKMARKSUI_H

#include "bookmark.h"

#include <QDialog>
class QAction;
class QUrl;
#ifdef QTOPIA_PHONE
class QMenu;
#endif
class BookmarkList;


class BookmarksUI : public QDialog
{
    Q_OBJECT

public:

    BookmarksUI(QWidget *parent = 0);

    virtual ~BookmarksUI();

    bool addBookmark(const QUrl &url,const QString &title);

public slots:

    void setReorganise(bool checked = false);

signals:

    void bookmarkSelected(Bookmark);

private slots:

    void deleteBookmarks();

    void selectionChanged();

    void handleBookmarkSelected(Bookmark bookmark);

    void handleDisplayModeChanged(bool displayMode);

private:

    void init();

#ifdef QTOPIA_PHONE
    QMenu *contextMenu;
#endif

    // List widget - holds a list of Bookmark objects.
    BookmarkList *listUI;

    // Menu action for deleting a Bookmark.
    QAction *deleteAction;

    // Menu action for toggling between 'Reorganise' (Shift) and 'Navigate' (Display) modes.
    QAction *reorganiseAction;
};

#endif
