/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SIMPLEWIDGETS_H
#define SIMPLEWIDGETS_H

#include <QtGui/qaccessiblewidget.h>

#ifndef QT_NO_ACCESSIBILITY

class QAbstractButton;
class QLineEdit;
class QToolButton;

class QAccessibleButton : public QAccessibleWidget
{
public:
    QAccessibleButton(QWidget *w, Role r);

    QString text(Text t, int child) const;
    State state(int child) const;

    QString actionText(int action, Text text, int child) const;
    bool doAction(int action, int child, const QVariantList &params);

protected:
    QAbstractButton *button() const;
};

#ifndef QT_NO_TOOLBUTTON
class QAccessibleToolButton : public QAccessibleButton
{
public:
    QAccessibleToolButton(QWidget *w, Role role);

    enum ToolButtonElements {
        ToolButtonSelf        = 0,
        ButtonExecute,
        ButtonDropMenu
    };

    Role role(int child) const;
    State state(int child) const;

    int childCount() const;
    QRect rect(int child) const;

    QString text(Text t, int child) const;

    int actionCount(int child) const;
    QString actionText(int action, Text text, int child) const;
    bool doAction(int action, int child, const QVariantList &params);

protected:
    QToolButton *toolButton() const;

    bool isSplitButton() const;
};
#endif // QT_NO_TOOLBUTTON

class QAccessibleDisplay : public QAccessibleWidget
{
public:
    explicit QAccessibleDisplay(QWidget *w, Role role = StaticText);

    QString text(Text t, int child) const;
    Role role(int child) const;

    Relation relationTo(int child, const QAccessibleInterface *other, int otherChild) const;
    int navigate(RelationFlag, int entry, QAccessibleInterface **target) const;
};

#ifndef QT_NO_LINEEDIT
class QAccessibleLineEdit : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleLineEdit(QWidget *o, const QString &name = QString());

    QString text(Text t, int child) const;
    void setText(Text t, int control, const QString &text);
    State state(int child) const;
    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);

protected:
    QLineEdit *lineEdit() const;
};
#endif // QT_NO_LINEEDIT

#endif // QT_NO_ACCESSIBILITY

#endif // SIMPLEWIDGETS_H
