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

#ifndef QTOOLTIP_H
#define QTOOLTIP_H

#include <QtGui/qwidget.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_TOOLTIP

class Q_GUI_EXPORT QToolTip
{
    QToolTip();
public:
    static void showText(const QPoint &pos, const QString &text, QWidget *w = 0);
    static void showText(const QPoint &pos, const QString &text, QWidget *w, const QRect &rect);
    static inline void hideText() { showText(QPoint(), QString()); }

    static QPalette palette();
    static void setPalette(const QPalette &);
    static QFont font();
    static void setFont(const QFont &);
#ifdef QT3_SUPPORT
    static inline QT3_SUPPORT void add(QWidget *w, const QString &s) { w->setToolTip(s); }
    static inline QT3_SUPPORT void add(QWidget *w, const QRect &, const QString &s)
    { w->setToolTip(s); }
    static inline QT3_SUPPORT void remove(QWidget *w) { w->setToolTip(QString()); }
#endif
};

#endif // QT_NO_TOOLTIP

QT_END_HEADER

#endif // QTOOLTIP_H
