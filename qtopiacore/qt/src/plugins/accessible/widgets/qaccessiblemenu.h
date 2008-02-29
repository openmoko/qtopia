/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QACCESSIBLEMENU_H
#define QACCESSIBLEMENU_H

#include <QtGui/qaccessiblewidget.h>

#ifndef QT_NO_ACCESSIBILITY

#ifndef QT_NO_MENU
class QMenu;
class QMenuBar;

class QAccessibleMenu : public QAccessibleWidget
{
public:
    explicit QAccessibleMenu(QWidget *w);

    int childCount() const;
    int childAt(int x, int y) const;

    QRect rect(int child) const;
    QString text(Text t, int child) const;
    Role role(int child) const;
    State state(int child) const;

    QString actionText(int action, QAccessible::Text text, int child) const;
    bool doAction(int action, int child, const QVariantList &params);

protected:
    QMenu *menu() const;
};
#endif // QT_NO_MENU

#ifndef QT_NO_MENUBAR
class QAccessibleMenuBar : public QAccessibleWidget
{
public:
    explicit QAccessibleMenuBar(QWidget *w);

    int childCount() const;
    int childAt(int x, int y) const;

    QRect rect(int child) const;
    QString text(Text t, int child) const;
    Role role(int child) const;
    State state(int child) const;

    QString actionText(int action, QAccessible::Text text, int child) const;
    bool doAction(int action, int child, const QVariantList &params);

protected:
    QMenuBar *menuBar() const;
};
#endif // QT_NO_MENUBAR

#endif // QT_NO_ACCESSIBILITY

#endif // QACCESSIBLEMENU_H
