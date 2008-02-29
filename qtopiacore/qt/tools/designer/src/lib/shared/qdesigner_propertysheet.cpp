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

#include "qdesigner_propertysheet_p.h"
#include "qdesigner_utils_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtCore/QVariant>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>

#include <QtGui/QLayout>
#include <QtGui/QImage>
#include <QtGui/QPixmap>

#include <QtGui/QDockWidget>
#include <QtGui/QDialog>
#include <QtGui/QStackedWidget>
#include <QtGui/QToolBar>
#include <QtGui/QStatusBar>
#include <QtGui/QLabel>
#include <QtGui/QCalendarWidget>
#include <QtGui/QDialogButtonBox>

namespace qdesigner_internal {

static const QMetaObject *introducedBy(const QMetaObject *meta, int index)
{
    if (index >= meta->propertyOffset())
        return meta;

    if (meta->superClass())
        return introducedBy(meta->superClass(), index);

    return 0;
}

} // qdesigner_internal

using namespace qdesigner_internal;

static bool hasLayoutAttributes(QObject *object)
{
    if (qobject_cast<QStackedWidget*>(object) != 0)
        return false;
    if (qobject_cast<QLabel*>(object) != 0)
        return false;
    if (qobject_cast<QDockWidget*>(object) != 0)
        return false;
    if (qobject_cast<QToolBar*>(object) != 0)
        return false;
    if (qobject_cast<QStatusBar*>(object) != 0)
        return false;
    if (qobject_cast<QCalendarWidget*>(object) != 0)
        return false;
    if (qobject_cast<QDialogButtonBox*>(object) != 0)
        return false;
    return true;
}

QDesignerPropertySheet::QDesignerPropertySheet(QObject *object, QObject *parent)
    : QObject(parent),
      m_object(object),
      meta(object->metaObject())
{
    const QMetaObject *baseMeta = meta;

    while (baseMeta && QString::fromUtf8(baseMeta->className()).startsWith(QLatin1String("QDesigner"))) {
        baseMeta = baseMeta->superClass();
    }
    Q_ASSERT(baseMeta != 0);

    for (int index=0; index<count(); ++index) {
        QMetaProperty p = meta->property(index);

        if (p.type() == QVariant::KeySequence)
            createFakeProperty(QString::fromUtf8(p.name()));
        else
            setVisible(index, false); // use the default for `real' properties

        QString pgroup = QString::fromUtf8(baseMeta->className());

        if (const QMetaObject *pmeta = qdesigner_internal::introducedBy(baseMeta, index)) {
            pgroup = QString::fromUtf8(pmeta->className());
        }

        setPropertyGroup(index, pgroup);
    }

    if (object->isWidgetType()) {
        createFakeProperty(QLatin1String("focusPolicy"));
        createFakeProperty(QLatin1String("cursor"));
        createFakeProperty(QLatin1String("toolTip"));
        createFakeProperty(QLatin1String("whatsThis"));
        createFakeProperty(QLatin1String("acceptDrops"));
        createFakeProperty(QLatin1String("dragEnabled"));
        createFakeProperty(QLatin1String("windowModality"));
        createFakeProperty(QLatin1String("autoFillBackground"));

        if (hasLayoutAttributes(object)) {
            int pindex = -1;

            pindex = count();
            createFakeProperty(QLatin1String("margin"), 0);
            setAttribute(pindex, true);
            setPropertyGroup(pindex, tr("Layout"));

            pindex = count();
            createFakeProperty(QLatin1String("spacing"), 0);
            setAttribute(pindex, true);
            setPropertyGroup(pindex, tr("Layout"));
        }
    }

    if (qobject_cast<QDialog*>(object)) {
        createFakeProperty(QLatin1String("modal"));
    }
    if (qobject_cast<QDockWidget*>(object)) {
        createFakeProperty(QLatin1String("floating"));
    }
}

QDesignerPropertySheet::~QDesignerPropertySheet()
{
}

void QDesignerPropertySheet::createFakeProperty(const QString &propertyName, const QVariant &value)
{
    // fake properties
    int index = meta->indexOfProperty(propertyName.toUtf8());
    if (index != -1) {
        setVisible(index, false);
        QVariant v = value.isValid() ? value : metaProperty(index);
        m_fakeProperties.insert(index, v);
    } else if (value.isValid()) { // additional properties
        int index = count();
        m_addIndex.insert(propertyName, index);
        m_addProperties.insert(index, value);
    }
}

