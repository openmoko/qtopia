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

#include "qmainwindowlayout_p.h"

#ifndef QT_NO_MAINWINDOW

#include "qdockwidgetlayout_p.h"

#include "qdockwidget.h"
#include "qdockwidget_p.h"
#include "qmainwindow.h"
#include "qtoolbar.h"
#include "qwidgetanimator_p.h"
#include "qrubberband.h"

#include <qapplication.h>
#include <qdebug.h>
#include <qstatusbar.h>
#include <qstyle.h>
#include <qvarlengtharray.h>
#include <qstack.h>
#include <qmap.h>
#include <qtimer.h>

#include <private/qlayoutengine_p.h>

// #define LAYOUT_DEBUG
#if defined(LAYOUT_DEBUG)
#  define DEBUG qDebug
#else
#  define DEBUG if(false)qDebug
#endif

// #define LAYOUT_DEBUG_VERBOSE
#if defined(LAYOUT_DEBUG_VERBOSE)
#  define VDEBUG qDebug
#else
#  define VDEBUG if(false)qDebug
#endif

// #define TOOLBAR_DEBUG
#if defined(TOOLBAR_DEBUG)
#  define TBDEBUG qDebug
#else
#  define TBDEBUG if(false)qDebug
#endif

enum POSITION {
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    CENTER,
    NPOSITIONS
};

static inline void validateToolBarArea(Qt::ToolBarArea &area)
{
    switch (area) {
    case Qt::LeftToolBarArea:
    case Qt::RightToolBarArea:
    case Qt::TopToolBarArea:
    case Qt::BottomToolBarArea:
        break;
    default:
        area = Qt::TopToolBarArea;
    }
}

static inline void validateDockWidgetArea(Qt::DockWidgetArea &area)
{
    switch (area) {
    case Qt::LeftDockWidgetArea:
    case Qt::RightDockWidgetArea:
    case Qt::TopDockWidgetArea:
    case Qt::BottomDockWidgetArea:
        break;
    default:
        area = Qt::LeftDockWidgetArea;
    }
}

static inline uint areaForPosition(int pos)
{ return ((1u << pos) & 0xf); }

static inline POSITION positionForArea(Qt::DockWidgetArea area)
{
    switch (area) {
    case Qt::LeftDockWidgetArea:   return LEFT;
    case Qt::RightDockWidgetArea:  return RIGHT;
    case Qt::TopDockWidgetArea:    return TOP;
    case Qt::BottomDockWidgetArea: return BOTTOM;
    default: break;
    }
    return CENTER;
}

#ifndef QT_NO_TOOLBAR
static inline POSITION positionForArea(Qt::ToolBarArea area)
{
    switch (area) {
    case Qt::LeftToolBarArea:   return LEFT;
    case Qt::RightToolBarArea:  return RIGHT;
    case Qt::TopToolBarArea:    return TOP;
    case Qt::BottomToolBarArea: return BOTTOM;
    default: break;
    }
    return CENTER;
}
#endif

static inline int pick(POSITION p, const QSize &s)
{ return p == TOP || p == BOTTOM ? s.height() : s.width(); }

static inline int pick(POSITION p, const QPoint &pt)
{ return p == TOP || p == BOTTOM ? pt.y() : pt.x(); }

static inline void set(POSITION p, QSize &s, int x)
{ if (p == LEFT || p == RIGHT) s.setWidth(x); else s.setHeight(x); }

static inline int pick_perp(POSITION p, const QSize &s)
{ return p == TOP || p == BOTTOM ? s.width() : s.height(); }

static inline int pick_perp(POSITION p, const QPoint &pt)
{ return p == TOP || p == BOTTOM ? pt.x() : pt.y(); }

static inline void set_perp(POSITION p, QSize &s, int x)
{ if (p == TOP || p == BOTTOM) s.setWidth(x); else s.setHeight(x); }

static inline void set_perp(POSITION p, QPoint &pt, int x)
{ if (p == TOP || p == BOTTOM) pt.setX(x); else pt.setY(x); }

class QMainWindowLayoutItem : public QWidgetItem
{
public:
    inline QMainWindowLayoutItem(QWidget *w, const QRect &r)
        : QWidgetItem(w), rect(r)
    { }

    inline QSize sizeHint() const
    { return rect.size(); }
    inline void setGeometry( const QRect &r)
    { rect = r; }
    inline QRect geometry() const
    { return rect; }
    inline bool isEmpty() const
    { return false; }

    QWidget *widget;
    QRect rect;
};


QMainWindowLayout::QMainWindowLayout(QMainWindow *mainwindow)
    : QLayout(mainwindow), statusbar(0)
#ifndef QT_NO_DOCKWIDGET
    , dockWidgetLayout(mainwindow), savedDockWidgetLayout(mainwindow)
#endif
#ifndef QT_NO_TOOLBAR
      , save_tb_layout_info(0)
#endif
{
#ifndef QT_NO_DOCKWIDGET
    gapIndicator = new QRubberBand(QRubberBand::Rectangle, mainwindow);
    gapIndicator->hide();
    pluggingWidget = 0;
    dockNestingEnabled = false;
    animationEnabled = true;
    separatorMoveTimer = new QTimer(this);
    separatorMoveTimer->setSingleShot(true);
    separatorMoveTimer->setInterval(0);
    connect(separatorMoveTimer, SIGNAL(timeout()), this, SLOT(doSeparatorMove()));
    widgetAnimator = new QWidgetAnimator(this);
    connect(widgetAnimator, SIGNAL(finished(QWidget*)),
            this, SLOT(animationFinished(QWidget*)), Qt::QueuedConnection);
    connect(widgetAnimator, SIGNAL(finishedAll()),
            this, SLOT(allAnimationsFinished()));
    setObjectName(mainwindow->objectName() + QLatin1String("_layout"));
#else
    centralWidgetItem = 0;
#endif
}

QMainWindowLayout::~QMainWindowLayout()
{
#ifndef QT_NO_TOOLBAR
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
        for (int i = 0; i < lineInfo.list.size(); ++i)
            delete lineInfo.list.at(i).item;
    }
    tb_layout_info.clear();
#endif

#ifndef QT_NO_DOCKWIDGET
    dockWidgetLayout.deleteAllLayoutItems();
    delete dockWidgetLayout.centralWidgetItem;
#else
    delete centralWidgetItem;
#endif

    delete statusbar;
}

#ifndef QT_NO_STATUSBAR
QStatusBar *QMainWindowLayout::statusBar() const
{ return statusbar ? qobject_cast<QStatusBar *>(statusbar->widget()) : 0; }

void QMainWindowLayout::setStatusBar(QStatusBar *sb)
{
    if (sb)
        addChildWidget(sb);
    delete statusbar;
    statusbar = sb ? new QWidgetItem(sb) : 0;
    invalidate();
}
#endif // QT_NO_STATUSBAR

QWidget *QMainWindowLayout::centralWidget() const
{
    QLayoutItem *item = 0;
#ifndef QT_NO_DOCKWIDGET
    item = dockWidgetLayout.centralWidgetItem;
#else
    item = centralWidgetItem;
#endif
    return item == 0 ? 0 : item->widget();
}

void QMainWindowLayout::setCentralWidget(QWidget *cw)
{
    QWidgetItem *item = 0;
    if (cw != 0) {
        addChildWidget(cw);
        item = new QWidgetItem(cw);
    }

#ifndef QT_NO_DOCKWIDGET
    delete dockWidgetLayout.centralWidgetItem;
    dockWidgetLayout.centralWidgetItem = item;
#else
    delete centralWidgetItem;
    centralWidgetItem = item;
#endif

    invalidate();
}

#ifndef QT_NO_TOOLBAR
void QMainWindowLayout::addToolBarBreak(Qt::ToolBarArea area)
{
    ToolBarLineInfo newLine;
    validateToolBarArea(area);
    newLine.pos = positionForArea(area);
    switch (newLine.pos) {
    case TOP:
    case BOTTOM:
        {
            for (int line = 0; line < tb_layout_info.size(); ++line) {
                ToolBarLineInfo &lineInfo = tb_layout_info[line];
                if (lineInfo.pos == LEFT || lineInfo.pos == RIGHT) {
                    tb_layout_info.insert(line, newLine);
                    return;
                }
            }
        }
        // fall through intended
    default:
        tb_layout_info.append(newLine);
        TBDEBUG() << "appended new line";
        break;
    }
}

