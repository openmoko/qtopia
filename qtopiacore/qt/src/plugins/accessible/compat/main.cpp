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

#include "qaccessiblecompat.h"
#include "q3simplewidgets.h"
#include "q3complexwidgets.h"

#include <qaccessibleplugin.h>
#include <qplugin.h>
#include <qstringlist.h>
#include <q3toolbar.h>

class CompatAccessibleFactory : public QAccessiblePlugin
{
public:
    CompatAccessibleFactory();

    QStringList keys() const;
    QAccessibleInterface *create(const QString &classname, QObject *object);
};

CompatAccessibleFactory::CompatAccessibleFactory()
{
}

QStringList CompatAccessibleFactory::keys() const
{
    QStringList list;
    list << "Q3TextEdit";
    list << "Q3IconView";
    list << "Q3ListView";
    list << "Q3WidgetStack";
    list << "Q3GroupBox";
    list << "Q3ToolBar";
    list << "Q3ToolBarSeparator";
    list << "Q3DockWindowHandle";
    list << "Q3DockWindowResizeHandle";
    list << "Q3MainWindow";
    list << "Q3Header";
    list << "Q3ListBox";
    list << "Q3Table";
    list << "Q3TitleBar";

    return list;
}

QAccessibleInterface *CompatAccessibleFactory::create(const QString &classname, QObject *object)
{
    QAccessibleInterface *iface = 0;
    if (!object || !object->isWidgetType())
        return iface;
    QWidget *widget = static_cast<QWidget*>(object);

    if (classname == "Q3TextEdit") {
        iface = new Q3AccessibleTextEdit(widget);
    } else if (classname == "Q3IconView") {
        iface = new QAccessibleIconView(widget);
    } else if (classname == "Q3ListView") {
        iface = new QAccessibleListView(widget);
    } else if (classname == "Q3WidgetStack") {
        iface = new QAccessibleWidgetStack(widget);
    } else if (classname == "Q3ListBox") {
        iface = new QAccessibleListBox(widget);
    } else if (classname == "Q3Table") {
        iface = new Q3AccessibleScrollView(widget, Table);
    } else if (classname == "Q3GroupBox") {
        iface = new Q3AccessibleDisplay(widget, Grouping);
    } else if (classname == "Q3ToolBar") {
        iface = new QAccessibleWidget(widget, ToolBar, static_cast<Q3ToolBar *>(widget)->label());
    } else if (classname == "Q3MainWindow") {
        iface = new QAccessibleWidget(widget, Application);
    } else if (classname == "Q3ToolBarSeparator") {
        iface = new QAccessibleWidget(widget, Separator);
    } else if (classname == "Q3DockWindowHandle") {
        iface = new QAccessibleWidget(widget, Grip);
    } else if (classname == "Q3DockWindowResizeHandle") {
        iface = new QAccessibleWidget(widget, Grip);
    } else if (classname == "Q3Header") {
        iface = new Q3AccessibleHeader(widget);
    } else if (classname == "Q3TitleBar") {
        iface = new Q3AccessibleTitleBar(widget);
    }

    return iface;
}

Q_EXPORT_STATIC_PLUGIN(CompatAccessibleFactory)
Q_EXPORT_PLUGIN2(qtaccessiblecompatwidgets, CompatAccessibleFactory)
