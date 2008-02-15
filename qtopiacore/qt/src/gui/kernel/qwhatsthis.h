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

#ifndef QWHATSTHIS_H
#define QWHATSTHIS_H

#include <QtCore/qobject.h>
#include <QtGui/qcursor.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_WHATSTHIS

class QAction;
#ifdef QT3_SUPPORT
class QToolButton;
#endif

class Q_GUI_EXPORT QWhatsThis
{
    QWhatsThis();

public:
    static void enterWhatsThisMode();
    static bool inWhatsThisMode();
    static void leaveWhatsThisMode();

    static void showText(const QPoint &pos, const QString &text, QWidget *w = 0);
    static void hideText();

    static QAction *createAction(QObject *parent = 0);

#ifdef QT3_SUPPORT
    static QT3_SUPPORT void add(QWidget *w, const QString &s);
    static QT3_SUPPORT void remove(QWidget *);
    static QT3_SUPPORT QToolButton *whatsThisButton(QWidget *parent);
#endif
};

#endif // QT_NO_WHATSTHIS

QT_END_HEADER

#endif // QWHATSTHIS_H
