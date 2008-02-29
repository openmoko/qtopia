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

#include "layout_p.h"
#include "qdesigner_widget_p.h"
#include "qdesigner_utils_p.h"
#include "qlayout_widget_p.h"
#include "spacer_widget_p.h"
#include "layoutdecoration.h"

#include <QtDesigner/QtDesigner>

#include <QtCore/qdebug.h>
#include <QtCore/QVector>

#include <QtGui/qevent.h>
#include <QtGui/QGridLayout>
#include <QtGui/QPainter>
#include <QtGui/QBitmap>
#include <QtGui/QSplitter>
#include <QtGui/QMainWindow>
#include <QtGui/QApplication>
#include <QtGui/QScrollArea>

namespace qdesigner_internal {

class FriendlyBoxLayout: public QBoxLayout
{
public:
    inline FriendlyBoxLayout(Direction d) : QBoxLayout(d) { Q_ASSERT(0); }

    friend void insert_into_box_layout(QBoxLayout *box, int index, QWidget *widget);
};

void add_to_box_layout(QBoxLayout *box, QWidget *widget)
{
    if (QLayoutWidget *layoutWidget = qobject_cast<QLayoutWidget*>(widget)) {
        QLayoutWidgetItem *item = new QLayoutWidgetItem(layoutWidget);
        item->addTo(box);
        box->addItem(item);
    } else {
        box->addWidget(widget);
    }
}

void insert_into_box_layout(QBoxLayout *box, int index, QWidget *widget)
{
    if (QLayoutWidget *layoutWidget = qobject_cast<QLayoutWidget*>(widget)) {
        QLayoutWidgetItem *item = new QLayoutWidgetItem(layoutWidget);
        item->addTo(box);
        static_cast<FriendlyBoxLayout*>(box)->insertItem(index, item);
    } else {
        box->insertWidget(index, widget);
    }
}

void add_to_grid_layout(QGridLayout *grid, QWidget *widget, int r, int c, int rs, int cs, Qt::Alignment align)
{
    if (QLayoutWidget *layoutWidget = qobject_cast<QLayoutWidget*>(widget)) {
        QLayoutWidgetItem *item = new QLayoutWidgetItem(layoutWidget);
        item->addTo(grid);
        grid->addItem(item, r, c, rs, cs, align);
    } else {
        grid->addWidget(widget, r, c, rs, cs, align);
    }
}


/*!
  \class Layout layout.h
  \brief Baseclass for layouting widgets in the Designer

  Classes derived from this abstract base class are used for layouting
  operations in the Designer.

*/

/*!  \a p specifies the parent of the layoutBase \a lb. The parent
  might be changed in setup(). If the layoutBase is a
  container, the parent and the layoutBase are the same. Also they
  always have to be a widget known to the designer (e.g. in the case
  of the tabwidget parent and layoutBase are the tabwidget and not the
  page which actually gets laid out. For actual usage the correct
  widget is found later by Layout.)
 */

Layout::Layout(const QList<QWidget*> &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, bool splitter)
    : m_widgets(wl), m_parentWidget(p), formWindow(fw), isBreak(false), useSplitter(splitter)
{
    layoutBase = lb;
    if (layoutBase)
        oldGeometry = layoutBase->geometry();
}

Layout::~Layout()
{
}

int Layout::margin() const
{
    if (layoutBase && layoutBase->layout())
        return layoutBase->layout()->margin();

    qWarning("unknown margin");
    return 0;
}

int Layout::spacing() const
{
    if (layoutBase && layoutBase->layout())
        return layoutBase->layout()->spacing();

    qWarning("unknown spacing");
    return 0;
}

/*!  The widget list we got in the constructor might contain too much
  widgets (like widgets with different parents, already laid out
  widgets, etc.). Here we set up the list and so the only the "best"
  widgets get laid out.
*/

void Layout::setup()
{
    startPoint = QPoint(32767, 32767);

    // Go through all widgets of the list we got. As we can only
    // layout widgets which have the same parent, we first do some
    // sorting which means create a list for each parent containing
    // its child here. After that we keep working on the list of
    // childs which has the most entries.
    // Widgets which are already laid out are thrown away here too

    QMultiMap<QWidget*, QWidget*> lists;
    foreach (QWidget *w, m_widgets) {
        QWidget *p = w->parentWidget();

        if (p && LayoutInfo::layoutType(formWindow->core(), p) != LayoutInfo::NoLayout
                && formWindow->core()->metaDataBase()->item(p->layout()) != 0)
            continue;

        lists.insert(p, w);
    }

    QList<QWidget*> lastList;
    QList<QWidget*> parents = lists.keys();
    foreach (QWidget *p, parents) {
        QList<QWidget*> children = lists.values(p);

        if (children.count() > lastList.count())
            lastList = children;
    }


    // If we found no list (because no widget did fit at all) or the
    // best list has only one entry and we do not layout a container,
    // we leave here.
    QDesignerWidgetDataBaseInterface *widgetDataBase = formWindow->core()->widgetDataBase();
    if (lastList.count() < 2 &&
                        (!layoutBase ||
                          (!widgetDataBase->isContainer(layoutBase, false) &&
                            layoutBase != formWindow->mainContainer()))
                       ) {
        m_widgets.clear();
        startPoint = QPoint(0, 0);
        return;
    }

    // Now we have a new and clean widget list, which makes sense
    // to layout
    m_widgets = lastList;
    // Also use the only correct parent later, so store it

    Q_ASSERT(m_widgets.isEmpty() == false);

    m_parentWidget = formWindow->core()->widgetFactory()->widgetOfContainer(m_widgets.first()->parentWidget());
    // Now calculate the position where the layout-meta-widget should
    // be placed and connect to widgetDestroyed() signals of the
    // widgets to get informed if one gets deleted to be able to
    // handle that and do not crash in this case
    foreach (QWidget *w, m_widgets) {
        connect(w, SIGNAL(destroyed()), this, SLOT(widgetDestroyed()));
        startPoint = QPoint(qMin(startPoint.x(), w->x()), qMin(startPoint.y(), w->y()));
        QRect rc(w->geometry());

        geometries.insert(w, rc);
        // Change the Z-order, as saving/loading uses the Z-order for
        // writing/creating widgets and this has to be the same as in
        // the layout. Else saving + loading will give different results
        w->raise();
    }

    sort();
}

void Layout::widgetDestroyed()
{
     if (sender() && sender()->isWidgetType()) {
         QWidget *w = static_cast<QWidget *>(sender());
         m_widgets.removeAt(m_widgets.indexOf(w));
         geometries.remove(w);
     }
}

bool Layout::prepareLayout(bool &needMove, bool &needReparent)
{
    foreach (QWidget *widget, m_widgets) {
        widget->raise();
    }

    needMove = !layoutBase;
    needReparent = needMove || qobject_cast<QLayoutWidget*>(layoutBase) || qobject_cast<QSplitter*>(layoutBase);

    QDesignerWidgetFactoryInterface *widgetFactory = formWindow->core()->widgetFactory();
    QDesignerMetaDataBaseInterface *metaDataBase = formWindow->core()->metaDataBase();

    if (layoutBase == 0) {
        QString baseWidgetClassName = QLatin1String("QLayoutWidget");

        if (useSplitter)
            baseWidgetClassName = QLatin1String("QSplitter");

        layoutBase = widgetFactory->createWidget(baseWidgetClassName, widgetFactory->containerOfWidget(m_parentWidget));
        if (useSplitter) {
            layoutBase->setObjectName("splitter");
            formWindow->ensureUniqueObjectName(layoutBase);
        }
    } else {
        LayoutInfo::deleteLayout(formWindow->core(), layoutBase);
    }

    metaDataBase->add(layoutBase);

    Q_ASSERT(layoutBase->layout() == 0 || metaDataBase->item(layoutBase->layout()) == 0);

    return true;
}

static bool isMainContainer(QDesignerFormWindowInterface *fw, const QWidget *w)
{
    return w && (w == fw || w == fw->mainContainer());
}

static bool isPageOfContainerWidget(QDesignerFormWindowInterface *fw, QWidget *widget)
{
    QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension*>(
            fw->core()->extensionManager(), widget->parentWidget());

    if (c != 0) {
        for (int i = 0; i<c->count(); ++i) {
            if (widget == c->widget(i))
                return true;
        }
    }

    return false;
}
void Layout::finishLayout(bool needMove, QLayout *layout)
{
    if (m_parentWidget == layoutBase) {
        QWidget *widget = layoutBase;
        oldGeometry = widget->geometry();

        bool done = false;
        while (!isMainContainer(formWindow, widget) && !done) {
            QDesignerContainerExtension *c = 0;
            c = qt_extension<QDesignerContainerExtension*>(formWindow->core()->extensionManager(),
                        widget->parentWidget());

            if (!formWindow->isManaged(widget)) {
                widget = widget->parentWidget();
                continue;
            } else if (LayoutInfo::isWidgetLaidout(formWindow->core(), widget)) {
                widget = widget->parentWidget();
                continue;
            } else if (isPageOfContainerWidget(formWindow, widget)) {
                widget = widget->parentWidget();
                continue;
            } else if (widget->parentWidget()) {
                QScrollArea *area = qobject_cast<QScrollArea*>(widget->parentWidget()->parentWidget());
                if (area && area->widget() == widget) {
                    widget = area;
                    continue;
                }
            }

            done = true;
        }

        QApplication::processEvents();
        // We don't want to resize the form window
        if (!Utils::isCentralWidget(formWindow, widget))
            widget->adjustSize();

        return;
    }

    if (needMove)
        layoutBase->move(startPoint);

    QRect g(layoutBase->pos(), layoutBase->size());

    if (LayoutInfo::layoutType(formWindow->core(), layoutBase->parentWidget()) == LayoutInfo::NoLayout && !isBreak)
        layoutBase->adjustSize();
    else if (isBreak)
        layoutBase->setGeometry(oldGeometry);

    oldGeometry = g;
    layout->invalidate();
    layoutBase->show();

    if (qobject_cast<QLayoutWidget*>(layoutBase) || qobject_cast<QSplitter*>(layoutBase)) {
        formWindow->manageWidget(layoutBase);
        formWindow->selectWidget(layoutBase);
    }
}

void Layout::undoLayout()
{
    if (!m_widgets.count())
        return;

    formWindow->selectWidget(layoutBase, false);

    QDesignerWidgetFactoryInterface *widgetFactory = formWindow->core()->widgetFactory();
    QHashIterator<QWidget *, QRect> it(geometries);
    while (it.hasNext()) {
        it.next();

        if (!it.key())
            continue;

        QWidget* w = it.key();
        QRect rc = it.value();

        bool showIt = w->isVisibleTo(formWindow);
        QWidget *container = widgetFactory->containerOfWidget(m_parentWidget);

        // ### remove widget here
        QWidget *parentWidget = w->parentWidget();
        QDesignerFormEditorInterface *core = formWindow->core();
        QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parentWidget);

        if (deco)
            deco->removeWidget(w);

        w->setParent(container);
        w->setGeometry(rc);

        if (showIt)
            w->show();
    }

