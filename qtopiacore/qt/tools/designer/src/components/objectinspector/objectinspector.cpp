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
TRANSLATOR qdesigner_internal::ObjectInspector
*/

#include "objectinspector.h"

// sdk
#include <QtDesigner/QtDesigner>

// shared
#include <tree_widget_p.h>
#include <qdesigner_promotedwidget_p.h>

// Qt
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QApplication>
#include <QtGui/QHeaderView>
#include <QtGui/QScrollBar>
#include <QtGui/QItemDelegate>
#include <QtGui/QPainter>
#include <QtGui/QVBoxLayout>

#include <QtCore/QStack>
#include <QtCore/QPair>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

ObjectInspector::ObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent)
    : QDesignerObjectInspectorInterface(parent),
      m_core(core)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setMargin(0);

    m_treeWidget = new TreeWidget(this);
    vbox->addWidget(m_treeWidget);

    m_treeWidget->setColumnCount(2);
    m_treeWidget->headerItem()->setText(0, tr("Object"));
    m_treeWidget->headerItem()->setText(1, tr("Class"));

    m_treeWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_treeWidget->header()->setResizeMode(1, QHeaderView::Stretch);

    m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotPopupContextMenu(QPoint)));

    connect(m_treeWidget, SIGNAL(itemPressed(QTreeWidgetItem*,int)),
            this, SLOT(slotSelectionChanged()));

    connect(m_treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotSelectionChanged()));
}

ObjectInspector::~ObjectInspector()
{
}

QDesignerFormEditorInterface *ObjectInspector::core() const
{
    return m_core;
}

void ObjectInspector::slotPopupContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = m_treeWidget->itemAt(pos);
    if (!item)
        return;

    QObject *object = qvariant_cast<QObject *>(item->data(0, 1000));
    if (!object)
        return;

#if defined(TASKMENU_INTEGRATION)
    QDesignerTaskMenuExtension *task;

    if (0 != (task = qt_extension<QDesignerTaskMenuExtension*>(core()->extensionManager(), object))) {
        QList<QAction*> actions = task->taskActions();

        if (!actions.isEmpty()) {
            QMenu menu(this);

            menu.addActions(actions);
            menu.exec(m_treeWidget->viewport()->mapToGlobal(pos));
        }
    }
#endif
}

bool ObjectInspector::sortEntry(const QObject *a, const QObject *b)
{
    return a->objectName() < b->objectName();
}

void ObjectInspector::setFormWindow(QDesignerFormWindowInterface *fw)
{
    m_formWindow = fw;

    if (fw && fw->cursor())
        m_selected = core()->propertyEditor()->object();

    int xoffset = m_treeWidget->horizontalScrollBar()->value();
    int yoffset = m_treeWidget->verticalScrollBar()->value();

    m_treeWidget->clear();

    if (!fw || !fw->mainContainer())
        return;

    QDesignerWidgetDataBaseInterface *db = fw->core()->widgetDataBase();

    m_treeWidget->setUpdatesEnabled(false);

    QStack< QPair<QTreeWidgetItem*, QObject*> > workingList;
    QObject *rootObject = fw->mainContainer();
    workingList.append(qMakePair(new QTreeWidgetItem(m_treeWidget), rootObject));
    QTreeWidgetItem *theSelectedItem = 0;

    while (!workingList.isEmpty()) {
        QTreeWidgetItem *item = workingList.top().first;
        QObject *object = workingList.top().second;
        workingList.pop();

        if (m_selected == object)
            theSelectedItem = item;

        QString className = object->metaObject()->className();
        if (QDesignerWidgetDataBaseItemInterface *widgetItem = db->item(db->indexOfObject(object, true))) {
            className = widgetItem->name();

            if (object->isWidgetType() && className == QLatin1String("QLayoutWidget")
                    && static_cast<QWidget*>(object)->layout()) {
                className = QLatin1String(static_cast<QWidget*>(object)->layout()->metaObject()->className());
            }

            item->setIcon(0, widgetItem->icon());
        }

        if (className.startsWith("QDesigner"))
            className.remove(1, 8);

        item->setText(1, className);

        item->setData(0, 1000, qVariantFromValue(object));

        if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(object))
            object = promoted->child();

        QString objectName = object->objectName();
        if (objectName.isEmpty())
            objectName = tr("<noname>");

        if (QAction *act = qobject_cast<QAction*>(object)) { // separator is reserved
            if (act->isSeparator()) {
                objectName = tr("separator");
            }
            item->setIcon(0, act->icon());
        }

        item->setText(0, objectName);

        if (QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension*>(fw->core()->extensionManager(), object)) {
            for (int i=0; i<c->count(); ++i) {
                QObject *page = c->widget(i);
                Q_ASSERT(page != 0);

                QTreeWidgetItem *pageItem = new QTreeWidgetItem(item);
                workingList.append(qMakePair(pageItem, page));
            }
        } else {
            QList<QObject*> children;
            if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(object))
                children = promoted->child()->children();
            else
                children = object->children();

            qSort(children.begin(), children.end(), ObjectInspector::sortEntry);

            foreach (QObject *child, children) {
                QWidget *widget = qobject_cast<QWidget*>(child);
                if (!widget || !fw->isManaged(widget))
                    continue;

                QTreeWidgetItem *childItem = new QTreeWidgetItem(item);
                workingList.append(qMakePair(childItem, child));
            }

            if (QWidget *widget = qobject_cast<QWidget*>(object)) {
                QList<QAction*> actions = widget->actions();
                foreach (QAction *action, actions) {
                    if (!fw->core()->metaDataBase()->item(action))
                        continue;

                    QObject *obj = action;
                    if (action->menu())
                        obj = action->menu();

                    QTreeWidgetItem *childItem = new QTreeWidgetItem(item);
                    workingList.append(qMakePair(childItem, obj));
                }
            }
        }

        m_treeWidget->expandItem(item);
    }

    m_treeWidget->horizontalScrollBar()->setValue(xoffset);
    m_treeWidget->verticalScrollBar()->setValue(yoffset);

    if (theSelectedItem) {
        m_treeWidget->setCurrentItem(theSelectedItem);
        m_treeWidget->scrollToItem(theSelectedItem);
    }

    m_treeWidget->setUpdatesEnabled(true);
    m_treeWidget->update();

    m_treeWidget->resizeColumnToContents(0);
}

void ObjectInspector::slotSelectionChanged()
{
    if (!m_formWindow)
        return;

    m_formWindow->clearSelection(false);

    QList<QTreeWidgetItem*> items = m_treeWidget->selectedItems();

    foreach (QTreeWidgetItem *item, items) {
        QObject *object = qvariant_cast<QObject *>(item->data(0, 1000));
        m_selected = object;

        QWidget *widget = qobject_cast<QWidget*>(object);

        if (widget && m_formWindow->isManaged(widget)) {
            m_formWindow->selectWidget(widget);
        } else if (core()->metaDataBase()->item(object)) {
            // refresh at least the property editor
            core()->propertyEditor()->setObject(object);
        }
    }
    QMetaObject::invokeMethod(m_formWindow->core()->formWindowManager(), "slotUpdateActions");
}

void ObjectInspector::showEvent(QShowEvent *event)
{
    m_treeWidget->resizeColumnToContents(0);
    QDesignerObjectInspectorInterface::showEvent(event);
}
