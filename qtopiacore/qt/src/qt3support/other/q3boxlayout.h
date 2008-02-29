/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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

#ifndef Q3BOXLAYOUT_H
#define Q3BOXLAYOUT_H

#include <QtGui/qboxlayout.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q3BoxLayout : public QBoxLayout
{
public:
    inline explicit Q3BoxLayout(Direction dir, QWidget *parent = 0)
        : QBoxLayout(dir, parent) { setMargin(0); setSpacing(0); }

    inline Q3BoxLayout(QWidget *parent, Direction dir, int margin = 0, int spacing = -1,
                       const char *name = 0)
        : QBoxLayout(parent, dir, margin, spacing, name) {}

    inline Q3BoxLayout(QLayout *parentLayout, Direction dir, int spacing = -1,
                       const char *name = 0)
        : QBoxLayout(parentLayout, dir, spacing, name) { setMargin(0); }

    inline Q3BoxLayout(Direction dir, int spacing, const char *name = 0)
        : QBoxLayout(dir, spacing, name) { setMargin(0); }

private:
    Q_DISABLE_COPY(Q3BoxLayout)
};

class Q3HBoxLayout : public Q3BoxLayout
{
public:
    inline Q3HBoxLayout() : Q3BoxLayout(LeftToRight) {}

    inline explicit Q3HBoxLayout(QWidget *parent) : Q3BoxLayout(LeftToRight, parent) {}

    inline Q3HBoxLayout(QWidget *parent, int margin,
                 int spacing = -1, const char *name = 0)
        : Q3BoxLayout(parent, LeftToRight, margin, spacing, name) {}

    inline Q3HBoxLayout(QLayout *parentLayout,
                 int spacing = -1, const char *name = 0)
        : Q3BoxLayout(parentLayout, LeftToRight, spacing, name) {}

    inline Q3HBoxLayout(int spacing, const char *name = 0)
        : Q3BoxLayout(LeftToRight, spacing, name) {}

private:
    Q_DISABLE_COPY(Q3HBoxLayout)
};

class Q3VBoxLayout : public Q3BoxLayout
{
public:
    inline Q3VBoxLayout() : Q3BoxLayout(TopToBottom) {}

    inline explicit Q3VBoxLayout(QWidget *parent) : Q3BoxLayout(TopToBottom, parent) {}

    inline Q3VBoxLayout(QWidget *parent, int margin,
                 int spacing = -1, const char *name = 0)
        : Q3BoxLayout(parent, TopToBottom, margin, spacing, name) {}

    inline Q3VBoxLayout(QLayout *parentLayout,
                 int spacing = -1, const char *name = 0)
        : Q3BoxLayout(parentLayout, TopToBottom, spacing, name) {}

    inline Q3VBoxLayout(int spacing, const char *name = 0)
        : Q3BoxLayout(TopToBottom, spacing, name) {}

private:
    Q_DISABLE_COPY(Q3VBoxLayout)
};

QT_END_HEADER

#endif // Q3BOXLAYOUT_H