    LayoutInfo::deleteLayout(formWindow->core(), layoutBase);

    if (m_parentWidget != layoutBase && !qobject_cast<QMainWindow*>(layoutBase)) {
        formWindow->unmanageWidget(layoutBase);
        layoutBase->hide();
    } else {
        QMainWindow *mw = qobject_cast<QMainWindow*>(formWindow->mainContainer());
        if (layoutBase != formWindow->mainContainer() &&
                    (!mw || mw->centralWidget() != layoutBase))
            layoutBase->setGeometry(oldGeometry);
    }

    QWidget *ww = m_widgets.size() ? m_widgets.front() : formWindow;
    formWindow->selectWidget(ww);
}

void Layout::breakLayout()
{
    QMap<QWidget*, QRect> rects;
    foreach (QWidget *w, m_widgets) {
        rects.insert(w, w->geometry());
    }

    QPoint layoutBasePos = layoutBase->pos();
    QDesignerWidgetDataBaseInterface *widgetDataBase = formWindow->core()->widgetDataBase();

    LayoutInfo::deleteLayout(formWindow->core(), layoutBase);

    bool needReparent = qobject_cast<QLayoutWidget*>(layoutBase) ||
                        qobject_cast<QSplitter*>(layoutBase)     ||
                        (!widgetDataBase->isContainer(layoutBase, false) &&
                          layoutBase != formWindow->mainContainer());
    bool needResize = qobject_cast<QSplitter*>(layoutBase);
    bool add = geometries.isEmpty();

    QMapIterator<QWidget*, QRect> it(rects);
    while (it.hasNext()) {
        it.next();

        QWidget *w = it.key();
        if (needReparent) {
            w->setParent(layoutBase->parentWidget(), 0);
            w->move(layoutBasePos + it.value().topLeft());
            w->show();
        }

        if (needResize)
            w->resize(it.value().size());

        if (add)
            geometries.insert(w, QRect(w->pos(), w->size()));
    }

    if (needReparent) {
        layoutBase->hide();
        m_parentWidget = layoutBase->parentWidget();
        formWindow->unmanageWidget(layoutBase);
    } else {
        m_parentWidget = layoutBase;
    }

    if (!m_widgets.isEmpty() && m_widgets.first() && m_widgets.first()->isVisibleTo(formWindow))
        formWindow->selectWidget(m_widgets.first());
    else
        formWindow->selectWidget(formWindow);
}