bool QDesignerPropertySheet::isAdditionalProperty(int index) const
{
    return m_addProperties.contains(index);
}

bool QDesignerPropertySheet::isFakeProperty(int index) const
{
    // additional properties must be fake
    return (m_fakeProperties.contains(index) || isAdditionalProperty(index));
}

int QDesignerPropertySheet::count() const
{
    return meta->propertyCount() + m_addProperties.count();
}

int QDesignerPropertySheet::indexOf(const QString &name) const
{
    int index = meta->indexOfProperty(name.toUtf8());

    if (index == -1)
        index = m_addIndex.value(name, -1);

    return index;
}

QString QDesignerPropertySheet::propertyName(int index) const
{
    if (isAdditionalProperty(index))
        return m_addIndex.key(index);

    return QString::fromUtf8(meta->property(index).name());
}

QString QDesignerPropertySheet::propertyGroup(int index) const
{
    QString g = m_info.value(index).group;

    if (!g.isEmpty())
        return g;

    if (propertyName(index).startsWith(QLatin1String("accessible")))
        return QString::fromUtf8("Accessibility");

    if (isAdditionalProperty(index))
        return QString::fromUtf8(meta->className());

    return g;
}

void QDesignerPropertySheet::setPropertyGroup(int index, const QString &group)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].group = group;
}

QVariant QDesignerPropertySheet::property(int index) const
{
    if (isAdditionalProperty(index)) {
        if (isFakeLayoutProperty(index) && m_object->isWidgetType()) {
            QWidget *widget = qobject_cast<QWidget*>(m_object);
            if (QLayout *l = widget->layout())
                return l->property(propertyName(index).toUtf8());
        }

        return m_addProperties.value(index);
    }

    if (isFakeProperty(index)) {
        return m_fakeProperties.value(index);
    }

    QMetaProperty p = meta->property(index);
    QVariant v = p.read(m_object);

    if (p.isFlagType()) {
        FlagType e;
        e.value = v;
        QMetaEnum me = p.enumerator();
        for (int i=0; i<me.keyCount(); ++i) {
            QString k = QString::fromUtf8(me.scope());
            k += QString::fromUtf8("::");
            k += QLatin1String(me.key(i));
            e.items.insert(k, me.keyToValue(k.toUtf8()));
        }

        qVariantSetValue(v, e);
    } else if (p.isEnumType()) {
        EnumType e;
        e.value = v;
        QMetaEnum me = p.enumerator();
        QString scope = QString::fromUtf8(me.scope());
        if (!scope.isEmpty())
            scope += QString::fromUtf8("::");
        for (int i=0; i<me.keyCount(); ++i) {
            QString key = scope + QLatin1String(me.key(i));
            e.items.insert(key, me.keyToValue(key.toUtf8()));
            e.names.append(key);
        }

        qVariantSetValue(v, e);
    }

    return v;
}

QVariant QDesignerPropertySheet::metaProperty(int index) const
{
    Q_ASSERT(!isFakeProperty(index));

    QMetaProperty p = meta->property(index);
    QVariant v = p.read(m_object);

    if (p.isFlagType()) {
        FlagType e;
        e.value = v;
        QMetaEnum me = p.enumerator();
        for (int i=0; i<me.keyCount(); ++i) {
            QString key;
            key += QLatin1String(me.scope());
            key += QLatin1String("::");
            key += QLatin1String(me.key(i));

            e.items.insert(key, me.keyToValue(key.toUtf8()));
        }

        qVariantSetValue(v, e);
    } else if (p.isEnumType()) {
        EnumType e;
        e.value = v;
        QMetaEnum me = p.enumerator();
        QString scope = QString::fromUtf8(me.scope());
        if (!scope.isEmpty())
            scope += QString::fromUtf8("::");
        for (int i=0; i<me.keyCount(); ++i) {
            QString key = scope + QLatin1String(me.key(i));
            e.items.insert(key, me.keyToValue(key.toUtf8()));
            e.names.append(key);
        }

        qVariantSetValue(v, e);
    }

    return v;
}

