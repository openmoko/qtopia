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

/*
TRANSLATOR qdesigner_internal::FormWindow
*/

#include "formwindow.h"
#include "formwindow_dnditem.h"
#include "formwindow_widgetstack.h"
#include "formwindowcursor.h"
#include "formwindowmanager.h"
#include "tool_widgeteditor.h"
#include "widgetselection.h"

// shared
#include <qdesigner_promotedwidget_p.h>
#include <qdesigner_tabwidget_p.h>
#include <qdesigner_toolbox_p.h>
#include <qdesigner_stackedbox_p.h>
#include "qdesigner_resource.h"
#include <qdesigner_command_p.h>
#include <qdesigner_widget_p.h>
#include <qdesigner_utils_p.h>
#include <qlayout_widget_p.h>
#include <spacer_widget_p.h>
#include <invisible_widget_p.h>
#include <layoutinfo_p.h>
#include <connectionedit_p.h>
#include <actionprovider_p.h>

// sdk
#include <QtDesigner/QtDesigner>

#include <QtGui>
#include <QtDebug>

namespace qdesigner_internal {

class FriendlyWidget: public QWidget
{
public:
    FriendlyWidget() { Q_ASSERT(0); }

    friend class FormWindow;
};

class BlockSelection
{
public:
    BlockSelection(FormWindow *fw)
        : m_formWindow(fw)
    {
        if (m_formWindow)
            m_blocked = m_formWindow->blockSelectionChanged(true);
    }

    ~BlockSelection()
    {
        if (m_formWindow)
            m_formWindow->blockSelectionChanged(m_blocked);
    }

private:
    QPointer<FormWindow> m_formWindow;
    bool m_blocked;
};

FormWindow::FormWindow(FormEditor *core, QWidget *parent, Qt::WindowFlags flags)
    : QDesignerFormWindowInterface(parent, flags),
      m_core(core), m_widgetStack(0)
{
    init();

    m_cursor = new FormWindowCursor(this, this);

    core->formWindowManager()->addFormWindow(this);

    setDirty(false);
}

FormWindow::~FormWindow()
{
    Q_ASSERT(core() != 0);
    Q_ASSERT(core()->metaDataBase() != 0);
    Q_ASSERT(core()->formWindowManager() != 0);

    core()->formWindowManager()->removeFormWindow(this);
    core()->metaDataBase()->remove(this);

    QList<QWidget*> l = widgets();
    foreach (QWidget *w, l)
        core()->metaDataBase()->remove(w);

    m_widgetStack = 0;
    m_rubberBand = 0;

    qDeleteAll(selections);
}

QDesignerFormEditorInterface *FormWindow::core() const
{
    return m_core;
}

QDesignerFormWindowCursorInterface *FormWindow::cursor() const
{
    return m_cursor;
}

bool FormWindow::hasFeature(Feature f) const
{
    return f & m_feature;
}

void FormWindow::updateWidgets()
{
    if (!m_mainContainer)
        return;
}

int FormWindow::widgetDepth(QWidget *w)
{
    int d = -1;
    while (w && !w->isWindow()) {
        d++;
        w = w->parentWidget();
    }

    return d;
}

bool FormWindow::isChildOf(QWidget *c, const QWidget *p)
{
    while (c /*&& !c->isWindow()*/) {
        if (c == p)
            return true;
        c = c->parentWidget();
    }
    return false;
}

FormWindow::Feature FormWindow::features() const
{
    return m_feature;
}

static void recursiveUpdate(QWidget *w)
{
    w->update();

    const QObjectList &l = w->children();
    QObjectList::const_iterator it = l.begin();
    for (; it != l.end(); ++it) {
        if (QWidget *w = qobject_cast<QWidget*>(*it))
            recursiveUpdate(w);
    }
}

void FormWindow::setFeatures(Feature f)
{
    m_feature = f;
    emit featureChanged(f);
    recursiveUpdate(this);
}

void FormWindow::setCursorToAll(const QCursor &c, QWidget *start)
{
    start->setCursor(c);
    QList<QWidget*> widgets = qFindChildren<QWidget*>(start);
    foreach (QWidget *widget, widgets) {
        if (!qobject_cast<WidgetHandle*>(widget)) {
            widget->setCursor(c);
        }
    }
}

void FormWindow::init()
{
    if (FormWindowManager *manager = qobject_cast<FormWindowManager*> (core()->formWindowManager())) {
        m_commandHistory = new QUndoStack(this);
        manager->undoGroup()->addStack(m_commandHistory);
    }

    m_blockSelectionChanged = false;

    m_defaultMargin = INT_MIN;
    m_defaultSpacing = INT_MIN;

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);

    m_widgetStack = new FormWindowWidgetStack(this);
    connect(m_widgetStack, SIGNAL(currentToolChanged(int)), this, SIGNAL(toolChanged(int)));
    layout->addWidget(m_widgetStack);

    m_feature = DefaultFeature;

    m_selectionChangedTimer = new QTimer(this);
    m_selectionChangedTimer->setSingleShot(true);
    connect(m_selectionChangedTimer, SIGNAL(timeout()), this, SLOT(selectionChangedTimerDone()));

    m_checkSelectionTimer = new QTimer(this);
    m_checkSelectionTimer->setSingleShot(true);
    connect(m_checkSelectionTimer, SIGNAL(timeout()), this, SLOT(checkSelectionNow()));

    m_geometryChangedTimer = new QTimer(this);
    m_geometryChangedTimer->setSingleShot(true);
    connect(m_geometryChangedTimer, SIGNAL(timeout()), this, SIGNAL(geometryChanged()));

    m_rubberBand = 0;

    setGrid(QPoint(10,10));

    setFocusPolicy(Qt::StrongFocus);

    m_mainContainer = 0;
    m_currentWidget = 0;
    oldRectValid = false;
    drawRubber = false;

    targetContainer = 0;
    hadOwnPalette = false;
    startWidget = 0;
    endWidget = 0;

    connect(m_commandHistory, SIGNAL(indexChanged(int)), this, SLOT(updateDirty()));
    connect(m_commandHistory, SIGNAL(indexChanged(int)), this, SIGNAL(changed()));
    connect(m_commandHistory, SIGNAL(indexChanged(int)), this, SLOT(checkSelection()));

    core()->metaDataBase()->add(this);

    initializeCoreTools();

    QAction *a = new QAction(this);
    a->setText(tr("Edit contents"));
    a->setShortcut(tr("F2"));
    connect(a, SIGNAL(triggered()), this, SLOT(editContents()));
    addAction(a);
}

QWidget *FormWindow::mainContainer() const
{
    return m_mainContainer;
}

void FormWindow::setMainContainer(QWidget *w)
{
    if (w == m_mainContainer) {
        // nothing to do
        return;
    }

    if (m_mainContainer) {
        unmanageWidget(m_mainContainer);
        delete m_mainContainer;
        m_mainContainer = 0;
    }

    m_mainContainer = w;
    w->setAutoFillBackground(true);
    QSize sz = m_mainContainer->size();

    m_mainContainer->setParent(m_widgetStack, 0);
    m_mainContainer->raise();
    m_mainContainer->show();

    m_widgetStack->setCurrentTool(m_widgetEditor);

    setCurrentWidget(m_mainContainer);

    manageWidget(m_mainContainer);

    if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), m_mainContainer)) {
        sheet->setVisible(sheet->indexOf(QLatin1String("windowTitle")), true);
        sheet->setVisible(sheet->indexOf(QLatin1String("windowIcon")), true);
        // ### generalize
    }

    m_mainContainer->setFocusPolicy(Qt::StrongFocus);
    m_mainContainer->resize(sz);

    emit mainContainerChanged(m_mainContainer);
}

QWidget *FormWindow::findTargetContainer(QWidget *widget) const
{
    Q_ASSERT(widget);

    while (QWidget *parentWidget = widget->parentWidget()) {
        if (LayoutInfo::layoutType(m_core, parentWidget) == LayoutInfo::NoLayout && isManaged(widget))
            return widget;

        widget = parentWidget;
    }

    return mainContainer();
}