HorizontalLayout::HorizontalLayout(const QList<QWidget*> &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, bool splitter)
    : Layout(wl, p, fw, lb, splitter)
{
}

void HorizontalLayout::sort()
{
    HorizontalLayoutList l(m_widgets);
    l.sort();
    m_widgets = l;
}

void HorizontalLayout::doLayout()
{
    bool needMove, needReparent;
    if (!prepareLayout(needMove, needReparent))
        return;

    QDesignerWidgetFactoryInterface *widgetFactory = formWindow->core()->widgetFactory();
    QHBoxLayout *layout = (QHBoxLayout*) widgetFactory->createLayout(layoutBase, 0, LayoutInfo::HBox);

    foreach (QWidget *w, m_widgets) {
        if (needReparent && w->parent() != layoutBase) {
            w->setParent(layoutBase, 0);
            w->move(QPoint(0,0));
        }

        if (useSplitter) {
            QSplitter *splitter = qobject_cast<QSplitter*>(layoutBase);
            Q_ASSERT(splitter != 0);
            splitter->addWidget(w);
        } else {
            if (Spacer *spacer = qobject_cast<Spacer*>(w))
                layout->addWidget(w, 0, spacer->alignment());
            else
                add_to_box_layout(layout, w);
        }
        w->show();
    }

    if (QSplitter *splitter = qobject_cast<QSplitter*>(layoutBase))
        splitter->setOrientation(Qt::Horizontal);

    finishLayout(needMove, layout);
}