QVariant QDesignerPropertySheet::resolvePropertyValue(const QVariant &value) const
{
    QVariant v;
    EnumType e;
    FlagType f;

    if (qVariantCanConvert<FlagType>(value))
        v = qvariant_cast<FlagType>(value).value;
    else if (qVariantCanConvert<EnumType>(value))
        v = qvariant_cast<EnumType>(value).value;
    else
        v = value;

    return v;
}

void QDesignerPropertySheet::setFakeProperty(int index, const QVariant &value)
{
    Q_ASSERT(isFakeProperty(index));

    QVariant &v = m_fakeProperties[index];

    if (qVariantCanConvert<FlagType>(value) || qVariantCanConvert<EnumType>(value)) {
        v = value;
    } else if (qVariantCanConvert<FlagType>(v)) {
        FlagType f = qvariant_cast<FlagType>(v);
        f.value = value;
        qVariantSetValue(v, f);
        Q_ASSERT(f.value.type() == QVariant::Int);
    } else if (qVariantCanConvert<EnumType>(v)) {
        EnumType e = qvariant_cast<EnumType>(v);
        e.value = value;
        qVariantSetValue(v, e);
        Q_ASSERT(e.value.type() == QVariant::Int);
    } else {
        v = value;
    }
}

void QDesignerPropertySheet::setProperty(int index, const QVariant &value)
{
    if (isAdditionalProperty(index)) {
        if (isFakeLayoutProperty(index) && m_object->isWidgetType()) {
            QWidget *widget = qobject_cast<QWidget*>(m_object);
            if (QLayout *l = widget->layout()) {
                l->setProperty(propertyName(index).toUtf8(), value);
                return;
            }
        }

        m_addProperties[index] = value;
    } else if (isFakeProperty(index)) {
        setFakeProperty(index, value);
    } else {
        QMetaProperty p = meta->property(index);
        p.write(m_object, resolvePropertyValue(value));
    }
}

bool QDesignerPropertySheet::hasReset(int index) const
{
    if (isAdditionalProperty(index))
        return m_info.value(index).reset;

    return true;
}

bool QDesignerPropertySheet::reset(int index)
{
    if (isAdditionalProperty(index))
        return false;
    else if (isFakeProperty(index)) {
        QMetaProperty p = meta->property(index);
        bool result = p.reset(m_object);
        m_fakeProperties[index] = p.read(m_object);
        return result;
    }

    // ### TODO: reset for fake properties.

    QMetaProperty p = meta->property(index);
    return p.reset(m_object);
}

bool QDesignerPropertySheet::isChanged(int index) const
{
    return m_info.value(index).changed;
}

void QDesignerPropertySheet::setChanged(int index, bool changed)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].changed = changed;
}

bool QDesignerPropertySheet::isFakeLayoutProperty(int index) const
{
    if (!isAdditionalProperty(index))
        return false;

    QString pname = propertyName(index);

    if (pname == QLatin1String("margin")
            || pname == QLatin1String("spacing")
            || pname == QLatin1String("sizeConstraint"))
        return true;

    return false;
}

bool QDesignerPropertySheet::isVisible(int index) const
{
    if (isAdditionalProperty(index)) {
        if (isFakeLayoutProperty(index) && m_object->isWidgetType()) {
            QWidget *widget = qobject_cast<QWidget*>(m_object);
            return (widget->layout() != 0);
        }

        return m_info.value(index).visible;
    }

    if (isFakeProperty(index))
        return true;

    QMetaProperty p = meta->property(index);
    return (p.isWritable() && p.isDesignable(m_object)) || m_info.value(index).visible;
}

void QDesignerPropertySheet::setVisible(int index, bool visible)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].visible = visible;
}

bool QDesignerPropertySheet::isAttribute(int index) const
{
    if (isAdditionalProperty(index))
        return m_info.value(index).attribute;

    if (isFakeProperty(index))
        return false;

    return m_info.value(index).attribute;
}

void QDesignerPropertySheet::setAttribute(int index, bool attribute)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].attribute = attribute;
}


QDesignerPropertySheetFactory::QDesignerPropertySheetFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerPropertySheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid == Q_TYPEID(QDesignerPropertySheetExtension))
        return new QDesignerPropertySheet(object, parent);

    return 0;
}

