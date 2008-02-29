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

#ifndef QACCESSIBLEWIDGETS_H
#define QACCESSIBLEWIDGETS_H

#include <QtGui/qaccessiblewidget.h>

#if !defined(QT_NO_ACCESSIBILITY) && !defined(QT_NO_TEXTEDIT)

class QTextEdit;

class QAccessibleTextEdit : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleTextEdit(QWidget *o);

    QString text(Text t, int child) const;
    void setText(Text t, int control, const QString &text);
    Role role(int child) const;

    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);

    QRect rect(int child) const;
    int childAt(int x, int y) const;

    int childCount() const;

protected:
    QTextEdit *textEdit() const;

private:
    int childOffset;
};

#endif // QT_NO_ACCESSIBILITY

#endif // QACESSIBLEWIDGETS_H