VerticalLayout::VerticalLayout(const QList<QWidget*> &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, bool splitter)
    : Layout(wl, p, fw, lb, splitter)
{
}

void VerticalLayout::sort()
{
    VerticalLayoutList l(m_widgets);
    l.sort();
    m_widgets = l;
}

void VerticalLayout::doLayout()
{
    bool needMove, needReparent;
    if (!prepareLayout(needMove, needReparent))
        return;

    QDesignerWidgetFactoryInterface *widgetFactory = formWindow->core()->widgetFactory();

    QVBoxLayout *layout = (QVBoxLayout*) widgetFactory->createLayout(layoutBase, 0, LayoutInfo::VBox);
    Q_ASSERT(layout != 0);

    foreach (QWidget *w, m_widgets) {
        if (needReparent && w->parent() != layoutBase) {
            w->setParent(layoutBase, 0);
            w->move(QPoint(0,0));
        }

        if (useSplitter) {
            QSplitter *splitter = qobject_cast<QSplitter*>(layoutBase);
            Q_ASSERT(splitter != 0);
            splitter->addWidget(w);
        } else {
            if (Spacer *spacer = qobject_cast<Spacer*>(w))
                layout->addWidget(w, 0, spacer->alignment());
            else
                add_to_box_layout(layout, w);
        }
        w->show();
    }

    if (QSplitter *splitter = qobject_cast<QSplitter*>(layoutBase)) { // ### useSplitter??
        splitter->setOrientation(Qt::Vertical);
    }

    finishLayout(needMove, layout);
}

class Grid
{
public:
    Grid(int rows, int cols);
    ~Grid();

