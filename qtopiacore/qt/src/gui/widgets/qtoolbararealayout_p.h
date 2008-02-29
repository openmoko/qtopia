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
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTOOLBARAREALAYOUT_P_H
#define QTOOLBARAREALAYOUT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QList>
#include <QSize>
#include <QRect>

#ifndef QT_NO_TOOLBAR

class QToolBar;
class QLayoutItem;
class QMainWindow;
class QStyleOptionToolBar;

class QToolBarAreaLayoutItem
{
public:
    QToolBarAreaLayoutItem(QLayoutItem *item = 0)
        : widgetItem(item), pos(0), size(-1), gap(false) {}

    bool skip() const;
    QSize minimumSize() const;
    QSize sizeHint() const;

    QLayoutItem *widgetItem;
    int pos;
    int size;
    bool gap;
};

class QToolBarAreaLayoutLine
{
public:
    QToolBarAreaLayoutLine(Qt::Orientation orientation);

    QSize sizeHint() const;
    QSize minimumSize() const;

    void fitLayout();
    bool skip() const;

    QRect rect;
    Qt::Orientation o;

    QList<QToolBarAreaLayoutItem> toolBarItems;
};

class QToolBarAreaLayoutInfo
{
public:
    enum { EmptyDockAreaSize = 80 }; // when a dock area is empty, how "wide" is it?

    QToolBarAreaLayoutInfo(QInternal::DockPosition pos = QInternal::TopDock);

    QList<QToolBarAreaLayoutLine> lines;

    QSize sizeHint() const;
    QSize minimumSize() const;

    void fitLayout();

    void insertToolBar(QToolBar *before, QToolBar *toolBar);
    void removeToolBar(QToolBar *toolBar);
    void insertToolBarBreak(QToolBar *before);
    void removeToolBarBreak(QToolBar *before);

    QList<int> gapIndex(const QPoint &pos) const;
    bool insertGap(QList<int> path, QLayoutItem *item);
    void clear();
    QRect itemRect(QList<int> path) const;
    QRect appendLineDropRect() const;

    QRect rect;
    Qt::Orientation o;
    QInternal::DockPosition dockPos;
};

class QToolBarAreaLayout
{
public:
    enum { // sentinel values used to validate state data
        ToolBarStateMarker = 0xfe,
        ToolBarStateMarkerEx = 0xfc
    };

    QRect rect;
    QMainWindow *mainWindow;
    QToolBarAreaLayoutInfo docks[4];
    bool visible;

    QToolBarAreaLayout(QMainWindow *win);

    QRect fitLayout();

    QSize minimumSize(const QSize &centerMin) const;
    QRect rectHint(const QRect &r) const;
    QSize sizeHint(const QSize &center) const;
    void apply(bool animate);

    QLayoutItem *itemAt(int *x, int index) const;
    QLayoutItem *takeAt(int *x, int index);
    void deleteAllLayoutItems();

    void insertToolBar(QToolBar *before, QToolBar *toolBar);
    void removeToolBar(QToolBar *toolBar);
    void addToolBar(QInternal::DockPosition pos, QToolBar *toolBar);
    void insertToolBarBreak(QToolBar *before);
    void removeToolBarBreak(QToolBar *before);
    void addToolBarBreak(QInternal::DockPosition pos);

    QInternal::DockPosition findToolBar(QToolBar *toolBar) const;
    bool toolBarBreak(QToolBar *toolBar) const;

    void getStyleOptionInfo(QStyleOptionToolBar *option, QToolBar *toolBar) const;

    QList<int> indexOf(QWidget *toolBar) const;
    QList<int> gapIndex(const QPoint &pos) const;
    bool insertGap(QList<int> path, QLayoutItem *item);
    void remove(QList<int> path);
    void clear();
    QToolBarAreaLayoutItem &item(QList<int> path);
    QRect itemRect(QList<int> path) const;
    QLayoutItem *plug(QList<int> path);
    QLayoutItem *unplug(QList<int> path);

    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream, const QList<QToolBar*> &toolBars);
    bool isEmpty() const;
};

#endif // QT_NO_TOOLBAR
#endif // QTOOLBARAREALAYOUT_P_H