void QMainWindowLayout::insertToolBarBreak(QToolBar *before)
{
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        ToolBarLineInfo &lineInfo = tb_layout_info[line];
        for (int i = 0; i < lineInfo.list.size(); ++i) {
            const ToolBarLayoutInfo &info = lineInfo.list.at(i);
            if (info.item->widget() == before) {
                ToolBarLineInfo newLine;
                newLine.pos = lineInfo.pos;
                int itemsToMove = lineInfo.list.size() - i;
                for (int j = 0 ; j < itemsToMove ; ++j)
                    newLine.list.prepend(lineInfo.list.takeLast());
                tb_layout_info.insert(line + 1, newLine);
                return;
            }
        }
    }
}

/*!
    Adds \a toolbar to \a area, continuing the current line.
*/
void QMainWindowLayout::addToolBar(Qt::ToolBarArea area,
                                   QToolBar *toolbar,
                                   bool needAddChildWidget)
{
    if (needAddChildWidget)
        addChildWidget(toolbar);
    else
        removeToolBarInfo(toolbar);

    validateToolBarArea(area);
    POSITION pos = positionForArea(area);
    // see if we have an existing line in the tb - append it in the last in line
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        if (tb_layout_info.at(line).pos == pos) {
            while (line < tb_layout_info.size() - 1 && tb_layout_info.at(line + 1).pos == pos)
                ++line;

            switch (pos) {
            case TOP:
            case BOTTOM:
                toolbar->setOrientation(Qt::Horizontal);
                break;
            case LEFT:
            case RIGHT:
                toolbar->setOrientation(Qt::Vertical);
                break;
            default:
                break;
            }

            ToolBarLineInfo &lineInfo = tb_layout_info[line];
            ToolBarLayoutInfo newinfo;
            newinfo.item = new QWidgetItem(toolbar);
            lineInfo.list.append(newinfo);
            return;
        }
    }

    // no line to continue, add one and recurse
    addToolBarBreak(area);
    addToolBar(area, toolbar, false);
}

/*!
    Adds \a toolbar before \a before
*/
void QMainWindowLayout::insertToolBar(QToolBar *before, QToolBar *toolbar)
{
    addChildWidget(toolbar);

    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
	for (int i = 0; i < lineInfo.list.size(); ++i) {
	    const ToolBarLayoutInfo &info = lineInfo.list.at(i);
	    if (info.item->widget() == before) {

                ToolBarLayoutInfo newInfo;
                newInfo.item = new QWidgetItem(toolbar);
                tb_layout_info[line].list.insert(i, newInfo);
                return;
            }
        }
    }
}

Qt::ToolBarArea QMainWindowLayout::toolBarArea(QToolBar *toolbar) const
{
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
	for (int i = 0; i < lineInfo.list.size(); ++i) {
	    const ToolBarLayoutInfo &info = lineInfo.list.at(i);
	    if (info.item->widget() == toolbar)
                return static_cast<Qt::ToolBarArea>(areaForPosition(lineInfo.pos));
        }
    }
    return Qt::NoToolBarArea;
}
#endif // QT_NO_TOOLBAR

#ifndef QT_NO_DOCKWIDGET

void QMainWindowLayout::setCorner(Qt::Corner corner, Qt::DockWidgetArea area)
{
    if (dockWidgetLayout.corners[corner] == area)
        return;
    dockWidgetLayout.corners[corner] = area;
    relayout();
}

Qt::DockWidgetArea QMainWindowLayout::corner(Qt::Corner corner) const
{
    return dockWidgetLayout.corners[corner];
}

void QMainWindowLayout::addDockWidget(Qt::DockWidgetArea area,
                                             QDockWidget *dockwidget,
                                             Qt::Orientation orientation)
{
    QDockWidgetLayout::DockPos pos
        = QDockWidgetLayout::DockPos(positionForArea(area));
    addChildWidget(dockwidget);
    dockWidgetLayout.addDockWidget(pos, dockwidget, orientation);
}

void QMainWindowLayout::tabifyDockWidget(QDockWidget *first, QDockWidget *second)
{
    addChildWidget(second);
    dockWidgetLayout.tabifyDockWidget(first, second);
}

void QMainWindowLayout::splitDockWidget(QDockWidget *after,
                                               QDockWidget *dockwidget,
                                               Qt::Orientation orientation)
{
    addChildWidget(dockwidget);
    dockWidgetLayout.splitDockWidget(after, dockwidget, orientation);
}
#endif // QT_NO_DOCKWIDGET

#ifndef QT_NO_DOCKWIDGET
static bool findWidgetRecursively(QLayoutItem *li, QWidget *w)
{
    QLayout *lay = li->layout();
    if (!lay)
        return false;
    int i = 0;
    QLayoutItem *child;
    while ((child = lay->itemAt(i))) {
        if (child->widget() == w) {
            return true;
        } else if (findWidgetRecursively(child, w)) {
            return true;
        } else {
            ++i;
        }
    }
    return false;
}

Qt::DockWidgetArea QMainWindowLayout::dockWidgetArea(QDockWidget *widget) const
{
    QList<int> pathToWidget = dockWidgetLayout.indexOf(widget, IndexOfFindsAll);
    if (pathToWidget.isEmpty())
        return Qt::NoDockWidgetArea;
    int pos = pathToWidget.first();
    Q_ASSERT(pos < QDockWidgetLayout::PosCount);
    return Qt::DockWidgetArea(areaForPosition(pos));
}
#endif // QT_NO_DOCKWIDGET

void QMainWindowLayout::saveState(QDataStream &stream) const
{
#ifndef QT_NO_TOOLBAR
    // save toolbar state
    stream << (uchar) ToolBarStateMarkerEx;
    stream << tb_layout_info.size(); // number of toolbar lines
    if (!tb_layout_info.isEmpty()) {
        for (int line = 0; line < tb_layout_info.size(); ++line) {
            const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
            stream << lineInfo.pos;
            stream << lineInfo.list.size();
            for (int i = 0; i < lineInfo.list.size(); ++i) {
                const ToolBarLayoutInfo &info = lineInfo.list.at(i);
                QWidget *widget = info.item->widget();
                QString objectName = widget->objectName();
                if (objectName.isEmpty()) {
                    qWarning("QMainWindow::saveState(): 'objectName' not set for QToolBar %p '%s'",
                             widget, widget->windowTitle().toLocal8Bit().constData());
                }
                stream << objectName;
                stream << (uchar) !widget->isHidden();
                stream << info.pos;
                stream << info.size;
                stream << info.offset;
                stream << info.user_pos;
            }
        }
    }
#endif // QT_NO_TOOLBAR

#ifndef QT_NO_DOCKWIDGET
    // save dockwidget state
    dockWidgetLayout.saveState(stream);
#endif // QT_NO_DOCKWIDGET
}

