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

#define INCLUDE_MENUITEM_DEF

#include <qstartmenuaccessory.h>
#include <qsettings.h>

#include <qapplication.h>
#include <qpointer.h>

#include <pda/taskbar.h>
#include <pda/startmenu.h>

struct QStartMenuAccessoryData {
    QStartMenuAccessoryData()
        : action(0), menu(0) {}
    ~QStartMenuAccessoryData() {
        if(menu) delete menu;
    }

    QAction * action;
    QMenu * menu;
};

/*!
  \internal
  \class QStartMenuAccessory
  \brief The QStartMenuAccessory class allows applet plugins to add themselves to
  the Q menu in Qtopia PDA edition.

  Accessories can add a single menu entry or add a sub-menu and are particularly useful
  when using real estate on the taskbar is wasteful.
*/

/*!
  \fn void QStartMenuAccessory::activated()

  This signal is emitted when the menu item is activated.
*/

/*!
  Constructs an QStartMenuAccessory object with parent \a parent.

  This constructor places a new menu entry in the Q menu, with the name \a text
  and using the iconset \a icon. When selected, the activated() signal is emitted.

*/
QStartMenuAccessory::QStartMenuAccessory(const QIcon& icon, const QString& text, QWidget* parent) :
    QObject(parent),
    d(new QStartMenuAccessoryData())
{
    IStartMenuAccessory * sma = IStartMenuAccessory::create();
    if(!sma) return;

    d->action = sma->add(this, icon, text);
}

/*!
  Constructs an QStartMenuAccessory object with parent \a parent.

  This constructor places a new menu entry in the Q menu, with the name \a text
  and using the iconset \a icon.

  The \a submenu, if non-zero, must have no parent object.  Ownership of \a submenu
  is transferred to this instance.
*/
QStartMenuAccessory::QStartMenuAccessory(const QIcon& icon, const QString& text, QMenu* submenu, QWidget* parent) :
    QObject(parent),
    d(new QStartMenuAccessoryData())
{
    IStartMenuAccessory * sma = IStartMenuAccessory::create();
    if(!sma) return;

    if(submenu) {
        d->action = sma->add(icon, text, submenu);
        d->menu = submenu;
    } else {
        d->action = sma->add(this, icon, text);
    }
}

/*!
  Destructs QStartMenuAccessory.
*/
QStartMenuAccessory::~QStartMenuAccessory()
{
    IStartMenuAccessory * sma = IStartMenuAccessory::create();
    if(sma && d->action)
        sma->rem(d->action);

    delete d;
}

/*!
  Returns the QIcon displayed in the menu.

  \sa setIcon()
*/
QIcon QStartMenuAccessory::icon() const
{
    if(d->action)
        return d->action->icon();
    else
        return QIcon();
}

/*!
  Returns the text displayed in the menu.

  \sa setText()
*/
QString QStartMenuAccessory::text() const
{
    if(d->action)
        return d->action->text();
    else
        return QString();
}

/*!
  Sets \a icon as the icon shown in the menu.

  \sa icon()
*/
void QStartMenuAccessory::setIcon(const QIcon& icon)
{
    if(d->action)
        d->action->setIcon(icon);
}

/*!
  Sets \a text as the text shown in the menu.

  \sa text()
*/
void QStartMenuAccessory::setText(const QString& text)
{
    if(d->action)
        d->action->setText(text);
}

/*!
  Returns a pointer to the sub-menu, or 0 if a sub-menu does not
  exist.
*/
QMenu* QStartMenuAccessory::popup() const
{
    return d->menu;
}

/*!
  \internal
*/
IStartMenuAccessory::~IStartMenuAccessory()
{
}


/*!
  \internal
*/
IStartMenuAccessory * IStartMenuAccessory::create()
{
    static IStartMenuAccessory * acc = 0;

    if(acc) return acc;

    foreach(QWidget * w, QApplication::allWidgets()) {
        if(w->inherits("StartMenu")) {
            acc = qobject_cast<IStartMenuAccessory *>(w);
            return acc;
        }
    }

    return 0;
}