bool FormWindow::handleMousePressEvent(QWidget * /*widget*/, QWidget *managedWidget, QMouseEvent *e)
{
    startPos = QPoint();
    e->accept();

    BlockSelection blocker(this);

    if (core()->formWindowManager()->activeFormWindow() != this)
        core()->formWindowManager()->setActiveFormWindow(this);

    if (e->buttons() != Qt::LeftButton)
        return true;

    startPos = mapFromGlobal(e->globalPos());

    bool inLayout = LayoutInfo::isWidgetLaidout(m_core, managedWidget);

    // if the dragged widget is not in a layout, raise it
    if (inLayout == false)
        managedWidget->raise();

    if (isMainContainer(managedWidget) == true) { // press was on the formwindow
        clearSelection(false);

        drawRubber = true;
        currRect = QRect();
        startRectDraw(mapFromGlobal(e->globalPos()), this, Rubber);
        return true;
    }

    bool selected = isWidgetSelected(managedWidget);

    if (e->modifiers() & Qt::ShiftModifier) {
        // shift-click - toggle selection state of widget
        selectWidget(managedWidget, !selected);
        return true;
    }

    QWidget *current = managedWidget;

    if (!selected)
        clearSelection(false);

    bool curLaidOut = false;
    bool curSelected = false;
    for (;;) {
        curLaidOut = current ? LayoutInfo::isWidgetLaidout(core(), current) : false;
        curSelected = current ? isWidgetSelected(current) : false;

        if (!current || !curLaidOut || !curSelected)
            break;

        current = designerWidget(current->parentWidget());
    }

    if (!(current == 0 || !curLaidOut && curSelected)) {
        selectWidget(current);
        raiseChildSelections(current);
    }

    return true;
}

bool FormWindow::isPageOfContainerWidget(QWidget *widget) const
{
    QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(),
                widget->parentWidget());

    if (c != 0) {
        for (int i = 0; i<c->count(); ++i) {
            if (widget == c->widget(i))
                return true;
        }
    }

    return false;
}

bool FormWindow::handleMouseMoveEvent(QWidget *, QWidget *, QMouseEvent *e)
{
    e->accept();

    if (e->buttons() != Qt::LeftButton || startPos.isNull())
        return true;

    QPoint pos = mapFromGlobal(e->globalPos());

    if (drawRubber == true) {
        continueRectDraw(pos, this, Rubber);
        return true;
    }

    bool canStartDrag = (startPos - pos).manhattanLength() > QApplication::startDragDistance();

    if (canStartDrag == false) {
        // nothing to do
        return true;
    }

    bool blocked = blockSelectionChanged(true);

    QList<QWidget*> sel = selectedWidgets();
    simplifySelection(&sel);

    QSet<QWidget*> widget_set;

    foreach (QWidget *child, sel) {
        QWidget *current = child;

        bool done = false;
        while (!isMainContainer(current) && !done) {
            QDesignerContainerExtension *c = 0;
            c = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), current->parentWidget());

            if (!isManaged(current)) {
                current = current->parentWidget();
                continue;
            } else if (LayoutInfo::isWidgetLaidout(core(), current)) {
                current = current->parentWidget();
                continue;
            } else if (isPageOfContainerWidget(current)) {
                current = current->parentWidget();
                continue;
            } else if (current->parentWidget()) {
                QScrollArea *area = qobject_cast<QScrollArea*>(current->parentWidget()->parentWidget());
                if (area && area->widget() == current) {
                    current = area;
                    continue;
                }
            }

            done = true;
        }

        if (current == mainContainer())
            continue;

        widget_set.insert(current);
    }

    sel = widget_set.toList();

    QDesignerResource builder(this);

    QList<QDesignerDnDItemInterface*> item_list;
    foreach (QWidget *widget, sel) {
        if (e->modifiers()
#ifndef Q_WS_MAC
        & Qt::ControlModifier
#else
        & Qt::AltModifier
#endif
        ) {
            item_list.append(new FormWindowDnDItem(QDesignerDnDItemInterface::CopyDrop,
                this, widget, mapToGlobal(startPos)));
        } else {
            item_list.append(new FormWindowDnDItem(QDesignerDnDItemInterface::MoveDrop,
                this, widget, mapToGlobal(startPos)));
            widget->hide();
        }
    }

    blockSelectionChanged(blocked);

    if (sel.count())
        core()->formWindowManager()->dragItems(item_list);

    startPos = QPoint();

    return true;
}

bool FormWindow::handleMouseReleaseEvent(QWidget *, QWidget *, QMouseEvent *e)
{
    e->accept();

    if (drawRubber) { // we were drawing a rubber selection
        endRectDraw(); // get rid of the rectangle

        bool blocked = blockSelectionChanged(true);
        selectWidgets(); // select widgets which intersect the rect
        blockSelectionChanged(blocked);

        drawRubber = false;
    }

    startPos = QPoint();

    emitSelectionChanged(); // inform about selection changes

    return true;
}

void FormWindow::checkPreviewGeometry(QRect &r)
{
    if (!rect().contains(r)) {
        if (r.left() < rect().left())
            r.moveTopLeft(QPoint(0, r.top()));
        if (r.right() > rect().right())
            r.moveBottomRight(QPoint(rect().right(), r.bottom()));
        if (r.top() < rect().top())
            r.moveTopLeft(QPoint(r.left(), rect().top()));
        if (r.bottom() > rect().bottom())
            r.moveBottomRight(QPoint(r.right(), rect().bottom()));
    }
}

void FormWindow::startRectDraw(const QPoint &pos, QWidget *, RectType t)
{
    oldRectValid = false;

    rectAnchor = (t == Insert) ? gridPoint(pos) : pos;

    currRect = QRect(rectAnchor, QSize(0, 0));
    if (!m_rubberBand)
        m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    m_rubberBand->setGeometry(currRect);
    m_rubberBand->show();
}

void FormWindow::continueRectDraw(const QPoint &pos, QWidget *, RectType t)
{
    QPoint p2 = (t == Insert) ? gridPoint(pos) : pos;

    QRect r(rectAnchor, p2);
    r = r.normalized();

    if (currRect == r)
        return;

    if (r.width() > 1 || r.height() > 1) {
        oldRectValid = true;
        currRect = r;
        if (m_rubberBand)
            m_rubberBand->setGeometry(currRect);
    } else {
        oldRectValid = false;
    }
}

void FormWindow::endRectDraw()
{
    if (m_rubberBand) {
        delete m_rubberBand;
        m_rubberBand = 0;
    }
}

QPoint FormWindow::gridPoint(const QPoint &p) const
{
    return QPoint(((p.x() + grid().x()/2) / grid().x()) * grid().x(),
                   ((p.y() + grid().y()/2) / grid().y()) * grid().y());
}

QWidget *FormWindow::currentWidget() const
{
    return m_currentWidget;
}

void FormWindow::setCurrentWidget(QWidget *currentWidget)
{
    m_currentWidget = currentWidget;
}

QSize FormWindow::sizeHint() const
{
    QMainWindow *mw = qobject_cast<QMainWindow*>(mainContainer());
    if (!mw) {
        return QSize(400, 300);
    }
    QSize sh = mw->sizeHint();
    if (sh.width() < 400)
        sh.setWidth(400);
    if (sh.height() < 300)
        sh.setHeight(300);
    return sh;

}