bool QMainWindowLayout::restoreState(QDataStream &stream)
{
#ifndef QT_NO_TOOLBAR
    // restore toolbar layout
    uchar tmarker;
    stream >> tmarker;
    if (tmarker != ToolBarStateMarker && tmarker != ToolBarStateMarkerEx)
        return false;

    int lines;
    stream >> lines;
    QList<ToolBarLineInfo> toolBarState;
    QList<QToolBar *> toolbars = qFindChildren<QToolBar *>(parentWidget());
    for (int line = 0; line < lines; ++line) {
        ToolBarLineInfo lineInfo;
        stream >> lineInfo.pos;
        int size;
        stream >> size;
        for (int i = 0; i < size; ++i) {
            ToolBarLayoutInfo info;
            QString objectName;
            stream >> objectName;
            uchar shown;
            stream >> shown;
            stream >> info.pos;
            stream >> info.size;
            stream >> info.offset;
            if (tmarker == ToolBarStateMarkerEx)
                stream >> info.user_pos;

            if (objectName.isEmpty()) {
                qWarning("QMainWindow::restoreState: Cannot restore a QToolBar with an empty 'objectName'");
                continue;
            }

            // find toolbar
            QToolBar *toolbar = 0;
            for (int t = 0; t < toolbars.size(); ++t) {
                QToolBar *tb = toolbars.at(t);
                if (tb && tb->objectName() == objectName) {
                    toolbar = tb;
                    toolbars[t] = 0;
                    break;
                }
            }
            if (!toolbar) {
                qWarning("QMainWindow::restoreState: Cannot find a QToolBar with "
                         "matching 'objectName' (looking for '%s').",
                         objectName.toLocal8Bit().constData());
                continue;
            }

            info.item = new QWidgetItem(toolbar);
            toolbar->setVisible(shown);
            toolbar->setOrientation((lineInfo.pos == LEFT  || lineInfo.pos == RIGHT)
                                    ? Qt::Vertical
                                    : Qt::Horizontal);
            lineInfo.list << info;
        }
        toolBarState << lineInfo;
    }

    if (stream.status() != QDataStream::Ok)
        return false;

    // remove restored toolbars from the existing toolbar layout
    for (int line = 0; line < toolBarState.size(); ++line) {
        const ToolBarLineInfo &lineInfo = toolBarState.at(line);
        for (int i = 0; i < lineInfo.list.size(); ++i) {
            const ToolBarLayoutInfo &info = lineInfo.list.at(i);

            bool found = false;
            for (int eline = 0; !found && eline < tb_layout_info.size(); ++eline) {
                ToolBarLineInfo &elineInfo = tb_layout_info[eline];
                for (int e = 0; !found && e < elineInfo.list.size(); ++e) {
                    ToolBarLayoutInfo &einfo = elineInfo.list[e];
                    if (info.item->widget() == einfo.item->widget()) {
                        // found it
                        found = true;
                        delete einfo.item;
                        elineInfo.list.removeAt(e);
                        if (elineInfo.list.isEmpty())
                            tb_layout_info.removeAt(eline);
                    }
                }
            }
        }
    }
    if (!tb_layout_info.isEmpty()) {
        // merge toolbars that have not been restored into the restored layout
        int lineCount[NPOSITIONS - 1] = { 0, 0, 0, 0 };
        while (!tb_layout_info.isEmpty()) {
            ToolBarLineInfo lineInfo = tb_layout_info.takeFirst();
            ++lineCount[lineInfo.pos];

            bool merged = false;
            int targetLine = 0;
            for (int line = 0; line < toolBarState.size(); ++line) {
                ToolBarLineInfo &restoredLineInfo = toolBarState[line];
                if (lineInfo.pos != restoredLineInfo.pos)
                    continue;
                if (++targetLine == lineCount[lineInfo.pos]) {
                    // merge!
                    restoredLineInfo.list << lineInfo.list;
                    merged = true;
                }
            }
            if (!merged) {
                // couldn't merge this toolbar line, append it to the new layout
                toolBarState << lineInfo;
            }
        }
    }
    // replace existing toolbar layout
    tb_layout_info = toolBarState;
#endif // QT_NO_TOOLBAR

#ifndef QT_NO_DOCKWIDGET
    // restore dockwidget layout
    QList<QDockWidget *> dockwidgets = qFindChildren<QDockWidget *>(parentWidget());
    QMainWindow *win = qobject_cast<QMainWindow*>(parentWidget());
    Q_ASSERT(win != 0);

    QDockWidgetLayout newLayout(win);

    if (!newLayout.restoreState(stream, dockwidgets))
        stream.setStatus(QDataStream::ReadCorruptData);

    if (stream.status() != QDataStream::Ok) {
        newLayout.deleteAllLayoutItems();
        applyDockWidgetLayout(dockWidgetLayout, false); // hides tabBars allocated by newLayout
        return false;
    }

    // if any of the dockwidgets have not been restored, append them
    // to the end of their current area

    for (int i = 0; i < dockwidgets.size(); ++i) {
        QDockWidget *w = dockwidgets.at(i);

        QList<int> path = newLayout.indexOf(w, IndexOfFindsAll);
        if (path.isEmpty()) {
            QList<int> old_path = dockWidgetLayout.indexOf(w, IndexOfFindsAll);
            if (old_path.isEmpty()) {
                qWarning("QMainWindowLayout::restoreState(): failed to find %p "
                         "in the old layout", w);
                continue;
            }
            QDockAreaLayoutInfo *info = newLayout.info(old_path);
            if (info == 0) {
                qWarning("QMainWindowLayout::restoreState(): failed to find location for %p "
                         "in the new layout", w);
                continue;
            }
            info->item_list.append(QDockAreaLayoutItem(new QWidgetItem(w)));
        }
    }

    newLayout.centralWidgetItem = dockWidgetLayout.centralWidgetItem;
    dockWidgetLayout.deleteAllLayoutItems();
    dockWidgetLayout = newLayout;
    applyDockWidgetLayout(dockWidgetLayout, false);
#ifndef QT_NO_TABBAR
    foreach (QTabBar *tab_bar, usedTabBars)
        tab_bar->show();
#endif
#endif // QT_NO_DOCKWIDGET

    return true;
}


int QMainWindowLayout::count() const
{
    qWarning("QMainWindowLayout::count: ?");
    return 0; //#################################################
}

QLayoutItem *QMainWindowLayout::itemAt(int index) const
{
    VDEBUG("QMainWindowLayout::itemAt: index %d", index);

    int x = 0;
#ifndef QT_NO_TOOLBAR
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
	for (int i = 0; i < lineInfo.list.size(); ++i) {
            if (x++ == index) {
                const ToolBarLayoutInfo &info = lineInfo.list.at(i);
                VDEBUG() << "END of itemAt(), found QToolBar item" << info.item;
                return info.item;
            }
        }
    }
#endif

#ifndef QT_NO_DOCKWIDGET
    if (QLayoutItem *ret = dockWidgetLayout.itemAt(&x, index)) {
        VDEBUG() << "END of itemAt(), found QDockWidget item" << ret;
        return ret;
    }
#else
    if (centralWidgetItem != 0 && x++ == index)
        return centralWidgetItem;
#endif

    if (statusbar && x++ == index) {
        VDEBUG() << "END of itemAt(), found QStatusBar item" << statusbar;
        return statusbar;
    }

    VDEBUG() << "END of itemAt(), no item found";
    return 0;
}

QLayoutItem *QMainWindowLayout::takeAt(int index)
{
    DEBUG("QMainWindowLayout::takeAt: index %d", index);

    int x = 0;
#ifndef QT_NO_TOOLBAR
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        ToolBarLineInfo &lineInfo = tb_layout_info[line];
	for (int i = 0; i < lineInfo.list.size(); ++i) {
            if (x++ == index) {
                QLayoutItem *ret = lineInfo.list.at(i).item;
                lineInfo.list.removeAt(i);
                if (lineInfo.list.size() == 0)
                    tb_layout_info.removeAt(line);
                VDEBUG() << "END of itemAt(), removed QToolBar item" << ret;
                return ret;
            }
	}
    }
#endif

#ifndef QT_NO_DOCKWIDGET
    if (QLayoutItem *ret = dockWidgetLayout.takeAt(&x, index)) {
        VDEBUG() << "END of itemAt(), removed QDockWidget item" << ret;
        return ret;
    }
#else
    if (centralWidgetItem != 0 && x++ == index) {
        QLayoutItem *ret = centralWidgetItem;
        centralWidgetItem = 0;
        return ret;
    }
#endif

    if (statusbar && x++ == index) {
        QLayoutItem *ret = statusbar;
        statusbar = 0;
        VDEBUG() << "END of itemAt(), removed QStatusBar item" << ret;
        return ret;
    }

    VDEBUG() << "END of QMainWindowLayout::takeAt (not found)";
    return 0;
}

#ifndef QT_NO_TOOLBAR
/*
  Returns the size hint for the first user item in a tb layout. This
  is used to contrain the minimum size a tb can have.
*/
static QSize get_first_item_sh(QLayout *layout)
{
    QLayoutItem *item = layout->itemAt(1);
    if (item && item->widget())
	return item->widget()->sizeHint();
    else
	return QSize(0, 0);
}

