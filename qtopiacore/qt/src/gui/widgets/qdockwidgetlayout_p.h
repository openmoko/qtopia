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

#ifndef QDYNAMICDOCKWIDGETLAYOUT_P_H
#define QDYNAMICDOCKWIDGETLAYOUT_P_H

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

#include "QtCore/qlist.h"
#include "QtCore/qrect.h"
#include "QtCore/qpair.h"
#include "QtGui/qsizepolicy.h"

#ifndef QT_NO_DOCKWIDGET

class QLayoutItem;
class QWidget;
class QWidgetItem;
class QDockAreaLayoutInfo;
class QDockWidget;
class QMainWindow;
class QWidgetAnimator;
class QMainWindowLayout;
struct QLayoutStruct;
class QTabBar;

enum IndexOfFlag {
    IndexOfFindsVisible,
    IndexOfFindsInvisible,
    IndexOfFindsAll
};

struct QDockAreaLayoutItem
{
    QDockAreaLayoutItem(QWidgetItem *_widgetItem = 0);
    QDockAreaLayoutItem(QDockAreaLayoutInfo *_subinfo);
    QDockAreaLayoutItem(const QDockAreaLayoutItem &other);
    ~QDockAreaLayoutItem();

    QDockAreaLayoutItem &operator = (const QDockAreaLayoutItem &other);

    bool skip() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    QSize sizeHint() const;
    bool expansive(Qt::Orientation o) const;

    QWidgetItem *widgetItem;
    QDockAreaLayoutInfo *subinfo;
    int pos;
    int size;
    bool gap;
    bool keep_size;
};

class QDockAreaLayoutInfo
{
public:
    QDockAreaLayoutInfo();
    QDockAreaLayoutInfo(int _sep, Qt::Orientation _o, int tbhape, QMainWindow *window);

    QSize minimumSize() const;
    QSize maximumSize() const;
    QSize sizeHint() const;
    QSize size() const;

    bool insertGap(QList<int> path, QWidgetItem *dockWidgetItem);
    QWidgetItem *convertToGap(QList<int> path);
    QRect convertToWidget(QList<int> path, QWidgetItem *dockWidget);
    QList<int> gapIndex(const QPoint &pos, bool nestingEnabled) const;
    void remove(QList<int> path);
    void unnest(int index);
    void split(int index, Qt::Orientation orientation, QWidgetItem *dockWidgetItem);
    void tab(int index, QWidgetItem *dockWidgetItem);
    QDockAreaLayoutItem &item(QList<int> path);
    QDockAreaLayoutInfo *info(QList<int> path);
    QDockAreaLayoutInfo *info(QWidget *widget);

    enum { // sentinel values used to validate state data
        SequenceMarker = 0xfc,
        TabMarker = 0xfa,
        WidgetMarker = 0xfb
    };
    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream, const QList<QDockWidget*> &widgets);

    void fitItems();
    bool expansive(Qt::Orientation o) const;
    int changeSize(int index, int size, bool below);
    QRect itemRect(int index) const;
    QRect itemRect(QList<int> path) const;
    QRect separatorRect(int index) const;
    QRect separatorRect(QList<int> path) const;

    void clear();
    bool isEmpty() const;
    QList<int> findSeparator(const QPoint &pos) const;
    int next(int idx) const;
    int prev(int idx) const;

    QList<int> indexOf(QWidget *widget, IndexOfFlag flag = IndexOfFindsVisible) const;

    void apply(bool animate);

    void paintSeparators(QPainter *p, QWidget *widget, const QRegion &clip,
                            const QPoint &mouse) const;
    QRegion separatorRegion() const;
    int separatorMove(int index, int delta, QVector<QLayoutStruct> *cache);

    QLayoutItem *itemAt(int *x, int index) const;
    QLayoutItem *takeAt(int *x, int index);
    void deleteAllLayoutItems();

    QMainWindowLayout *mainWindowLayout() const;

    int sep;
    Qt::Orientation o;
    QRect rect;
    QMainWindow *mainWindow;
    QList<QDockAreaLayoutItem> item_list;

#ifndef QT_NO_TABBAR
    quintptr currentTabId() const;
    void setCurrentTab(QWidget *widget);
    void setCurrentTabId(quintptr id);
    QRect tabContentRect() const;
    bool tabbed;
    QTabBar *tabBar;
    QSize tabBarMin, tabBarHint;
    int tabBarShape;
    bool tabBarVisible;

    void updateTabBar() const;
    QSize tabBarMinimumSize() const;
    QSize tabBarSizeHint() const;

    QSet<QTabBar*> usedTabBars() const;
#endif // QT_NO_TABBAR
};

// utilities

#endif

static inline int pick(Qt::Orientation o, const QPoint &pos)
{ return o == Qt::Horizontal ? pos.x() : pos.y(); }