void FormWindow::selectWidget(QWidget* w, bool select)
{
    if (!isManaged(w) && !isCentralWidget(w))
        return;

    if (!select && !isWidgetSelected(w))
        return;

    if (!mainContainer())
        return;

    if (isMainContainer(w)) {
        QWidget *opw = m_currentWidget;
        setCurrentWidget(mainContainer());
        repaintSelection(opw);
        emitSelectionChanged();
        return;
    }

    if (isCentralWidget(w)) {
        QWidget *opw = m_currentWidget;
        setCurrentWidget(mainContainer());
        repaintSelection(opw);
        emitSelectionChanged();
        return;
    }

    if (select) {
        QWidget *opw = m_currentWidget;
        setCurrentWidget(w);
        repaintSelection(opw);
        WidgetSelection *s = usedSelections.value(w);
        if (s != 0) {
            s->show();
            return;
        }

        QListIterator<WidgetSelection*> it(selections);
        while (it.hasNext()) {
            WidgetSelection *sel = it.next();
            if (!sel->isUsed()) {
                s = sel;
                break;
            }
        }

        if (s == 0) {
            s = new WidgetSelection(this, &usedSelections);
            selections.append(s);
        }

        s->setWidget(w);
    } else {
        if (WidgetSelection *s = usedSelections.value(w))
            s->setWidget(0);

        if (!usedSelections.isEmpty())
            setCurrentWidget((*usedSelections.begin())->widget());
        else
            setCurrentWidget(mainContainer());

        repaintSelection(currentWidget());
    }

    emitSelectionChanged();
}

void FormWindow::hideSelection(QWidget *w)
{
    selectWidget(w, false);
}

void FormWindow::clearSelection(bool changePropertyDisplay)
{
    for (QHash<QWidget *, WidgetSelection *>::Iterator it = usedSelections.begin(); it != usedSelections.end(); ++it) {
        it.value()->setWidget(0, false);
    }

    usedSelections.clear();

    if (changePropertyDisplay == false)
        return;

    setCurrentWidget(mainContainer());

    if (m_currentWidget)
        repaintSelection(m_currentWidget);

    emitSelectionChanged();
}

void FormWindow::emitSelectionChanged()
{
    if (m_blockSelectionChanged == true) {
        // nothing to do
        return;
    }

    m_selectionChangedTimer->start(0);
}

void FormWindow::selectionChangedTimerDone()
{
    emit selectionChanged();
}

void FormWindow::repaintSelection(QWidget *w)
{
    if (WidgetSelection *s = usedSelections.value(w))
        s->update();
}

bool FormWindow::isWidgetSelected(QWidget *w) const
{
    return usedSelections.contains(w);
}

bool FormWindow::isMainContainer(const QWidget *w) const
{
    return w && (w == this || w == mainContainer());
}

void FormWindow::updateChildSelections(QWidget *w)
{
    QList<QWidget*> l = qFindChildren<QWidget*>(w);

    QListIterator<QWidget*> it(l);
    while (it.hasNext()) {
        QWidget *w = it.next();
        if (isManaged(w)) {
            updateSelection(w);
        }
    }
}

void FormWindow::updateSelection(QWidget *w)
{
    WidgetSelection *s = usedSelections.value(w);
    if (!w->isVisibleTo(this)) {
        selectWidget(w, false);
    } else if (s)
        s->updateGeometry();
}

QWidget *FormWindow::designerWidget(QWidget *w) const
{
    while (w && !isMainContainer(w) && !isManaged(w) || isCentralWidget(w))
        w = w->parentWidget();

    return w;
}

bool FormWindow::isCentralWidget(QWidget *w) const
{
    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(mainContainer()))
        return w == mainWindow->centralWidget();

    return false;
}

void FormWindow::ensureUniqueObjectName(QObject *object)
{
    QString name = object->objectName();

    if (!unify(object, name, true)) {
        object->setObjectName(name);
    }
}

template <class T, class P>
void merge(QList<T> *target, const QList<P> &source)
{
    foreach (P item, source) {
        target->append(item);
    }
}

bool FormWindow::unify(QObject *w, QString &s, bool changeIt)
{
    bool found = !isMainContainer(static_cast<QWidget*>(w)) && objectName() == s;

    if (!found) {
        QList<QObject*> objects;

        if (mainContainer()) {
            objects.append(mainContainer());
            merge(&objects, qFindChildren<QWidget*>(mainContainer()));
            merge(&objects, qFindChildren<QAction*>(mainContainer()));
        }

        QMutableListIterator<QObject*> mit(objects);
        while (mit.hasNext()) {
            if (!core()->metaDataBase()->item(mit.next())) {
                mit.remove();
            }
        }

        QListIterator<QObject*> it(objects);
        while (it.hasNext()) {
            QObject *child = it.next();

            if (child != w && child->objectName() == s) {
                found = true;

                if (!changeIt)
                    break;

                qlonglong num = 0;
                qlonglong factor = 1;
                int idx = s.length()-1;
                for ( ; (idx > 0) && s.at(idx).isDigit(); --idx) {
                    num += (s.at(idx).unicode() - QLatin1Char('0').unicode()) * factor;
                    factor *= 10;
                }

                if ((idx >= 0) && (QLatin1Char('_') == s.at(idx)))
                    s = s.left(idx+1) + QString::number(num+1);
                else
                    s = s + QLatin1String("_2");

                it.toFront();
            }
        }
    }

    return !found;
}

/* already_in_form is true when we are moving a widget from one parent to another inside the same
   form. All this means is that InsertWidgetCommand::undo() must not unmanage it. */

void FormWindow::insertWidget(QWidget *w, const QRect &rect, QWidget *container, bool already_in_form)
{
    clearSelection(false);

    beginCommand(tr("Insert widget '%1").arg(QString::fromUtf8(w->metaObject()->className()))); // ### use the WidgetDatabaseItem

    /* Reparenting into a QSplitter automatically adjusts child's geometry. We create the geometry
       command before we push the reparent command, so that the geometry command has the original
       geometry of the widget. */
    QRect r = rect;
    Q_ASSERT(r.isValid());
    r.moveTopLeft(gridPoint(container->mapFromGlobal(r.topLeft())));
    SetPropertyCommand *geom_cmd = new SetPropertyCommand(this);
    geom_cmd->init(w, QLatin1String("geometry"), r); // ### use rc.size()

    if (w->parentWidget() != container) {
        ReparentWidgetCommand *cmd = new ReparentWidgetCommand(this);
        cmd->init(w, container);
        m_commandHistory->push(cmd);
    }

    m_commandHistory->push(geom_cmd);

    InsertWidgetCommand *cmd = new InsertWidgetCommand(this);
    cmd->init(w, already_in_form);
    m_commandHistory->push(cmd);

    endCommand();

    w->show();
}

QWidget *FormWindow::createWidget(DomUI *ui, const QRect &rc, QWidget *target)
{
    QWidget *container = findContainer(target, false);
    if (!container)
        return 0;

    if (isMainContainer(container)) {
        if (QMainWindow *mw = qobject_cast<QMainWindow*>(container)) {
            Q_ASSERT(mw->centralWidget() != 0);
            container = mw->centralWidget();
        }
    }

    QDesignerResource resource(this);
    QList<QWidget*> widgets = resource.paste(ui, container);
    Q_ASSERT(widgets.size() == 1); // multiple-paste from DomUI not supported yet

    insertWidget(widgets.first(), rc, container);
    return widgets.first();
}

static bool isDescendant(const QWidget *parent, const QWidget *child)
{
    for (; child != 0; child = child->parentWidget()) {
        if (child == parent)
            return true;
    }
    return false;
}

void FormWindow::resizeWidget(QWidget *widget, const QRect &geometry)
{
    Q_ASSERT(isDescendant(this, widget));

    QRect r = geometry;
    r.moveTopLeft(gridPoint(geometry.topLeft()));
    SetPropertyCommand *cmd = new SetPropertyCommand(this);
    cmd->init(widget, QLatin1String("geometry"), r);
    cmd->setText(tr("Resize"));
    m_commandHistory->push(cmd);
}