    QWidget* cell(int row, int col) const { return cells[ row * ncols + col]; }
    void setCell(int row, int col, QWidget* w) { cells[ row*ncols + col] = w; }
    void setCells(QRect c, QWidget* w) {
        for (int rows = c.bottom()-c.top(); rows >= 0; rows--)
            for (int cols = c.right()-c.left(); cols >= 0; cols--) {
                setCell(c.top()+rows, c.left()+cols, w);
            }
    }
    int numRows() const { return nrows; }
    int numCols() const { return ncols; }

    void simplify();
    bool locateWidget(QWidget* w, int& row, int& col, int& rowspan, int& colspan);

private:
    void merge();
    int countRow(int r, int c) const;
    int countCol(int r, int c) const;
    void setRow(int r, int c, QWidget* w, int count);
    void setCol(int r, int c, QWidget* w, int count);
    bool isWidgetStartCol(int c) const;
    bool isWidgetEndCol(int c) const;
    bool isWidgetStartRow(int r) const;
    bool isWidgetEndRow(int r) const;
    bool isWidgetTopLeft(int r, int c) const;
    void extendLeft();
    void extendRight();
    void extendUp();
    void extendDown();
    QWidget** cells;
    bool* cols;
    bool* rows;
    int nrows, ncols;

};

Grid::Grid(int r, int c)
    : nrows(r), ncols(c)
{
    cells = new QWidget*[ r * c ];
    memset(cells, 0, sizeof(cells) * r * c);
    rows = new bool[ r ];
    cols = new bool[ c ];

}

Grid::~Grid()
{
    delete [] cells;
    delete [] cols;
    delete [] rows;
}

int Grid::countRow(int r, int c) const
{
    QWidget* w = cell(r, c);
    int i = c + 1;
    while (i < ncols && cell(r, i) == w)
        i++;
    return i - c;
}

int Grid::countCol(int r, int c) const
{
    QWidget* w = cell(r, c);
    int i = r + 1;
    while (i < nrows && cell(i, c) == w)
        i++;
    return i - r;
}

void Grid::setCol(int r, int c, QWidget* w, int count)
{
    for (int i = 0; i < count; i++)
        setCell(r + i, c, w);
}

void Grid::setRow(int r, int c, QWidget* w, int count)
{
    for (int i = 0; i < count; i++)
        setCell(r, c + i, w);
}

bool Grid::isWidgetStartCol(int c) const
{
    int r;
    for (r = 0; r < nrows; r++) {
        if (cell(r, c) && ((c==0) || (cell(r, c)  != cell(r, c-1)))) {
            return true;
        }
    }
    return false;
}

bool Grid::isWidgetEndCol(int c) const
{
    int r;
    for (r = 0; r < nrows; r++) {
        if (cell(r, c) && ((c == ncols-1) || (cell(r, c) != cell(r, c+1))))
            return true;
    }
    return false;
}

bool Grid::isWidgetStartRow(int r) const
{
    int c;
    for (c = 0; c < ncols; c++) {
        if (cell(r, c) && ((r==0) || (cell(r, c) != cell(r-1, c))))
            return true;
    }
    return false;
}

bool Grid::isWidgetEndRow(int r) const
{
    int c;
    for (c = 0; c < ncols; c++) {
        if (cell(r, c) && ((r == nrows-1) || (cell(r, c) != cell(r+1, c))))
            return true;
    }
    return false;
}


bool Grid::isWidgetTopLeft(int r, int c) const
{
    QWidget* w = cell(r, c);
    if (!w)
        return false;
    return (!r || cell(r-1, c) != w) && (!c || cell(r, c-1) != w);
}

void Grid::extendLeft()
{
    int r,c,i;
    for (c = 1; c < ncols; c++) {
        for (r = 0; r < nrows; r++) {
            QWidget* w = cell(r, c);
            if (!w)
                continue;

            int cc = countCol(r, c);
            int stretch = 0;
            for (i = c-1; i >= 0; i--) {
                if (cell(r, i))
                    break;
                if (countCol(r, i) < cc)
                    break;
                if (isWidgetEndCol(i))
                    break;
                if (isWidgetStartCol(i)) {
                    stretch = c - i;
                    break;
                }
            }
            if (stretch) {
                for (i = 0; i < stretch; i++)
                    setCol(r, c-i-1, w, cc);
            }
        }
    }
}