/*
  Returns the real size hint for items in a layout - including sizes
  for hidden items.
*/
static QSize get_real_sh(QLayout *layout)
{
    QSize real_sh(0, 0);
    int i = 0;
    while (layout && layout->itemAt(i))
        real_sh += layout->itemAt(i++)->widget()->sizeHint();
    --i;
    int spacing = layout->spacing();
    int margin = layout->margin();
    real_sh += QSize(spacing*i + margin*2, spacing*i + margin*2);
    return real_sh;
}
#endif // QT_NO_TOOLBAR

void QMainWindowLayout::setGeometry(const QRect &_r)
{
#ifndef QT_NO_DOCKWIDGET
    if (savedDockWidgetLayout.isValid())
        return;
#endif

    QLayout::setGeometry(_r);
    QRect r = _r;

    if (statusbar) {
        QRect sbr(QPoint(0, 0),
                  QSize(r.width(), statusbar->heightForWidth(r.width()))
                  .expandedTo(statusbar->minimumSize()));
        sbr.moveBottom(r.bottom());
        QRect vr = QStyle::visualRect(QApplication::layoutDirection(), _r, sbr);
        statusbar->setGeometry(vr);
        r.setBottom(sbr.top() - 1);
    }

#ifndef QT_NO_TOOLBAR
    // layout toolbars

    // calculate the respective tool bar rectangles and store the
    // width/height of the largest tb on each line
    QVarLengthArray<QRect> tb_rect(tb_layout_info.size());

    QMap<int, QSize> rest_sz;
    QStack<QSize> bottom_sz;
    QStack<QSize> right_sz;

    QStack<QRect> bottom_rect;
    QStack<QRect> right_rect;

    // calculate the width/height of the different tool bar lines -
    // the order of the bottom and right tool bars needs to reversed
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);

        bool lineHidden = true;
        for (int i = 0; i < lineInfo.list.size(); ++i)
            lineHidden &= lineInfo.list.at(i).item->isEmpty();

        if (lineHidden)
            continue;

	QSize tb_sz;
        for (int i = 0; i < lineInfo.list.size(); ++i) {
	    const ToolBarLayoutInfo &info = lineInfo.list.at(i);
            if (info.item->isEmpty())
                continue;
	    QSize sh = info.item->sizeHint();
	    if (tb_sz.width() < sh.width())
		tb_sz.setWidth(sh.width());
	    if (tb_sz.height() < sh.height())
		tb_sz.setHeight(sh.height());
	}
	switch(lineInfo.pos) {
	case TOP:
	case LEFT:
	    rest_sz[line] = tb_sz;
	    break;
	case BOTTOM:
	    bottom_sz.push(tb_sz);
	    break;
	case RIGHT:
	    right_sz.push(tb_sz);
	    break;
	default:
	    Q_ASSERT_X(false, "QMainWindowLayout", "internal error");
	}
    }

    // calculate the rect for each tool bar line
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
	QSize tb_sz;
	tb_rect[line] = r;

        bool lineHidden = true;
        for (int i = 0; i < lineInfo.list.size(); ++i)
            lineHidden &= lineInfo.list.at(i).item->isEmpty();

        if (lineHidden)
            continue;

	switch (lineInfo.pos) {
	case TOP:
	    tb_sz = rest_sz[line];
	    tb_rect[line].setBottom(tb_rect[line].top() + tb_sz.height() - 1);
	    r.setTop(tb_rect[line].bottom() + 1);
	    break;
	case LEFT:
	    tb_sz = rest_sz[line];
	    tb_rect[line].setRight(tb_rect[line].x() + tb_sz.width() - 1);
	    r.setLeft(tb_rect[line].right() + 1);
	    break;
	case BOTTOM:
 	    tb_sz = bottom_sz.pop();
	    tb_rect[line].setTop(tb_rect[line].bottom() - tb_sz.height() + 1);
	    bottom_rect.push(tb_rect[line]);
	    r.setBottom(tb_rect[line].top() - 1);
	    break;
	case RIGHT:
 	    tb_sz = right_sz.pop();
	    tb_rect[line].setLeft(tb_rect[line].right() - tb_sz.width() + 1);
	    right_rect.push(tb_rect[line]);
	    r.setRight(tb_rect[line].left() - 1);
	    break;
	default:
	    Q_ASSERT_X(false, "QMainWindowLayout", "internal error");
	}
    }

    // put the right and bottom rects back in correct order
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);

        bool lineHidden = true;
        for (int i = 0; i < lineInfo.list.size(); ++i)
            lineHidden &= lineInfo.list.at(i).item->isEmpty();

        if (lineHidden)
            continue;

	if (lineInfo.pos == BOTTOM)
	    tb_rect[line] = bottom_rect.pop();
	else if (lineInfo.pos == RIGHT)
	    tb_rect[line] = right_rect.pop();
    }

    // at this point the space for the tool bars have been shaved off
    // the main rect, continue laying out each tool bar line
    int tb_fill = 0;
    if (!tb_layout_info.isEmpty() && !tb_layout_info.at(0).list.isEmpty()) {
	tb_fill = tb_layout_info.at(0).list.at(0).item->widget()->layout()->margin() * 2
                  + parentWidget()->style()->pixelMetric(QStyle::PM_ToolBarHandleExtent)
		  + parentWidget()->style()->pixelMetric(QStyle::PM_ToolBarItemSpacing) * 2
                  + parentWidget()->style()->pixelMetric(QStyle::PM_ToolBarExtensionExtent);
    }
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        ToolBarLineInfo &lineInfo = tb_layout_info[line];
        int num_tbs = lineInfo.list.size();
	POSITION where = static_cast<POSITION>(lineInfo.pos);

        bool first = true;
        for (int i = 0; i < num_tbs; ++i) {
            ToolBarLayoutInfo &info = lineInfo.list[i];
            if (info.item->isEmpty())
                continue;

 	    set(where, info.size, pick(where, tb_rect[line].size()));

	    // position
 	    if (first) { // first tool bar can't have an offset
                first = false;
                int nextIndex = nextVisible(i, lineInfo);
		if (nextIndex != -1 && !info.size.isEmpty()
		    && pick_perp(where, info.offset) > pick_perp(where, info.size)) {
		    // swap if dragging it past the next one
                    ToolBarLayoutInfo &next = lineInfo.list[nextIndex];
                    next.pos = tb_rect[line].topLeft();
                    next.size.setHeight(pick_perp(where, get_first_item_sh(next.item->widget()->layout())) + tb_fill);
                    next.offset = QPoint();
                    if (where == LEFT || where == RIGHT)
			info.pos = QPoint(tb_rect[line].left(), next.pos.y() + next.size.height());
                    else
			info.pos = QPoint(next.pos.x() + next.size.width(), tb_rect[line].top());
                    info.offset = QPoint(); // has to be done before swapping
                    lineInfo.list.swap(i, nextIndex);
                } else {
		    info.pos = tb_rect[line].topLeft();
		    info.offset = QPoint();
		}
	    } else {
                int prevIndex = prevVisible(i, lineInfo);
                Q_ASSERT_X(prevIndex != -1, "QMainWindowLayout", "internal error");

                ToolBarLayoutInfo &prev = lineInfo.list[prevIndex];
		QSize min_size = info.item->widget()->minimumSize();
                int cur_pt = info.size.isEmpty()
                             ? (pick_perp(where, prev.pos) + pick_perp(where, get_real_sh(prev.item->widget()->layout())))
                             : (info.user_pos.isNull()
                                ? (pick_perp(where, prev.pos) + pick_perp(where, get_real_sh(prev.item->widget()->layout())))
                                : pick_perp(where, info.user_pos));
                cur_pt = qMax(cur_pt, 0);
                const int prev_min = pick_perp(where, prev.item->widget()->minimumSize());
                const int snap_dist = 12;

                info.pos = tb_rect[line].topLeft();
                set_perp(where, info.pos, cur_pt);

		if (pick_perp(where, info.offset) < 0) { // left/up motion
                    int to_shave = pick_perp(where, -info.offset);
                    int can_shave = qMin(to_shave,
                                         pick_perp(where, prev.size) - prev_min);
		    if (can_shave > 0) {
                        // shrink the previous one and increase size of current with same
                        QSize real_sh = get_real_sh(prev.item->widget()->layout());
                        QSize sz(0, 0);
			set_perp(where, sz, can_shave);
                        if ((pick_perp(where, prev.size) + pick_perp(where, sz)
                             < pick_perp(where, real_sh) - snap_dist)
                            || (pick_perp(where, prev.size) + pick_perp(where, sz)
                                > pick_perp(where, real_sh) + snap_dist))
                        {
                            prev.size += sz;
                            info.size -= sz;
                            set_perp(where, info.pos, cur_pt - can_shave);
                            info.user_pos = info.pos;

                            to_shave -= can_shave;
                        } else {
                            info.user_pos = QPoint();

                            info.pos = tb_rect[line].topLeft();
                            set_perp(where, prev.size, pick_perp(where, real_sh));
                            int pt = pick_perp(where, prev.pos) + pick_perp(where, prev.size);
                            set_perp(where, info.pos, pt);

                            // when snapping, don't push anything
                            to_shave = 0;
                        }
                    }
                    if (to_shave > 0) {
			// can't shrink - push the previous one if possible
			bool pushed = false;
			for (int l = i-2; to_shave > 0 && l >= 0; --l) {
			    ToolBarLayoutInfo &t = lineInfo.list[l];
                            can_shave = qMin(to_shave,
                                             pick_perp(where, t.size)
                                             - pick_perp(where, get_first_item_sh(t.item->widget()->layout()))
                                             - tb_fill);
			    if (can_shave > 0) {
				pushed = true;

				QSize sz(0, 0);
				set_perp(where, sz, can_shave);
                                t.size -= sz;
                                to_shave -= can_shave;

                                for (int j = l+1; j < i; ++j) {
                                    set_perp(where, lineInfo.list[j].pos,
                                             pick_perp(where, lineInfo.list[j-1].pos)
                                             + pick_perp(where, lineInfo.list[j-1].size));
                                    lineInfo.list[j].user_pos = lineInfo.list[j].pos;
                                }
			    }
			}
			if (!pushed) {
			    if (to_shave > prev_min) {
                                // this is not a ugle hack
                                QLayoutItem *tmp = info.item;
                                info.item = prev.item;
                                prev.item = tmp;
                                info.item->widget()->update();
                                prev.item->widget()->update();
                            }
			}
		    }

		} else if (pick_perp(where, info.offset) > 0) { // right/down motion
                    int to_shave = pick_perp(where, info.offset);
                    int can_shave = qMin(to_shave,
                                         pick_perp(where, info.size) - pick_perp(where, min_size));
		    if (can_shave > 0) {
                        QSize real_sh = get_real_sh(prev.item->widget()->layout());
			QSize sz(0, 0);
			set_perp(where, sz, pick_perp(where, info.offset));
                        if ((pick_perp(where, prev.size) + pick_perp(where, sz)
                             > pick_perp(where, real_sh) + snap_dist)
                            || (pick_perp(where, prev.size) < pick_perp(where, real_sh) - snap_dist))
                        {
                            info.size -= sz;
                            set_perp(where, info.pos, cur_pt + can_shave);
                            info.user_pos = info.pos;

                            to_shave -= can_shave;
                        } else {
                            info.user_pos = QPoint();
                            info.pos = tb_rect[line].topLeft();
                            set_perp(where, prev.size, pick_perp(where, real_sh));
                            int pt = pick_perp(where, prev.pos) + pick_perp(where, prev.size);
                            set_perp(where, info.pos, pt);

                            // when snapping, don't push anything
                            to_shave = 0;
                        }
		    }
                    if (to_shave > 0) {
			bool pushed = false;
			for (int l = i+1; to_shave > 0 && l < num_tbs; ++l) {
			    ToolBarLayoutInfo &t = lineInfo.list[l];
                            can_shave = qMin(to_shave,
                                             pick_perp(where, t.size)
                                             - pick_perp(where, get_first_item_sh(t.item->widget()->layout()))
                                             - tb_fill);
			    if (can_shave > 0) {
                                pushed = true;

				QPoint pt;
				set_perp(where, pt, can_shave);
				t.pos += pt;
                                t.user_pos = t.pos;
				t.size -= QSize(pt.x(), pt.y());
                                to_shave -= can_shave;

                                for (int j = l - 1; j > i; --j) {
                                    set_perp(where, lineInfo.list[j].pos,
                                             pick_perp(where, lineInfo.list[j].pos) + can_shave);
                                    lineInfo.list[j].user_pos = lineInfo.list[j].pos;
                                }
			    }
			}
			if (!pushed) {
		            int nextIndex = nextVisible(i, lineInfo);
			    if (nextIndex != -1) {
                                ToolBarLayoutInfo &t = lineInfo.list[nextIndex];
                                if (pick_perp(where, info.offset) > pick_perp(where, info.size)) {
                                    // this is not a ugle hack
                                    QLayoutItem *tmp = info.item;
                                    info.item = t.item;
                                    t.item = tmp;
                                    info.item->widget()->update();
                                     prev.item->widget()->update();
                                }
			    }
			}
		    }
		}

		// fix this item's position
		if (pick_perp(where, info.pos) < pick_perp(where, prev.pos) + prev_min) {
                    if (info.user_pos.isNull() && prev.user_pos.isNull()) {
                        int sz = pick_perp(where, get_real_sh(prev.item->widget()->layout()));
                        // don't go beyond min size hint
                        if (sz < prev_min)
                            sz = prev_min;
                        set_perp(where, info.pos, pick_perp(where, prev.pos) + sz);
                    } else {
                        set_perp(where, info.pos, pick_perp(where, prev.pos) + prev_min);
                    }
                }
		info.offset = QPoint();
	    }

	    // size
	    if (num_tbs == 1) {
		set_perp(where, info.size, pick_perp(where, tb_rect[line].size()));
            } else {
                int nextIndex = nextVisible(i, lineInfo);
                if (nextIndex == -1) {
                    set_perp(where, info.size, pick_perp(where, tb_rect[line].size()));
                    if (where == LEFT || where == RIGHT)
                        info.size.setHeight(tb_rect[line].bottom() - info.pos.y() + 1);
                    else
                        info.size.setWidth(tb_rect[line].right() - info.pos.x() + 1);
                    if (pick_perp(where, info.size) < 1)
                        set_perp(where, info.size, pick_perp(where, get_first_item_sh(info.item->widget()->layout())) + tb_fill);
                }
            }

	    if (i > 0) {
		// assumes that all tbs are positioned at the correct
		// pos - fill in the gaps btw them
                int prevIndex = prevVisible(i, lineInfo);
                if (prevIndex != -1) {
                    ToolBarLayoutInfo &prev = lineInfo.list[prevIndex];
                    set_perp(where, prev.size, pick_perp(where, info.pos) - pick_perp(where, prev.pos));
                }
            }
	}

        if (num_tbs > 1) {
            const ToolBarLayoutInfo &last = lineInfo.list.last();
            int target_size = pick_perp(where, tb_rect[line].size());
            int shave_off = (pick_perp(where, last.pos) - pick_perp(where, tb_rect[line].topLeft()))
                            + pick_perp(where, get_first_item_sh(last.item->widget()->layout()))
                            + tb_fill - target_size;
            for (int i = num_tbs-2; shave_off > 0 && i >= 0; --i) {
                ToolBarLayoutInfo &info = lineInfo.list[i];
                int can_shave = qMin(pick_perp(where, info.size)
                                     - (pick_perp(where, get_first_item_sh(info.item->widget()->layout())) + tb_fill),
                                     shave_off);
                if (can_shave > 0) {
                    // shave size off this item
                    QSize sz(0, 0);
                    set_perp(where, sz, can_shave);
                    info.size -= sz;
                    shave_off -= can_shave;
                    // move the next item by the amount we shaved
                    int nextIndex = nextVisible(i, lineInfo);
                    while (nextIndex != -1) {
                        QPoint p(0,0);
                        set_perp(where, p, can_shave);
                        lineInfo.list[nextIndex].pos -= p;

                        nextIndex = nextVisible(nextIndex, lineInfo);
                    }
                }
            }
        }

        for (int i = 0; i < num_tbs; ++i) {
	    ToolBarLayoutInfo &info = lineInfo.list[i];
            if (info.item->isEmpty()) {
                info.size = QSize();
                continue;
            }

	    QRect tb(info.pos, info.size);
            tb = QStyle::visualRect(QApplication::layoutDirection(), tb_rect[line], tb);
	    if (!tb.isEmpty())
		info.item->setGeometry(tb);
	}
    }
