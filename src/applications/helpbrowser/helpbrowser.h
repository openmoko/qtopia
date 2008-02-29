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

#ifndef HELPBROWSER_H
#define HELPBROWSER_H

#include "bookmark.h"

#ifdef QTOPIA_PHONE
# include <qtopia/qsoftmenubar.h>
#endif
#include <qtopiaabstractservice.h>

#include <QMainWindow>
#include <QTextBrowser>
#include <QStack>
#include <QUrl>
#include "navigationbar_p.h"

class QAction;
class QLabel;
class BookmarksUI;
//class Bookmark;

// PagePlaceInfo retains information about a URL page prior to leaving it. This information
// includes its filename and the vertical scrollbar location. Although this struct is not
// part of the public API, a complete declaration is required here, since it is used as the
// content of generic lists.
struct PagePlaceInfo
{
    PagePlaceInfo() {} // Required for generic lists
    PagePlaceInfo(const QString &_name,const QString &_title,int _vPlace) : name(_name), title(_title), vPlace(_vPlace) {}

    // The filename of the URL.
    QString name;
    // The document title.
    QString title;
    // The location of the vertical scrollbar when the page was last seen.
    int vPlace;
};

class MagicTextBrowser : public QTextBrowser {
    Q_OBJECT
public:
    MagicTextBrowser( QWidget* parent );

    // Clear page source and history
    void clear();

    // Return page source
    QString source() { return current; }

    // Returns the current documents title.
    QString title() { return documentTitle(); }

    // Set page source
    void setSource( const QUrl& );

    // Checks the history, and emits the signals hasBack(bool) and hasForward(bool) appropriately.
    // This is useful to call when initialising widgets that rely on these signals.
    void emitStatus();

    virtual QVariant loadResource (int type, const QUrl &name);

signals:
    // Back status changed
    void hasBack( bool );

    // Forward status changed
    void hasForward( bool );

    // When the current document changes, the pointer in the history queue changes, and this signal
    // is emitted, supplying the titles of the changed documents on either side of the current document.
    // If either document is not available, an empty string is supplied. This signal should be emitted
    // at the same time as either hasBack(bool) and hasForward(bool).
    void historyChanged(const QString &previous,const QString &next);

public slots:
    // Go to previous page in history
    void backward();

    // Go to next page in history
    void forward();

private:
    // Display source and set as current
    void setCurrent( const QString& file );

    // Replace qtopia tags with help page links
    bool magic( const QString&, const QString&, const QString& );

    // Generate help page links
    QString generate( const QString& );

    // Determines the current state of the history, and emits a historyChanged() signal.
    void emitHistoryChanged();

    QString current;
    QStack< PagePlaceInfo > backStack, forwardStack;
};

class HelpBrowser : public QMainWindow
{
    Q_OBJECT
public:
    HelpBrowser( QWidget* parent=0, Qt::WFlags f=0 );

    virtual ~HelpBrowser();

#ifdef QTOPIA_PHONE
    bool eventFilter( QObject*, QEvent* );
#endif

public slots:
    void setDocument( const QString &doc );

    void bookmarkSelected(Bookmark);

private slots:
    void goHome();

    void bookmarks();

    void addBookmark();

    void textChanged();

    void forward();

    void backward();

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

#ifdef QTOPIA_PHONE
    QMenu *contextMenu;
#endif
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
