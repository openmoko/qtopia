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

#include "startmenu.h"
#include <qcontentset.h>

#include <qtopiaapplication.h>
#include <qtranslatablesettings.h>

#include <qmimetype.h>
#include <qstartmenuaccessory.h>

#include <QDir>
#include <QKeyEvent>
#include <QStyle>

#include <stdlib.h>

class StartPopupMenu : public QMenu
{
public:
    StartPopupMenu( QWidget *parent ) : QMenu( parent ) {}
protected:
    void keyPressEvent( QKeyEvent *e );
};

void StartPopupMenu::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Qt::Key_F33 || e->key() == Qt::Key_Space ) {
        // "OK" button, little hacky
        QKeyEvent ke(QEvent::KeyPress, Qt::Key_Enter, 0);
        QMenu::keyPressEvent( &ke );
    } else {
        QMenu::keyPressEvent( e );
    }
}

//---------------------------------------------------------------------------
class StartMenuAction : public QAction
{
Q_OBJECT
public:
    StartMenuAction()
        : QAction(0), type(Separator) { setSeparator(true); }

    StartMenuAction(const QIcon &icon, const QString &text,
                    const QString &_tab)
        : QAction(icon, text,0), type(Tab), tab(_tab) {}

    StartMenuAction(const QIcon &icon, const QString &text,
                    QContent *_lnk)
        : QAction(icon, text,0), type(Link), lnk(_lnk) {}

    StartMenuAction(const QIcon &icon, const QString &text,
                    QStartMenuAccessory *_acc)
        : QAction(icon, text,0), type(AccessoryItem),
          access(_acc) {}

    virtual ~StartMenuAction() {
        if(Link == type) delete lnk;
    }

    enum Type { Tab, Link, AccessoryItem, Separator };
    Type type;
    QString tab;
    union {
        QContent * lnk;
        QStartMenuAccessory * access;
    };
};

StartMenu::StartMenu(QWidget *parent) : QLabel( parent )
{
    startButtonPixmap = "go"; // No tr

    int sz = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize) + 3;
    QPixmap pm = QPixmap::fromImage(QImage(":image/"+startButtonPixmap).scaled(sz,sz));
    setPixmap(pm);
    setFocusPolicy( Qt::NoFocus );

    launchMenu = 0;
    refreshMenu();
}


void StartMenu::mousePressEvent( QMouseEvent * )
{
    launch();
}

StartMenu::~StartMenu()
{
}

void StartMenu::createMenu()
{
    delete launchMenu;
    launchMenu = new StartPopupMenu( this );
    loadMenu( launchMenu );
    connect( launchMenu, SIGNAL(triggered(QAction*)), SLOT(itemSelected(QAction*)) );
}

void StartMenu::refreshMenu()
{
    QSettings cfg("Trolltech","Taskbar");
    cfg.beginGroup("Menu");
    bool ltabs = cfg.value("LauncherTabs",true).toBool();
    bool lot = cfg.value("LauncherOther",true).toBool();
    bool lt = ltabs || lot;

    if ( launchMenu && !lt )
        return; // nothing to do

    if ( launchMenu ) {
        // remove non-applet added menu items...
        for(int ii = items.count() - 1; ii >= 0; ii--) {
            QAction * action = items.at(ii);
            StartMenuAction * sma = qobject_cast<StartMenuAction *>(action);
            if(sma && StartMenuAction::AccessoryItem != sma->type) {
                launchMenu->removeAction(action);
                delete action;
                items.removeAt(ii);
            }
        }

        // Now load all that stuff back in...
        loadMenu(launchMenu);
    } else {
        // We don't have a menu at the moment, so create one
        createMenu();
    }
}

void StartMenu::itemSelected( QAction *a )
{
    StartMenuAction * sma = qobject_cast<StartMenuAction *>(a);
    if(!sma) {
        // Must be a accessory menu
        return;
    }

    switch(sma->type) {
        case StartMenuAction::Tab:
            emit tabSelected(sma->tab);
            break;
        case StartMenuAction::Link:
            sma->lnk->execute();
            break;
        case StartMenuAction::AccessoryItem:
            // Need to let the accessory know...
            emit sma->access->activated();
            break;
        case StartMenuAction::Separator:
        default:
            break;
    }
}

bool StartMenu::loadMenu( QMenu *menu )
{
    QSettings cfg("Trolltech","Taskbar");
    cfg.beginGroup("Menu");


    bool ltabs = cfg.value("LauncherTabs",true).toBool();
    bool lot = cfg.value("LauncherOther",true).toBool();

    int ntabs = 0;
    int nother = 0;

    // Add a separator if the item list is non-empty
    if(items.count()) {
        StartMenuAction * a = new StartMenuAction();
        launchMenu->addAction(a);
        items.append(a);
    }

    bool f=true;
    // !! move it over to a category filtered content set.
    // this should be moved up to the constructor somewhere, so that we can possibly use the signals to update the start menu/tabs
    QContentSet *als = new QContentSet( QContentFilter::Role, "Application" );  // NO TR
    //const AppLnkSet *als = NULL;
    if ( (ltabs || lot) && als )
    {
    // todo: emulate the .directory commands.
    }

    bool result = nother || ntabs;

    return result;
}


void StartMenu::launch()
{
    if ( launchMenu->isVisible() )
        launchMenu->hide();
    else {
        QSize sh = launchMenu->sizeHint();
        int y =mapToGlobal( QPoint() ).y() - sh.height();
        bool rtl = QApplication::layoutDirection() == Qt::RightToLeft;
        int x;
        if ( rtl )
            x = mapToGlobal( QPoint(width(), height())).x() - sh.width();
        else
            x = 1;

        launchMenu->popup( QPoint( x, y ) );
    }
}

void StartMenu::doSep()
{
    // Add a separator if the item list is non-empty, and the first entry
    // in the list is not an accessory or a separator
    return;
    if(items.count()) {
        StartMenuAction * sma = qobject_cast<StartMenuAction *>(items.at(0));
        if(sma && StartMenuAction::AccessoryItem != sma->type &&
           StartMenuAction::Separator != sma->type) {
            StartMenuAction * a = new StartMenuAction();
            launchMenu->insertAction(items.at(0), a);
            items.prepend(a);
        }
    }
}

QAction * StartMenu::add(QStartMenuAccessory * a, const QIcon& i, const QString& t)
{
    doSep(); // Check for separator

    StartMenuAction *ac = new StartMenuAction(i, t, a);
    launchMenu->insertAction(items.count()?items.at(0):0, ac);
    items.prepend(ac);
    return ac;
}

QAction * StartMenu::add(const QIcon& i, const QString& t, QMenu * m)
{
    doSep(); // Check for separator

    QAction * rv = launchMenu->insertMenu(items.count()?items.at(0):0, m);
    rv->setIcon(i);
    rv->setText(t);
    items.prepend(rv);
    return rv;
}

void StartMenu::rem(QAction * a)
{
    for(int ii = items.count() - 1; ii >= 0; ii--) {
        QAction * action = items.at(ii);
        if(action == a) {
            launchMenu->removeAction(action);
            delete action;
            items.removeAt(ii);
        }
    }
}

StartMenu * StartMenu::create(QWidget * parent)
{
    static StartMenu * me = 0;
    Q_ASSERT((parent && !me) ||
             (!parent && me));

    me = new StartMenu(parent);
    return me;
}

#include "startmenu.moc"