#endif // QT_NO_TOOLBAR

#ifndef QT_NO_DOCKWIDGET
    dockWidgetLayout.rect = r;
    dockWidgetLayout.fitLayout();
    applyDockWidgetLayout(dockWidgetLayout, savedDockWidgetLayout.isValid());
//    dump(qDebug() << "QMainWindowLayout::setGeometry()", dockWidgetLayout);
#else
    if (centralWidgetItem != 0)
        centralWidgetItem->setGeometry(r);
#endif
}

void QMainWindowLayout::addItem(QLayoutItem *)
{ qWarning("QMainWindowLayout::addItem: Please use the public QMainWindow API instead"); }

QSize QMainWindowLayout::sizeHint() const
{
    if (!szHint.isValid()) {
        int left = 0, right = 0, top = 0, bottom = 0;

#ifndef QT_NO_TOOLBAR
        // layout toolbars
        for (int line = 0; line < tb_layout_info.size(); ++line) {
            const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
            QSize sz;
            // need to get the biggest size hint for all tool bars on each line
            for (int i = 0; i < lineInfo.list.size(); ++i) {
                const ToolBarLayoutInfo &info = lineInfo.list.at(i);
                QSize ms = info.item->sizeHint();

                if (((lineInfo.pos == LEFT || lineInfo.pos == RIGHT) && (ms.width() > sz.width()))
                    || (ms.height() > sz.height()))
                    sz = ms;
            }
            switch (lineInfo.pos) {
            case LEFT:
                left += sz.width();
                break;

            case RIGHT:
                right += sz.width();
                break;

            case TOP:
                top += sz.height();
                break;

            case BOTTOM:
                bottom += sz.height();
                break;

            default:
                Q_ASSERT_X(false, "QMainWindowLayout", "internal error");
            }
        }
#endif // QT_NO_TOOLBAR

        QSize szDW(0, 0);
#ifndef QT_NO_DOCKWIDGET
        szDW = dockWidgetLayout.sizeHint();
#else
        if (centralWidgetItem != 0)
            szDW = centralWidgetItem->sizeHint();
#endif
        const QSize szSB = statusbar ? statusbar->sizeHint() : QSize(0, 0);
        szHint = QSize(qMax(szSB.width(), szDW.width() + left + right),
                       szSB.height() + szDW.height() + top + bottom);
    }
    return szHint;
}

