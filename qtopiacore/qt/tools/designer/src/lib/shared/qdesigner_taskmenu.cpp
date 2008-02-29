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

#include "qdesigner_taskmenu_p.h"
#include "qdesigner_command_p.h"
#include "qdesigner_promotedwidget_p.h"
#include <QtGui/QUndoCommand>
#include "richtexteditor_p.h"
#include "stylesheeteditor_p.h"
#include "promotetocustomwidgetdialog_p.h"
#include "widgetfactory_p.h"
#include "widgetdatabase_p.h"
#include "qlayout_widget_p.h"
#include "spacer_widget_p.h"
#include "layout_p.h"

#include <QtDesigner/QtDesigner>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerLayoutDecorationExtension>

#include <QtGui/QAction>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>
#include <QtGui/QMenuBar>
#include <QtGui/QInputDialog>
#include <QtGui/QMainWindow>
#include <QtGui/QDockWidget>
#include <QtGui/QStatusBar>
#include <QtCore/QVariant>
#include <QtGui/QDialogButtonBox>

#include <QtCore/qdebug.h>

namespace qdesigner_internal {

static QMenuBar *findMenuBar(const QWidget *widget)
{
    QList<QObject*> children = widget->children();
    foreach (QObject *obj, widget->children()) {
        if (QMenuBar *mb = qobject_cast<QMenuBar*>(obj)) {
            return mb;
        }
    }

    return 0;
}

static QStatusBar *findStatusBar(const QWidget *widget)
{
    QList<QObject*> children = widget->children();
    foreach (QObject *obj, widget->children()) {
        if (QStatusBar *sb = qobject_cast<QStatusBar*>(obj)) {
            return sb;
        }
    }

    return 0;
}

QDesignerTaskMenu::QDesignerTaskMenu(QWidget *widget, QObject *parent)
    : QObject(parent),
      m_widget(widget)
{
    Q_ASSERT(qobject_cast<QDesignerFormWindowInterface*>(widget) == 0);

    m_separator = new QAction(this);
    m_separator->setSeparator(true);

    m_separator2 = new QAction(this);
    m_separator2->setSeparator(true);


    m_changeObjectNameAction = new QAction(tr("Change objectName..."), this);
    connect(m_changeObjectNameAction, SIGNAL(triggered()), this, SLOT(changeObjectName()));

    m_changeStatusTip = new QAction(tr("Change statusTip..."), this);
    connect(m_changeStatusTip, SIGNAL(triggered()), this, SLOT(changeStatusTip()));

    m_changeToolTip = new QAction(tr("Change toolTip..."), this);
    connect(m_changeToolTip, SIGNAL(triggered()), this, SLOT(changeToolTip()));

    m_changeWhatsThis = new QAction(tr("Change whatsThis..."), this);
    connect(m_changeWhatsThis, SIGNAL(triggered()), this, SLOT(changeWhatsThis()));

    m_changeStyleSheet = new QAction(tr("Change styleSheet..."), this);
    connect(m_changeStyleSheet, SIGNAL(triggered()), this, SLOT(changeStyleSheet()));

    m_addMenuBar = new QAction(tr("Create Menu Bar"), this);
    connect(m_addMenuBar, SIGNAL(triggered()), this, SLOT(createMenuBar()));

    m_addToolBar = new QAction(tr("Add Tool Bar"), this);
    connect(m_addToolBar, SIGNAL(triggered()), this, SLOT(addToolBar()));

    m_addStatusBar = new QAction(tr("Create Status Bar"), this);
    connect(m_addStatusBar, SIGNAL(triggered()), this, SLOT(createStatusBar()));

    m_removeStatusBar = new QAction(tr("Remove Status Bar"), this);
    connect(m_removeStatusBar, SIGNAL(triggered()), this, SLOT(removeStatusBar()));

    m_createDockWidgetAction = new QAction(tr("Create Dock Window"), this);
    connect(m_createDockWidgetAction, SIGNAL(triggered()), this, SLOT(createDockWidget()));

    m_promoteToCustomWidgetAction = new QAction(tr("Promote to Custom Widget"), this);
    m_promoteToCustomWidgetAction->setObjectName(QLatin1String("__qt__promoteToCustomWidgetAction"));
    connect(m_promoteToCustomWidgetAction, SIGNAL(triggered()), this, SLOT(promoteToCustomWidget()));

    QString demote_string = tr("Demote from Custom Widget");
    if (const QDesignerPromotedWidget *promoted = qobject_cast<const QDesignerPromotedWidget*>(widget))
        demote_string = tr("Demote to ") + promoted->item()->extends();

    m_demoteFromCustomWidgetAction = new QAction(demote_string, this);
    connect(m_demoteFromCustomWidgetAction, SIGNAL(triggered()), this, SLOT(demoteFromCustomWidget()));
}

QDesignerTaskMenu::~QDesignerTaskMenu()
{
}

QWidget *QDesignerTaskMenu::widget() const
{
    return m_widget;
}

QDesignerFormWindowInterface *QDesignerTaskMenu::formWindow() const
{
    QDesignerFormWindowInterface *result = QDesignerFormWindowInterface::findFormWindow(widget());
    Q_ASSERT(result != 0);
    return result;
}

void QDesignerTaskMenu::createMenuBar()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());
        if (!mw) {
            // ### warning message
            return;
        }

        CreateMenuBarCommand *cmd = new CreateMenuBarCommand(fw);
        cmd->init(mw);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerTaskMenu::addToolBar()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());
        if (!mw) {
            // ### warning message
            return;
        }

        AddToolBarCommand *cmd = new AddToolBarCommand(fw);
        cmd->init(mw);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerTaskMenu::createStatusBar()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());
        if (!mw) {
            // ### warning message
            return;
        }

        CreateStatusBarCommand *cmd = new CreateStatusBarCommand(fw);
        cmd->init(mw);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerTaskMenu::removeStatusBar()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());
        if (!mw) {
            // ### warning message
            return;
        }

        DeleteStatusBarCommand *cmd = new DeleteStatusBarCommand(fw);
        cmd->init(findStatusBar(mw));
        fw->commandHistory()->push(cmd);
    }
}

