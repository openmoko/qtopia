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

#ifndef ACTIONLISTVIEW_H
#define ACTIONLISTVIEW_H

#include <qlistwidget.h>
#include <quuid.h>
#include "emailfolderlist.h"

class MailboxList;
class ActionListItem;

class ActionListView : public QListWidget
{
    Q_OBJECT
public:
    ActionListView(QWidget *parent);
    virtual ~ActionListView();
    ActionListItem *newItem( const char *name, const char* context, bool useTr);
    ActionListItem *actionItemFromIndex( QModelIndex index ) const;
    void updateFolderStatus( const QString &, const QString &, IconType );
    void setCurrentFolder(const QString &);

signals:
    void composeMessage();
    void displayFolder(const QString &);
    void emailSelected();
    void currentFolderChanged(const QString &);

protected slots:
    void itemSlot(QListWidgetItem *item);
    void currentFolderChanged(int); 

private:
    ActionListItem *folder(const QString &);

    QListWidgetItem *mComposeItem;
    QListWidgetItem *mInboxItem;
    QListWidgetItem *mSentItem;
    QListWidgetItem *mDraftsItem;
    QListWidgetItem *mTrashItem;
    QListWidgetItem *mOutboxItem;
    QListWidgetItem *mEmailItem;
    QListWidgetItem *mSearchItem;
};

#endif
