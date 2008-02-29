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

#include "q3widgetstack_container.h"
#include "qdesigner_q3widgetstack_p.h"

#include <QtCore/qdebug.h>

Q3WidgetStackContainer::Q3WidgetStackContainer(QDesignerQ3WidgetStack *widget, QObject *parent)
    : QObject(parent),
      m_widget(widget)
{}

int Q3WidgetStackContainer::count() const
{ return m_pages.count(); }

QWidget *Q3WidgetStackContainer::widget(int index) const
{
    if (index == -1)
        return 0;

    return m_pages.at(index);
}

int Q3WidgetStackContainer::currentIndex() const
{ return m_pages.indexOf(m_widget->visibleWidget()); }

void Q3WidgetStackContainer::setCurrentIndex(int index)
{ m_widget->raiseWidget(m_pages.at(index)); }

void Q3WidgetStackContainer::addWidget(QWidget *widget)
{
    m_pages.append(widget);
    m_widget->addWidget(widget);
}

void Q3WidgetStackContainer::insertWidget(int index, QWidget *widget)
{
    m_pages.insert(index, widget);
    m_widget->addWidget(widget);
    m_widget->setCurrentIndex(index);
}

void Q3WidgetStackContainer::remove(int index)
{
    int current = currentIndex();
    m_widget->removeWidget(m_pages.at(index));
    m_pages.removeAt(index);
    if (index == current) {
        if (count() > 0)
            m_widget->setCurrentIndex((index == count()) ? index-1 : index);
    } else if (index < current) {
        if (current > 0)
            m_widget->setCurrentIndex(current-1);
    }
}

Q3WidgetStackContainerFactory::Q3WidgetStackContainerFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *Q3WidgetStackContainerFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerContainerExtension))
        return 0;

    if (QDesignerQ3WidgetStack *w = qobject_cast<QDesignerQ3WidgetStack*>(object))
        return new Q3WidgetStackContainer(w, parent);

    return 0;
}

