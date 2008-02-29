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

#include "default_container.h"
#include "qdesigner_stackedbox_p.h"
#include "qdesigner_tabwidget_p.h"
#include "qdesigner_toolbox_p.h"

using namespace qdesigner_internal;

QDesignerContainer::QDesignerContainer(QWidget *widget, QObject *parent)
    : QObject(parent),
      m_widget(widget)
{
}

QDesignerContainer::~QDesignerContainer()
{
}

int QDesignerContainer::count() const
{
    if (QDesignerStackedWidget *stackedWidget = qobject_cast<QDesignerStackedWidget*>(m_widget)) {
        return stackedWidget->count();
    } else if (QDesignerTabWidget *tabWidget = qobject_cast<QDesignerTabWidget*>(m_widget)) {
        return tabWidget->count();
    } else if (QDesignerToolBox *toolBox = qobject_cast<QDesignerToolBox*>(m_widget)) {
        return toolBox->count();
    }

    Q_ASSERT(0);
    return 0;
}

QWidget *QDesignerContainer::widget(int index) const
{
    if (QDesignerStackedWidget *stackedWidget = qobject_cast<QDesignerStackedWidget*>(m_widget))
        return stackedWidget->widget(index);
    else if (QDesignerTabWidget *tabWidget = qobject_cast<QDesignerTabWidget*>(m_widget))
        return tabWidget->widget(index);
    else if (QDesignerToolBox *toolBox = qobject_cast<QDesignerToolBox*>(m_widget))
        return toolBox->widget(index);

    Q_ASSERT(0);
    return 0;
}

int QDesignerContainer::currentIndex() const
{
    if (qobject_cast<QDesignerStackedWidget*>(m_widget))
        return static_cast<QDesignerStackedWidget*>(m_widget)->currentIndex();
    else if (qobject_cast<QDesignerTabWidget*>(m_widget))
        return static_cast<QDesignerTabWidget*>(m_widget)->currentIndex();
    else if (qobject_cast<QDesignerToolBox*>(m_widget))
        return static_cast<QDesignerToolBox*>(m_widget)->currentIndex();

    Q_ASSERT(0);
    return -1;
}

void QDesignerContainer::setCurrentIndex(int index)
{
    if (qobject_cast<QDesignerStackedWidget*>(m_widget))
        static_cast<QDesignerStackedWidget*>(m_widget)->setCurrentIndex(index);
    else if (qobject_cast<QDesignerTabWidget*>(m_widget))
        static_cast<QDesignerTabWidget*>(m_widget)->setCurrentIndex(index);
    else if (qobject_cast<QDesignerToolBox*>(m_widget))
        static_cast<QDesignerToolBox*>(m_widget)->setCurrentIndex(index);
    else
        Q_ASSERT(0);
}

void QDesignerContainer::addWidget(QWidget *widget)
{
    if (widget->parentWidget())
        widget->setParent(0);

    if (qobject_cast<QDesignerStackedWidget*>(m_widget))
        static_cast<QDesignerStackedWidget*>(m_widget)->addWidget(widget);
    else if (qobject_cast<QDesignerTabWidget*>(m_widget))
        static_cast<QDesignerTabWidget*>(m_widget)->addTab(widget, QString::fromUtf8("Page"));
    else if (qobject_cast<QDesignerToolBox*>(m_widget))
        static_cast<QDesignerToolBox*>(m_widget)->addItem(widget, QString::fromUtf8("Page"));
    else
        Q_ASSERT(0);
}

void QDesignerContainer::insertWidget(int index, QWidget *widget)
{
    if (widget->parentWidget())
        widget->setParent(0);

    if (qobject_cast<QDesignerStackedWidget*>(m_widget))
        static_cast<QDesignerStackedWidget*>(m_widget)->insertWidget(index, widget);
    else if (qobject_cast<QDesignerTabWidget*>(m_widget))
        static_cast<QDesignerTabWidget*>(m_widget)->insertTab(index, widget, QString::fromUtf8("Page"));
    else if (qobject_cast<QDesignerToolBox*>(m_widget))
        static_cast<QDesignerToolBox*>(m_widget)->insertItem(index, widget, QString::fromUtf8("Page"));
    else
        Q_ASSERT(0);
}

void QDesignerContainer::remove(int index)
{
    if (qobject_cast<QDesignerStackedWidget*>(m_widget))
        static_cast<QDesignerStackedWidget*>(m_widget)->removeWidget(widget(index));
    else if (qobject_cast<QDesignerTabWidget*>(m_widget))
        static_cast<QDesignerTabWidget*>(m_widget)->removeTab(index);
    else if (qobject_cast<QDesignerToolBox*>(m_widget))
        static_cast<QDesignerToolBox*>(m_widget)->removeItem(index);
    else
        Q_ASSERT(0);
}

QDesignerContainerFactory::QDesignerContainerFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerContainerFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerContainerExtension))
        return 0;

    if (qobject_cast<QDesignerStackedWidget*>(object)
            || qobject_cast<QDesignerTabWidget*>(object)
            || qobject_cast<QDesignerToolBox*>(object))
        return new QDesignerContainer(static_cast<QWidget*>(object), parent);

    return 0;
}
