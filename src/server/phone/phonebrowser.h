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

#ifndef _PHONEBROWSER_H_
#define _PHONEBROWSER_H_

#include "phone/ui/phonelauncherview.h"

#include <QStackedWidget>
#include <QCloseEvent>
#include <QMap>
#include <QSet>
#include <QStack>
#include <QString>
#include <qcontent.h>
#include "launcherview.h"
#include <QPointer>
#include "messagebox.h"
#include <qcategorymanager.h>
#include "qabstractbrowserscreen.h"
#include "applicationmonitor.h"

class LazyContentStack : public QWidget
{
Q_OBJECT
public:
    enum Flags { NoFlags = 0x00000000, NoStack = 0x00000001 };
    Q_FLAGS(Flags);

    LazyContentStack(Flags lcsFlags = NoFlags, QWidget *parent = 0,
                     Qt::WFlags wflags = 0);

    virtual void reset();
    virtual void resetToView(const QString &);
    virtual void showView(const QString &);

    bool isDone() const;
protected:
    // Lazy creation methods
    virtual QObject* createView(const QString &) = 0;
    virtual void raiseView(const QString &, bool reset) = 0;

    // UI methods
    virtual void noView(const QString &);
    virtual void busy(const QContent &);
    virtual void notBusy();

    // Stack manipulation
    void back();

signals:
    void done();
    void viewContentSet( const QContentSet &set );

private slots:
    void appStateChanged(const QString &);
    void execContent(QContent);

private:
    void addView(const QString &, bool);

    QString busyApp;
    UIApplicationMonitor monitor;
    Flags m_flags;
    QSet<QString> m_views;
    QStack<QString> m_viewStack;
};

class QAction;
class QMenu;
struct TypeView {
    TypeView() : view(0) {};
    LauncherView *view;
    QString name;
    QIcon icon;
};

class QSettings;
class PhoneMainMenu;
class DocumentLauncherView;
class PhoneBrowserStack : public LazyContentStack
{
Q_OBJECT
public:
    PhoneBrowserStack(QWidget *parent = 0);
    virtual void show();
    virtual void hide();
    void insertPhoneMenu(QWidget *m);

    void showType(const QString &);

    QString currentName() const;

    void back();
protected:
    void keyPressEvent(QKeyEvent *ke);

    virtual void busy(const QContent &);
    virtual void notBusy();
    virtual void noView(const QString &);

    virtual QObject* createView(const QString &);
    virtual void raiseView(const QString &, bool);
    void closeEvent(QCloseEvent *e);

signals:
    void visibilityChanged();
    void applicationLaunched(const QString &);

private:
    LauncherView *addType(const QString& type,
                          const QString& name, const QIcon &icon);
    LauncherView *createAppView(const QString &);
    LauncherView *createContentSetView();

    LauncherView *currentView();
    int menuIdx;
    void showMessageBox(const QString& title, const QString& text, QAbstractMessageBox::Icon icon=QAbstractMessageBox::Information);

    QCategoryManager appCategories;

public:
    QPointer<QAbstractMessageBox> warningBox;
    PhoneMainMenu *phoneLauncher;
    QMap<QString, TypeView> map;
    QStackedWidget *stack;
};

class PhoneMainMenu : public PhoneLauncherView
{
Q_OBJECT
public:
    PhoneMainMenu(QSettings &, QWidget * parent = 0);

    void showDefaultSelection();

private slots:
    void expressionChanged();

private:
    void activateItem(const QChar &, int);

    void setMainMenuItemEnabled(const QString &file, const QString &name, const QString &icon, bool enabled);
    void makeLauncherMenu(QSettings &);
    QContent *readLauncherMenuItem(const QString &);

    class ItemExpression;
    struct Item {
        QContent lnk;
        bool exprTrue();
        ItemExpression *expr;
        bool enabled;
    };
    struct Items : public QList<Item>
    {
        Items() : activeItem(-1) {}
        int activeItem;
    };
    QMap<QChar,Items> mainMenuEntries;
    QString menuKeyMap;
    int defaultSelection;
};

class RunningAppsLauncherView : public LauncherView
{
Q_OBJECT
public:
    RunningAppsLauncherView(QWidget * = 0);

private slots:
    void applicationStateChanged();

private:
    UIApplicationMonitor monitor;
};

class PhoneBrowserScreen : public QAbstractBrowserScreen
{
Q_OBJECT
public:
    PhoneBrowserScreen(QWidget *parent = 0, Qt::WFlags flags = 0);

    virtual QString currentView() const;
    virtual bool viewAvailable(const QString &) const;
    virtual void resetToView(const QString &);
    virtual void moveToView(const QString &);

protected:
    void closeEvent(QCloseEvent *e);

private:
    PhoneBrowserStack *m_stack;
};

#endif // _PHONEBROWSER_H_
