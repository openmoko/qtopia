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

#include "helpbrowser.h"
#include "helppreprocessor.h"
#include "bookmarksui.h"
#include "bookmark.h"

#include <qtopiaapplication.h>
#include <qtopialog.h>

#include <qcontent.h>
#include <qcontentset.h>

#include <qtopianamespace.h>
#include <qsoftmenubar.h>

#include <QAction>
#include <QMenu>
#include <QFile>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QScrollBar>
#include <QFileInfo>
#include <QStyle>
#include <QImageReader>

#define HOMEPAGE QUrl( "index.html" )


MagicTextBrowser::MagicTextBrowser( QWidget* parent )
    : QTextBrowser( parent )
{ }

QVariant MagicTextBrowser::loadResource(int type, const QUrl &name)
{
    if (type == QTextDocument::ImageResource) {
        QString filename(name.toLocalFile());
        // QUrl drops the ':' from the start of the name.
        if (filename.startsWith(QLatin1String("image/"))) {
            filename.prepend(":");
            QFileInfo fi(filename);
            if (fi.suffix() == "svg" || fi.suffix() == "pic") {
                // We'll force a sensible size, otherwise we could get
                // anything.
                int size = style()->pixelMetric(QStyle::PM_ListViewIconSize);
                QImageReader reader(filename);
                reader.setScaledSize(QSize(size, size));
                QImage img = reader.read();
                QPixmap pm = QPixmap::fromImage(img);
                return QVariant(pm);
            }
        } else if (filename.startsWith(QLatin1String("icon/"))) {
            filename.prepend(":");
            QFileInfo fi(filename);
            if (fi.exists()) {
                int size = style()->pixelMetric(QStyle::PM_SmallIconSize);
                QIcon icon(filename);
                return QVariant(icon.pixmap(size, size));
            }
        }
    } else if (type == QTextDocument::HtmlResource) {
        QString filename(name.toLocalFile());
        HelpPreProcessor hpp(filename);
        QString result = hpp.text();

        static const char* special[] = { "applications", "games", "settings", 0 };
        for (int i=0; special[i]; ++i) {
            QString specialname = special[i];
            if (filename.endsWith("qpe-" + specialname + ".html")) {
                QRegExp re( "<qtopia-" + specialname + ">.*</qtopia-" + specialname + ">" );
                int start;
                if( ( start = re.indexIn( result ) ) >= 0 ) {
                    specialname[0] = specialname[0].toUpper();
                    result.replace( start, re.matchedLength(), generate( specialname ) );
                }
                break;
            }
        }

        return QVariant(result);
    }

    return QTextBrowser::loadResource(type, name);
}


QString MagicTextBrowser::generate( const QString& name )
{
    QString s;
    QContentSet lnkset( QContentFilter::Category, name );
    typedef QMap<QString,QContent> OrderingMap;
    OrderingMap ordered;
    QContentList linkList = lnkset.items();
    foreach (const QContent &lnk, linkList) {
        ordered[Qtopia::dehyphenate( lnk.name() )] = lnk;
    }
    for( OrderingMap::ConstIterator mit=ordered.begin(); mit!=ordered.end(); ++mit ) {
        QString name = mit.key();
        const QContent &lnk = *mit;
        QString icon = ":image/" + lnk.iconName();
        QString helpFile = lnk.executableName() + ".html";
        QStringList helpPath = Qtopia::helpPaths();
        QStringList::ConstIterator it;
        const char* prefix[]={"","qpe-",0};
        int pref=0;
        for (; prefix[pref]; ++pref) {
            for (it = helpPath.begin(); it != helpPath.end() && !QFile::exists( *it + "/" + prefix[pref] + helpFile ); ++it)
                ;
            if (it != helpPath.end())
                break;
        }
        if (it != helpPath.end()) {
            s += QString("<br><a href=") + prefix[pref] + helpFile + "><img src=" + icon + "> " + name + "</a>\n";
#ifdef DEBUG
        } else {
            s += "<br>No <tt>" + helpFile + "</tt> for " + name + "\n";
#endif
        }
    }
    return s;
}

/*
XXX - needs Qt functionality.

void MagicTextBrowser::emitHistoryChanged()
{
    // Construct the parameters for the historyChanged() signal.
    QString prevDocTitle, nextDocTitle;
    if ( !backStack.isEmpty() ) {
        prevDocTitle = backStack.top().title;
    }
    if ( !forwardStack.isEmpty() ) {
        nextDocTitle = forwardStack.top().title;
    }

    emit historyChanged(prevDocTitle,nextDocTitle);
}
*/


