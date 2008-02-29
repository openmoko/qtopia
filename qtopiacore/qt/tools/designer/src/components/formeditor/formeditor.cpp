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

#include "formeditor.h"
#include "metadatabase_p.h"
#include "widgetdatabase_p.h"
#include "widgetfactory_p.h"
#include "formwindowmanager.h"
#include "qmainwindow_container.h"
#include "qdockwidget_container.h"
#include "qworkspace_container.h"
#include "default_container.h"
#include "default_layoutdecoration.h"
#include "default_actionprovider.h"
#include "qlayoutwidget_propertysheet.h"
#include "spacer_propertysheet.h"
#include "line_propertysheet.h"
#include "qtbrushmanager.h"
#include "brushmanagerproxy.h"
#include "iconcache.h"

// sdk
#include <QtDesigner/QExtensionManager>

// shared
#include <pluginmanager_p.h>
#include <qdesigner_taskmenu_p.h>
#include <qdesigner_propertysheet_p.h>
#include <qdesigner_promotedwidget_p.h>

using namespace qdesigner_internal;

FormEditor::FormEditor(QObject *parent)
    : QDesignerFormEditorInterface(parent)
{
    QDesignerPluginManager *pluginManager = new QDesignerPluginManager(this);
    setPluginManager(pluginManager);

    WidgetDataBase *widgetDatabase = new WidgetDataBase(this);
    setWidgetDataBase(widgetDatabase);

    MetaDataBase *metaDataBase = new MetaDataBase(this);
    setMetaDataBase(metaDataBase);

    WidgetFactory *widgetFactory = new WidgetFactory(this);
    setWidgetFactory(widgetFactory);

    FormWindowManager *formWindowManager = new FormWindowManager(this, this);
    setFormManager(formWindowManager);

    QExtensionManager *mgr = new QExtensionManager(this);

    mgr->registerExtensions(new QDesignerContainerFactory(mgr),             Q_TYPEID(QDesignerContainerExtension));
    mgr->registerExtensions(new QMainWindowContainerFactory(mgr),           Q_TYPEID(QDesignerContainerExtension));
    mgr->registerExtensions(new QDockWidgetContainerFactory(mgr),           Q_TYPEID(QDesignerContainerExtension));
    mgr->registerExtensions(new QWorkspaceContainerFactory(mgr),            Q_TYPEID(QDesignerContainerExtension));

    mgr->registerExtensions(new QDesignerLayoutDecorationFactory(mgr),      Q_TYPEID(QDesignerLayoutDecorationExtension));
    mgr->registerExtensions(new QDesignerActionProviderFactory(mgr),        Q_TYPEID(QDesignerActionProviderExtension));

    mgr->registerExtensions(new QDesignerPropertySheetFactory(mgr),         Q_TYPEID(QDesignerPropertySheetExtension));
    mgr->registerExtensions(new QLayoutWidgetPropertySheetFactory(mgr),     Q_TYPEID(QDesignerPropertySheetExtension));
    mgr->registerExtensions(new SpacerPropertySheetFactory(mgr),            Q_TYPEID(QDesignerPropertySheetExtension));
    mgr->registerExtensions(new LinePropertySheetFactory(mgr),              Q_TYPEID(QDesignerPropertySheetExtension));
    mgr->registerExtensions(new PromotedWidgetPropertySheetFactory(mgr),    Q_TYPEID(QDesignerPropertySheetExtension));

    mgr->registerExtensions(new QDesignerTaskMenuFactory(mgr),              Q_TYPEID(QDesignerTaskMenuExtension));

    setExtensionManager(mgr);

    setIconCache(new IconCache(this));

    QtBrushManager *brushManager = new QtBrushManager(this);
    setBrushManager(brushManager);

    BrushManagerProxy *brushProxy = new BrushManagerProxy(this, this);
    brushProxy->setBrushManager(brushManager);
}

FormEditor::~FormEditor()
{
    delete formWindowManager();
}
