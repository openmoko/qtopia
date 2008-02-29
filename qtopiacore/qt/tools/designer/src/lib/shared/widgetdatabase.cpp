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

#include "widgetdatabase_p.h"
#include "widgetfactory_p.h"
#include "spacer_widget_p.h"
#include "abstractlanguage.h"

#include <pluginmanager_p.h>
#include <QtDesigner/customwidget.h>
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/QExtensionManager>

#include <qalgorithms.h>
#include <QtCore/qdebug.h>
#include <QtCore/QMetaProperty>

namespace qdesigner_internal {

// ----------------------------------------------------------
WidgetDataBaseItem::WidgetDataBaseItem(const QString &name, const QString &group)
    : m_name(name),
      m_group(group),
      m_compat(0),
      m_container(0),
      m_form(0),
      m_custom(0),
      m_promoted(0)
{
}

QString WidgetDataBaseItem::name() const
{
    return m_name;
}

void WidgetDataBaseItem::setName(const QString &name)
{
    m_name = name;
}

QString WidgetDataBaseItem::group() const
{
    return m_group;
}

void WidgetDataBaseItem::setGroup(const QString &group)
{
    m_group = group;
}

QString WidgetDataBaseItem::toolTip() const
{
    return m_toolTip;
}

void WidgetDataBaseItem::setToolTip(const QString &toolTip)
{
    m_toolTip = toolTip;
}

QString WidgetDataBaseItem::whatsThis() const
{
    return m_whatsThis;
}

void WidgetDataBaseItem::setWhatsThis(const QString &whatsThis)
{
    m_whatsThis = whatsThis;
}

QString WidgetDataBaseItem::includeFile() const
{
    return m_includeFile;
}

void WidgetDataBaseItem::setIncludeFile(const QString &includeFile)
{
    m_includeFile = includeFile;
}

QIcon WidgetDataBaseItem::icon() const
{
    return m_icon;
}

void WidgetDataBaseItem::setIcon(const QIcon &icon)
{
    m_icon = icon;
}

bool WidgetDataBaseItem::isCompat() const
{
    return m_compat;
}

void WidgetDataBaseItem::setCompat(bool b)
{
    m_compat = b;
}

bool WidgetDataBaseItem::isContainer() const
{
    return m_container;
}

void WidgetDataBaseItem::setContainer(bool b)
{
    m_container = b;
}

bool WidgetDataBaseItem::isCustom() const
{
    return m_custom;
}

void WidgetDataBaseItem::setCustom(bool b)
{
    m_custom = b;
}

QString WidgetDataBaseItem::pluginPath() const
{
    return m_pluginPath;
}

void WidgetDataBaseItem::setPluginPath(const QString &path)
{
    m_pluginPath = path;
}

bool WidgetDataBaseItem::isPromoted() const
{
    return m_promoted;
}

void WidgetDataBaseItem::setPromoted(bool b)
{
    m_promoted = b;
}

QString WidgetDataBaseItem::extends() const
{
    return m_extends;
}

void WidgetDataBaseItem::setExtends(const QString &s)
{
    m_extends = s;
}

void WidgetDataBaseItem::setDefaultPropertyValues(const QList<QVariant> &list)
{
    m_defaultPropertyValues = list;
}

QList<QVariant> WidgetDataBaseItem::defaultPropertyValues() const
{
    return m_defaultPropertyValues;
}

// ----------------------------------------------------------
WidgetDataBase::WidgetDataBase(QDesignerFormEditorInterface *core, QObject *parent)
    : QDesignerWidgetDataBaseInterface(parent),
      m_core(core)
{
#define DECLARE_LAYOUT(L, C)
#define DECLARE_COMPAT_WIDGET(W, C) DECLARE_WIDGET(W, C)
#define DECLARE_WIDGET(W, C) append(new WidgetDataBaseItem(QString::fromUtf8(#W)));

#include "widgets.table"

#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_LAYOUT
#undef DECLARE_WIDGET
#undef DECLARE_WIDGET_1

    append(new WidgetDataBaseItem(QString::fromUtf8("Line")));
    append(new WidgetDataBaseItem(QString::fromUtf8("Spacer")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QSplitter")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QLayoutWidget")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerWidget")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerDialog")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerToolBar")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerMenu")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerMenuBar")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerDockWidget")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerQ3WidgetStack")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QAction")));

    // ### remove me
    // ### check the casts