QList<QAction*> QDesignerTaskMenu::taskActions() const
{
    QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(widget());
    Q_ASSERT(formWindow);

    bool isMainContainer = formWindow->mainContainer() == widget();

    QList<QAction*> actions;

    if (const QMainWindow *mw = qobject_cast<const QMainWindow*>(formWindow->mainContainer()))  {
        if (isMainContainer || mw->centralWidget() == widget()) {
            if (!findMenuBar(mw)) {
                actions.append(m_addMenuBar);
            }

            actions.append(m_addToolBar);
            // ### create the status bar
            if (!findStatusBar(mw))
                actions.append(m_addStatusBar);
            else
                actions.append(m_removeStatusBar);
            actions.append(m_separator2);
        }
    }
    actions.append(m_changeObjectNameAction);
    actions.append(m_separator);
    actions.append(m_changeToolTip);
    actions.append(m_changeWhatsThis);
    actions.append(m_changeStyleSheet);

    if (!isMainContainer) {
        actions.append(m_separator);
        if (qobject_cast<const QDesignerPromotedWidget*>(m_widget) == 0)
            actions.append(m_promoteToCustomWidgetAction);
        else
            actions.append(m_demoteFromCustomWidgetAction);
    }

    return actions;
}

void QDesignerTaskMenu::changeObjectName()
{
    QDesignerFormWindowInterface *fw = formWindow();
    Q_ASSERT(fw != 0);

    QDesignerFormEditorInterface *core = fw->core();
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), widget());
    Q_ASSERT(sheet != 0);

    bool ok;
    QString newObjectName = QInputDialog::getText(widget(), tr("Change Object Name"),
            tr("Object Name"), QLineEdit::Normal, sheet->property(sheet->indexOf(QLatin1String("objectName"))).toString(), &ok);

    if (ok && !newObjectName.isEmpty()) {
        fw->cursor()->setProperty(QLatin1String("objectName"), newObjectName);
    }
}

