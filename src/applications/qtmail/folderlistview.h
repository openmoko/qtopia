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



#ifndef FOLDERLISTVIEW_H
#define FOLDERLISTVIEW_H

#include <qsettings.h>

#include "account.h"
#include "search.h"
#include "folder.h"

#include <qtreewidget.h>
#include <qtimer.h>
#include <qmenubar.h>
#include <qmenu.h>
#include <qwidget.h>
#include <qlineedit.h>
#include <qitemdelegate.h>

class AccountList;
class MailboxList;


class FolderListItem : public QTreeWidgetItem
{
public:
    FolderListItem(QTreeWidget *parent, Folder *in);
    FolderListItem(QTreeWidgetItem *parent, Folder *in);
    Folder* folder();
    void setStatusText( const QString &str, bool highlight, const QColor &col );
    void statusText( QString *str, bool *highlight, QColor *col );
    int depth();

protected:
#ifdef QTOPIA4_TODO
    void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
    int width( const QFontMetrics& fm,  const QTreeWidget* lv, int c ) const;
#endif
    QString key(int c, bool) const;


private:
    Folder *_folder;
    QString _statusText;
    bool _highlight;
    QColor _col;
};

class FolderListView : public QTreeWidget
{
    Q_OBJECT
public:
    FolderListView(MailboxList *list, QWidget *parent, const char *name);
    virtual ~FolderListView();
    QMenu* folderParentMenu(QMenuBar *host);
    void showFolderChoice(FolderListItem *);
    void setupFolders(AccountList *list);
    QModelIndex next(QModelIndex mi, bool nextParent = false);
    QTreeWidgetItem* next(QTreeWidgetItem *item);
    void updateAccountFolder(MailAccount *account);
    void deleteAccountFolder(MailAccount *account);

    Folder* currentFolder();
    MailAccount* currentAccount();
    void changeToSystemFolder(const QString &str);
    void updateFolderStatus(const QString &mailbox, const QString &txt, bool highlight, const QColor &col = Qt::blue);
    void updateAccountStatus(const Folder *account, const QString &txt, bool highlight, const QColor &col = Qt::blue);

    void setLastSearch(Search *search);
    Search *lastSearch();

    void saveQueries(QString mainGroup, QSettings *config);
    void readQueries(QString mainGroup, QSettings *config);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    FolderListItem *folderItemFromIndex( QModelIndex index );

signals:
    void emptyFolder();
    void folderSelected(Folder *);
    void viewMessageList();

public slots:
    void newQuery();
    void modifyQuery();
    void deleteFolder();

    void showFolderMenu();
    void popFolderSelected(int);

protected slots:
    virtual void keyPressEvent( QKeyEvent *e );

    void folderChanged(QTreeWidgetItem *);
    void cancelMenuTimer();
    void itemClicked(QTreeWidgetItem *);
    void mousePressEvent( QMouseEvent * e );
    void mouseReleaseEvent( QMouseEvent * e );

private:
    bool selectedChildOf(FolderListItem *folder);
    void buildImapFolder(FolderListItem* item, MailAccount* account);
    void saveSingleQuery(QSettings *config, Folder *folder);
    Search* readSingleQuery(QSettings *config);
    FolderListItem* getParent(FolderListItem *parent, QString name, QString delimiter);

private:
    QTimer menuTimer;
    FolderListItem *itemToRename;
    QMenu *folderBar;
    QAction *newQueryAction, *modifyQueryAction, *deleteFolderAction;

    SystemFolder *searchFolder;

    MailboxList *_mailboxList;
};

class FolderListItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    FolderListItemDelegate(FolderListView *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

private:
    FolderListView *mParent;
};

#endif