HelpBrowser::~HelpBrowser()
{
    if ( bookmarksUI ) {
        delete bookmarksUI;
    }
}

HelpBrowser::HelpBrowser( QWidget* parent, Qt::WFlags f )
    : QMainWindow( parent, f ), bookmarksUI(0)
{
    init();
}

void HelpBrowser::init()
{
    QWidget *box = new QWidget(this);
    QBoxLayout *boxLayout = new QVBoxLayout( box );
    boxLayout->setMargin(0);
    boxLayout->setSpacing(0);
    browser = new MagicTextBrowser( box );
    boxLayout->addWidget(browser);

#ifdef DEBUG
    location = new QLabel( box );
    boxLayout->addWidget(location);
#endif

    navigationBar = new NavigationBar(this);

    // Ensure that when either navigationBar's left or right arrow button is clicked,
    // we pick up the emitted signal and handle it by moving forwards or backwards
    // in the document hierarchy.
    connect(navigationBar,SIGNAL(forwards()),browser,SLOT(forward()));
    connect(navigationBar,SIGNAL(backwards()),browser,SLOT(backward()));

    // Ensure that when the browser's document queue changes condition at either end
    // (i.e. it has a 'backwards' document when previously it had none, or it no
    // longer has a 'forwards' document, etc), navigationBar's left or right
    // arrow button is enabled or disabled appropriately.
    connect(browser,SIGNAL(backwardAvailable(bool)),navigationBar,(SLOT(setBackwardsEnabled(bool))));
    connect(browser,SIGNAL(forwardAvailable(bool)),navigationBar,(SLOT(setForwardsEnabled(bool))));
    // When the browser's document queue changes condition, the previous and next
    // documents will also undergo changes. When this happens, navigationBar will
    // need to update its previous and next document titles.
    /* XXX No such signal yet
    connect(browser,SIGNAL(historyChanged(QString,QString)),
            navigationBar,SLOT(labelsChanged(QString,QString)));
    */

    boxLayout->addWidget(navigationBar);

    QStringList helpPath = Qtopia::helpPaths();
    helpPath.append("pics");
    browser->setSearchPaths(helpPath);
    connect( browser, SIGNAL(sourceChanged(QUrl)),
             this, SLOT(textChanged()) );

    setCentralWidget( box );

    // Hook onto the application channel to process Help service messages.
    new HelpService( this );

    backAction = new QAction(QIcon(":icon/i18n/previous"), tr("Back"), this);
    backAction->setWhatsThis(tr("Move backward one page."));
    backAction->setEnabled( false );
    connect( backAction, SIGNAL(triggered()), browser, SLOT(backward()) );
    connect( browser, SIGNAL(backwardAvailable(bool)), backAction, SLOT(setEnabled(bool)) );

    forwardAction = new QAction(QIcon(":icon/i18n/next"), tr("Forward"), this );
    forwardAction->setWhatsThis( tr( "Move forward one page." ) );
    forwardAction->setEnabled( false );
    connect( forwardAction, SIGNAL(triggered()), browser, SLOT(forward()) );
    connect( browser, SIGNAL(forwardAvailable(bool)), forwardAction, SLOT(setEnabled(bool)) );

    QAction *bookmarksAction = new QAction(QIcon(":icon/list"), tr("Bookmarks"), this );
    bookmarksAction->setWhatsThis( tr( "Go to Bookmarks listing." ) );
    connect( bookmarksAction, SIGNAL(triggered()), this, SLOT(bookmarks()) );

    QAction *addBookmarkAction = new QAction(QIcon(":icon/new"),tr("Add Bookmark"),this);
    addBookmarkAction->setWhatsThis( tr( "Add this page to your set of bookmarks." ) );
    connect( addBookmarkAction, SIGNAL(triggered()), this, SLOT(addBookmark()) );

    QAction *homeAction = new QAction(QIcon(":icon/home"), tr("Home"), this );
    homeAction->setWhatsThis( tr( "Go to the home page." ) );
    connect( homeAction, SIGNAL(triggered()), this, SLOT(goHome()) );

    contextMenu = QSoftMenuBar::menuFor(this);

    contextMenu->addAction( backAction );
    contextMenu->addAction( forwardAction );
    contextMenu->addAction( bookmarksAction );
    contextMenu->addAction( addBookmarkAction );
    contextMenu->addAction( homeAction );

    setFocusProxy( browser );
    browser->setFrameStyle( QFrame::NoFrame );

    browser->installEventFilter( this );
    QString sheet = "a { color: palette(link) } a:visited { color: palette(link-visted) }";
    if (browser->document())
        browser->document()->setDefaultStyleSheet(sheet);
    browser->setSource( HOMEPAGE );
}