#if 0 // ### enable me after 4.1
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QToolBar"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDesignerToolBar"))))->setContainer(true);
#endif

    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QTabWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QGroupBox"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QStackedWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QToolBox"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QFrame"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QLayoutWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDesignerWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDesignerDialog"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QSplitter"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QMainWindow"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDockWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDesignerDockWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDesignerQ3WidgetStack"))))->setContainer(true);

    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDialog"))))->setContainer(true);
}

WidgetDataBase::~WidgetDataBase()
{
}

QDesignerFormEditorInterface *WidgetDataBase::core() const
{
    return m_core;
}

int WidgetDataBase::indexOfObject(QObject *object, bool /*resolveName*/) const
{
    QExtensionManager *mgr = m_core->extensionManager();
    QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*> (mgr, m_core);

    QString id;

    if (lang)
        id = lang->classNameOf(object);

    if (id.isEmpty())
        id = QString::fromUtf8 (WidgetFactory::classNameOf(object));

    return QDesignerWidgetDataBaseInterface::indexOfClassName(id);
}

QDesignerWidgetDataBaseItemInterface *WidgetDataBase::item(int index) const
{
    return QDesignerWidgetDataBaseInterface::item(index);
}

void WidgetDataBase::loadPlugins()
{
    QDesignerPluginManager *pluginManager = m_core->pluginManager();

    QStringList plugins = pluginManager->registeredPlugins();

    QMutableListIterator<QDesignerWidgetDataBaseItemInterface *> it(m_items);
    while (it.hasNext()) {
        QDesignerWidgetDataBaseItemInterface *item = it.next();

        if (item->isCustom()) {
            it.remove();
            delete item;
        }
    }

    pluginManager->ensureInitialized();

    foreach (QString plugin, plugins) {
        QObject *o = pluginManager->instance(plugin);

        if (QDesignerCustomWidgetInterface *c = qobject_cast<QDesignerCustomWidgetInterface*>(o)) {
            WidgetDataBaseItem *item = createCustomWidgetItem(c);
            item->setPluginPath(plugin);
            append(item);
        } else if (QDesignerCustomWidgetCollectionInterface *coll = qobject_cast<QDesignerCustomWidgetCollectionInterface*>(o)) {
            foreach (QDesignerCustomWidgetInterface *c, coll->customWidgets()) {
                WidgetDataBaseItem *item = createCustomWidgetItem(c);
                item->setPluginPath(plugin);
                append(item);
            }
        }
    }
}

WidgetDataBaseItem *WidgetDataBase::createCustomWidgetItem(QDesignerCustomWidgetInterface *c) const
{
    WidgetDataBaseItem *item = new WidgetDataBaseItem();
    item->setContainer(c->isContainer());
    item->setCustom(true);
    item->setGroup(c->group());
    item->setIcon(c->icon());
    item->setIncludeFile(c->includeFile());
    item->setName(c->name());
    item->setToolTip(c->toolTip());
    item->setWhatsThis(c->whatsThis());

    return item;
}

QList<QVariant> WidgetDataBase::defaultPropertyValues(const QString &name)
{
    QList<QVariant> result;

    WidgetFactory factory(m_core);
    QWidget *w = factory.createWidget(name, 0);
    if (w == 0) {
        return result;
    }

    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(m_core->extensionManager(), w);
    if (sheet == 0) {
        delete w;
        return result;
    }

    for (int i = 0; i < sheet->count(); ++i) {
        result.append(sheet->property(i));
    }

    delete w;

    return result;
}

void WidgetDataBase::grabDefaultPropertyValues()
{
    for (int i = 0; i < count(); ++i) {
        QDesignerWidgetDataBaseItemInterface *item = this->item(i);
        QList<QVariant> default_prop_values = defaultPropertyValues(item->name());
        item->setDefaultPropertyValues(default_prop_values);

    }
}

} // namespace qdesigner_internal