void Grid::extendRight()
{
    int r,c,i;
    for (c = ncols - 2; c >= 0; c--) {
        for (r = 0; r < nrows; r++) {
            QWidget* w = cell(r, c);
            if (!w)
                continue;
            int cc = countCol(r, c);
            int stretch = 0;
            for (i = c+1; i < ncols; i++) {
                if (cell(r, i))
                    break;
                if (countCol(r, i) < cc)
                    break;
                if (isWidgetStartCol(i))
                    break;
                if (isWidgetEndCol(i)) {
                    stretch = i - c;
                    break;
                }
            }
            if (stretch) {
                for (i = 0; i < stretch; i++)
                    setCol(r, c+i+1, w, cc);
            }
        }
    }

}

void Grid::extendUp()
{
    int r,c,i;
    for (r = 1; r < nrows; r++) {
        for (c = 0; c < ncols; c++) {
            QWidget* w = cell(r, c);
            if (!w)
                continue;
            int cr = countRow(r, c);
            int stretch = 0;
            for (i = r-1; i >= 0; i--) {
                if (cell(i, c))
                    break;
                if (countRow(i, c) < cr)
                    break;
                if (isWidgetEndRow(i))
                    break;
                if (isWidgetStartRow(i)) {
                    stretch = r - i;
                    break;
                }
            }
            if (stretch) {
                for (i = 0; i < stretch; i++)
                    setRow(r-i-1, c, w, cr);
            }
        }
    }
}

void Grid::extendDown()
{
    int r,c,i;
    for (r = nrows - 2; r >= 0; r--) {
        for (c = 0; c < ncols; c++) {
            QWidget* w = cell(r, c);
            if (!w)
                continue;
            int cr = countRow(r, c);
            int stretch = 0;
            for (i = r+1; i < nrows; i++) {
                if (cell(i, c))
                    break;
                if (countRow(i, c) < cr)
                    break;
                if (isWidgetStartRow(i))
                    break;
                if (isWidgetEndRow(i)) {
                    stretch = i - r;
                    break;
                }
            }
            if (stretch) {
                for (i = 0; i < stretch; i++)
                    setRow(r+i+1, c, w, cr);
            }
        }
    }

}

void Grid::simplify()
{
    extendLeft();
    extendRight();
    extendUp();
    extendDown();
    merge();
}


void Grid::merge()
{
    int r,c;
    for (c = 0; c < ncols; c++)
        cols[c] = false;

    for (r = 0; r < nrows; r++)
        rows[r] = false;

    for (c = 0; c < ncols; c++) {
        for (r = 0; r < nrows; r++) {
            if (isWidgetTopLeft(r, c)) {
                rows[r] = true;
                cols[c] = true;
            }
        }
    }
}

bool Grid::locateWidget(QWidget *w, int &row, int &col, int &rowspan, int &colspan)
{
    int r, c, r2, c2;

    for (c = 0; c < ncols; c++) {
        for (r = 0; r < nrows; r++) {
            if (cell(r, c) == w) {
                row = 0;
                for (r2 = 1; r2 <= r; r2++) {
                    if (rows[r2-1])
                        row++;
                }
                col = 0;
                for (c2 = 1; c2 <= c; c2++) {
                    if (cols[c2-1])
                        col++;
                }
                rowspan = 0;
                for (r2 = r ; r2 < nrows && cell(r2, c) == w; r2++) {
                    if (rows[r2])
                        rowspan++;
                }
                colspan = 0;
                for (c2 = c; c2 < ncols && cell(r, c2) == w; c2++) {
                    if (cols[c2])
                        colspan++;
                }
                return true;
            }
        }
    }
    return false;
}




GridLayout::GridLayout(const QList<QWidget*> &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, const QSize &res)
    : Layout(wl, p, fw, lb), resolution(res)
{
    grid = 0;
}

GridLayout::~GridLayout()
{
    delete grid;
}

QWidget *GridLayout::widgetAt(QGridLayout *layout, int row, int column) const
{
    int index = 0;
    while (QLayoutItem *item = layout->itemAt(index)) {
        if (item->widget()) {
            int r, c, rowspan, colspan;
            layout->getItemPosition(index, &r, &c, &rowspan, &colspan);
            if (row == r && column == c)
                return item->widget();
        }
        ++index;
    }
    return 0;
}