void FormWindow::raiseChildSelections(QWidget *w)
{
    QList<QWidget*> l = qFindChildren<QWidget*>(w);
    if (l.isEmpty())
        return;

    QHashIterator<QWidget *, WidgetSelection *> it(usedSelections);
    while (it.hasNext()) {
        it.next();

        WidgetSelection *w = it.value();
        if (l.contains(w->widget()))
            w->show();
    }
}

QWidget *FormWindow::containerAt(const QPoint &pos, QWidget *notParentOf)
{
    QWidget *container = 0;
    int depth = -1;
    QList<QWidget*> selected = selectedWidgets();
    if (rect().contains(mapFromGlobal(pos))) {
        container = mainContainer();
        depth = widgetDepth(container);
    }

    QListIterator<QWidget*> it(m_widgets);
    while (it.hasNext()) {
        QWidget *wit = it.next();
        if (qobject_cast<QLayoutWidget*>(wit) || qobject_cast<QSplitter*>(wit))
            continue;
        if (!wit->isVisibleTo(this))
            continue;
        if (selected.indexOf(wit) != -1)
            continue;
        if (!core()->widgetDataBase()->isContainer(wit) &&
             wit != mainContainer())
            continue;

        // the rectangles of all ancestors of the container must contain the insert position
        QWidget *w = wit;
        while (w && !w->isWindow()) {
            if (!w->rect().contains((w->mapFromGlobal(pos))))
                break;
            w = w->parentWidget();
        }
        if (!(w == 0 || w->isWindow()))
            continue; // we did not get through the full while loop

        int wd = widgetDepth(wit);
        if (wd == depth && container) {
            if (wit->parentWidget()->children().indexOf(wit) >
                 container->parentWidget()->children().indexOf(container))
                wd++;
        }
        if (wd > depth && !isChildOf(wit, notParentOf)) {
            depth = wd;
            container = wit;
        }
    }
    return container;
}

QList<QWidget*> FormWindow::selectedWidgets() const
{
    return usedSelections.keys();
}

void FormWindow::raiseSelection(QWidget *w)
{
    WidgetSelection *s = usedSelections.value(w);
    if (s)
        s->show();
}

void FormWindow::selectWidgets()
{
    QList<QWidget*> l = qFindChildren<QWidget*>(mainContainer());
    QListIterator <QWidget*> it(l);
    const QRect selRect(mapToGlobal(currRect.topLeft()), currRect.size());
    while (it.hasNext()) {
        QWidget *w = it.next();
        if (w->isVisibleTo(this) && isManaged(w)) {
            QPoint p = w->mapToGlobal(QPoint(0,0));
            QRect r(p, w->size());
            if (r.intersects(selRect) && !r.contains(selRect))
                selectWidget(w);
        }
    }

    emitSelectionChanged();
}

bool FormWindow::handleKeyPressEvent(QWidget *widget, QWidget *, QKeyEvent *e)
{
    if (qobject_cast<FormWindow*>(widget) || qobject_cast<QMenu*>(widget))
        return false;

    e->accept(); // we always accept!

    switch (e->key()) {
        default: break; // we don't care about the other keys

        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            deleteWidgets();
            break;

        case Qt::Key_Tab:
            cursor()->movePosition(QDesignerFormWindowCursorInterface::Next);
            break;

        case Qt::Key_Backtab:
            cursor()->movePosition(QDesignerFormWindowCursorInterface::Prev);
            break;

        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
            if (e->modifiers() & Qt::ControlModifier)
                handleArrowKeyEvent(e->key(), e->modifiers() == Qt::ControlModifier);
            break;
    }

    return true;
}

void FormWindow::handleArrowKeyEvent(int key, bool modifier)
{
    bool startMacro = false;
    QDesignerFormWindowCursorInterface *c = cursor();
    if (!c->hasSelection())
        return;

    int selCount = c->selectedWidgetCount();
    int x = grid().x();
    int y = grid().y();

    if (modifier) {
        x = 1;
        y = 1;
    }

    // check if a layed out widget is selected
    for (int index=0; index<c->selectedWidgetCount(); ++index) {
        QWidget *w = c->selectedWidget(index);
        if (LayoutInfo::isWidgetLaidout(m_core, w))
            return;
    }

    // check if selection is the same as last time
    if (selCount != m_moveSelection.count() ||
        m_lastUndoIndex != m_commandHistory->index()) {
        m_moveSelection.clear();
        startMacro = true;
    } else {
        for (int index=0; index<selCount; ++index) {
            if (m_moveSelection[index]->object() != c->selectedWidget(index)) {
                m_moveSelection.clear();
                startMacro = true;
                break;
            }
        }
    }

    if (startMacro)
        beginCommand(tr("Key Move"));

    for (int index=0; index<selCount; ++index) {
        QRect geom = c->selectedWidget(index)->geometry();
        switch(key) {
            case Qt::Key_Left:
                geom.adjust(-x, 0,-x, 0);
                break;
            case Qt::Key_Right:
                geom.adjust( x, 0, x, 0);
                break;
            case Qt::Key_Up:
                geom.adjust( 0,-y, 0,-y);
                break;
            case Qt::Key_Down:
                geom.adjust( 0, y, 0, y);
                break;
        }

        if (!modifier)
            geom.moveTopLeft(gridPoint(geom.topLeft()));

        SetPropertyCommand *cmd = 0;

        if (m_moveSelection.count() > index)
            cmd = m_moveSelection[index];

        if (!cmd) {
            cmd = new SetPropertyCommand(this);
            cmd->init(c->selectedWidget(index), QLatin1String("geometry"), geom);
            cmd->setText(tr("Key Move"));
            m_commandHistory->push(cmd);

            if (m_moveSelection.count() > index)
                m_moveSelection.replace(index, cmd);
            else
                m_moveSelection.append(cmd);
        } else {
            cmd->setNewValue(geom);
            cmd->redo();
        }
    }

    if (startMacro) {
        endCommand();
        m_lastUndoIndex = m_commandHistory->index();
    }
}

bool FormWindow::handleKeyReleaseEvent(QWidget *, QWidget *, QKeyEvent *e)
{
    e->accept();
    return true;
}

void FormWindow::selectAll()
{
    bool blocked = blockSignals(true);
    foreach (QWidget *widget, m_widgets) {
        if (widget->isVisibleTo(this))
            selectWidget(widget);
    }

    blockSignals(blocked);
    emitSelectionChanged();
}

void FormWindow::layoutHorizontal()
{
    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), selectedWidgets(), LayoutInfo::HBox);
    clearSelection(false);
    commandHistory()->push(cmd);
}

void FormWindow::layoutVertical()
{
    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), selectedWidgets(), LayoutInfo::VBox);
    clearSelection(false);
    commandHistory()->push(cmd);
}

void FormWindow::layoutGrid()
{
    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), selectedWidgets(), LayoutInfo::Grid);
    clearSelection(false);
    commandHistory()->push(cmd);
}

void FormWindow::deleteWidgets(const QList<QWidget*> &widget_list)
{
    if (widget_list.isEmpty())
        return;

    beginCommand(tr("Delete"));

    foreach (QWidget *w, widget_list) {
        emit widgetRemoved(w);
        DeleteWidgetCommand *cmd = new DeleteWidgetCommand(this);
        cmd->init(w);
        m_commandHistory->push(cmd);
    }

    endCommand();
}


void FormWindow::deleteWidgets()
{
    QList<QWidget*> selection = selectedWidgets();
    simplifySelection(&selection);

    deleteWidgets(selection);
}

QString FormWindow::fileName() const
{
    return m_fileName;
}

void FormWindow::setFileName(const QString &fileName)
{
    if (m_fileName == fileName)
        return;

    m_fileName = fileName;
    emit fileNameChanged(fileName);
}

QString FormWindow::contents() const
{
    QBuffer b;
    if (!b.open(QIODevice::WriteOnly))
        return QString();

    QDesignerResource resource(const_cast<FormWindow*>(this));
    resource.save(&b, mainContainer());

    return QString::fromUtf8(b.buffer());
}

