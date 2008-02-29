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

#include "qdockwidget_container.h"

#include <QtCore/qdebug.h>

#include <QtGui/QDockWidget>
#include <QtGui/QMenuBar>
#include <QtGui/QToolBar>
#include <QtGui/QStatusBar>

using namespace qdesigner_internal;

QDockWidgetContainer::QDockWidgetContainer(QDockWidget *widget, QObject *parent)
    : QObject(parent),
      m_dockWidget(widget)
{
    Q_ASSERT(m_dockWidget->widget() == 0);
}

int QDockWidgetContainer::count() const
{
    return m_dockWidget->widget() ? 1 : 0;
}

QWidget *QDockWidgetContainer::widget(int index) const
{
    Q_UNUSED(index);

    return m_dockWidget->widget();
}

int QDockWidgetContainer::currentIndex() const
{
    return m_dockWidget->widget() ? 0 : -1;
}

void QDockWidgetContainer::setCurrentIndex(int index)
{
    Q_UNUSED(index);
}

void QDockWidgetContainer::addWidget(QWidget *widget)
{
    Q_ASSERT(m_dockWidget->widget() == 0);

    widget->setParent(m_dockWidget);
    m_dockWidget->setWidget(widget);
}

void QDockWidgetContainer::insertWidget(int index, QWidget *widget)
{
    Q_UNUSED(index);

    addWidget(widget);
}

void QDockWidgetContainer::remove(int index)
{
    Q_UNUSED(index);
}

QDockWidgetContainerFactory::QDockWidgetContainerFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDockWidgetContainerFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerContainerExtension))
        return 0;

    if (QDockWidget *w = qobject_cast<QDockWidget*>(object))
        return new QDockWidgetContainer(w, parent);

    return 0;
}

