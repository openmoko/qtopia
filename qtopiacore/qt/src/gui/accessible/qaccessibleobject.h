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

#ifndef QACCESSIBLEOBJECT_H
#define QACCESSIBLEOBJECT_H

#include <QtGui/qaccessible.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_ACCESSIBILITY

class QAccessibleObjectPrivate;
class QObject;

class Q_GUI_EXPORT QAccessibleObject : public QAccessibleInterface
{
public:
    explicit QAccessibleObject(QObject *object);

    bool isValid() const;
    QObject *object() const;

    // properties
    QRect rect(int child) const;
    void setText(Text t, int child, const QString &text);

    // actions
    int userActionCount(int child) const;
    bool doAction(int action, int child, const QVariantList &params);
    QString actionText(int action, Text t, int child) const;

protected:
    virtual ~QAccessibleObject();

private:
    friend class QAccessibleObjectEx;
    QAccessibleObjectPrivate *d;
    Q_DISABLE_COPY(QAccessibleObject)
};

class Q_GUI_EXPORT QAccessibleObjectEx : public QAccessibleInterfaceEx
{
public:
    explicit QAccessibleObjectEx(QObject *object);

    bool isValid() const;
    QObject *object() const;

    // properties
    QRect rect(int child) const;
    void setText(Text t, int child, const QString &text);

    // actions
    int userActionCount(int child) const;
    bool doAction(int action, int child, const QVariantList &params);
    QString actionText(int action, Text t, int child) const;

protected:
    virtual ~QAccessibleObjectEx();

private:
    QAccessibleObjectPrivate *d;
    Q_DISABLE_COPY(QAccessibleObjectEx)
};

class Q_GUI_EXPORT QAccessibleApplication : public QAccessibleObject
{
public:
    QAccessibleApplication();

    // relations
    int childCount() const;
    int indexOfChild(const QAccessibleInterface*) const;
    Relation relationTo(int, const QAccessibleInterface *, int) const;

    // navigation
    int childAt(int x, int y) const;
    int navigate(RelationFlag, int, QAccessibleInterface **) const;

    // properties and state
    QString text(Text t, int child) const;
    Role role(int child) const;
    State state(int child) const;

    // actions
    int userActionCount(int child) const;
    bool doAction(int action, int child, const QVariantList &params);
    QString actionText(int action, Text t, int child) const;
};

#endif // QT_NO_ACCESSIBILITY

QT_END_HEADER

#endif // QACCESSIBLEOBJECT_H