void GridLayout::doLayout()
{
    bool needMove, needReparent;
    if (!prepareLayout(needMove, needReparent))
        return;

    QDesignerWidgetFactoryInterface *ff = formWindow->core()->widgetFactory();
    QGridLayout *layout = static_cast<QGridLayout*>(ff->createLayout(layoutBase, 0, LayoutInfo::Grid));

    if (!grid)
        buildGrid();

    foreach (QWidget *w, m_widgets) {
        int r = 0, c = 0, rs = 0, cs = 0;

        if (grid->locateWidget(w, r, c, rs, cs)) {
            if (needReparent && w->parent() != layoutBase) {
                w->setParent(layoutBase, 0);
                w->move(QPoint(0,0));
            }

            Qt::Alignment alignment = Qt::Alignment(0);
            if (Spacer *spacer = qobject_cast<Spacer*>(w))
                alignment = spacer->alignment();

            if (rs * cs == 1) {
                add_to_grid_layout(layout, w, r, c, 1, 1, alignment);
            } else {
                add_to_grid_layout(layout, w, r, c, rs, cs, alignment);
            }

            w->show();
        } else {
            qWarning("ooops, widget '%s' does not fit in layout", w->objectName().toUtf8().constData());
        }
    }

    QLayoutSupport::createEmptyCells(layout);

    finishLayout(needMove, layout);
}

void GridLayout::sort()
{
    buildGrid();
}

void GridLayout::buildGrid()
{
    if (!m_widgets.count())
        return;
#if 0
    QMap<int, int> x_dict;
    QMap<int, int> y_dict;

    foreach (QWidget *w, m_widgets) {
        QRect g = w->geometry();

        x_dict.insert(g.left(), g.left());
        x_dict.insert(g.right(), g.right());

        y_dict.insert(g.top(), g.top());
        y_dict.insert(g.bottom(), g.bottom());
    }

    QList<int> x = x_dict.keys();
    QList<int> y = y_dict.keys();
#else
    // Pixel to cell conversion:
    // By keeping a list of start'n'stop values (x & y) for each widget,
    // it is possible to create a very small grid of cells to represent
    // the widget layout.
    // -----------------------------------------------------------------

    // We need a list of both start and stop values for x- & y-axis
    QVector<int> x( m_widgets.count()*2 );
    QVector<int> y( m_widgets.count()*2 );

    // Using push_back would look nicer, but operator[] is much faster
    int index  = 0;
    QWidget* w = 0;
    for (int i = 0; i < m_widgets.size(); ++i) {
        w = m_widgets.at(i);
        QRect widgetPos = w->geometry();
        x[index]   = widgetPos.left();
        x[index+1] = widgetPos.right();
        y[index]   = widgetPos.top();
        y[index+1] = widgetPos.bottom();
        index += 2;
    }

    qSort(x);
    qSort(y);

    // Remove duplicate x enteries (Remove next, if equal to current)
    if ( !x.empty() ) {
        for (QVector<int>::iterator current = x.begin() ;
             (current != x.end()) && ((current+1) != x.end()) ; )
            if ( (*current == *(current+1)) )
                x.erase(current+1);
            else
                current++;
    }

    // Remove duplicate y enteries (Remove next, if equal to current)
    if ( !y.empty() ) {
        for (QVector<int>::iterator current = y.begin() ;
             (current != y.end()) && ((current+1) != y.end()) ; )
            if ( (*current == *(current+1)) )
                y.erase(current+1);
            else
                current++;
    }
#endif

    delete grid;
    grid = new Grid(y.size() - 1, x.size() - 1);

    // Mark the cells in the grid that contains a widget
    foreach (QWidget *w, m_widgets) {
        QRect widgetPos = w->geometry();

        QRect c(0, 0, 0, 0);

        // From left til right (not including)
        for (int cw=0; cw<x.size(); cw++) {
            if (x[cw] == widgetPos.left())
                c.setLeft(cw);
            if (x[cw] <  widgetPos.right())
                c.setRight(cw);
        }

        // From top til bottom (not including)
        for (int ch=0; ch<y.size(); ch++) {
            if (y[ch] == widgetPos.top()   )
                c.setTop(ch);
            if (y[ch] <  widgetPos.bottom())
                c.setBottom(ch);
        }

        grid->setCells(c, w); // Mark cellblock
    }

    grid->simplify();
}


} // namespace qdesigner_internal
