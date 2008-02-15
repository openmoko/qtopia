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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "taskmenu_component.h"
#include "button_taskmenu.h"
#include "groupbox_taskmenu.h"
#include "label_taskmenu.h"
#include "lineedit_taskmenu.h"
#include "listwidget_taskmenu.h"
#include "treewidget_taskmenu.h"
#include "tablewidget_taskmenu.h"
#include "containerwidget_taskmenu.h"
#include "combobox_taskmenu.h"
#include "textedit_taskmenu.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QExtensionManager>

using namespace qdesigner_internal;

TaskMenuComponent::TaskMenuComponent(QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent),
      m_core(core)
{
    Q_ASSERT(m_core != 0);

    QExtensionManager *mgr = core->extensionManager();

    ContainerWidgetTaskMenuFactory *containerwidget_factory = new ContainerWidgetTaskMenuFactory(mgr);
    mgr->registerExtensions(containerwidget_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    ButtonTaskMenuFactory *button_factory = new ButtonTaskMenuFactory(mgr);
    mgr->registerExtensions(button_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    GroupBoxTaskMenuFactory *groupbox_factory = new GroupBoxTaskMenuFactory(mgr);
    mgr->registerExtensions(groupbox_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    LabelTaskMenuFactory *label_factory = new LabelTaskMenuFactory(mgr);
    mgr->registerExtensions(label_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    LineEditTaskMenuFactory *lineEdit_factory = new LineEditTaskMenuFactory(mgr);
    mgr->registerExtensions(lineEdit_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    ListWidgetTaskMenuFactory *listWidget_factory = new ListWidgetTaskMenuFactory(mgr);
    mgr->registerExtensions(listWidget_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    TreeWidgetTaskMenuFactory *treeWidget_factory = new TreeWidgetTaskMenuFactory(mgr);
    mgr->registerExtensions(treeWidget_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    TableWidgetTaskMenuFactory *tableWidget_factory = new TableWidgetTaskMenuFactory(mgr);
    mgr->registerExtensions(tableWidget_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    ComboBoxTaskMenuFactory *comboBox_factory = new ComboBoxTaskMenuFactory(mgr);
    mgr->registerExtensions(comboBox_factory, Q_TYPEID(QDesignerTaskMenuExtension));

    TextEditTaskMenuFactory *textEdit_factory = new TextEditTaskMenuFactory(mgr);
    mgr->registerExtensions(textEdit_factory, Q_TYPEID(QDesignerTaskMenuExtension));
}

TaskMenuComponent::~TaskMenuComponent()
{
}

QDesignerFormEditorInterface *TaskMenuComponent::core() const
{
    return m_core;
}