void QDesignerTaskMenu::createDockWidget()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());
        if (!mw) {
            // ### warning message
            return;
        }

        AddDockWidgetCommand *cmd = new AddDockWidgetCommand(fw);
        cmd->init(mw);
        fw->commandHistory()->push(cmd);
    }
}

QDesignerTaskMenuFactory::QDesignerTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *QDesignerTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
        return 0;

    QWidget *widget = qobject_cast<QWidget*>(object);
    if (widget == 0)
        return 0;

    // check if is an internal widget (### generalize)
    if (qobject_cast<QLayoutWidget*>(widget) || qobject_cast<Spacer*>(widget))
        return 0;

    return new QDesignerTaskMenu(widget, parent);
}

void QDesignerTaskMenu::promoteToCustomWidget()
{
    QDesignerFormWindowInterface *fw = formWindow();
    QDesignerFormEditorInterface *core = fw->core();
    QWidget *wgt = widget();
    QDesignerWidgetDataBaseInterface *db = core->widgetDataBase();

    Q_ASSERT(qobject_cast<QDesignerPromotedWidget*>(wgt) == 0);

    QString base_class_name = QLatin1String(WidgetFactory::classNameOf(wgt));

    PromoteToCustomWidgetDialog dialog(db, base_class_name);
    if (!dialog.exec())
        return;

    QString custom_class_name = dialog.customClassName();
    QString include_file = dialog.includeFile();

    QDesignerWidgetDataBaseItemInterface *item = 0;
    int idx = db->indexOfClassName(custom_class_name);
    if (idx == -1) {
        item = new WidgetDataBaseItem(custom_class_name, tr("Promoted Widgets"));
        item->setCustom(true);
        item->setPromoted(true);
        item->setExtends(base_class_name);
        db->append(item);
    } else {
        item = db->item(idx);
    }
    item->setIncludeFile(include_file);

    // ### use the undo stack
    // fw->beginCommand(tr("Promote to custom widget"));

    PromoteToCustomWidgetCommand *cmd = new PromoteToCustomWidgetCommand(fw);
    cmd->init(item, wgt);
    fw->commandHistory()->push(cmd);
}

void QDesignerTaskMenu::demoteFromCustomWidget()
{
    QDesignerFormWindowInterface *fw = formWindow();
    QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(widget());
    Q_ASSERT(promoted != 0);

    // ### use the undo stack
    //fw->beginCommand(tr("Demote to ") + promoted->item()->extends());

    DemoteFromCustomWidgetCommand *cmd = new DemoteFromCustomWidgetCommand(fw);
    cmd->init(promoted);
    fw->commandHistory()->push(cmd);
}

void QDesignerTaskMenu::changeRichTextProperty(const QString &propertyName)
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        RichTextEditorDialog *dlg = new RichTextEditorDialog(fw);
        Q_ASSERT(m_widget->parentWidget() != 0);
        RichTextEditor *editor = dlg->editor();

        QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(fw->core()->extensionManager(), m_widget);
        Q_ASSERT(sheet != 0);

        editor->setDefaultFont(m_widget->font());
        editor->setText(sheet->property(sheet->indexOf(propertyName)).toString());
        editor->selectAll();
        editor->setFocus();

        if (dlg->exec()) {
            QString text = editor->text(Qt::RichText);
            fw->cursor()->setWidgetProperty(m_widget, propertyName, QVariant(text));
        }

        delete dlg;
    }
}

void QDesignerTaskMenu::changeToolTip()
{
    changeRichTextProperty(QLatin1String("toolTip"));
}

void QDesignerTaskMenu::changeStatusTip()
{
    changeRichTextProperty(QLatin1String("statusTip"));
}

void QDesignerTaskMenu::changeWhatsThis()
{
    changeRichTextProperty(QLatin1String("whatsThis"));
}

void QDesignerTaskMenu::changeStyleSheet()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        StyleSheetEditorDialog *dlg = new StyleSheetEditorDialog(fw, m_widget);
        dlg->exec();
        delete dlg;
    }
}

} // namespace qdesigner_internal