void FormWindow::copy()
{
    QBuffer b;
    if (!b.open(QIODevice::WriteOnly))
        return;

    QDesignerResource resource(this);
    QList<QWidget*> sel = selectedWidgets();
    simplifySelection(&sel);
    resource.copy(&b, sel);

    qApp->clipboard()->setText(QString::fromUtf8(b.buffer()), QClipboard::Clipboard);
}

void FormWindow::cut()
{
    copy();
    deleteWidgets();
}

void FormWindow::paste()
{
    QWidget *w = mainContainer();
    QList<QWidget*> l(selectedWidgets());
    if (l.count() == 1) {
        w = l.first();
        w = m_core->widgetFactory()->containerOfWidget(w);
        if (LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout ||
             (!core()->widgetDataBase()->isContainer(w) &&
               w != mainContainer()))
            w = mainContainer();
    }

    if (w && LayoutInfo::layoutType(m_core, w) == LayoutInfo::NoLayout) {
        clearSelection(true);

        QByteArray code = qApp->clipboard()->text().toUtf8();
        QBuffer b(&code);
        b.open(QIODevice::ReadOnly);

        QDesignerResource resource(this);
        QWidget *widget = core()->widgetFactory()->containerOfWidget(w);
        QList<QWidget*> widgets = resource.paste(&b, widget);

        beginCommand(tr("Paste"));
        foreach (QWidget *w, widgets) {
            InsertWidgetCommand *cmd = new InsertWidgetCommand(this);
            cmd->init(w);
            m_commandHistory->push(cmd);
            selectWidget(w);
        }
        endCommand();

        /* This will put the freshly pasted widgets into the clipboard, replacing the original.
           The point here is that the copied widgets are shifted a little with respect to the original.
           If the user presses paste again, the pasted widgets will be shifted again, rather than
           appearing on top of the previously pasted widgets. */
        copy();

    } else {
        QMessageBox::information(this, tr("Paste error"),
                                  tr("Can't paste widgets. Designer couldn't find a container\n"
                                      "to paste into which does not contain a layout. Break the layout\n"
                                      "of the container you want to paste into and select this container\n"
                                      "and then paste again."));
    }

}

void FormWindow::manageWidget(QWidget *w)
{
    if (isManaged(w))
        return;

    Q_ASSERT(qobject_cast<QMenu*>(w) == 0);

    if (w->hasFocus())
        setFocus();

    core()->metaDataBase()->add(w);

    m_insertedWidgets.insert(w);
    m_widgets.append(w);

    setCursorToAll(Qt::ArrowCursor, w);

    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(w))
        manageWidget(promoted->child());

    emit changed();
    emit widgetManaged(w);
}

void FormWindow::unmanageWidget(QWidget *w)
{
    if (!isManaged(w))
        return;

    if (usedSelections.contains(w))
        usedSelections.value(w)->setWidget(0);

    emit aboutToUnmanageWidget(w);

    core()->metaDataBase()->remove(w);

    m_insertedWidgets.remove(w);
    m_widgets.removeAt(m_widgets.indexOf(w));

    emit changed();
    emit widgetUnmanaged(w);
}

bool FormWindow::isManaged(QWidget *w) const
{
    return m_insertedWidgets.contains(w);
}

void FormWindow::breakLayout(QWidget *w)
{
    if (w == this)
        w = mainContainer();

    w = core()->widgetFactory()->containerOfWidget(w);

    beginCommand(tr("Break layout"));

    for (;;) {
        if (!w || w == this)
            break;

        if (LayoutInfo::layoutType(m_core, core()->widgetFactory()->containerOfWidget(w)) != LayoutInfo::NoLayout
                && core()->widgetDataBase()->isContainer(w, false)) {

            if (BreakLayoutCommand *cmd = breakLayoutCommand(w)) {
                commandHistory()->push(cmd);
            }

            if (!qobject_cast<QLayoutWidget*>(w) && !qobject_cast<QSplitter*>(w))
                break;
        }

        w = w->parentWidget();
    }

    clearSelection(false);
    endCommand();
}

BreakLayoutCommand *FormWindow::breakLayoutCommand(QWidget *w)
{
    QList<QWidget*> widgets;

    QListIterator<QObject*> it(w->children());
    while (it.hasNext()) {
        QObject *obj = it.next();

        if (!obj->isWidgetType()
                || !core()->metaDataBase()->item(obj))
            continue;

        widgets.append(static_cast<QWidget*>(obj));
    }

    BreakLayoutCommand *cmd = new BreakLayoutCommand(this);
    cmd->init(widgets, core()->widgetFactory()->widgetOfContainer(w));
    return cmd;
}

void FormWindow::breakLayout()
{
    QWidget *w = currentWidget() ? currentWidget() : mainContainer();

    if (LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout ||
         w->parentWidget() && LayoutInfo::layoutType(m_core, w->parentWidget()) != LayoutInfo::NoLayout) {
        breakLayout(w);
        return;
    } else {
        QList<QWidget*> widgets = selectedWidgets();
        QListIterator<QWidget*> it(widgets);
        while (it.hasNext()) {
            QWidget *w = it.next();
            if (LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout ||
                 w->parentWidget() && LayoutInfo::layoutType(m_core, w->parentWidget()) != LayoutInfo::NoLayout)
                break;
        }
        if (w) {
            breakLayout(w);
            return;
        }
    }

    w = mainContainer();

    if (LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout ||
         w->parentWidget() && LayoutInfo::layoutType(m_core, w->parentWidget()) != LayoutInfo::NoLayout)
        breakLayout(w);
}

void FormWindow::beginCommand(const QString &description)
{
    m_commandHistory->beginMacro(description);
}

void FormWindow::endCommand()
{
    m_commandHistory->endMacro();
}

void FormWindow::raiseWidgets()
{
    QList<QWidget*> widgets = selectedWidgets();
    simplifySelection(&widgets);

    foreach (QWidget *widget, widgets) {
        widget->raise();
    }
}

void FormWindow::lowerWidgets()
{
    QList<QWidget*> widgets = selectedWidgets();
    simplifySelection(&widgets);

    foreach (QWidget *widget, widgets) {
        widget->lower();
    }
}

bool FormWindow::handleMouseButtonDblClickEvent(QWidget *, QWidget *managedWidget, QMouseEvent *e)
{
    e->accept();

    clearSelection(false);
    selectWidget(managedWidget);

    emit activated(managedWidget);
    return true;
}

void FormWindow::finishContextMenu(QWidget *w, QWidget *, QContextMenuEvent *e)
{
    e->accept();

    QDesignerTaskMenuExtension *taskMenu = qt_extension<QDesignerTaskMenuExtension*>(core()->extensionManager(), w);
    QMenu *menu = createPopupMenu(w);
    if (menu && taskMenu) {
        QList<QAction *> acts = taskMenu->taskActions();
        if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(w)) {
            QDesignerTaskMenuExtension *baseTaskMenu =
                qt_extension<QDesignerTaskMenuExtension*>(core()->extensionManager(),
                                                          promoted->child());
            if (baseTaskMenu) {
                // "inherit" proper actions from base class's task menu
                QList<QAction *> baseActs = baseTaskMenu->taskActions();
                QList<QAction *>::iterator it;
                for (it = baseActs.begin(); it != baseActs.end(); ) {
                    // special case: don't want the "Promote to custom widget" action
                    if ((*it)->objectName() == QLatin1String("__qt__promoteToCustomWidgetAction")) {
                        it = baseActs.erase(it);
                        continue;
                    }
                    // inherit the action only if it is not "reimplemented"
                    // by the promoted widget's task menu
                    QList<QAction *>::iterator it2;
                    bool inherit = true;
                    for (it2 = acts.begin(); it2 != acts.end(); ++it2) {
                        if ((*it)->text() == (*it2)->text()) {
                            inherit = false;
                            break;
                        }
                    }
                    if (inherit)
                        ++it;
                    else
                        it = baseActs.erase(it);
                }
                if (!baseActs.isEmpty()) {
                    // prepend the inherited actions to the action list
                    QAction *sep = new QAction(menu);
                    sep->setSeparator(true);
                    baseActs.append(sep);
                    for (int i = 0; i < baseActs.size(); ++i)
                        acts.insert(i, baseActs.at(i));
                }
            }
        }
        QAction *sep = new QAction(menu);
        sep->setSeparator(true);
        acts.append(sep);
        menu->insertActions(menu->actions().at(0), acts);
    }

    if (menu) {
        emit contextMenuRequested(menu, w);
        menu->exec(e->globalPos());
        delete menu;
    }
}


