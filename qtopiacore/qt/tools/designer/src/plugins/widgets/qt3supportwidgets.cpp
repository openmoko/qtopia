/****************************************************************************
**
** Copyright (C) 2005-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3toolbar/q3toolbar_plugin.h"
#include "q3iconview/q3iconview_plugin.h"
#include "q3groupbox/q3groupbox_plugin.h"
#include "q3frame/q3frame_plugin.h"
#include "q3wizard/q3wizard_plugin.h"
#include "q3mainwindow/q3mainwindow_plugin.h"
#include "q3widgetstack/q3widgetstack_plugin.h"
#include "q3buttongroup/q3buttongroup_plugin.h"
#include "q3listview/q3listview_plugin.h"
#include "q3table/q3table_plugin.h"
#include "q3listbox/q3listbox_plugin.h"
#include "q3listview/q3listview_plugin.h"
#include "q3textedit/q3textedit_plugin.h"
#include "q3dateedit/q3dateedit_plugin.h"
#include "q3timeedit/q3timeedit_plugin.h"
#include "q3datetimeedit/q3datetimeedit_plugin.h"
#include "q3progressbar/q3progressbar_plugin.h"
#include "q3textbrowser/q3textbrowser_plugin.h"

#include <QtDesigner/QtDesigner>
#include <QtCore/qplugin.h>
#include <QtCore/qdebug.h>

class Qt3SupportWidgets: public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
public:
    Qt3SupportWidgets(QObject *parent = 0);

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
    QList<QDesignerCustomWidgetInterface*> m_plugins;
};

Qt3SupportWidgets::Qt3SupportWidgets(QObject *parent)
    : QObject(parent)
{
    m_plugins.append(new Q3ToolBarPlugin(this));
    m_plugins.append(new Q3IconViewPlugin(this));
    m_plugins.append(new Q3GroupBoxPlugin(this));
    m_plugins.append(new Q3FramePlugin(this));
    m_plugins.append(new Q3WizardPlugin(this));
    m_plugins.append(new Q3MainWindowPlugin(this));
    m_plugins.append(new Q3WidgetStackPlugin(this));
    m_plugins.append(new Q3ButtonGroupPlugin(this));
    m_plugins.append(new Q3TablePlugin(this));
    m_plugins.append(new Q3ListBoxPlugin(this));
    m_plugins.append(new Q3ListViewPlugin(this));
    m_plugins.append(new Q3TextEditPlugin(this));
    m_plugins.append(new Q3DateEditPlugin(this));
    m_plugins.append(new Q3TimeEditPlugin(this));
    m_plugins.append(new Q3DateTimeEditPlugin(this));
    m_plugins.append(new Q3ProgressBarPlugin(this));
    m_plugins.append(new Q3TextBrowserPlugin(this));
}

QList<QDesignerCustomWidgetInterface*> Qt3SupportWidgets::customWidgets() const
{
    return m_plugins;
}

Q_EXPORT_PLUGIN(Qt3SupportWidgets)

#include "qt3supportwidgets.moc"
