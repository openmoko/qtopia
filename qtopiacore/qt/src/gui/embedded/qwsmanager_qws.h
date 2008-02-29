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

#ifndef QWSMANAGER_QWS_H
#define QWSMANAGER_QWS_H

#include <QtGui/qpixmap.h>
#include <QtCore/qobject.h>
#include <QtGui/qdecoration_qws.h>
#include <QtGui/qevent.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_QWS_MANAGER

class QAction;
class QPixmap;
class QWidget;
class QPopupMenu;
class QRegion;
class QMouseEvent;
class QWSManagerPrivate;

class Q_GUI_EXPORT QWSManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWSManager)
public:
    explicit QWSManager(QWidget *);
    ~QWSManager();

    static QDecoration *newDefaultDecoration();

    QWidget *widget();
    static QWidget *grabbedMouse();
    void maximize();
    void startMove();
    void startResize();

    QRegion region();
    QRegion &cachedRegion();

protected Q_SLOTS:
    void menuTriggered(QAction *action);

protected:
    void handleMove(QPoint g);

    virtual bool event(QEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void paintEvent(QPaintEvent *);
    bool repaintRegion(int region, QDecoration::DecorationState state);

    void menu(const QPoint &);

private:
    friend class QWidget;
    friend class QETWidget;
    friend class QWidgetPrivate;
    friend class QApplication;
    friend class QApplicationPrivate;
    friend class QWidgetBackingStore;
    friend class QWSWindowSurface;
};

#include <QtGui/qdecorationdefault_qws.h>

#endif // QT_NO_QWS_MANAGER

QT_END_HEADER

#endif // QWSMANAGER_QWS_H
