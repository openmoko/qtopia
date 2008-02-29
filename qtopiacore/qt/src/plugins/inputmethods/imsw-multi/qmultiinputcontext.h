/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the plugins of the Qt Toolkit.
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

/****************************************************************************
**
** Definition of QMultiInputContext class
**
** Copyright (C) 2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to Trolltech ASA under their own
** licence. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifndef QMULTIINPUTCONTEXT_H
#define QMULTIINPUTCONTEXT_H

#ifndef QT_NO_IM

#include <QtGui/qwidget.h>
#include <QtGui/qinputcontext.h>
#include <QtCore/qstring.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qmap.h>
#include <QtCore/qpointer.h>
#include <QtCore/qlist.h>

class QMultiInputContext : public QInputContext
{
    Q_OBJECT
public:
    QMultiInputContext();
    ~QMultiInputContext();

    QString identifierName();
    QString language();

#if defined(Q_WS_X11)
    bool x11FilterEvent( QWidget *keywidget, XEvent *event );
#endif // Q_WS_X11
    bool filterEvent( const QEvent *event );

    void reset();
    void update();
    void mouseHandler( int x, QMouseEvent *event );
    QFont font() const;
    bool isComposing() const;

    QList<QAction *> actions();

    QWidget *focusWidget() const;
    void setFocusWidget(QWidget *w);

    void widgetDestroyed( QWidget *w );

    QInputContext *slave() { return slaves.at(current); }
    const QInputContext *slave() const { return slaves.at(current); }

protected Q_SLOTS:
    void changeSlave(QAction *);
private:
    void *unused;
    int current;
    QList<QInputContext *> slaves;
    QMenu *menu;
    QAction *separator;
};

#endif // Q_NO_IM

#endif // QMULTIINPUTCONTEXT_H
