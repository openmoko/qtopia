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

#include "default_membersheet.h"

#include <QtGui/QWidget>
#include <QtCore/QVariant>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>
#include <QtCore/qdebug.h>

Q_GLOBAL_STATIC(QWidget, someWidget)

QDesignerMemberSheet::QDesignerMemberSheet(QObject *object, QObject *parent)
    : QObject(parent),
      m_object(object),
      meta(object->metaObject())
{
}

QDesignerMemberSheet::~QDesignerMemberSheet()
{
}

int QDesignerMemberSheet::count() const
{
    return meta->methodCount();
}

int QDesignerMemberSheet::indexOf(const QString &name) const
{
    return meta->indexOfMethod(name.toUtf8());
}

QString QDesignerMemberSheet::memberName(int index) const
{
    return QString::fromUtf8(meta->method(index).tag());
}

QString QDesignerMemberSheet::declaredInClass(int index) const
{
    const char *member = meta->method(index).signature();

    const QMetaObject *meta_obj = meta;

    for (;;) {
        const QMetaObject *tmp = meta_obj->superClass();
        if (tmp == 0)
            break;
        if (tmp->indexOfMethod(member) == -1)
            break;
        meta_obj = tmp;
    }

    return QLatin1String(meta_obj->className());
}

QString QDesignerMemberSheet::memberGroup(int index) const
{
    return m_info.value(index).group;
}

void QDesignerMemberSheet::setMemberGroup(int index, const QString &group)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].group = group;
}

QString QDesignerMemberSheet::signature(int index) const
{
    return QString::fromUtf8(QMetaObject::normalizedSignature(meta->method(index).signature()));
}

bool QDesignerMemberSheet::isVisible(int index) const
{
    if (m_info.contains(index))
        return m_info.value(index).visible;

   return meta->method(index).methodType() == QMetaMethod::Signal
           || meta->method(index).access() == QMetaMethod::Public;
}

void QDesignerMemberSheet::setVisible(int index, bool visible)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].visible = visible;
}

bool QDesignerMemberSheet::isSignal(int index) const
{
    return meta->method(index).methodType() == QMetaMethod::Signal;
}

bool QDesignerMemberSheet::isSlot(int index) const
{
    return meta->method(index).methodType() == QMetaMethod::Slot;
}

bool QDesignerMemberSheet::inheritedFromWidget(int index) const
{
    const char *name = meta->method(index).signature();
    return someWidget()->metaObject()->indexOfMethod(name) != -1;
}


QList<QByteArray> QDesignerMemberSheet::parameterTypes(int index) const
{
    return meta->method(index).parameterTypes();
}

QList<QByteArray> QDesignerMemberSheet::parameterNames(int index) const
{
    return meta->method(index).parameterNames();
}

QDesignerMemberSheetFactory::QDesignerMemberSheetFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerMemberSheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid == Q_TYPEID(QDesignerMemberSheetExtension))
        return new QDesignerMemberSheet(object, parent);

    return 0;
}