QSize QMainWindowLayout::minimumSize() const
{
    if (!minSize.isValid()) {
        int left = 0, right = 0, top = 0, bottom = 0;

#ifndef QT_NO_TOOLBAR
        // layout toolbars
        for (int line = 0; line < tb_layout_info.size(); ++line) {
            const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
            QSize sz;
            // need to get the biggest min size for all tool bars on each line
            for (int i = 0; i < lineInfo.list.size(); ++i) {
                const ToolBarLayoutInfo &info = lineInfo.list.at(i);
                QSize ms = info.item->minimumSize();

                if (((lineInfo.pos == LEFT || lineInfo.pos == RIGHT) && (ms.width() > sz.width()))
                    || (ms.height() > sz.height()))
                    sz = ms;
            }
            switch (lineInfo.pos) {
            case LEFT:
                left += sz.width();
                break;

            case RIGHT:
                right += sz.width();
                break;

            case TOP:
                top += sz.height();
                break;

            case BOTTOM:
                bottom += sz.height();
                break;

            default:
                Q_ASSERT_X(false, "QMainWindowLayout", "internal error");
            }
        }
#endif // QT_NO_TOOLBAR

        QSize szDW;
#ifndef QT_NO_DOCKWIDGET
        szDW = dockWidgetLayout.minimumSize();
#else
        if (centralWidgetItem != 0)
            szDW = centralWidgetItem->minimumSize();
#endif
        const QSize szSB = statusbar ? statusbar->minimumSize() : QSize(0, 0);
        minSize =  QSize(qMax(szSB.width(), szDW.width() + left + right),
                         szSB.height() + szDW.height() + top + bottom);
    }
    return minSize;
}

void QMainWindowLayout::relayout()
{
    const QRect g = geometry();
    if (g.isValid())
        setGeometry(g);
    else
        update();
}

void QMainWindowLayout::invalidate()
{
    QLayout::invalidate();
    minSize = szHint = QSize();
}

#ifndef QT_NO_DOCKWIDGET
void QMainWindowLayout::applyDockWidgetLayout(QDockWidgetLayout &newLayout, bool animate)
{
#ifndef QT_NO_TABBAR
    QSet<QTabBar*> used = newLayout.usedTabBars();
    QSet<QTabBar*> retired = usedTabBars - used;
    usedTabBars = used;
    foreach (QTabBar *tab_bar, retired) {
        tab_bar->hide();
        while (tab_bar->count() > 0)
            tab_bar->removeTab(0);
        unusedTabBars.append(tab_bar);
    }
#endif // QT_NO_TABBAR

    newLayout.apply(animationEnabled && animate);
}

#ifndef QT_NO_TABBAR

class QMainWindowTabBar : public QTabBar
{
public:
    QMainWindowTabBar(QWidget *parent)
        : QTabBar(parent) {}
protected:
    bool event(QEvent *e);
};

bool QMainWindowTabBar::event(QEvent *e)
{
    if (e->type() != QEvent::ToolTip)
        return QTabBar::event(e);
    if (size().width() < sizeHint().width())
        return QTabBar::event(e);
    e->accept();
    return true;
}

QTabBar *QMainWindowLayout::getTabBar()
{
    QTabBar *result = 0;
    if (!unusedTabBars.isEmpty()) {
        result = unusedTabBars.takeLast();
    } else {
        result = new QMainWindowTabBar(parentWidget());
        result->setShape(QTabBar::RoundedSouth);
        result->setDrawBase(true);
        result->setElideMode(Qt::ElideRight);
        connect(result, SIGNAL(currentChanged(int)), this, SLOT(tabChanged()));
    }

    usedTabBars.insert(result);
    return result;
}

void QMainWindowLayout::tabChanged()
{
    QTabBar *tb = qobject_cast<QTabBar*>(sender());
    if (tb == 0)
        return;
    QDockAreaLayoutInfo *info = dockWidgetLayout.info(tb);
    if (info == 0)
        return;
    info->apply(false);
}
#endif // QT_NO_TABBAR

void QMainWindowLayout::keepSize(QDockWidget *w)
{
    dockWidgetLayout.keepSize(w);
}

QWidgetItem *QMainWindowLayout::unplug(QDockWidget *dockWidget)
{
    QList<int> pathToDockWidget = dockWidgetLayout.indexOf(dockWidget);
    if (pathToDockWidget.isEmpty()) {
        // the dock widget is already floating
        pathToDockWidget = dockWidgetLayout.indexOf(dockWidget, IndexOfFindsInvisible);
        if (pathToDockWidget.isEmpty())
            return 0;
        return dockWidgetLayout.item(pathToDockWidget).widgetItem;
    }

    QRect r = dockWidgetLayout.itemRect(pathToDockWidget);
    savedDockWidgetLayout = dockWidgetLayout;
    dockWidget->d_func()->unplug(r);
    savedDockWidgetLayout.fitLayout();

    QWidgetItem *dockWidgetItem = dockWidgetLayout.convertToGap(pathToDockWidget);
    currentGapPos = pathToDockWidget;
    currentGapRect = r;
    updateGapIndicator();

//    dump(qDebug() << "QMainWindowLayout::unplug()", savedDockWidgetLayout);

    return dockWidgetItem;
}