bool FormWindow::handleContextMenu(QWidget *, QWidget *managedWidget, QContextMenuEvent *e)
{
    e->accept();

    bool selected = isWidgetSelected(managedWidget);
    if (selected == false) {
        clearSelection(false);

        selectWidget(managedWidget);
        raiseChildSelections(managedWidget); // raise selections and select widget

        QMetaObject::invokeMethod(core()->formWindowManager(), "slotUpdateActions");
    }

    if (!isMainContainer(managedWidget)) { // press on a child widget
        // if widget is laid out, find the first non-laid out super-widget
        QWidget *realWidget = managedWidget; // but store the original one
        QMainWindow *mw = qobject_cast<QMainWindow*>(mainContainer());

        if (mw && mw->centralWidget() == realWidget) {
            finishContextMenu(managedWidget, this, e);
        } else {
            finishContextMenu(realWidget, realWidget, e);
        }
    } else {
        finishContextMenu(mainContainer(), mainContainer(), e);
    }

    return true;
}

void FormWindow::setContents(QIODevice *dev)
{
    bool saved = updatesEnabled();

    setUpdatesEnabled(false);
    clearSelection();

    if (mainContainer()) {
        core()->metaDataBase()->remove(mainContainer());
        delete mainContainer();
        m_mainContainer = 0;
    }

    m_insertedWidgets.clear();
    m_widgets.clear();
    emit changed();

    QDesignerResource r(this);
    QWidget *w = r.load(dev, this);
    if (w == 0) {
        setFileName(QString());
        w = core()->widgetFactory()->createWidget(QLatin1String("QWidget"), this);
        w->resize(400, 300);
    }

    setMainContainer(w);

    setUpdatesEnabled(saved);
}

void FormWindow::setContents(const QString &contents)
{
    QByteArray data = contents.toUtf8();
    QBuffer b(&data);
    if (b.open(QIODevice::ReadOnly))
        setContents(&b);
}

void FormWindow::layoutHorizontalContainer(QWidget *w)
{
    if (w == this)
        w = mainContainer();

    w = core()->widgetFactory()->containerOfWidget(w);

    QList<QObject*> l = w->children();
    if (l.isEmpty())
        return;

    QList<QWidget*> widgets;
    QListIterator<QObject*> it(l);
    while (it.hasNext()) {
        QObject* o = it.next();
        if (!o->isWidgetType())
            continue;

        QWidget *widget = static_cast<QWidget*>(o);
        if (widget->isVisibleTo(this) && isManaged(widget))
            widgets.append(widget);
    }

    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), widgets, LayoutInfo::HBox, w);
    clearSelection(false);
    commandHistory()->push(cmd);
}

void FormWindow::layoutVerticalContainer(QWidget *w)
{
    if (w == this)
        w = mainContainer();

    w = core()->widgetFactory()->containerOfWidget(w);

    QList<QObject*> l = w->children();
    if (l.isEmpty())
        return;

    QListIterator<QObject*> it(l);
    QList<QWidget*> widgets;
    while (it.hasNext()) {
        QObject* o = it.next();
        if (!o->isWidgetType())
            continue;

        QWidget *widget = static_cast<QWidget*>(o);
        if (widget->isVisibleTo(this) && isManaged(widget))
            widgets.append(widget);
    }

    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), widgets, LayoutInfo::VBox, w);
    clearSelection(false);
    commandHistory()->push(cmd);
}

void FormWindow::layoutGridContainer(QWidget *w)
{
    if (w == this)
        w = mainContainer();

    w = core()->widgetFactory()->containerOfWidget(w);

    QList<QObject*> l = w->children();
    if (l.isEmpty())
        return;

    QList<QWidget*> widgets;
    QListIterator<QObject*> it(l);
    while (it.hasNext()) {
        QObject* o = it.next();

        if (!o->isWidgetType())
            continue;

        QWidget *widget = static_cast<QWidget*>(o);
        if (widget->isVisibleTo(this) && isManaged(widget))
            widgets.append(widget);
    }

    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), widgets, LayoutInfo::Grid, w);
    clearSelection(false);
    commandHistory()->push(cmd);
}

bool FormWindow::hasInsertedChildren(QWidget *widget) const // ### move
{
    if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), widget)) {
        widget = container->widget(container->currentIndex());
    }

    QList<QWidget*> l = widgets(widget);

    foreach (QWidget *child, l) {
        if (isManaged(child) && !LayoutInfo::isWidgetLaidout(core(), child) && child->isVisibleTo(const_cast<FormWindow*>(this)))
            return true;
    }

    return false;
}

void FormWindow::layoutHorizontalSplit()
{
    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), selectedWidgets(), LayoutInfo::HBox, /*layoutBase=*/ 0, /*splitter=*/ true);
    clearSelection(false);
    commandHistory()->push(cmd);
}

void FormWindow::layoutVerticalSplit()
{
    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), selectedWidgets(), LayoutInfo::VBox, /*layoutBase=*/ 0, /*splitter=*/ true);
    clearSelection(false);
    commandHistory()->push(cmd);
}

QMenu *FormWindow::createPopupMenu(QWidget *w)
{
    QDesignerFormWindowManagerInterface *manager = core()->formWindowManager();
    bool isFormWindow = qobject_cast<FormWindow*>(w);

    QMenu *popup = new QMenu(this);

    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(w))
        w = promoted->child();

    if (qobject_cast<QDesignerTabWidget*>(w)) {
        QDesignerTabWidget *tabWidget = static_cast<QDesignerTabWidget*>(w);
        if (tabWidget->count()) {
            popup->addAction(tabWidget->actionDeletePage());
        }
        QMenu *insertPageMenu = popup->addMenu(tr("Insert Page"));
        insertPageMenu->addAction(tabWidget->actionInsertPageAfter());
        insertPageMenu->addAction(tabWidget->actionInsertPage());
        popup->addSeparator();
    } else if (qobject_cast<QDesignerStackedWidget*>(w)) {
        QDesignerStackedWidget *stackedWidget = static_cast<QDesignerStackedWidget*>(w);
        if (stackedWidget->count()) {
            popup->addAction(stackedWidget->actionDeletePage());
        }
        QMenu *insertPageMenu = popup->addMenu(tr("Insert Page"));
        insertPageMenu->addAction(stackedWidget->actionInsertPageAfter());
        insertPageMenu->addAction(stackedWidget->actionInsertPage());
        popup->addAction(stackedWidget->actionNextPage());
        popup->addAction(stackedWidget->actionPreviousPage());
        if (stackedWidget->count() > 1) {
            popup->addAction(stackedWidget->actionChangePageOrder());
        }
        popup->addSeparator();
    } else if (qobject_cast<QDesignerToolBox*>(w)) {
        QDesignerToolBox *toolBox = static_cast<QDesignerToolBox*>(w);
        if (toolBox->count()) {
            popup->addAction(toolBox->actionDeletePage());
        }
        QMenu *insertPageMenu = popup->addMenu(tr("Insert Page"));
        insertPageMenu->addAction(toolBox->actionInsertPageAfter());
        insertPageMenu->addAction(toolBox->actionInsertPage());
        if (toolBox->count() > 1) {
            popup->addAction(toolBox->actionChangePageOrder());
        }
        popup->addSeparator();
    }

    if (!isFormWindow) {
        popup->addAction(manager->actionCut());
        popup->addAction(manager->actionCopy());
    }

    popup->addAction(manager->actionPaste());
    popup->addAction(manager->actionSelectAll());

    if (!isFormWindow) {
        popup->addAction(manager->actionDelete());
    }

    popup->addSeparator();
    QMenu *menu = popup->addMenu(tr("Lay out"));
    menu->addAction(manager->actionAdjustSize());
    menu->addAction(manager->actionHorizontalLayout());
    menu->addAction(manager->actionVerticalLayout());
    menu->addAction(manager->actionGridLayout());

    if (!isFormWindow) {
        menu->addAction(manager->actionSplitHorizontal());
        menu->addAction(manager->actionSplitVertical());
    }

    menu->addAction(manager->actionBreakLayout());

    return popup;
}

