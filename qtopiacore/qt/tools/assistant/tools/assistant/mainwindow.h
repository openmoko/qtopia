/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QUrl>
#include <QtGui/QMainWindow>

QT_BEGIN_NAMESPACE

class QAction;
class QLineEdit;
class QComboBox;
class QMenu;

class IndexWindow;
class QHelpEngineCore;
class QHelpEngine;
class CentralWidget;
class ContentWindow;
class BookmarkManager;
class BookmarkWidget;
class CmdLineParser;
class QtDocInstaller;

class SearchWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(CmdLineParser *cmdLine, QWidget *parent = 0);
    ~MainWindow();

    static void activateCurrentBrowser();
    static void activateCurrentCentralWidgetTab();
    static QString collectionFileDirectory(bool createDir = false,
        const QString &cacheDir = QString());
    static QString defaultHelpCollectionFileName();

public:
    void hideContents();
    void hideIndex();
    void hideBookmarks();
    void hideSearch();
    void setIndexString(const QString &str);
    void expandTOC(int depth);
    bool usesDefaultCollection() const;

signals:
    void initDone();

public slots:
    void showContents();
    void showIndex();
    void showBookmarks();
    void showSearch();
    void syncContents();

private slots:
    void insertLastPages();
    void addBookmark();
    void gotoAddress();
    void showPreferences();
    void showNewAddress();
    void showAboutDialog();    
    void copyAvailable(bool yes);
    void updateNavigationItems();
    void showNewAddress(const QUrl &url);
    void addNewBookmark(const QString &title, const QString &url);
    void showTopicChooser(const QMap<QString, QUrl> &links, const QString &keyword);    
    void updateApplicationFont();
    void filterDocumentation(const QString &customFilter);
    void setupFilterCombo();
    void lookForNewQtDocumentation();
    void indexingStarted();
    void indexingFinished();
    void displayInstallationError(const QString &errorMessage);
    void qtDocumentationInstalled(bool newDocsInstalled);
    void checkInitState();

private:
    bool initHelpDB();
    void setupActions();
    void closeEvent(QCloseEvent *e);
    void activateDockWidget(QWidget *w);
    void updateAboutMenuText();
    void setupFilterToolbar();
    void setupAddressToolbar();
    QMenu *toolBarMenu();
    QWidget *setupBookmarkWidget();    

    QHelpEngine *m_helpEngine;
    CentralWidget *m_centralWidget;
    IndexWindow *m_indexWindow;
    ContentWindow *m_contentWindow;
    BookmarkWidget *m_bookmarkWidget;
    BookmarkManager *m_bookmarkManager;
    QLineEdit *m_addressLineEdit;
    QComboBox *m_filterCombo;

    QAction *m_backAction;
    QAction *m_nextAction;
    QAction *m_homeAction;
    QAction *m_syncAction;
    QAction *m_copyAction;
    QAction *m_findAction;
    QAction *m_printAction;
    QAction *m_printPreviewAction;
    QAction *m_pageSetupAction;
    QAction *m_zoomInAction;
    QAction *m_zoomOutAction;
    QAction *m_resetZoomAction;
    QAction *m_aboutAction;
    QAction *m_closeTabAction;
    QAction *m_newTabAction;

    QMenu *m_viewMenu;
    QMenu *m_toolBarMenu;

    CmdLineParser *m_cmdLine;
    SearchWidget *m_searchWidget;

    QWidget *m_progressWidget;
    QtDocInstaller *m_qtDocInstaller;

    bool m_connectedInitSignals;
};

QT_END_NAMESPACE

#endif // MAINWINDOW_H