static inline int pick(Qt::Orientation o, const QSize &size)
{ return o == Qt::Horizontal ? size.width() : size.height(); }

#ifndef QT_NO_DOCKWIDGET

static inline int &rpick(Qt::Orientation o, QPoint &pos)
{ return o == Qt::Horizontal ? pos.rx() : pos.ry(); }

static inline int &rpick(Qt::Orientation o, QSize &size)
{ return o == Qt::Horizontal ? size.rwidth() : size.rheight(); }

static inline QSizePolicy::Policy pick(Qt::Orientation o, const QSizePolicy &policy)
{ return o == Qt::Horizontal ? policy.horizontalPolicy() : policy.verticalPolicy(); }

static inline int perp(Qt::Orientation o, const QPoint &pos)
{ return o == Qt::Vertical ? pos.x() : pos.y(); }

static inline int perp(Qt::Orientation o, const QSize &size)
{ return o == Qt::Vertical ? size.width() : size.height(); }

static inline int &rperp(Qt::Orientation o, QPoint &pos)
{ return o == Qt::Vertical ? pos.rx() : pos.ry(); }

static inline int &rperp(Qt::Orientation o, QSize &size)
{ return o == Qt::Vertical ? size.rwidth() : size.rheight(); }

// the rest of QMainWindow uses this instead...
static inline int pick_perp(Qt::Orientation o, const QPoint &p)
{ return o == Qt::Vertical ? p.x() : p.y(); }
static inline int pick_perp(Qt::Orientation o, const QSize &s)
{ return o == Qt::Vertical ? s.width() : s.height(); }

class QWidgetItem;
class QDockWidgetLayout
{
public:
    enum DockPos {
        LeftPos,
        RightPos,
        TopPos,
        BottomPos,
        PosCount,
        CenterPos = PosCount
    };
    enum { EmptyDropAreaSize = 80 }; // when a dock area is empty, how "wide" is it?

    Qt::DockWidgetArea corners[4]; // use a Qt::Corner for indexing
    QRect rect;
    QWidgetItem *centralWidgetItem;
    QMainWindow *mainWindow;
    QRect centralWidgetRect;
    QDockWidgetLayout(QMainWindow *win);
    QDockAreaLayoutInfo docks[4];
    int sep; // separator extent

    bool isValid() const;

    enum { DockWidgetStateMarker = 0xfd };
    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream, const QList<QDockWidget*> &widgets);

    QList<int> indexOf(QDockWidget *dockWidget, IndexOfFlag flag = IndexOfFindsVisible) const;
    QList<int> gapIndex(const QPoint &pos, bool nestingEnabled) const;
    QList<int> findSeparator(const QPoint &pos) const;

    QDockAreaLayoutItem &item(QList<int> path);
    QDockAreaLayoutInfo *info(QList<int> path);
    QDockAreaLayoutInfo *info(QWidget *widget);
    QRect itemRect(QList<int> path) const;
    QRect separatorRect(int index) const;
    QRect separatorRect(QList<int> path) const;

    bool insertGap(QList<int> path, QWidgetItem *dockWidgetItem);
    QWidgetItem *convertToGap(QList<int> path);
    QRect convertToWidget(QList<int> path, QWidgetItem *dockWidgetItem);
    void remove(QList<int> path);

    void fitLayout();

    void clear();

    QSize sizeHint() const;
    QSize minimumSize() const;

    void addDockWidget(DockPos pos, QDockWidget *dockWidget, Qt::Orientation orientation);
    void splitDockWidget(QDockWidget *after, QDockWidget *dockWidget,
                         Qt::Orientation orientation);
    void tabifyDockWidget(QDockWidget *first, QDockWidget *second);

    void apply(bool animate);

    void paintSeparators(QPainter *p, QWidget *widget, const QRegion &clip,
                            const QPoint &mouse) const;
    QRegion separatorRegion() const;
    int separatorMove(QList<int> separator, const QPoint &origin, const QPoint &dest,
                        QVector<QLayoutStruct> *cache);

    QLayoutItem *itemAt(int *x, int index) const;
    QLayoutItem *takeAt(int *x, int index);
    void deleteAllLayoutItems();

    void getGrid(QVector<QLayoutStruct> *ver_struct_list,
                    QVector<QLayoutStruct> *hor_struct_list);
    void setGrid(QVector<QLayoutStruct> *ver_struct_list,
                    QVector<QLayoutStruct> *hor_struct_list);

    QRect gapRect(QList<int> path);

    void keepSize(QDockWidget *w);

    QSet<QTabBar*> usedTabBars() const;
};

// void dump(QDebug debug, const QDockWidgetLayout &layout);

#endif // QT_NO_MAINWINDOW

#endif // QDYNAMICDOCKWIDGETLAYOUT_P_H