void FormWindow::resizeEvent(QResizeEvent *e)
{
/*    if (editMode() == TabOrderEditMode)
        repositionOrderIndicators(); */

    m_geometryChangedTimer->start(10);

    QWidget::resizeEvent(e);
}

/*!
  Maps \a pos in \a w's coordinates to the form's coordinate system.

  This is the equivalent to mapFromGlobal(w->mapToGlobal(pos)) but
  avoids the two roundtrips to the X-Server on Unix/X11.
 */
QPoint FormWindow::mapToForm(const QWidget *w, const QPoint &pos) const
{
    QPoint p = pos;
    const QWidget* i = w;
    while (i && !i->isWindow() && !isMainContainer(i)) {
        p = i->mapToParent(p);
        i = i->parentWidget();
    }

    return mapFromGlobal(w->mapToGlobal(pos));
}

bool FormWindow::canBeBuddy(QWidget *w) const // ### rename me.
{
    if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), w)) {
        int index = sheet->indexOf(QLatin1String("focusPolicy"));
        if (index != -1) {
            bool ok = false;
            Qt::FocusPolicy q = (Qt::FocusPolicy) Utils::valueOf(sheet->property(index), &ok);
            return ok && q != Qt::NoFocus;
        }
    }

    return false;
}

QWidget *FormWindow::findContainer(QWidget *w, bool excludeLayout) const
{
    if (!isChildOf(w, this)
        || const_cast<const QWidget *>(w) == this)
        return 0;

    QDesignerWidgetFactoryInterface *widgetFactory = core()->widgetFactory();
    QDesignerWidgetDataBaseInterface *widgetDataBase = core()->widgetDataBase();
    QDesignerMetaDataBaseInterface *metaDataBase = core()->metaDataBase();

    QWidget *container = widgetFactory->containerOfWidget(mainContainer()); // default parent for new widget is the formwindow
    if (!isMainContainer(w)) { // press was not on formwindow, check if we can find another parent
        while (w) {
            if (qobject_cast<InvisibleWidget*>(w) || !metaDataBase->item(w)) {
                w = w->parentWidget();
                continue;
            }

            bool isContainer = widgetDataBase->isContainer(w, true) || w == mainContainer();

            if (!isContainer || (excludeLayout && qobject_cast<QLayoutWidget*>(w))) { // ### skip QSplitter
                w = w->parentWidget();
            } else {
                container = w;
                break;
            }
        }
    }

    return container;
}

void FormWindow::simplifySelection(QList<QWidget*> *sel) const
{
    // Figure out which widgets should be removed from selection.
    // We want to remove those whose parent widget is also in the
    // selection (because the child widgets are contained by
    // their parent, they shouldn't be in the selection --
    // they are "implicitly" selected)
    QList<QWidget*> toBeRemoved;
    QListIterator<QWidget*> it(*sel);
    while (it.hasNext()) {
        QWidget *child = it.next();
        QWidget *w = child;

        while (w->parentWidget() && sel->contains(w->parentWidget()))
            w = w->parentWidget();

        if (child != w)
            toBeRemoved.append(child);
    }
    // Now we can actually remove the widgets that were marked
    // for removal in the previous pass.
    while (!toBeRemoved.isEmpty())
        sel->removeAll(toBeRemoved.takeFirst());
}

FormWindow *FormWindow::findFormWindow(QWidget *w)
{
    return qobject_cast<FormWindow*>(QDesignerFormWindowInterface::findFormWindow(w));
}

void FormWindow::repaintSelection()
{
    QList<QWidget*> sel = selectedWidgets();
    foreach (QWidget *ww, sel)
        repaintSelection(ww);
}

bool FormWindow::isDirty() const
{
    return m_dirty;
}

void FormWindow::setDirty(bool dirty)
{
    m_dirty = dirty;

    if (!m_dirty)
        m_lastIndex = m_commandHistory->index();
}

void FormWindow::updateDirty()
{
    m_dirty = m_commandHistory->index() != m_lastIndex;
}

QWidget *FormWindow::containerAt(const QPoint &pos)
{
    QWidget *widget = widgetAt(pos);
    return findContainer(widget, true);
}

static QWidget *childAt_SkipDropLine(QWidget *w, QPoint pos)
{
    QObjectList child_list = w->children();
    for (int i = child_list.size() - 1; i >= 0; --i) {
        QObject *child_obj = child_list[i];
        if (qobject_cast<WidgetHandle*>(child_obj) != 0)
            continue;
        QWidget *child = qobject_cast<QWidget*>(child_obj);
        if (!child || child->isWindow() || !child->isVisible() ||
                !child->geometry().contains(pos) || child->testAttribute(Qt::WA_TransparentForMouseEvents))
            continue;
        QPoint childPos = child->mapFromParent(pos);
        if (QWidget *res = childAt_SkipDropLine(child, childPos))
            return res;
        if (child->testAttribute(Qt::WA_MouseNoMask) || child->mask().contains(pos)
                || child->mask().isEmpty())
            return child;
    }

    return 0;
}

QWidget *FormWindow::widgetAt(const QPoint &pos)
{
    QWidget *w = childAt(pos);
    if (qobject_cast<WidgetHandle*>(w) != 0)
        w = childAt_SkipDropLine(this, pos);
    return w == 0 ? this : w;
}

void FormWindow::highlightWidget(QWidget *widget, const QPoint &pos, HighlightMode mode)
{
    Q_ASSERT(widget);

    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*> (widget)) {
        widget = mainWindow->centralWidget();
    }

    QWidget *container = findContainer(widget, false);

    if (container == 0 || core()->metaDataBase()->item(container) == 0)
        return;

    if (QDesignerActionProviderExtension *g = qt_extension<QDesignerActionProviderExtension*>(core()->extensionManager(), container)) {
        if (mode == Restore) {
            g->adjustIndicator(QPoint());
        } else {
            QPoint pt = widget->mapTo(container, pos);
            g->adjustIndicator(pt);
        }
    } else if (QDesignerLayoutDecorationExtension *g = qt_extension<QDesignerLayoutDecorationExtension*>(core()->extensionManager(), container)) {
        if (mode == Restore) {
            g->adjustIndicator(QPoint(), -1);
        } else {
            QPoint pt = widget->mapTo(container, pos);
            int index = g->findItemAt(pt);
            g->adjustIndicator(pt, index);
        }
    }

    QMainWindow *mw = qobject_cast<QMainWindow*> (container);
    if (container == mainContainer() || (mw && mw->centralWidget() && mw->centralWidget() == container))
        return;

    if (mode == Restore) {
        QPair<QPalette, bool> paletteAndFill = palettesBeforeHighlight.take(container);
        container->setPalette(paletteAndFill.first);
        container->setAutoFillBackground(paletteAndFill.second);
    } else {
        QPalette p = container->palette();
        if (!palettesBeforeHighlight.contains(container)) {
            QPair<QPalette, bool> paletteAndFill;
            if (container->testAttribute(Qt::WA_SetPalette))
                paletteAndFill.first = p;
            paletteAndFill.second = container->autoFillBackground();
            palettesBeforeHighlight[container] = paletteAndFill;
        }

        p.setColor(backgroundRole(), p.midlight().color());
        container->setPalette(p);
        container->setAutoFillBackground(true);
    }
}

