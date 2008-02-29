/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDYNAMICMAINWINDOWLAYOUT_P_H
#define QDYNAMICMAINWINDOWLAYOUT_P_H

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

#include "qmainwindow.h"

#ifndef QT_NO_MAINWINDOW

#include "QtGui/qlayout.h"
#include "QtCore/qvector.h"
#include "QtCore/qset.h"
#include "private/qlayoutengine_p.h"

#include "qdockwidgetlayout_p.h"

class QToolBar;
class QWidgetAnimator;
class QTabBar;
class QRubberBand;

class QMainWindowLayout : public QLayout
{
    Q_OBJECT

public:
    explicit QMainWindowLayout(QMainWindow *mainwindow);
    ~QMainWindowLayout();

    QLayoutItem *statusbar;
#ifndef QT_NO_STATUSBAR
    QStatusBar *statusBar() const;
    void setStatusBar(QStatusBar *sb);
#endif

    QWidget *centralWidget() const;
    void setCentralWidget(QWidget *cw);

#ifndef QT_NO_TOOLBAR
    void addToolBarBreak(Qt::ToolBarArea area);
    void insertToolBarBreak(QToolBar *before);
    void addToolBar(Qt::ToolBarArea area, QToolBar *toolbar, bool needAddChildWidget = true);
    void insertToolBar(QToolBar *before, QToolBar *toolbar);
    Qt::ToolBarArea toolBarArea(QToolBar *toolbar) const;
#endif

#ifndef QT_NO_DOCKWIDGET
    void setCorner(Qt::Corner corner, Qt::DockWidgetArea area);
    Qt::DockWidgetArea corner(Qt::Corner corner) const;

    void addDockWidget(Qt::DockWidgetArea area,
                       QDockWidget *dockwidget,
                       Qt::Orientation orientation);
    void splitDockWidget(QDockWidget *after,
                         QDockWidget *dockwidget,
                         Qt::Orientation orientation);
    void tabifyDockWidget(QDockWidget *first, QDockWidget *second);
    Qt::DockWidgetArea dockWidgetArea(QDockWidget *dockwidget) const;
#endif

    enum { // sentinel values used to validate state data
        VersionMarker = 0xff,
        ToolBarStateMarker = 0xfe,
        ToolBarStateMarkerEx = 0xfc
    };
    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream);

    // QLayout interface
    void addItem(QLayoutItem *item);
    void setGeometry(const QRect &r);
    QLayoutItem *itemAt(int index) const;
    QLayoutItem *takeAt(int index);
    int count() const;

    QSize sizeHint() const;
    QSize minimumSize() const;
    mutable QSize szHint;
    mutable QSize minSize;

    void invalidate();

    // returns true if \a widget is a toolbar or dockwidget that we know about
    bool contains(QWidget *widget) const;

    // utility functions
    void relayout();
    void updateToolbarsInArea(Qt::ToolBarArea area);

#ifndef QT_NO_DOCKWIDGET
    QWidgetAnimator *widgetAnimator;
    bool dockNestingEnabled;
    bool animationEnabled;
    QDockWidgetLayout dockWidgetLayout, savedDockWidgetLayout;

    void applyDockWidgetLayout(QDockWidgetLayout &newLayout, bool animate = true);

    QWidgetItem *unplug(QDockWidget *dockWidget);
    QList<int> hover(QWidgetItem *dockWidgetItem, const QPoint &mousePos);
    void plug(QWidgetItem *dockWidgetItem, const QList<int> &pathToGap);
    void restore();
    QList<int> currentGapPos;
    QRect currentGapRect;
    QDockWidget *pluggingWidget;
    QRubberBand *gapIndicator;
    void updateGapIndicator();
    void paintDropIndicator(QPainter *p, QWidget *widget, const QRegion &clip);
    void raise(QDockWidget *widget);

    bool startSeparatorMove(const QPoint &pos);
    bool separatorMove(const QPoint &pos);
    bool endSeparatorMove(const QPoint &pos);
    QList<int> movingSeparator;
    QPoint movingSeparatorOrigin, movingSeparatorPos;
    QTimer *separatorMoveTimer;
    QVector<QLayoutStruct> separatorMoveCache;

    void keepSize(QDockWidget *w);

#ifndef QT_NO_TABBAR
    QTabBar *getTabBar();
    QSet<QTabBar*> usedTabBars;
    QList<QTabBar*> unusedTabBars;
#endif

private slots:
    void animationFinished(QWidget *widget);
    void allAnimationsFinished();
    void doSeparatorMove();
#ifndef QT_NO_TABBAR
    void tabChanged();
#endif

public:
#else
    QLayoutItem *centralWidgetItem; // a window compiled with QT_NO_TOOLBAR still needs
                                    // a centralWidget
#endif

#ifndef QT_NO_TOOLBAR
    int locateToolBar(QToolBar *toolbar, const QPoint &mouse) const;
    bool dropToolBar(QToolBar *toolbar, const QPoint &mouse, const QPoint &offset);

    void removeToolBarInfo(QToolBar *toolbar);

    // toolbar layout data
    struct ToolBarLayoutInfo
    {
        ToolBarLayoutInfo() : item(0) {}
	QLayoutItem *item;
	QPoint pos;
        QSize size;
	QPoint offset;
        QPoint user_pos;
    };

    struct ToolBarLineInfo
    {
        int pos;
        QList<ToolBarLayoutInfo> list;
    };

    /*
      helpers to return the index of next/prev visible toolbar... they
      return -1 if none is found
    */
    static int nextVisible(int index, const ToolBarLineInfo &lineInfo);
    static int prevVisible(int index, const ToolBarLineInfo &lineInfo);
    QList<ToolBarLineInfo> tb_layout_info, *save_tb_layout_info;
#endif
};

#endif // QT_NO_MAINWINDOW

#endif // QDYNAMICMAINWINDOWLAYOUT_P_H
