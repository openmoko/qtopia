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

#include "ui_mainwindow.h"
#include "config.h"

#include <QPointer>
#include <QMap>

QT_BEGIN_NAMESPACE

class TabbedBrowser;
class HelpDialog;
class HelpWindow;
class QMenu;
class QDockWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    virtual ~MainWindow();

    TabbedBrowser *browsers() const;
    HelpDialog *helpDialog() const;

    void setupPopupMenu(QMenu *menu);
    static QString urlifyFileName(const QString &fileName);

    void removePendingBrowser(HelpWindow *win);

public slots:
    MainWindow *newWindow();

    void setup();
    void showLink(const QString &link);
    void showLinks(const QStringList &links);
    void saveSettings();
    void updateBookmarkMenu();
    void printingFinished();

private slots:
    void on_actionNewWindow_triggered();
    void on_actionGoHome_triggered();
    void on_actionFilePrint_triggered();
    void on_actionClose_triggered();
    void on_actionHelpWhatsThis_triggered();
    void on_actionHelpAssistant_triggered();
    void on_actionAboutApplication_triggered();
    void on_actionAboutAssistant_triggered();
    void on_actionSaveAs_triggered();
    void on_actionSyncToc_triggered();

    void about();
    void setupBookmarkMenu();
    void showBookmark(QAction *action);
    void showLinkFromClient(const QString &link);
    void showQtHelp();
    void showSearchLink(const QString &link, const QStringList &terms);
    void showGoActionLink();
    void updateProfileSettings();
    void backwardAvailable(bool);
    void forwardAvailable(bool);

    void browserTabChanged();
    void copyAvailable(bool yes);
    void updateTabActions(int index);
    void showFontSettingsDialog();

protected:
    void closeEvent(QCloseEvent *);
    void timerEvent(QTimerEvent *);

private:
    void setupGoActions();
    bool insertActionSeparator();
    void updateApplicationFontSettings(FontSettings &settings);

private:
    Ui::MainWindow ui;

    QList<QAction*> goActions;
    uint setupCompleted:1;
    TabbedBrowser *tabs;
    QMap<QAction*, QString> bookmarks;
    HelpDialog *helpDock;
    QDockWidget *dw;
    static QList<MainWindow*> windows;
    QMap<QAction*,QString> *goActionDocFiles;
    QList<QPair<HelpWindow*,QString> > pendingBrowsers;
};

#endif // MAINWINDOW_H

QT_END_NAMESPACE
