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
