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



#ifndef FOLDERLISTVIEW_H
#define FOLDERLISTVIEW_H

#include <qsettings.h>

#include "account.h"
#include "search.h"
#include "folder.h"
#include "emailfolderlist.h"

#include <QtopiaItemDelegate>

#include <qtreewidget.h>
#include <qtimer.h>
#include <qmenubar.h>
#include <qmenu.h>
#include <qwidget.h>
#include <qlineedit.h>

class AccountList;
class MailboxList;

class FolderListItem : public QTreeWidgetItem
{
public:
    FolderListItem(QTreeWidget *parent, Folder *in, const QString& mailboxName = QString::null);
    FolderListItem(QTreeWidgetItem *parent, Folder *in, const QString& mailboxName = QString::null);
    Folder* folder();
    void setStatusText( const QString &str, bool highlight, IconType type );
    void statusText( QString *str, bool *highlight, IconType *type );
    int depth();

protected:
    void init(QString name);

#ifdef QTOPIA4_TODO
    void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
    int width( const QFontMetrics& fm,  const QTreeWidget* lv, int c ) const;
#endif
    QString key(int c, bool) const;

private:
    Folder *_folder;
    QString _statusText;
    bool _highlight;
    IconType _type;
};

class FolderListView : public QTreeWidget
{
    Q_OBJECT
public:
    FolderListView(MailboxList *list, QWidget *parent, const char *name);
    virtual ~FolderListView();
    void setupFolders(AccountList *list);
    QModelIndex next(QModelIndex mi, bool nextParent = false);
    QTreeWidgetItem* next(QTreeWidgetItem *item);
    void updateAccountFolder(MailAccount *account);
    void deleteAccountFolder(MailAccount *account);

    Folder* currentFolder() const;
    bool setCurrentFolder(const Folder* folder);

    MailAccount* currentAccount() const;

    void changeToSystemFolder(const QString &str);
    void updateFolderStatus(const QString &mailbox, const QString &txt, bool highlight, IconType type);
    void updateAccountStatus(const Folder *account, const QString &txt, bool highlight, IconType type);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    FolderListItem *folderItemFromIndex( QModelIndex index ) const;
    FolderListItem *currentFolderItem() const;
    void restoreCurrentFolder();
    void rememberCurrentFolder();

signals:
    void emptyFolder();
    void folderSelected(Folder *);
    void viewMessageList();
    void finished();

public slots:
    void popFolderSelected(int);

protected slots:
    virtual void keyPressEvent( QKeyEvent *e );

    void folderChanged(QTreeWidgetItem *);
    void itemClicked(QTreeWidgetItem *);

private:
    bool selectedChildOf(FolderListItem *folder);
    void buildImapFolder(FolderListItem* item, MailAccount* account);
    FolderListItem* getParent(FolderListItem *parent, QString name, QString delimiter);

private:
    MailboxList *_mailboxList;
    QModelIndex currentIndex;
};

class FolderListItemDelegate : public QtopiaItemDelegate
{
    Q_OBJECT

public:
    FolderListItemDelegate(FolderListView *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                    const QModelIndex &index) const;
    void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                     const QRect &rect, const QString &text) const;

private:
    FolderListView *mParent;
    mutable QString statusText;
    mutable IconType type;
};

#endif