QList<QWidget *> FormWindow::widgets(QWidget *widget) const
{
    QList<QWidget *> l;

    foreach (QObject *o, widget->children()) {
        QWidget *w = qobject_cast<QWidget*>(o);
        if (w && isManaged(w))
            l.append(w);
    }

    return l;
}

int FormWindow::toolCount() const
{
    return m_widgetStack->count();
}

QDesignerFormWindowToolInterface *FormWindow::tool(int index) const
{
    return m_widgetStack->tool(index);
}

void FormWindow::registerTool(QDesignerFormWindowToolInterface *tool)
{
    Q_ASSERT(tool != 0);

    m_widgetStack->addTool(tool);

    if (m_mainContainer)
        m_mainContainer->update();
}

void FormWindow::setCurrentTool(int index)
{
    m_widgetStack->setCurrentTool(index);
}

int FormWindow::currentTool() const
{
    return m_widgetStack->currentIndex();
}

bool FormWindow::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
    if (m_widgetStack == 0)
        return false;

    QDesignerFormWindowToolInterface *tool = m_widgetStack->currentTool();
    if (tool == 0)
        return false;

    return tool->handleEvent(widget, managedWidget, event);
}

void FormWindow::initializeCoreTools()
{
    m_widgetEditor = new WidgetEditorTool(this);
    registerTool(m_widgetEditor);
}

void FormWindow::checkSelection()
{
    m_checkSelectionTimer->start(0);
}

void FormWindow::checkSelectionNow()
{
    m_checkSelectionTimer->stop();

    foreach (QWidget *widget, selectedWidgets()) {
        updateSelection(widget);

        if (LayoutInfo::layoutType(core(), widget) != LayoutInfo::NoLayout)
            updateChildSelections(widget);
    }
}

QString FormWindow::author() const
{
    return m_author;
}

QString FormWindow::comment() const
{
     return m_comment;
}

void FormWindow::setAuthor(const QString &author)
{
    m_author = author;
}

void FormWindow::setComment(const QString &comment)
{
    m_comment = comment;
}

void FormWindow::editWidgets()
{
    m_widgetEditor->action()->trigger();
}

QStringList FormWindow::resourceFiles() const
{
    return m_resourceFiles;
}

void FormWindow::addResourceFile(const QString &path)
{
    if (!m_resourceFiles.contains(path)) {
        m_resourceFiles.append(path);
        setDirty(true);
        emit resourceFilesChanged();
    }
}

void FormWindow::removeResourceFile(const QString &path)
{
    if (m_resourceFiles.removeAll(path) > 0) {
        setDirty(true);
        emit resourceFilesChanged();
    }
}

bool FormWindow::blockSelectionChanged(bool b)
{
    bool blocked = m_blockSelectionChanged;
    m_blockSelectionChanged = b;
    return blocked;
}

void FormWindow::editContents()
{
    QList<QWidget*> sel = selectedWidgets();
    if (sel.count() == 1) {
        QWidget *widget = sel.first();

        if (QDesignerTaskMenuExtension *taskMenu = qt_extension<QDesignerTaskMenuExtension*>(core()->extensionManager(), widget)) {
            if (QAction *a = taskMenu->preferredEditAction()) {
                a->trigger();
            } else if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(widget)) {
                QDesignerTaskMenuExtension *baseTaskMenu =
                    qt_extension<QDesignerTaskMenuExtension*>(core()->extensionManager(),
                                                              promoted->child());
                if (QAction *b = baseTaskMenu->preferredEditAction())
                    b->trigger();
            }
        }
    }
}

void FormWindow::dropWidgets(QList<QDesignerDnDItemInterface*> &item_list, QWidget *target,
                                const QPoint &global_mouse_pos)
{
    beginCommand(tr("Drop widget"));

    QWidget *parent = target;
    if (parent == 0)
        parent = mainContainer();

    // You can only drop stuff onto the central widget of a QMainWindow
    // ### generalize to use container extension
    if (QMainWindow *main_win = qobject_cast<QMainWindow*>(target)) {
        QPoint main_win_pos = main_win->mapFromGlobal(global_mouse_pos);
        QRect central_wgt_geo = main_win->centralWidget()->geometry();
        if (!central_wgt_geo.contains(main_win_pos)) {
            foreach (QDesignerDnDItemInterface *item, item_list) {
                if (item->widget() != 0)
                    item->widget()->show();
            }
            return;
        }
    }

    core()->formWindowManager()->setActiveFormWindow(this);
    mainContainer()->activateWindow();
    clearSelection(false);

    highlightWidget(target, target->mapFromGlobal(global_mouse_pos), FormWindow::Restore);

    foreach (QDesignerDnDItemInterface *item, item_list) {
        DomUI *dom_ui = item->domUi();
        QRect geometry = item->decoration()->geometry();
        Q_ASSERT(dom_ui != 0);

        if (item->type() == QDesignerDnDItemInterface::CopyDrop) {
            QWidget *widget = createWidget(dom_ui, geometry, parent);
            if (!widget)
                return;
            selectWidget(widget, true);
            mainContainer()->setFocus(Qt::MouseFocusReason); // in case focus was in e.g. object inspector
        } else {
            QWidget *widget = item->widget();
            Q_ASSERT(widget != 0);
            QDesignerFormWindowInterface *dest = findFormWindow(widget);

            QWidget *container = findContainer(parent, false);
            QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core()->extensionManager(), container);

            if (dest == this) {
                if (deco == 0) {
                    parent = container;

                    if (parent != widget->parent()) {
                        ReparentWidgetCommand *cmd = new ReparentWidgetCommand(dest);
                        cmd->init(widget, parent);
                        commandHistory()->push(cmd);
                    }

                    geometry.moveTopLeft(parent->mapFromGlobal(geometry.topLeft()));
                    resizeWidget(widget, geometry);
                    selectWidget(widget, true);
                    widget->show();
                } else {
                    insertWidget(widget, geometry, container, true);
                }
            } else {
                FormWindow *source = qobject_cast<FormWindow*>(item->source());
                Q_ASSERT(source != 0);

                source->deleteWidgets(QList<QWidget*>() << widget);
                QWidget *new_widget = createWidget(dom_ui, geometry, parent);

                selectWidget(new_widget, true);
            }
        }
    }

    endCommand();
}

QDir FormWindow::absoluteDir() const
{
    if (fileName().isEmpty())
        return QDir::current();

    return QFileInfo(fileName()).absoluteDir();
}

void FormWindow::layoutDefault(int *margin, int *spacing)
{
    *margin = m_defaultMargin;
    *spacing = m_defaultSpacing;
}

void FormWindow::setLayoutDefault(int margin, int spacing)
{
    m_defaultMargin = margin;
    m_defaultSpacing = spacing;
}

void FormWindow::layoutFunction(QString *margin, QString *spacing)
{
    *margin = m_marginFunction;
    *spacing = m_spacingFunction;
}

void FormWindow::setLayoutFunction(const QString &margin, const QString &spacing)
{
    m_marginFunction = margin;
    m_spacingFunction = spacing;
}

QString FormWindow::pixmapFunction() const
{
    return m_pixmapFunction;
}

void FormWindow::setPixmapFunction(const QString &pixmapFunction)
{
    m_pixmapFunction = pixmapFunction;
}

QStringList FormWindow::includeHints() const
{
    return m_includeHints;
}

void FormWindow::setIncludeHints(const QStringList &includeHints)
{
    m_includeHints = includeHints;
}

QString FormWindow::exportMacro() const
{
    return m_exportMacro;
}

void FormWindow::setExportMacro(const QString &exportMacro)
{
    m_exportMacro = exportMacro;
}

} // namespace