// Fetches 'bookmarksUI', creating it if necessary.
BookmarksUI *HelpBrowser:: getBookmarksUI()
{
    if ( bookmarksUI ) {
        return bookmarksUI;
    }

    // Create the UI on an as-needed basis.
    bookmarksUI = new BookmarksUI(this);
    // Ensure that when the user chooses a Bookmark for display within the BookmarksUI, the
    // appropriate handler is called to display that Bookmark within this browser.
    connect(bookmarksUI,SIGNAL(bookmarkSelected(Bookmark)),this,SLOT(bookmarkSelected(Bookmark)));

    return bookmarksUI;
}

void HelpBrowser::setDocument( const QString &doc )
{
    if ( !doc.isEmpty() ) {
        browser->clearHistory();

        browser->setSource( doc );
        QtopiaApplication::instance()->showMainWidget();
    }
}

/*! \fn void HelpBrowser::bookmarkSelected(Bookmark bookmark)
  Handles the event that a Bookmark has been chosen for display within the BookmarksUI.
  Assumes that the BookmarksUI has been closed.
*/
void HelpBrowser::bookmarkSelected(Bookmark bookmark)
{
    // Clear out history.
    browser->clearHistory();
    // Display the Url.
    browser->setSource(bookmark.getUrl());
}

void HelpBrowser::goHome()
{
    browser->setSource( HOMEPAGE );
}

// Displays 'bookmarksUI', creating it if necessary.
void HelpBrowser::bookmarks()
{
    // Display the BookmarksUI in place of this HelpBrowser.
    BookmarksUI *ui = getBookmarksUI();

    // Ensure that the ui will always start in Navigation mode.
    ui->setReorganise(false);

    ui->setWindowTitle(tr("Bookmarks"));
    setWindowTitle(tr("Bookmarks"));

    ui->showMaximized();
}

// Adds the current page as a new bookmark.
void HelpBrowser::addBookmark()
{
    // Add the current filename as a bookmark, with the current title, if there is one.
    QString title;
    if ( browser->documentTitle().isNull() ) {
        title = browser->source().path().section('/',-1);
    } else {
        title = browser->documentTitle();
    }

    // Ask the UI to create a new Bookmark.
    getBookmarksUI()->addBookmark(browser->source(),title);
}

// Private slot to handle browser's sourceChanged() signal. Ensures the browser's
// title and the debug location's filename are up-to-date.
void HelpBrowser::textChanged()
{
    if ( browser->documentTitle().isNull() )
        setWindowTitle( tr("Help Browser") );
    else
        setWindowTitle( browser->documentTitle() ) ;
#ifdef DEBUG
    location->setText( browser->source().toString() );
#endif
}

bool HelpBrowser::eventFilter( QObject*obj, QEvent* e )
{
    Q_UNUSED(obj);

    switch( e->type() ) {
    case QEvent::KeyPress:
        {
            QKeyEvent *ke = (QKeyEvent*)e;

            // The left and right hardware keys cause the NavigationBar keys to be
            // triggered, which in turn are connected to the slots to navigate through
            // the document hierarchy.
            if ( ke->key() == Qt::Key_Left ) {
                // Cause the left key in the NavigationBar to be 'pressed'.
                navigationBar->triggerBackwards();
            } else if ( ke->key() == Qt::Key_Right ) {
                // Cause the right key in the NavigationBar to be 'pressed'.
                navigationBar->triggerForwards();
            }
        }
        break;
    default:
        break;
    }

    // Allow the parent to handle it.
    return false;
}

void HelpBrowser::closeEvent( QCloseEvent* e )
{
    e->accept();
}

/*!
    \service HelpService Help
    \brief Provides the Qtopia Help service.

    The \i Help service enables applications to display context-sensitive help.
*/

/*!
    \internal
*/
HelpService::~HelpService()
{
}

/*!
    Display \a doc within the help browser.

    This slot corresponds to the QCop service message
    \c{Help::setDocument(QString)}.
*/
void HelpService::setDocument( const QString& doc )
{
    parent->setDocument( doc );
}

