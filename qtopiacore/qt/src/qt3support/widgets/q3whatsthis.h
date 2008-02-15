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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3WHATSTHIS_H
#define Q3WHATSTHIS_H

#include <QtGui/qcursor.h>
#include <QtGui/qwhatsthis.h>
#include <QtGui/qwidget.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

#ifndef QT_NO_WHATSTHIS

class QToolButton;

class Q_COMPAT_EXPORT Q3WhatsThis: public QObject
{
    Q_OBJECT
public:
    Q3WhatsThis(QWidget *);
    ~Q3WhatsThis();
    bool eventFilter(QObject *, QEvent *);

    static inline void enterWhatsThisMode() { QWhatsThis::enterWhatsThisMode(); }
    static inline bool inWhatsThisMode() { return QWhatsThis::inWhatsThisMode(); }

    static inline void add(QWidget *w, const QString &s) { w->setWhatsThis(s); }
    static inline void remove(QWidget *w) { w->setWhatsThis(QString()); }
    static QToolButton * whatsThisButton(QWidget * parent);
    static inline void leaveWhatsThisMode(const QString& text = QString(), const QPoint& pos = QCursor::pos(), QWidget* w = 0)
        { QWhatsThis::showText(pos, text, w); }
    static inline void display(const QString& text, const QPoint& pos = QCursor::pos(), QWidget* w = 0)
        { QWhatsThis::showText(pos, text, w); }

    virtual QString text(const QPoint &);
    virtual bool clicked(const QString& href);

};

#endif // QT_NO_WHATSTHIS

QT_END_HEADER

#endif // Q3WHATSTHIS_H