void QMainWindowLayout::updateGapIndicator()
{
    if (widgetAnimator->animating() || currentGapPos.isEmpty()) {
        gapIndicator->hide();
    } else {
        if (gapIndicator->geometry() != currentGapRect)
            gapIndicator->setGeometry(currentGapRect);
        if (!gapIndicator->isVisible())
            gapIndicator->show();
    }
}

QList<int> QMainWindowLayout::hover(QWidgetItem *dockWidgetItem, const QPoint &mousePos)
{
    if (pluggingWidget != 0)
        return QList<int>();

    QDockWidget *dockWidget = qobject_cast<QDockWidget*>(dockWidgetItem->widget());
    Q_ASSERT(dockWidget != 0);

    QPoint pos = parentWidget()->mapFromGlobal(mousePos);

    if (!savedDockWidgetLayout.isValid())
        savedDockWidgetLayout = dockWidgetLayout;

    QList<int> pathToGap = savedDockWidgetLayout.gapIndex(pos, dockNestingEnabled);

    if (!pathToGap.isEmpty()) {
        Qt::DockWidgetArea area
            = static_cast<Qt::DockWidgetArea>(areaForPosition(pathToGap.first()));
        if (!dockWidget->isAreaAllowed(area))
            pathToGap.clear();
    }

    if (pathToGap == currentGapPos)
        return currentGapPos; // the gap is already there

    currentGapPos = pathToGap;
    if (pathToGap.isEmpty()) {
        restore();
        return QList<int>();
    }

    QDockWidgetLayout newLayout = savedDockWidgetLayout;

    if (!newLayout.insertGap(pathToGap, dockWidgetItem)) {
        restore(); // not enough space
        return QList<int>();
    }
    QSize min = newLayout.minimumSize();
    QSize size = newLayout.rect.size();
    newLayout.fitLayout();

    if (min.width() > size.width() || min.height() > size.height()) {
        restore();
        return QList<int>();
    }

    currentGapRect = newLayout.gapRect(currentGapPos);

    parentWidget()->update(dockWidgetLayout.separatorRegion());
    dockWidgetLayout = newLayout;
    applyDockWidgetLayout(dockWidgetLayout);

    updateGapIndicator();

    return pathToGap;
}


void QMainWindowLayout::plug(QWidgetItem *dockWidgetItem, const QList<int> &pathToGap)
{
    QDockWidget *dockWidget = qobject_cast<QDockWidget *>(dockWidgetItem->widget());
    Q_ASSERT(dockWidget != 0);

    QList<int> previousPath = dockWidgetLayout.indexOf(dockWidget, IndexOfFindsInvisible);
    dockWidgetLayout.convertToWidget(pathToGap, dockWidgetItem);
    if (!previousPath.isEmpty())
        dockWidgetLayout.remove(previousPath);

    if (animationEnabled) {
        pluggingWidget = dockWidget;
        QRect globalRect = currentGapRect;
        globalRect.moveTopLeft(parentWidget()->mapToGlobal(globalRect.topLeft()));
        widgetAnimator->animate(dockWidget, globalRect, animationEnabled);
    } else {
        dockWidget->d_func()->plug(currentGapRect);
        applyDockWidgetLayout(dockWidgetLayout);
        savedDockWidgetLayout.clear();
        currentGapPos.clear();
        parentWidget()->update(dockWidgetLayout.separatorRegion());
        updateGapIndicator();
//        dump(qDebug() << "QMainWindowLayout::plug()", dockWidgetLayout);
    }
}

void QMainWindowLayout::allAnimationsFinished()
{
    parentWidget()->update(dockWidgetLayout.separatorRegion());

#ifndef QT_NO_TABBAR
    foreach (QTabBar *tab_bar, usedTabBars)
        tab_bar->show();
#endif

    updateGapIndicator();
}

void QMainWindowLayout::animationFinished(QWidget *widget)
{
    if (widget != pluggingWidget)
        return;

    pluggingWidget->d_func()->plug(currentGapRect);

    applyDockWidgetLayout(dockWidgetLayout, false);
#ifndef QT_NO_TABBAR
    QDockAreaLayoutInfo *info = dockWidgetLayout.info(widget);
    Q_ASSERT(info != 0);
    info->setCurrentTab(widget);
#endif
    savedDockWidgetLayout.clear();

    currentGapPos.clear();
    pluggingWidget = 0;

    updateGapIndicator();
}

void QMainWindowLayout::restore()
{
    if (!savedDockWidgetLayout.isValid())
        return;

    dockWidgetLayout = savedDockWidgetLayout;
    applyDockWidgetLayout(dockWidgetLayout);
    savedDockWidgetLayout.clear();
    currentGapPos.clear();
    updateGapIndicator();

//    dump(qDebug() << "QMainWindowLayout::restore()", dockWidgetLayout);
}

bool QMainWindowLayout::startSeparatorMove(const QPoint &pos)
{
    movingSeparator = dockWidgetLayout.findSeparator(pos);
    // qDebug() << "QMainWindowLayout::startSeparatorMove()" << movingSeparator;

    if (movingSeparator.isEmpty())
        return false;

    savedDockWidgetLayout = dockWidgetLayout;
    movingSeparatorPos = movingSeparatorOrigin = pos;

    return true;
}

bool QMainWindowLayout::separatorMove(const QPoint &pos)
{
    if (movingSeparator.isEmpty())
        return false;
    movingSeparatorPos = pos;
    separatorMoveTimer->start();
    return true;
}

void QMainWindowLayout::doSeparatorMove()
{
    if (movingSeparator.isEmpty())
        return;
    if (movingSeparatorOrigin == movingSeparatorPos)
        return;

    dockWidgetLayout = savedDockWidgetLayout;
    dockWidgetLayout.separatorMove(movingSeparator, movingSeparatorOrigin,
                                                movingSeparatorPos,
                                                &separatorMoveCache);
    movingSeparatorPos = movingSeparatorOrigin;
}

bool QMainWindowLayout::endSeparatorMove(const QPoint&)
{
    bool result = !movingSeparator.isEmpty();
    movingSeparator.clear();
    savedDockWidgetLayout.clear();
    separatorMoveCache.clear();
    return result;
}

void QMainWindowLayout::raise(QDockWidget *widget)
{
    QDockAreaLayoutInfo *info = dockWidgetLayout.info(widget);
    if (info == 0)
        return;
#ifndef QT_NO_TABBAR
    if (!info->tabbed)
        return;
    info->setCurrentTab(widget);
#endif
}

#endif // QT_NO_DOCKWIDGET

bool QMainWindowLayout::contains(QWidget *widget) const
{
#ifndef QT_NO_TOOLBAR
    // is it a toolbar?
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
	for (int i = 0; i < lineInfo.list.size(); ++i) {
	    const ToolBarLayoutInfo &info = lineInfo.list.at(i);
	    if (info.item->widget() == widget)
                return true;
        }
    }
#endif

#ifndef QT_NO_DOCKWIDGET
    // is it a dock widget?
    if (QDockWidget *dockWidget = qobject_cast<QDockWidget *>(widget)) {
        if (!dockWidgetLayout.indexOf(dockWidget, IndexOfFindsAll).isEmpty())
            return true;
    }
#endif //QT_NO_DOCKWIDGET

    return false;
}

