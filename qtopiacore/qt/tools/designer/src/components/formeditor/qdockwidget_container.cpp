/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

