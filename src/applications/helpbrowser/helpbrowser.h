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

#ifndef HELPBROWSER_H
#define HELPBROWSER_H

#include "bookmark.h"

#include <qtopia/qsoftmenubar.h>
#include <qtopiaabstractservice.h>

#include <QMainWindow>
#include <QTextBrowser>
#include <QStack>
#include <QUrl>
#include "navigationbar_p.h"

class QAction;
class QLabel;
class BookmarksUI;

class MagicTextBrowser : public QTextBrowser {
    Q_OBJECT
public:
    MagicTextBrowser( QWidget* parent );

    virtual QVariant loadResource (int type, const QUrl &name);

private:
    // Generate help page links
    QString generate( const QString& );
};

class HelpBrowser : public QMainWindow
{
    Q_OBJECT
public:
    HelpBrowser( QWidget* parent=0, Qt::WFlags f=0 );

    virtual ~HelpBrowser();

    bool eventFilter( QObject*, QEvent* );

public slots:
    void setDocument( const QString &doc );

    void bookmarkSelected(Bookmark);

private slots:
    void goHome();

    void bookmarks();

    void addBookmark();

    void textChanged();

protected:
    void closeEvent( QCloseEvent* );

private:
    void init();

    BookmarksUI *getBookmarksUI();

    MagicTextBrowser *browser;
    QAction *backAction, *forwardAction;
#ifdef DEBUG
    QLabel *location;
#endif
    NavigationBar *navigationBar;


    // The UI for displaying and selecting bookmarks. Created lazily. This data member
    // should never be referenced directly, but should be fetched via getBookmarksUI().
    BookmarksUI *bookmarksUI;

    QMenu *contextMenu;
};

class HelpService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class HelpBrowser;
private:
    HelpService( HelpBrowser *parent )
        : QtopiaAbstractService( "Help", parent )
        { this->parent = parent; publishAll(); }

public:
    ~HelpService();

public slots:
    void setDocument( const QString& doc );

private:
    HelpBrowser *parent;
};

#endif // HELPBROWSER_H
