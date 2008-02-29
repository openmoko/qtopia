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

#include "default_actionprovider.h"
#include "invisible_widget_p.h"

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QToolBar>
#include <QtGui/QApplication>

#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

QDesignerActionProvider::QDesignerActionProvider(QWidget *widget, QObject *parent)
    : QObject(parent),
      m_widget(widget)
{
    Q_ASSERT(m_widget != 0);

    m_indicator = new InvisibleWidget(m_widget);
    m_indicator->setAutoFillBackground(true);
    m_indicator->setBackgroundRole(QPalette::Window);

    QPalette p;
    p.setColor(m_indicator->backgroundRole(), Qt::red);
    m_indicator->setPalette(p);
    m_indicator->hide();
}

QDesignerActionProvider::~QDesignerActionProvider()
{
}

QRect QDesignerActionProvider::actionGeometry(QAction *action) const
{
    if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(m_widget))
        return menuBar->actionGeometry(action);
    else if (QMenu *menu = qobject_cast<QMenu*>(m_widget))
        return menu->actionGeometry(action);
    else if (QToolBar *toolBar = qobject_cast<QToolBar*>(m_widget))
        return toolBar->actionGeometry(action);

    Q_ASSERT(0);
    return QRect();
}

QAction *QDesignerActionProvider::actionAt(const QPoint &pos) const
{
    if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(m_widget))
        return menuBar->actionAt(pos);
    else if (QMenu *menu = qobject_cast<QMenu*>(m_widget))
        return menu->actionAt(pos);
    else if (QToolBar *toolBar = qobject_cast<QToolBar*>(m_widget))
        return toolBar->actionAt(pos);

    Q_ASSERT(0);
    return 0;
}

void QDesignerActionProvider::adjustIndicator(const QPoint &pos)
{
    if (pos == QPoint(-1, -1)) {
        m_indicator->hide();
        return;
    }

    if (QAction *action = actionAt(pos)) {
        QRect g = actionGeometry(action);

        if (orientation() == Qt::Horizontal) {
            if (QApplication::layoutDirection() == Qt::LeftToRight)
                g.setRight(g.left() + 1);
            else
                g.setLeft(g.right() - 1);
        } else {
            g.setHeight(2);
        }

        m_indicator->setGeometry(g);

        QPalette p = m_indicator->palette();
        if (p.color(m_indicator->backgroundRole()) != Qt::red) {
            p.setColor(m_indicator->backgroundRole(), Qt::red);
            m_indicator->setPalette(p);
        }

        m_indicator->show();
        m_indicator->raise();
    } else {
        m_indicator->hide();
    }
}

Qt::Orientation QDesignerActionProvider::orientation() const
{
    if (QToolBar *toolBar = qobject_cast<QToolBar*>(m_widget)) {
        return toolBar->orientation();
    } else if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(m_widget)) {
        Q_UNUSED(menuBar);
        return Qt::Horizontal;
    }

    return Qt::Vertical;
}


// ---- QDesignerActionProviderFactory ----
QDesignerActionProviderFactory::QDesignerActionProviderFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerActionProviderFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerActionProviderExtension))
        return 0;

    if (qobject_cast<QMenu*>(object)
            || qobject_cast<QMenuBar*>(object)
            || qobject_cast<QToolBar*>(object))
        return new QDesignerActionProvider(qobject_cast<QWidget*>(object), parent);

    return 0;
}
