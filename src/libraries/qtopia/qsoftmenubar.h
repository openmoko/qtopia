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

#ifndef CONTEXT_BAR_H
#define CONTEXT_BAR_H

#include <qtopiaglobal.h>

#include <qstring.h>
#include <QList>

class QWidget;
class QMenu;

class QTOPIA_EXPORT QSoftMenuBar
{
public:
    enum FocusState {
        EditFocus=1, NavigationFocus=2, AnyFocus=3
    };

    enum StandardLabel {
        NoLabel, Options, Ok, Edit, Select, View, Cancel, Back, BackSpace, Next, Previous, EndEdit, RevertEdit, Deselect
    };

    enum LabelType {
        IconLabel, TextLabel
    };

    static void setLabel(QWidget *, int key, const QString &, const QString &, FocusState state=AnyFocus);
    static void setLabel(QWidget *, int key, StandardLabel, FocusState state=AnyFocus);
    static void clearLabel(QWidget *, int key, FocusState state=AnyFocus);

    static const QList<int> &keys();

    static int menuKey();
    static QMenu *menuFor(QWidget *w, FocusState state=AnyFocus);
    static bool hasMenu(QWidget *w, FocusState state=AnyFocus);
    static void addMenuTo(QWidget *w, QMenu *menu, FocusState state=AnyFocus);
    static void removeMenuFrom(QWidget *w, QMenu *menu, FocusState state=AnyFocus);
    static void setHelpEnabled(QWidget *w, bool enable);
    static void setCancelEnabled(QWidget *w, bool enable);
    static void setInputMethodEnabled(QWidget *widget, bool enable);
    static QMenu *createEditMenu();
    static QMenu *activeMenu();

private:
    QSoftMenuBar();
};

#endif
