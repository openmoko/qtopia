/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QACCESSIBLEWIDGET_H
#define QACCESSIBLEWIDGET_H

#include <QtGui/qaccessibleobject.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_ACCESSIBILITY

class QAccessibleWidgetPrivate;

class Q_GUI_EXPORT QAccessibleWidget : public QAccessibleObject
{
public:
    explicit QAccessibleWidget(QWidget *o, Role r = Client, const QString& name = QString());

    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    Relation relationTo(int child, const QAccessibleInterface *other, int otherChild) const;

    int childAt(int x, int y) const;
    QRect rect(int child) const;
    int navigate(RelationFlag rel, int entry, QAccessibleInterface **target) const;

    QString text(Text t, int child) const;
    Role role(int child) const;
    State state(int child) const;

    QString actionText(int action, Text t, int child) const;
    bool doAction(int action, int child, const QVariantList &params);

protected:
    ~QAccessibleWidget();
    QWidget *widget() const;
    QObject *parentObject() const;

    void addControllingSignal(const QString &signal);
    void setValue(const QString &value);
    void setDescription(const QString &desc);
    void setHelp(const QString &help);
    void setAccelerator(const QString &accel);

private:
    friend class QAccessibleWidgetEx;
    QAccessibleWidgetPrivate *d;
    Q_DISABLE_COPY(QAccessibleWidget)
};

class Q_GUI_EXPORT QAccessibleWidgetEx : public QAccessibleObjectEx
{
public:
    explicit QAccessibleWidgetEx(QWidget *o, Role r = Client, const QString& name = QString());

    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    Relation relationTo(int child, const QAccessibleInterface *other, int otherChild) const;

    int childAt(int x, int y) const;
    QRect rect(int child) const;
    int navigate(RelationFlag rel, int entry, QAccessibleInterface **target) const;

    QString text(Text t, int child) const;
    Role role(int child) const;
    State state(int child) const;

    QString actionText(int action, Text t, int child) const;
    bool doAction(int action, int child, const QVariantList &params);

protected:
    ~QAccessibleWidgetEx();
    QWidget *widget() const;
    QObject *parentObject() const;

    void addControllingSignal(const QString &signal);
    void setValue(const QString &value);
    void setDescription(const QString &desc);
    void setHelp(const QString &help);
    void setAccelerator(const QString &accel);

private:
    QAccessibleWidgetPrivate *d;
    Q_DISABLE_COPY(QAccessibleWidgetEx)
};

#endif // QT_NO_ACCESSIBILITY

QT_END_HEADER

#endif // QACCESSIBLEWIDGET_H