#ifndef QT_NO_TOOLBAR
int QMainWindowLayout::locateToolBar(QToolBar *toolbar, const QPoint &mouse) const
{
    const int width = parentWidget()->width(),
             height = parentWidget()->height();
    const QPoint p = parentWidget()->mapFromGlobal(mouse),
                p2 = QPoint(width / 2, height / 2);
    const int dx = qAbs(p.x() - p2.x()),
              dy = qAbs(p.y() - p2.y());

    POSITION pos = positionForArea(toolBarArea(toolbar));
    if (dx > dy) {
        if (p.x() < p2.x() && toolbar->isAreaAllowed(Qt::LeftToolBarArea)) {
            pos = LEFT;
        } else if (p.x() >= p2.x() && toolbar->isAreaAllowed(Qt::RightToolBarArea)) {
            pos = RIGHT;
        } else {
            if (p.y() < p2.y() && toolbar->isAreaAllowed(Qt::TopToolBarArea))
                pos = TOP;
            else if (p.y() >= p2.y() && toolbar->isAreaAllowed(Qt::BottomToolBarArea))
                pos = BOTTOM;
        }
    } else {
        if (p.y() < p2.y() && toolbar->isAreaAllowed(Qt::TopToolBarArea)) {
            pos = TOP;
        } else if (p.y() >= p2.y() && toolbar->isAreaAllowed(Qt::BottomToolBarArea)) {
            pos = BOTTOM;
        } else {
            if (p.x() < p2.x() && toolbar->isAreaAllowed(Qt::LeftToolBarArea))
                pos = LEFT;
            else if (p.x() >= p2.x() && toolbar->isAreaAllowed(Qt::RightToolBarArea))
                pos = RIGHT;
        }
    }

    for (int line = 0; line < tb_layout_info.size(); ++line) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(line);
        if (!toolbar->isAreaAllowed(static_cast<Qt::ToolBarArea>(areaForPosition(lineInfo.pos))))
            continue;
	bool break_it = false;
        for (int i = 0; i < lineInfo.list.size(); ++i) {
	    const ToolBarLayoutInfo &info = lineInfo.list.at(i);
	    if (!info.item)
                continue;
 	    if (info.item->isEmpty())
                continue;
            if (!info.item->geometry().contains(p))
                continue;
	    pos = static_cast<POSITION>(lineInfo.pos);
	    break_it = true;
	    break;
	}
	if (break_it)
	    break;
    }
    return pos;
}

bool QMainWindowLayout::dropToolBar(QToolBar *toolbar, const QPoint &mouse, const QPoint &offset)
{
    bool toolBarPositionSwapped = false;
    POSITION where = static_cast<POSITION>(locateToolBar(toolbar, mouse));

    if (positionForArea(toolBarArea(toolbar)) == where) {
#ifdef TOOLBAR_DEBUG
	TBDEBUG() << "###";
	for (int l = 0; l < tb_layout_info.size(); ++l) {
            const ToolBarLineInfo &lineInfo = tb_layout_info.at(l);
	    for (int i = 0; i < lineInfo.list.size(); ++i) {
		const ToolBarLayoutInfo &tmp = lineInfo.list.at(i);
 		// qDebug() << "bar: " << lineInfo.pos << l << i << tmp.item->widget() << tmp.item->widget()->geometry();
	    }
	}
#endif
	// move the toolbars more than magic_offset pixels past a boundary
	// in either dir in order to move it to a different tb line
	const int magic_offset = 5;
	int l = 0, i = 0;
	ToolBarLayoutInfo info;

	for (l = 0; l < tb_layout_info.size(); ++l) {
            ToolBarLineInfo &lineInfo = tb_layout_info[l];
	    bool break_it = false;
	    for (i = 0; i < lineInfo.list.size(); ++i) {
                ToolBarLayoutInfo &tmp = lineInfo.list[i];
		if (tmp.item->widget() == toolbar) {
		    info = tmp;
 		    tmp.offset += offset;
		    break_it = true;
		    break;
		}
	    }
	    if (break_it)
                break;
	}

	if (pick(where, offset) < -magic_offset) { // move left/up
	    toolBarPositionSwapped = true;
        TBDEBUG() << "left/up" << offset << "line: " << l << where;
	    if (l > 0 && tb_layout_info.at(l-1).pos == where) { // is this the first line in this tb area?
                tb_layout_info[l].list.removeAt(i);
                if (tb_layout_info[l].list.size() == 0)
                    tb_layout_info.removeAt(l);
                if (tb_layout_info.at(l-1).pos == where) {
                    TBDEBUG() << "1. appending to existing" << info.item->widget() << info.item->widget()->geometry();
                    tb_layout_info[l-1].list.append(info);

                } else {
                    ToolBarLineInfo line;
                    line.pos = where;
                    line.list.append(info);
                    tb_layout_info.insert(l, line);
                    TBDEBUG() << "2. inserting new";
                }
            } else if ((tb_layout_info.at(l).list.size() > 1)
                       && ((l > 0 && tb_layout_info.at(l-1).pos == where)
                           || where == BOTTOM || where == RIGHT))
            {
                tb_layout_info[l].list.removeAt(i);
                ToolBarLineInfo line;
                line.pos = where;
                line.list.append(info);
                tb_layout_info.insert(l, line);
                TBDEBUG() << "3. inserting new" << l << toolbar;
            }
        } else if (pick(where, offset) > pick(where, info.size) + magic_offset) { // move right/down
            toolBarPositionSwapped = true;
            TBDEBUG() << "right/down" << offset << "line: " << l;
            if (l < tb_layout_info.size()-1 && tb_layout_info.at(l+1).pos == where) {
	            tb_layout_info[l].list.removeAt(i);
                if (tb_layout_info.at(l).list.size() == 0)
                    tb_layout_info.removeAt(l--);
                if (tb_layout_info.at(l+1).pos == where) {
                    tb_layout_info[l+1].list.append(info);
                    TBDEBUG() << "1. appending to exisitng";
                } else {
                    ToolBarLineInfo line;
                    line.pos = where;
                    line.list.append(info);
                    tb_layout_info.insert(l, line);
                    TBDEBUG() << "2. inserting new line";
                }
            } else if ((tb_layout_info.at(l).list.size() > 1)
                       && ((l < tb_layout_info.size()-1 && tb_layout_info.at(l+1).pos == where)
                           || where == TOP || where == LEFT))
            {
                tb_layout_info[l].list.removeAt(i);
                ToolBarLineInfo line;
                line.pos = where;
                line.list.append(info);
                tb_layout_info.insert(l+1, line);
                TBDEBUG() << "3. inserting new line";
            }
        }
    } else {
        toolBarPositionSwapped = true;
        TBDEBUG() << "changed area";
        addToolBar(static_cast<Qt::ToolBarArea>(areaForPosition(where)), toolbar, false);
        return toolBarPositionSwapped;
    }
    relayout();
    return toolBarPositionSwapped;
}

void QMainWindowLayout::updateToolbarsInArea(Qt::ToolBarArea area)
{
    POSITION pos = positionForArea(area);
    for (int i = 0; i < tb_layout_info.size(); ++i) {
        const ToolBarLineInfo &lineInfo = tb_layout_info.at(i);
        if ( lineInfo.pos == pos ) {
            for (int j = 0; j < lineInfo.list.size(); ++j) {
                lineInfo.list.at(j).item->widget()->update();
            }
        }
    }
}

void QMainWindowLayout::removeToolBarInfo(QToolBar *toolbar)
{
    for (int line = 0; line < tb_layout_info.size(); ++line) {
        ToolBarLineInfo &lineInfo = tb_layout_info[line];
	for (int i = 0; i < lineInfo.list.size(); ++i) {
	    const ToolBarLayoutInfo &info = lineInfo.list.at(i);
	    if (info.item->widget() == toolbar) {
		delete info.item;
		lineInfo.list.removeAt(i);
		if (lineInfo.list.size() == 0)
		    tb_layout_info.removeAt(line);
		break;
	    }
	}
    }
}

int QMainWindowLayout::nextVisible(int index, const ToolBarLineInfo &lineInfo)
{
    for (++index; index < lineInfo.list.size(); ++index) {
        if (!lineInfo.list.at(index).item->isEmpty())
            break;
    }
    return (index >= 0 && index < lineInfo.list.size()) ? index : -1;
}

int QMainWindowLayout::prevVisible(int index, const ToolBarLineInfo &lineInfo)
{
    for (--index; index >= 0; --index) {
        if (!lineInfo.list.at(index).item->isEmpty())
            break;
    }
    return (index >= 0 && index < lineInfo.list.size()) ? index : -1;
}

#endif // QT_NO_TOOLBAR

#endif // QT_NO_MAINWINDOW
