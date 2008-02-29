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

#ifndef BOOKMARKLIST_H
#define BOOKMARKLIST_H

#include "bookmark.h"

#include <QListView>

class BookmarkModel;
class BookmarkDelegate;
class QKeyEvent;
class QModelIndex;


class BookmarkList : public QListView
{
    Q_OBJECT

public:

    BookmarkList(QWidget *parent = 0);

    virtual ~BookmarkList();

    BookmarkModel *getBookmarkModel() const;

    bool addItem(const Bookmark &,bool refresh = false);

    Bookmark getItem(int row) const;

    void select(int row);

    void setDisplayMode(bool displaymode);

signals:

    void bookmarkSelected(Bookmark);

    void displayModeChanged(bool displayMode);

protected:

    void keyPressEvent(QKeyEvent * event);

private:

    void init();

    void select(const QModelIndex &);
    void selectCurrent();

    // Data model for this list.
    BookmarkModel *bookmarkModel;

    // Renderer for this list.
    BookmarkDelegate *delegate;

    // List's mode - Display/Shift.
    bool displayMode;
};

#endif
