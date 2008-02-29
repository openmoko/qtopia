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

#include <QtDesigner/QtDesigner>

#include "qdesigner_command_p.h"
#include "qdesigner_utils_p.h"
#include "layout_p.h"
#include "qlayout_widget_p.h"
#include "qdesigner_widget_p.h"
#include "qdesigner_promotedwidget_p.h"
#include "qdesigner_menu_p.h"

#include <QtCore/qdebug.h>

#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QToolBox>
#include <QtGui/QStackedWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QListWidget>
#include <QtGui/QComboBox>
#include <QtGui/QSplitter>
#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QApplication>

namespace qdesigner_internal {

// ---- QDesignerFormEditorCommand ----
QDesignerFormEditorCommand::QDesignerFormEditorCommand(const QString &description, QDesignerFormEditorInterface *core)
    : QUndoCommand(description),
      m_core(core)
{
}

QDesignerFormEditorInterface *QDesignerFormEditorCommand::core() const
{
    return m_core;
}

// ---- QDesignerFormWindowManagerCommand ----
QDesignerFormWindowManagerCommand::QDesignerFormWindowManagerCommand(const QString &description, QDesignerFormWindowManagerInterface *formWindowManager)
    : QUndoCommand(description),
      m_formWindowManager(formWindowManager)
{
}

QDesignerFormWindowManagerInterface *QDesignerFormWindowManagerCommand::formWindowManager() const
{
    return m_formWindowManager;
}

// ---- QDesignerFormWindowCommand ----
QDesignerFormWindowCommand::QDesignerFormWindowCommand(const QString &description, QDesignerFormWindowInterface *formWindow)
    : QUndoCommand(description),
      m_formWindow(formWindow)
{
}

QDesignerFormWindowInterface *QDesignerFormWindowCommand::formWindow() const
{
    return m_formWindow;
}

QDesignerFormEditorInterface *QDesignerFormWindowCommand::core() const
{
    if (QDesignerFormWindowInterface *fw = formWindow())
        return fw->core();

    return 0;
}

void QDesignerFormWindowCommand::undo()
{
    cheapUpdate();
}

void QDesignerFormWindowCommand::redo()
{
    cheapUpdate();
}

void QDesignerFormWindowCommand::cheapUpdate()
{
    if (core()->objectInspector())
        core()->objectInspector()->setFormWindow(formWindow());

    if (core()->actionEditor())
        core()->actionEditor()->setFormWindow(formWindow());
}

bool QDesignerFormWindowCommand::hasLayout(QWidget *widget) const
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    if (widget && LayoutInfo::layoutType(core, widget) != LayoutInfo::NoLayout) {
        QDesignerMetaDataBaseItemInterface *item = core->metaDataBase()->item(widget);
        return item != 0;
    }

    return false;
}

void QDesignerFormWindowCommand::checkObjectName(QObject *)
{
}

void QDesignerFormWindowCommand::updateBuddies(const QString &old_name,
                                                const QString &new_name)
{
    QDesignerFormEditorInterface *core = formWindow()->core();

    QList<QDesignerLabel*> label_list = qFindChildren<QDesignerLabel*>(formWindow());
    foreach (QDesignerLabel *label, label_list) {
        QDesignerPropertySheetExtension* propertySheet
            = qt_extension<QDesignerPropertySheetExtension*>
                (core->extensionManager(), label);
        if (propertySheet == 0)
            continue;
        int idx = propertySheet->indexOf(QLatin1String("buddy"));
        if (idx == -1)
            continue;
        if (propertySheet->property(idx).toString() == old_name)
            propertySheet->setProperty(idx, new_name);
    }
}

void QDesignerFormWindowCommand::checkSelection(QWidget *widget)
{
    Q_UNUSED(widget);
}

void QDesignerFormWindowCommand::checkParent(QWidget *widget, QWidget *parentWidget)
{
    Q_ASSERT(widget);

    if (widget->parentWidget() != parentWidget)
        widget->setParent(parentWidget);
}

// ---- SetPropertyCommand ----
SetPropertyCommand::SetPropertyCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow),
      m_index(-1),
      m_propertySheet(0),
      m_changed(false)
{
}

QObject *SetPropertyCommand::object() const
{
    return m_object;
}

QWidget *SetPropertyCommand::widget() const
{
    return qobject_cast<QWidget*>(m_object);
}

QWidget *SetPropertyCommand::parentWidget() const
{
    if (QWidget *w = widget()) {
        return w->parentWidget();
    }

    return 0;
}

void SetPropertyCommand::init(QObject *object, const QString &propertyName, const QVariant &newValue)
{
    Q_ASSERT(object);

    m_object = object;
    m_parentWidget = parentWidget();
    m_propertyName = propertyName;
    m_newValue = newValue;

    QDesignerFormEditorInterface *core = formWindow()->core();
    m_propertySheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), object);
    Q_ASSERT(m_propertySheet);

    m_index = m_propertySheet->indexOf(m_propertyName);
    Q_ASSERT(m_index != -1);

    m_changed = m_propertySheet->isChanged(m_index);
    m_oldValue = m_propertySheet->property(m_index);

    setText(QApplication::translate("Command", "changed '%1' of '%2'").arg(m_propertyName).arg(object->objectName()));
}

static QWidget *containerWindow(QWidget *widget)
{
    while (widget) {
        if (widget->metaObject()->className() == QLatin1String("QDesignerFormWindow"))
            return widget;

        widget = widget->parentWidget();
    }
    return 0;
}

static QSize checkSize(const QSize &size)
{
    QSize s = size;
    if (s.width() > 0xFFFFFF)
        s.setWidth(0xFFFFFF);
    if (s.height() > 0xFFFFFF)
        s.setHeight(0xFFFFFF);
    return s;
}

static void setTopMinMaxSize(QDesignerFormWindowInterface *fw, QWidget *w, const QString &propertyName, const QVariant &value)
{
    QDesignerFormWindowCursorInterface *cursor = fw->cursor();
    if (w && cursor->isWidgetSelected(w)) {
        if (cursor->isWidgetSelected(fw->mainContainer())) {
            if (propertyName == QLatin1String("minimumSize")) {
                if (QWidget *container = containerWindow(fw)) {
                    if (container->parentWidget() && container->parentWidget()->metaObject()->className() == QLatin1String("QWorkspaceChild")) {
                        QSize diff = container->parentWidget()->geometry().size() - container->geometry().size();
                        container->parentWidget()->setMinimumSize(checkSize(value.toSize() + diff));
                    }
                    container->setMinimumSize(value.toSize());
                }
            } else if (propertyName == QLatin1String("maximumSize")) {
                if (QWidget *container = containerWindow(fw)) {
                    if (container->parentWidget() && container->parentWidget()->metaObject()->className() == QLatin1String("QWorkspaceChild")) {
                        QSize diff = container->parentWidget()->geometry().size() - container->geometry().size();
                        container->parentWidget()->setMaximumSize(checkSize(value.toSize() + diff));
                    }
                    container->setMaximumSize(value.toSize());
                }
            }
        }
    }
}

void SetPropertyCommand::redo()
{
    Q_ASSERT(m_propertySheet);
    Q_ASSERT(m_index != -1);

    m_propertySheet->setProperty(m_index, m_newValue);
    m_changed = m_propertySheet->isChanged(m_index);
    m_propertySheet->setChanged(m_index, true);

    if (m_propertyName == QLatin1String("geometry") && widget()) {
        checkSelection(widget());
        checkParent(widget(), parentWidget());
    } else if (m_propertyName == QLatin1String("objectName")) {
        checkObjectName(m_object);
        updateBuddies(m_oldValue.toString(), m_newValue.toString());
    }

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == object())
            propertyEditor->setPropertyValue(propertyName(), m_newValue, true);
        else
            propertyEditor->setObject(propertyEditor->object()); // this is needed when f.ex. undo
                                                                 // changes parent's palette, but
                                                                 // the child is the active widget.
    }

    setTopMinMaxSize(formWindow(), qobject_cast<QWidget *>(m_object), m_propertyName, m_newValue);

    QAction *act = qobject_cast<QAction *>(m_object);
    if (m_propertyName == QLatin1String("objectName") ||
                m_propertyName == QLatin1String("icon") && act ||
                m_propertyName == QLatin1String("currentTabName")) {
        if (QDesignerObjectInspectorInterface *oi = formWindow()->core()->objectInspector())
            oi->setFormWindow(formWindow());
    }

    if (m_propertyName == QLatin1String("objectName") && act) {
        // emit act->changed(); cannot emit, signal is protected
        act->setData(QVariant(true)); // it triggers signal "changed" in QAction
        act->setData(QVariant(false));
    }

    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(m_object)) {
        if (m_propertyName == QLatin1String("minimumSize"))
            promoted->setMinimumSize(m_newValue.toSize());
        else if (m_propertyName == QLatin1String("maximumSize"))
            promoted->setMaximumSize(m_newValue.toSize());
    }
}

void SetPropertyCommand::undo()
{
    Q_ASSERT(m_propertySheet);
    Q_ASSERT(m_index != -1);

    m_propertySheet->setProperty(m_index, m_oldValue);
    m_propertySheet->setChanged(m_index, m_changed);

    if (m_propertyName == QLatin1String("geometry") && widget()) {
        checkSelection(widget());
        checkParent(widget(), parentWidget());
    } else if (m_propertyName == QLatin1String("objectName")) {
        checkObjectName(m_object);
        updateBuddies(m_newValue.toString(), m_oldValue.toString());
    }

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == widget())
            propertyEditor->setPropertyValue(propertyName(), m_oldValue, m_changed);
        else
            propertyEditor->setObject(propertyEditor->object()); // this is needed when f.ex. undo
                                                                 // changes parent's palette, but
                                                                 // the child is the active widget.
    }

    setTopMinMaxSize(formWindow(), qobject_cast<QWidget *>(m_object), m_propertyName, m_oldValue);

    QAction *act = qobject_cast<QAction *>(m_object);
    if (m_propertyName == QLatin1String("objectName") ||
                m_propertyName == QLatin1String("icon") && act ||
                m_propertyName == QLatin1String("currentTabName")) {
        if (QDesignerObjectInspectorInterface *oi = formWindow()->core()->objectInspector())
            oi->setFormWindow(formWindow());
    }

    if (m_propertyName == QLatin1String("objectName") && act) {
        // emit act->changed(); cannot emit, signal is protected
        act->setData(QVariant(true)); // it triggers signal "changed" in QAction
        act->setData(QVariant(false));
    }

    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(m_object)) {
        if (m_propertyName == QLatin1String("minimumSize"))
            promoted->setMinimumSize(m_oldValue.toSize());
        else if (m_propertyName == QLatin1String("maximumSize"))
            promoted->setMaximumSize(m_oldValue.toSize());
    }
}

int SetPropertyCommand::id() const
{
    return 1976;
}

bool SetPropertyCommand::mergeWith(const QUndoCommand *other)
{
    if (id() != other->id())
        return false;

    if (const SetPropertyCommand *cmd = static_cast<const SetPropertyCommand*>(other)) {
        if (cmd->propertyName() == propertyName() && cmd->object() == object()) {
            if (!formWindow()->isDirty())
                return false;

            m_newValue = cmd->newValue();
            return true;
        }
    }

    return false;
}

// ---- SetFormPropertyCommand ----
SetFormPropertyCommand::SetFormPropertyCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow),
      m_index(-1),
      m_changed(false),
      m_propertySheet(0)
{
}

void SetFormPropertyCommand::init(QObject *object, const QString &propertyName, const QVariant &newValue)
{
    Q_ASSERT(object);

    m_newValue = newValue;
    m_propertyName = propertyName;

    QDesignerFormEditorInterface *core = formWindow()->core();
    m_propertySheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), object);
    Q_ASSERT(m_propertySheet);

    m_index = m_propertySheet->indexOf(m_propertyName);
    Q_ASSERT(m_index != -1);

    m_changed = m_propertySheet->isChanged(m_index);
    m_oldValue = m_propertySheet->property(m_index);

    setText(QApplication::translate("Command", "changed '%1' of '%2'").arg(m_propertyName).arg(object->objectName()));
}

void SetFormPropertyCommand::redo()
{
    m_changed = m_propertySheet->isChanged(m_index);
    m_propertySheet->setChanged(m_index, true);

    if (m_propertyName == QLatin1String("geometry"))
        updateFormWindowGeometry(m_newValue);
}

void SetFormPropertyCommand::undo()
{
    m_propertySheet->setChanged(m_index, m_changed);

    if (m_propertyName == QLatin1String("geometry"))
        updateFormWindowGeometry(m_oldValue);
}


int SetFormPropertyCommand::id() const
{
    return 1977;
}

bool SetFormPropertyCommand::mergeWith(const QUndoCommand *other)
{
    if (id() != other->id())
        return false;

    if (const SetFormPropertyCommand *cmd = static_cast<const SetFormPropertyCommand*>(other)) {
        if (cmd->propertyName() == propertyName() && cmd->formWindow() == formWindow()) {
            m_newValue = cmd->newValue();
            return true;
        }
    }

    return false;
}

void SetFormPropertyCommand::updateFormWindowGeometry(const QVariant &value)
{
    if (QWidget *container = containerWindow(formWindow())) {
        QRect r = container->geometry();
        if (container->parentWidget() && container->parentWidget()->metaObject()->className() == QLatin1String("QWorkspaceChild")) {
            QRect windowRect = container->parentWidget()->rect();
            QSize diff = windowRect.size() - r.size();
            windowRect.setSize(value.toRect().size() + diff);
            container->parentWidget()->setGeometry(windowRect);
        } else {
            r.setSize(value.toRect().size());
            container->setGeometry(r);
        }
    }
}

QWidget *SetFormPropertyCommand::containerWindow(QWidget *widget)
{
    while (widget) {
        if (widget->isWindow())
            break;
        if (widget->parentWidget() && !qstrcmp(widget->parentWidget()->metaObject()->className(), "QWorkspaceChild"))
            break;

        widget = widget->parentWidget();
    }

    return widget;
}
// ---- ResetPropertyCommand ----
ResetPropertyCommand::ResetPropertyCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow),
      m_index(-1),
      m_propertySheet(0),
      m_changed(false)
{
}

QObject *ResetPropertyCommand::object() const
{
    return m_object;
}

QObject *ResetPropertyCommand::parentObject() const
{
    return m_parentObject;
}

void ResetPropertyCommand::init(QObject *object, const QString &propertyName)
{
    Q_ASSERT(object);

    m_object = object;
    m_parentObject = object->parent();
    m_propertyName = propertyName;

    QDesignerFormEditorInterface *core = formWindow()->core();
    m_propertySheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), object);
    Q_ASSERT(m_propertySheet);

    m_index = m_propertySheet->indexOf(m_propertyName);
    Q_ASSERT(m_index != -1);

    m_changed = m_propertySheet->isChanged(m_index);
    m_oldValue = m_propertySheet->property(m_index);

    setText(QApplication::translate("Command", "reset '%1' of '%2'").arg(m_propertyName).arg(m_object->objectName()));
}

void ResetPropertyCommand::redo()
{
    Q_ASSERT(m_propertySheet);
    Q_ASSERT(m_index != -1);

    QObject *obj = m_object;
    QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(obj);
    if (promoted)
        obj = promoted->child();

    QVariant new_value;

    if (m_propertySheet->reset(m_index)) {
        new_value = m_propertySheet->property(m_index);
    } else {
        int item_idx =  formWindow()->core()->widgetDataBase()->indexOfObject(obj);
        if (item_idx == -1) {
            new_value = m_oldValue; // We simply don't know the value in this case
        } else {
            QDesignerWidgetDataBaseItemInterface *item
                = formWindow()->core()->widgetDataBase()->item(item_idx);
            QList<QVariant> default_prop_values = item->defaultPropertyValues();
            if (m_index < default_prop_values.size())
                new_value = default_prop_values.at(m_index);
            else
                new_value = m_oldValue; // Again, we just don't know
        }

        m_propertySheet->setProperty(m_index, new_value);
    }

    m_propertySheet->setChanged(m_index, false);

    setTopMinMaxSize(formWindow(), qobject_cast<QWidget *>(m_object), m_propertyName, new_value);

    QWidget *widget = qobject_cast<QWidget *>(m_object);
    QWidget *parentWidget = qobject_cast<QWidget *>(m_parentObject);
    if (m_propertyName == QLatin1String("geometry") && widget) {
        checkSelection(widget);
        checkParent(widget, parentWidget);
    } else if (m_propertyName == QLatin1String("objectName")) {
        checkObjectName(m_object);
    }

    if (promoted) {
        if (m_propertyName == QLatin1String("minimumSize"))
            promoted->setMinimumSize(new_value.toSize());
        else if (m_propertyName == QLatin1String("maximumSize"))
            promoted->setMaximumSize(new_value.toSize());
    }

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == object())
            propertyEditor->setPropertyValue(propertyName(), new_value, false);
    }
}

void ResetPropertyCommand::undo()
{
    Q_ASSERT(m_propertySheet);
    Q_ASSERT(m_index != -1);

    m_propertySheet->setProperty(m_index, m_oldValue);
    m_propertySheet->setChanged(m_index, m_changed);

    setTopMinMaxSize(formWindow(), qobject_cast<QWidget *>(m_object), m_propertyName, m_oldValue);

    QWidget *widget = qobject_cast<QWidget *>(m_object);
    QWidget *parentWidget = qobject_cast<QWidget *>(m_parentObject);
    if (m_propertyName == QLatin1String("geometry") && widget) {
        checkSelection(widget);
        checkParent(widget, parentWidget);
    } else if (m_propertyName == QLatin1String("objectName")) {
        checkObjectName(m_object);
    }

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == object())
            propertyEditor->setPropertyValue(propertyName(), m_oldValue, m_changed);
    }

    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(m_object)) {
        if (m_propertyName == QLatin1String("minimumSize"))
            promoted->setMinimumSize(m_oldValue.toSize());
        else if (m_propertyName == QLatin1String("maximumSize"))
            promoted->setMaximumSize(m_oldValue.toSize());
    }
}

// ---- InsertWidgetCommand ----
InsertWidgetCommand::InsertWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{
}

void InsertWidgetCommand::init(QWidget *widget, bool already_in_form)
{
    m_widget = widget;

    setText(QApplication::translate("Command", "Insert '%1'").arg(widget->objectName()));

    QWidget *parentWidget = m_widget->parentWidget();
    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parentWidget);

    m_insertMode = deco ? deco->currentInsertMode() : QDesignerLayoutDecorationExtension::InsertWidgetMode;
    m_cell = deco ? deco->currentCell() : qMakePair(0, 0);
    m_widgetWasManaged = already_in_form;
}

static void recursiveUpdate(QWidget *w)
{
    w->update();

    const QObjectList &l = w->children();
    QObjectList::const_iterator it = l.begin();
    for (; it != l.end(); ++it) {
        if (QWidget *w = qobject_cast<QWidget*>(*it))
            recursiveUpdate(w);
    }
}

void InsertWidgetCommand::redo()
{
    checkObjectName(m_widget);

    QWidget *parentWidget = m_widget->parentWidget();
    Q_ASSERT(parentWidget);

    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parentWidget);

    if (deco != 0) {
        if (LayoutInfo::layoutType(core, parentWidget) == LayoutInfo::Grid) {
            switch (m_insertMode) {
                case QDesignerLayoutDecorationExtension::InsertRowMode: {
                    deco->insertRow(m_cell.first);
                } break;

                case QDesignerLayoutDecorationExtension::InsertColumnMode: {
                    deco->insertColumn(m_cell.second);
                } break;

                default: break;
            } // end switch
        }
        deco->insertWidget(m_widget, m_cell);
    }

    if (!m_widgetWasManaged)
        formWindow()->manageWidget(m_widget);
    m_widget->show();
    formWindow()->emitSelectionChanged();

    if (parentWidget && parentWidget->layout()) {
        recursiveUpdate(parentWidget);
        parentWidget->layout()->invalidate();
    }

    QList<QDesignerLabel*> label_list = qFindChildren<QDesignerLabel*>(formWindow());
    foreach (QDesignerLabel *label, label_list) {
        QDesignerPropertySheetExtension* propertySheet
            = qt_extension<QDesignerPropertySheetExtension*>
                (core->extensionManager(), label);
        if (propertySheet == 0)
            continue;
        int idx = propertySheet->indexOf(QLatin1String("buddy"));
        if (idx == -1)
            continue;
        if (propertySheet->property(idx).toString() == m_widget->objectName())
            propertySheet->setProperty(idx, m_widget->objectName());
    }
}

void InsertWidgetCommand::undo()
{
    QWidget *parentWidget = m_widget->parentWidget();

    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parentWidget);

    if (deco) {
        deco->removeWidget(m_widget);
        deco->simplify();
    }

    if (!m_widgetWasManaged) {
        formWindow()->unmanageWidget(m_widget);
        m_widget->hide();
    }
    formWindow()->emitSelectionChanged();

    QList<QDesignerLabel*> label_list = qFindChildren<QDesignerLabel*>(formWindow());
    foreach (QDesignerLabel *label, label_list) {
        QDesignerPropertySheetExtension* propertySheet
            = qt_extension<QDesignerPropertySheetExtension*>
                (core->extensionManager(), label);
        if (propertySheet == 0)
            continue;
        int idx = propertySheet->indexOf(QLatin1String("buddy"));
        if (idx == -1)
            continue;
        if (propertySheet->property(idx).toString() == m_widget->objectName())
            propertySheet->setProperty(idx, m_widget->objectName());
    }
}

// ---- RaiseWidgetCommand ----
RaiseWidgetCommand::RaiseWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{
}

void RaiseWidgetCommand::init(QWidget *widget)
{
    m_widget = widget;
    setText(QApplication::translate("Command", "Raise '%1'").arg(widget->objectName()));
}

void RaiseWidgetCommand::redo()
{
    m_widget->raise();
}

void RaiseWidgetCommand::undo()
{
}

// ---- LowerWidgetCommand ----
LowerWidgetCommand::LowerWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{
}

void LowerWidgetCommand::init(QWidget *widget)
{
    m_widget = widget;
    setText(QApplication::translate("Command", "Lower '%1'").arg(widget->objectName()));
}

void LowerWidgetCommand::redo()
{
    m_widget->raise();
}

void LowerWidgetCommand::undo()
{
}

// ---- DeleteWidgetCommand ----
DeleteWidgetCommand::DeleteWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{
}

void DeleteWidgetCommand::init(QWidget *widget)
{
    m_widget = widget;
    m_parentWidget = widget->parentWidget();
    m_geometry = widget->geometry();

    m_layoutType = LayoutInfo::NoLayout;
    m_index = -1;
    if (hasLayout(m_parentWidget)) {
        m_layoutType = LayoutInfo::layoutType(formWindow()->core(), m_parentWidget);

        switch (m_layoutType) {
            case LayoutInfo::VBox:
                m_index = static_cast<QVBoxLayout*>(m_parentWidget->layout())->indexOf(m_widget);
                break;
            case LayoutInfo::HBox:
                m_index = static_cast<QHBoxLayout*>(m_parentWidget->layout())->indexOf(m_widget);
                break;
            case LayoutInfo::Grid: {
                m_index = 0;
                while (QLayoutItem *item = m_parentWidget->layout()->itemAt(m_index)) {
                    if (item->widget() == m_widget)
                        break;
                    ++m_index;
                }

                static_cast<QGridLayout*>(m_parentWidget->layout())->getItemPosition(m_index, &m_row, &m_col, &m_rowspan, &m_colspan);
            } break;

            default:
                break;
        } // end switch
    }

    m_formItem = formWindow()->core()->metaDataBase()->item(formWindow());
    m_tabOrderIndex = m_formItem->tabOrder().indexOf(widget);

    // Build the list of managed children
    m_managedChildren = QList<QPointer<QWidget> >();
    QList<QWidget *>children = qFindChildren<QWidget *>(m_widget);
    foreach (QPointer<QWidget> child, children) {
        if (formWindow()->isManaged(child))
            m_managedChildren.append(child);
    }

    setText(QApplication::translate("Command", "Delete '%1'").arg(widget->objectName()));
}

void DeleteWidgetCommand::redo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();

    if (QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), m_parentWidget)) {
        for (int i=0; i<c->count(); ++i) {
            if (c->widget(i) == m_widget) {
                c->remove(i);
                formWindow()->emitSelectionChanged();
                return;
            }
        }
    }

    if (QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), m_parentWidget)) {
        deco->removeWidget(m_widget);
    }

    // Unmanage the managed children first
    foreach (QWidget *child, m_managedChildren)
        formWindow()->unmanageWidget(child);

    formWindow()->unmanageWidget(m_widget);
    m_widget->setParent(formWindow());
    m_widget->hide();

    if (m_tabOrderIndex != -1) {
        QList<QWidget*> tab_order = m_formItem->tabOrder();
        tab_order.removeAt(m_tabOrderIndex);
        m_formItem->setTabOrder(tab_order);
    }

    formWindow()->emitSelectionChanged();
}

void DeleteWidgetCommand::undo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();

    m_widget->setParent(m_parentWidget);

    if (QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), m_parentWidget)) {
        c->addWidget(m_widget);
        formWindow()->emitSelectionChanged();
        return;
    }

    m_widget->setGeometry(m_geometry);
    formWindow()->manageWidget(m_widget);

    // Manage the managed children
    foreach (QWidget *child, m_managedChildren)
        formWindow()->manageWidget(child);

    // ### set up alignment
    switch (m_layoutType) {
        case LayoutInfo::VBox: {
            QVBoxLayout *vbox = static_cast<QVBoxLayout*>(m_parentWidget->layout());
            insert_into_box_layout(vbox, m_index, m_widget);
        } break;

        case LayoutInfo::HBox: {
            QHBoxLayout *hbox = static_cast<QHBoxLayout*>(m_parentWidget->layout());
            insert_into_box_layout(hbox, m_index, m_widget);
        } break;

        case LayoutInfo::Grid: {
            QGridLayout *grid = static_cast<QGridLayout*>(m_parentWidget->layout());
            add_to_grid_layout(grid, m_widget, m_row, m_col, m_rowspan, m_colspan);
        } break;

        default:
            break;
    } // end switch

    m_widget->show();

    if (m_tabOrderIndex != -1) {
        QList<QWidget*> tab_order = m_formItem->tabOrder();
        tab_order.insert(m_tabOrderIndex, m_widget);
        m_formItem->setTabOrder(tab_order);
    }

    formWindow()->emitSelectionChanged();
}

// ---- ReparentWidgetCommand ----
ReparentWidgetCommand::ReparentWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{
}

void ReparentWidgetCommand::init(QWidget *widget, QWidget *parentWidget)
{
    Q_ASSERT(widget);

    m_widget = widget;
    m_oldParentWidget = widget->parentWidget();
    m_newParentWidget = parentWidget;

    m_oldPos = m_widget->pos();
    m_newPos = m_newParentWidget->mapFromGlobal(m_oldParentWidget->mapToGlobal(m_oldPos));

    setText(QApplication::translate("Command", "Reparent '%1'").arg(widget->objectName()));
}

void ReparentWidgetCommand::redo()
{
    m_widget->setParent(m_newParentWidget);
    m_widget->move(m_newPos);

    m_widget->show();
}

void ReparentWidgetCommand::undo()
{
    m_widget->setParent(m_oldParentWidget);
    m_widget->move(m_oldPos);

    m_widget->show();
}

// ---- PromoteToCustomWidgetCommand ----

static void replace_widget_item(QDesignerFormWindowInterface *fw, QWidget *wgt, QWidget *promoted)
{
    QDesignerFormEditorInterface *core = fw->core();
    QWidget *parent = wgt->parentWidget();

    promoted->setMinimumSize(wgt->minimumSize());
    promoted->setMaximumSize(wgt->maximumSize());

    QRect info;
    int splitter_idx = -1;
    if (QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parent)) {
        if (QSplitter *splitter = qobject_cast<QSplitter*>(parent)) {
            splitter_idx = splitter->indexOf(wgt);
            Q_ASSERT(splitter_idx != -1);
            wgt->setParent(0);
        } else {
            QLayout *layout = LayoutInfo::managedLayout(core, parent);
            Q_ASSERT(layout != 0);

            int old_index = layout->indexOf(wgt);
            Q_ASSERT(old_index != -1);

            info = deco->itemInfo(old_index);

            QLayoutItem *item = layout->takeAt(old_index);
            delete item;
            layout->activate();
        }
    }

    if (qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parent)) {
        if (QSplitter *splitter = qobject_cast<QSplitter*>(parent)) {
            splitter->insertWidget(splitter_idx, promoted);
        } else {
            QLayout *layout = LayoutInfo::managedLayout(core, parent);
            Q_ASSERT(layout != 0);

            // ### check if `info' is valid!

            switch (LayoutInfo::layoutType(core, layout)) {
                default: Q_ASSERT(0); break;

                case LayoutInfo::VBox:
                    insert_into_box_layout(static_cast<QBoxLayout*>(layout), info.top(), promoted);
                    break;

                case LayoutInfo::HBox:
                    insert_into_box_layout(static_cast<QBoxLayout*>(layout), info.left(), promoted);
                    break;

                case LayoutInfo::Grid:
                    add_to_grid_layout(static_cast<QGridLayout*>(layout), promoted, info.top(), info.left(), info.height(), info.width());
                    break;
            }
        }
    }
}

PromoteToCustomWidgetCommand::PromoteToCustomWidgetCommand
                                (QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Promote to custom widget"), formWindow)
{
    m_widget = 0;
    m_promoted = 0;
}

void PromoteToCustomWidgetCommand::init(QDesignerWidgetDataBaseItemInterface *item,
                                        QWidget *widget)
{
    m_widget = widget;
    m_promoted = new QDesignerPromotedWidget(item, widget->parentWidget());
}

void PromoteToCustomWidgetCommand::redo()
{
    m_promoted->setObjectName(QLatin1String("__qt__promoted_") + m_widget->objectName());
    m_promoted->setGeometry(m_widget->geometry());

    replace_widget_item(formWindow(), m_widget, m_promoted);

    m_promoted->setChildWidget(m_widget);
    formWindow()->manageWidget(m_promoted);

    formWindow()->clearSelection();
    formWindow()->selectWidget(m_promoted);
    m_promoted->show();
}

void PromoteToCustomWidgetCommand::undo()
{
    m_promoted->setChildWidget(0);
    m_widget->setParent(m_promoted->parentWidget());
    m_widget->setGeometry(m_promoted->geometry());

    replace_widget_item(formWindow(), m_promoted, m_widget);

    formWindow()->manageWidget(m_widget);
    formWindow()->unmanageWidget(m_promoted);

    m_promoted->hide();
    m_widget->show();

    formWindow()->clearSelection();
    formWindow()->selectWidget(m_promoted);
}

// ---- DemoteFromCustomWidgetCommand ----

DemoteFromCustomWidgetCommand::DemoteFromCustomWidgetCommand
                                    (QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Demote from custom widget"), formWindow)
{
    m_promote_cmd = new PromoteToCustomWidgetCommand(formWindow);
}

void DemoteFromCustomWidgetCommand::init(QDesignerPromotedWidget *promoted)
{
    m_promote_cmd->m_widget = promoted->child();
    m_promote_cmd->m_promoted = promoted;
}

void DemoteFromCustomWidgetCommand::redo()
{
    m_promote_cmd->undo();
    m_promote_cmd->m_widget->show();
}

void DemoteFromCustomWidgetCommand::undo()
{
    m_promote_cmd->redo();
}

// ---- LayoutCommand ----
LayoutCommand::LayoutCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{
}

LayoutCommand::~LayoutCommand()
{
    m_layout->deleteLater();
}

void LayoutCommand::init(QWidget *parentWidget, const QList<QWidget*> &widgets, LayoutInfo::Type layoutType,
        QWidget *layoutBase, bool splitter)
{
    m_parentWidget = parentWidget;
    m_widgets = widgets;
    formWindow()->simplifySelection(&m_widgets);
    QPoint grid = formWindow()->grid();
    QSize sz(qMax(5, grid.x()), qMax(5, grid.y()));

    switch (layoutType) {
        case LayoutInfo::Grid:
            m_layout = new GridLayout(widgets, m_parentWidget, formWindow(), layoutBase, sz);
            setText(QApplication::translate("Command", "Lay out using grid"));
            break;

        case LayoutInfo::VBox:
            m_layout = new VerticalLayout(widgets, m_parentWidget, formWindow(), layoutBase, splitter);
            setText(QApplication::translate("Command", "Lay out vertically"));
            break;

        case LayoutInfo::HBox:
            m_layout = new HorizontalLayout(widgets, m_parentWidget, formWindow(), layoutBase, splitter);
            setText(QApplication::translate("Command", "Lay out horizontally"));
            break;
        default:
            Q_ASSERT(0);
    }

    m_layout->setup();
}

void LayoutCommand::redo()
{
    m_layout->doLayout();
    checkSelection(m_parentWidget);
}

void LayoutCommand::undo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();

    QWidget *lb = m_layout->layoutBaseWidget();
    QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), lb);

    QWidget *p = m_layout->parentWidget();
    if (!deco && hasLayout(p)) {
        deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), p);
    }

    m_layout->undoLayout();
    delete deco; // release the extension

    // ### generalize (put in function)
    if (!m_layoutBase && lb != 0 && !(qobject_cast<QLayoutWidget*>(lb) || qobject_cast<QSplitter*>(lb))) {
        core->metaDataBase()->add(lb);
        lb->show();
    }

    checkSelection(m_parentWidget);
}

// ---- BreakLayoutCommand ----
BreakLayoutCommand::BreakLayoutCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Break layout"), formWindow)
{
}

BreakLayoutCommand::~BreakLayoutCommand()
{
}

void BreakLayoutCommand::init(const QList<QWidget*> &widgets, QWidget *layoutBase)
{
    QDesignerFormEditorInterface *core = formWindow()->core();

    m_widgets = widgets;
    m_layoutBase = core->widgetFactory()->containerOfWidget(layoutBase);
    m_layout = 0;

    QPoint grid = formWindow()->grid();

    LayoutInfo::Type lay = LayoutInfo::layoutType(core, m_layoutBase);
    if (lay == LayoutInfo::HBox)
        m_layout = new HorizontalLayout(widgets, m_layoutBase, formWindow(), m_layoutBase, qobject_cast<QSplitter*>(m_layoutBase) != 0);
    else if (lay == LayoutInfo::VBox)
        m_layout = new VerticalLayout(widgets, m_layoutBase, formWindow(), m_layoutBase, qobject_cast<QSplitter*>(m_layoutBase) != 0);
    else if (lay == LayoutInfo::Grid)
        m_layout = new GridLayout(widgets, m_layoutBase, formWindow(), m_layoutBase, QSize(qMax(5, grid.x()), qMax(5, grid.y())));
    // ### StackedLayout

    Q_ASSERT(m_layout != 0);

    m_layout->sort();

    m_margin = m_layout->margin();
    m_spacing = m_layout->spacing();
}

void BreakLayoutCommand::redo()
{
    if (!m_layout)
        return;

    QDesignerFormEditorInterface *core = formWindow()->core();
    QWidget *lb = m_layout->layoutBaseWidget();
    QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), lb);
    QWidget *p = m_layout->parentWidget();
    if (!deco && hasLayout(p))
        deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), p);

    formWindow()->clearSelection(false);
    m_layout->breakLayout();
    delete deco; // release the extension

    foreach (QWidget *widget, m_widgets) {
        widget->resize(widget->size().expandedTo(QSize(16, 16)));
    }
}

void BreakLayoutCommand::undo()
{
    if (!m_layout)
        return;

    formWindow()->clearSelection(false);
    m_layout->doLayout();

    if (m_layoutBase && m_layoutBase->layout()) {
        m_layoutBase->layout()->setSpacing(m_spacing);
        m_layoutBase->layout()->setMargin(m_margin);
    }
}

// ---- ToolBoxCommand ----
ToolBoxCommand::ToolBoxCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{
}

ToolBoxCommand::~ToolBoxCommand()
{
}

void ToolBoxCommand::init(QToolBox *toolBox)
{
    m_toolBox = toolBox;
    m_index = m_toolBox->currentIndex();
    m_widget = m_toolBox->widget(m_index);
    m_itemText = m_toolBox->itemText(m_index);
    m_itemIcon = m_toolBox->itemIcon(m_index);
}

void ToolBoxCommand::removePage()
{
    m_toolBox->removeItem(m_index);

    m_widget->hide();
    m_widget->setParent(formWindow());
}

void ToolBoxCommand::addPage()
{
    m_widget->setParent(m_toolBox);
    m_toolBox->insertItem(m_index, m_widget, m_itemIcon, m_itemText);
    m_toolBox->setCurrentIndex(m_index);

    m_widget->show();
}

// ---- MoveToolBoxPageCommand ----
MoveToolBoxPageCommand::MoveToolBoxPageCommand(QDesignerFormWindowInterface *formWindow)
    : ToolBoxCommand(formWindow)
{
}

MoveToolBoxPageCommand::~MoveToolBoxPageCommand()
{
}

void MoveToolBoxPageCommand::init(QToolBox *toolBox, QWidget *page, int newIndex)
{
    ToolBoxCommand::init(toolBox);
    setText(QApplication::translate("Command", "Move Page"));

    m_widget = page;
    m_oldIndex = m_toolBox->indexOf(m_widget);
    m_itemText = m_toolBox->itemText(m_oldIndex);
    m_itemIcon = m_toolBox->itemIcon(m_oldIndex);
    m_newIndex = newIndex;
}

void MoveToolBoxPageCommand::redo()
{
    m_toolBox->removeItem(m_oldIndex);
    m_toolBox->insertItem(m_newIndex, m_widget, m_itemIcon, m_itemText);
}

void MoveToolBoxPageCommand::undo()
{
    m_toolBox->removeItem(m_newIndex);
    m_toolBox->insertItem(m_oldIndex, m_widget, m_itemIcon, m_itemText);
}

// ---- DeleteToolBoxPageCommand ----
DeleteToolBoxPageCommand::DeleteToolBoxPageCommand(QDesignerFormWindowInterface *formWindow)
    : ToolBoxCommand(formWindow)
{
}

DeleteToolBoxPageCommand::~DeleteToolBoxPageCommand()
{
}

void DeleteToolBoxPageCommand::init(QToolBox *toolBox)
{
    ToolBoxCommand::init(toolBox);
    setText(QApplication::translate("Command", "Delete Page"));
}

void DeleteToolBoxPageCommand::redo()
{
    removePage();
    cheapUpdate();
}

void DeleteToolBoxPageCommand::undo()
{
    addPage();
    cheapUpdate();
}

// ---- AddToolBoxPageCommand ----
AddToolBoxPageCommand::AddToolBoxPageCommand(QDesignerFormWindowInterface *formWindow)
    : ToolBoxCommand(formWindow)
{
}

AddToolBoxPageCommand::~AddToolBoxPageCommand()
{
}

void AddToolBoxPageCommand::init(QToolBox *toolBox)
{
    init(toolBox, InsertBefore);
}

void AddToolBoxPageCommand::init(QToolBox *toolBox, InsertionMode mode)
{
    m_toolBox = toolBox;

    m_index = m_toolBox->currentIndex();
    if (mode == InsertAfter)
        m_index++;
    m_widget = new QDesignerWidget(formWindow(), m_toolBox);
    m_itemText = QApplication::translate("Command", "Page");
    m_itemIcon = QIcon();
    m_widget->setObjectName(QApplication::translate("Command", "page"));
    formWindow()->ensureUniqueObjectName(m_widget);

    setText(QApplication::translate("Command", "Insert Page"));

    QDesignerFormEditorInterface *core = formWindow()->core();
    core->metaDataBase()->add(m_widget);
}

void AddToolBoxPageCommand::redo()
{
    addPage();
    cheapUpdate();
}

void AddToolBoxPageCommand::undo()
{
    removePage();
    cheapUpdate();
}

// ---- TabWidgetCommand ----
TabWidgetCommand::TabWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{
}

TabWidgetCommand::~TabWidgetCommand()
{
}

void TabWidgetCommand::init(QTabWidget *tabWidget)
{
    m_tabWidget = tabWidget;
    m_index = m_tabWidget->currentIndex();
    m_widget = m_tabWidget->widget(m_index);
    m_itemText = m_tabWidget->tabText(m_index);
    m_itemIcon = m_tabWidget->tabIcon(m_index);
}

void TabWidgetCommand::removePage()
{
    m_tabWidget->removeTab(m_index);

    m_widget->hide();
    m_widget->setParent(formWindow());
    m_tabWidget->setCurrentIndex(qMin(m_index, m_tabWidget->count()));
}

void TabWidgetCommand::addPage()
{
    m_widget->setParent(0);
    m_tabWidget->insertTab(m_index, m_widget, m_itemIcon, m_itemText);
    m_widget->show();
    m_tabWidget->setCurrentIndex(m_index);
}

// ---- DeleteTabPageCommand ----
DeleteTabPageCommand::DeleteTabPageCommand(QDesignerFormWindowInterface *formWindow)
    : TabWidgetCommand(formWindow)
{
}

DeleteTabPageCommand::~DeleteTabPageCommand()
{
}

void DeleteTabPageCommand::init(QTabWidget *tabWidget)
{
    TabWidgetCommand::init(tabWidget);
    setText(QApplication::translate("Command", "Delete Page"));
}

void DeleteTabPageCommand::redo()
{
    removePage();
    cheapUpdate();
}

void DeleteTabPageCommand::undo()
{
    addPage();
    cheapUpdate();
}

// ---- AddTabPageCommand ----
AddTabPageCommand::AddTabPageCommand(QDesignerFormWindowInterface *formWindow)
    : TabWidgetCommand(formWindow)
{
}

AddTabPageCommand::~AddTabPageCommand()
{
}

void AddTabPageCommand::init(QTabWidget *tabWidget)
{
    init(tabWidget, InsertBefore);
}

void AddTabPageCommand::init(QTabWidget *tabWidget, InsertionMode mode)
{
    m_tabWidget = tabWidget;

    m_index = m_tabWidget->currentIndex();
    if (mode == InsertAfter)
        m_index++;
    m_widget = new QDesignerWidget(formWindow(), m_tabWidget);
    m_itemText = QApplication::translate("Command", "Page");
    m_itemIcon = QIcon();
    m_widget->setObjectName(QApplication::translate("Command", "tab"));
    formWindow()->ensureUniqueObjectName(m_widget);

    setText(QApplication::translate("Command", "Insert Page"));

    QDesignerFormEditorInterface *core = formWindow()->core();
    core->metaDataBase()->add(m_widget);
}

void AddTabPageCommand::redo()
{
    addPage();
    cheapUpdate();
}

void AddTabPageCommand::undo()
{
    removePage();
    cheapUpdate();
}

// ---- MoveTabPageCommand ----
MoveTabPageCommand::MoveTabPageCommand(QDesignerFormWindowInterface *formWindow)
    : TabWidgetCommand(formWindow)
{
}

MoveTabPageCommand::~MoveTabPageCommand()
{
}

void MoveTabPageCommand::init(QTabWidget *tabWidget, QWidget *page,
                      const QIcon &icon, const QString &label,
                      int index, int newIndex)
{
    TabWidgetCommand::init(tabWidget);
    setText(QApplication::translate("Command", "Move Page"));

    m_page = page;
    m_newIndex = newIndex;
    m_oldIndex = index;
    m_label = label;
    m_icon = icon;
}

void MoveTabPageCommand::redo()
{
    m_tabWidget->removeTab(m_oldIndex);
    m_tabWidget->insertTab(m_newIndex, m_page, m_icon, m_label);
    m_tabWidget->setCurrentIndex(m_newIndex);
}

void MoveTabPageCommand::undo()
{
    m_tabWidget->removeTab(m_newIndex);
    m_tabWidget->insertTab(m_oldIndex, m_page, m_icon, m_label);
    m_tabWidget->setCurrentIndex(m_oldIndex);
}

// ---- StackedWidgetCommand ----
StackedWidgetCommand::StackedWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{
}

StackedWidgetCommand::~StackedWidgetCommand()
{
}

void StackedWidgetCommand::init(QStackedWidget *stackedWidget)
{
    m_stackedWidget = stackedWidget;
    m_index = m_stackedWidget->currentIndex();
    m_widget = m_stackedWidget->widget(m_index);
}

void StackedWidgetCommand::removePage()
{
    m_stackedWidget->removeWidget(m_stackedWidget->widget(m_index));

    m_widget->hide();
    m_widget->setParent(formWindow());
}

void StackedWidgetCommand::addPage()
{
    m_stackedWidget->insertWidget(m_index, m_widget);

    m_widget->show();
    m_stackedWidget->setCurrentIndex(m_index);
}

// ---- MoveStackedWidgetCommand ----
MoveStackedWidgetCommand::MoveStackedWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : StackedWidgetCommand(formWindow)
{
}

MoveStackedWidgetCommand::~MoveStackedWidgetCommand()
{
}

void MoveStackedWidgetCommand::init(QStackedWidget *stackedWidget, QWidget *page, int newIndex)
{
    StackedWidgetCommand::init(stackedWidget);
    setText(QApplication::translate("Command", "Move Page"));

    m_widget = page;
    m_newIndex = newIndex;
    m_oldIndex = m_stackedWidget->indexOf(m_widget);
}

void MoveStackedWidgetCommand::redo()
{
    m_stackedWidget->removeWidget(m_widget);
    m_stackedWidget->insertWidget(m_newIndex, m_widget);
}

void MoveStackedWidgetCommand::undo()
{
    m_stackedWidget->removeWidget(m_widget);
    m_stackedWidget->insertWidget(m_oldIndex, m_widget);
}

// ---- DeleteStackedWidgetPageCommand ----
DeleteStackedWidgetPageCommand::DeleteStackedWidgetPageCommand(QDesignerFormWindowInterface *formWindow)
    : StackedWidgetCommand(formWindow)
{
}

DeleteStackedWidgetPageCommand::~DeleteStackedWidgetPageCommand()
{
}

void DeleteStackedWidgetPageCommand::init(QStackedWidget *stackedWidget)
{
    StackedWidgetCommand::init(stackedWidget);
    setText(QApplication::translate("Command", "Delete Page"));
}

void DeleteStackedWidgetPageCommand::redo()
{
    removePage();
    cheapUpdate();
}

void DeleteStackedWidgetPageCommand::undo()
{
    addPage();
    cheapUpdate();
}

// ---- AddStackedWidgetPageCommand ----
AddStackedWidgetPageCommand::AddStackedWidgetPageCommand(QDesignerFormWindowInterface *formWindow)
    : StackedWidgetCommand(formWindow)
{
}

AddStackedWidgetPageCommand::~AddStackedWidgetPageCommand()
{
}

void AddStackedWidgetPageCommand::init(QStackedWidget *stackedWidget)
{
    init(stackedWidget, InsertBefore);
}

void AddStackedWidgetPageCommand::init(QStackedWidget *stackedWidget, InsertionMode mode)
{
    m_stackedWidget = stackedWidget;

    m_index = m_stackedWidget->currentIndex();
    if (mode == InsertAfter)
        m_index++;
    m_widget = new QDesignerWidget(formWindow(), m_stackedWidget);
    m_widget->setObjectName(QApplication::translate("Command", "page"));
    formWindow()->ensureUniqueObjectName(m_widget);

    setText(QApplication::translate("Command", "Insert Page"));

    QDesignerFormEditorInterface *core = formWindow()->core();
    core->metaDataBase()->add(m_widget);
}

void AddStackedWidgetPageCommand::redo()
{
    addPage();
    cheapUpdate();
}

void AddStackedWidgetPageCommand::undo()
{
    removePage();
    cheapUpdate();
}

// ---- TabOrderCommand ----
TabOrderCommand::TabOrderCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Change Tab order"), formWindow),
      m_widgetItem(0)
{
}

void TabOrderCommand::init(const QList<QWidget*> &newTabOrder)
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    Q_ASSERT(core);

    m_widgetItem = core->metaDataBase()->item(formWindow());
    Q_ASSERT(m_widgetItem);
    m_oldTabOrder = m_widgetItem->tabOrder();
    m_newTabOrder = newTabOrder;
}

void TabOrderCommand::redo()
{
    m_widgetItem->setTabOrder(m_newTabOrder);
}

void TabOrderCommand::undo()
{
    m_widgetItem->setTabOrder(m_oldTabOrder);
}

// ---- CreateMenuBarCommand ----
CreateMenuBarCommand::CreateMenuBarCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Create Menu Bar"), formWindow)
{
}

void CreateMenuBarCommand::init(QMainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    QDesignerFormEditorInterface *core = formWindow()->core();
    m_menuBar = qobject_cast<QMenuBar*>(core->widgetFactory()->createWidget("QMenuBar", m_mainWindow));
    core->widgetFactory()->initialize(m_menuBar);
}

void CreateMenuBarCommand::redo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerContainerExtension *c;
    c = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), m_mainWindow);
    c->addWidget(m_menuBar);

    m_menuBar->setObjectName("menuBar");
    formWindow()->ensureUniqueObjectName(m_menuBar);
    core->metaDataBase()->add(m_menuBar);
    formWindow()->emitSelectionChanged();
    m_menuBar->setFocus();
}

void CreateMenuBarCommand::undo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerContainerExtension *c;
    c = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), m_mainWindow);
    for (int i = 0; i < c->count(); ++i) {
        if (c->widget(i) == m_menuBar) {
            c->remove(i);
            break;
        }
    }

    core->metaDataBase()->remove(m_menuBar);
    formWindow()->emitSelectionChanged();
}

// ---- DeleteMenuBarCommand ----
DeleteMenuBarCommand::DeleteMenuBarCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Delete Menu Bar"), formWindow)
{
}

void DeleteMenuBarCommand::init(QMenuBar *menuBar)
{
    m_menuBar = menuBar;
    m_mainWindow = qobject_cast<QMainWindow*>(menuBar->parentWidget());
}

void DeleteMenuBarCommand::redo()
{
    if (m_mainWindow) {
        QDesignerContainerExtension *c;
        c = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), m_mainWindow);
        Q_ASSERT(c != 0);
        for (int i=0; i<c->count(); ++i) {
            if (c->widget(i) == m_menuBar) {
                c->remove(i);
                break;
            }
        }
    }

    core()->metaDataBase()->remove(m_menuBar);
    m_menuBar->hide();
    m_menuBar->setParent(formWindow());
    formWindow()->emitSelectionChanged();
}

void DeleteMenuBarCommand::undo()
{
    if (m_mainWindow) {
        m_menuBar->setParent(m_mainWindow);
        QDesignerContainerExtension *c;
        c = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), m_mainWindow);

        c->addWidget(m_menuBar);

        core()->metaDataBase()->add(m_menuBar);
        m_menuBar->show();
        formWindow()->emitSelectionChanged();
    }
}

// ---- CreateStatusBarCommand ----
CreateStatusBarCommand::CreateStatusBarCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Create Status Bar"), formWindow)
{
}

void CreateStatusBarCommand::init(QMainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    QDesignerFormEditorInterface *core = formWindow()->core();
    m_statusBar = qobject_cast<QStatusBar*>(core->widgetFactory()->createWidget("QStatusBar", m_mainWindow));
    core->widgetFactory()->initialize(m_statusBar);
}

void CreateStatusBarCommand::redo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerContainerExtension *c;
    c = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), m_mainWindow);
    c->addWidget(m_statusBar);

    m_statusBar->setObjectName("statusBar");
    formWindow()->ensureUniqueObjectName(m_statusBar);
    core->metaDataBase()->add(m_statusBar);
    formWindow()->emitSelectionChanged();
}

void CreateStatusBarCommand::undo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerContainerExtension *c;
    c = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), m_mainWindow);
    for (int i = 0; i < c->count(); ++i) {
        if (c->widget(i) == m_statusBar) {
            c->remove(i);
            break;
        }
    }

    core->metaDataBase()->remove(m_statusBar);
    formWindow()->emitSelectionChanged();
}

// ---- DeleteStatusBarCommand ----
DeleteStatusBarCommand::DeleteStatusBarCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Delete Status Bar"), formWindow)
{
}

void DeleteStatusBarCommand::init(QStatusBar *statusBar)
{
    m_statusBar = statusBar;
    m_mainWindow = qobject_cast<QMainWindow*>(statusBar->parentWidget());
}

void DeleteStatusBarCommand::redo()
{
    if (m_mainWindow) {
        QDesignerContainerExtension *c;
        c = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), m_mainWindow);
        Q_ASSERT(c != 0);
        for (int i=0; i<c->count(); ++i) {
            if (c->widget(i) == m_statusBar) {
                c->remove(i);
                break;
            }
        }
    }

    core()->metaDataBase()->remove(m_statusBar);
    m_statusBar->hide();
    m_statusBar->setParent(formWindow());
    formWindow()->emitSelectionChanged();
}

void DeleteStatusBarCommand::undo()
{
    if (m_mainWindow) {
        m_statusBar->setParent(m_mainWindow);
        QDesignerContainerExtension *c;
        c = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), m_mainWindow);

        c->addWidget(m_statusBar);

        core()->metaDataBase()->add(m_statusBar);
        m_statusBar->show();
        formWindow()->emitSelectionChanged();
    }
}

// ---- AddToolBarCommand ----
AddToolBarCommand::AddToolBarCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Add Tool Bar"), formWindow)
{
}

void AddToolBarCommand::init(QMainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    QDesignerFormEditorInterface *core = formWindow()->core();
    m_toolBar = qobject_cast<QToolBar*>(core->widgetFactory()->createWidget("QToolBar", m_mainWindow));
    m_toolBar->hide();
}

void AddToolBarCommand::redo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    core->metaDataBase()->add(m_toolBar);

    QDesignerContainerExtension *c;
    c = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), m_mainWindow);
    c->addWidget(m_toolBar);

    m_toolBar->setObjectName("toolBar");
    formWindow()->ensureUniqueObjectName(m_toolBar);
    formWindow()->emitSelectionChanged();
}

void AddToolBarCommand::undo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    core->metaDataBase()->remove(m_toolBar);
    QDesignerContainerExtension *c;
    c = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), m_mainWindow);
    for (int i = 0; i < c->count(); ++i) {
        if (c->widget(i) == m_toolBar) {
            c->remove(i);
            break;
        }
    }
    formWindow()->emitSelectionChanged();
}

// ---- DockWidgetCommand:: ----
DockWidgetCommand::DockWidgetCommand(const QString &description, QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(description, formWindow)
{
}

DockWidgetCommand::~DockWidgetCommand()
{
}

void DockWidgetCommand::init(QDockWidget *dockWidget)
{
    m_dockWidget = dockWidget;
}

// ---- SetDockWidgetCommand ----
SetDockWidgetCommand::SetDockWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : DockWidgetCommand(QApplication::translate("Command", "Set Dock Window Widget"), formWindow)
{
}

void SetDockWidgetCommand::init(QDockWidget *dockWidget, QWidget *widget)
{
    DockWidgetCommand::init(dockWidget);
    m_widget = widget;
    m_oldWidget = dockWidget->widget();
}

void SetDockWidgetCommand::undo()
{
    m_dockWidget->setWidget(m_oldWidget);
}

void SetDockWidgetCommand::redo()
{
    formWindow()->unmanageWidget(m_widget);
    formWindow()->core()->metaDataBase()->add(m_widget);
    m_dockWidget->setWidget(m_widget);
}

// ---- AddDockWidgetCommand ----
AddDockWidgetCommand::AddDockWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Add Dock Window"), formWindow)
{
}

void AddDockWidgetCommand::init(QMainWindow *mainWindow, QDockWidget *dockWidget)
{
    m_mainWindow = mainWindow;
    m_dockWidget = dockWidget;
}

void AddDockWidgetCommand::init(QMainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    QDesignerFormEditorInterface *core = formWindow()->core();
    m_dockWidget = qobject_cast<QDockWidget*>(core->widgetFactory()->createWidget("QDockWidget", m_mainWindow));
}

void AddDockWidgetCommand::redo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerContainerExtension *c;
    c = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), m_mainWindow);
    c->addWidget(m_dockWidget);

    m_dockWidget->setObjectName("dockWidget");
    formWindow()->ensureUniqueObjectName(m_dockWidget);
    formWindow()->manageWidget(m_dockWidget);
    formWindow()->emitSelectionChanged();
}

void AddDockWidgetCommand::undo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerContainerExtension *c;
    c = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), m_mainWindow);
    for (int i = 0; i < c->count(); ++i) {
        if (c->widget(i) == m_dockWidget) {
            c->remove(i);
            break;
        }
    }

    formWindow()->unmanageWidget(m_dockWidget);
    formWindow()->emitSelectionChanged();
}

// ---- AdjustWidgetSizeCommand ----
AdjustWidgetSizeCommand::AdjustWidgetSizeCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{
}

void AdjustWidgetSizeCommand::init(QWidget *widget)
{
    m_widget = widget;
    setText(QApplication::translate("Command", "Adjust Size of '%1'").arg(widget->objectName()));
    m_geometry = m_widget->geometry();
}

void AdjustWidgetSizeCommand::redo()
{
    QWidget *widget = m_widget;
    if (Utils::isCentralWidget(formWindow(), widget) && formWindow()->parentWidget())
        widget = formWindow()->parentWidget();

    m_geometry = widget->geometry();
    if (widget != m_widget && widget->parentWidget()) {
        QApplication::processEvents();
        widget->parentWidget()->adjustSize();
    }
    QApplication::processEvents();
    widget->adjustSize();

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == m_widget)
            propertyEditor->setPropertyValue(QLatin1String("geometry"), m_widget->geometry(), true);
    }
}

void AdjustWidgetSizeCommand::undo()
{
    if (formWindow()->mainContainer() == m_widget && formWindow()->parentWidget()) {
        formWindow()->parentWidget()->resize(m_geometry.width(), m_geometry.height());
        QWidget *widget = formWindow()->parentWidget();
        if (widget->parentWidget()) {
            widget->parentWidget()->setGeometry(m_geometry);
        }
    } else {
        m_widget->setGeometry(m_geometry);
    }

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == m_widget)
            propertyEditor->setPropertyValue(QLatin1String("geometry"), m_widget->geometry(), true);
    }
}

// ---- ChangeLayoutItemGeometry ----
ChangeLayoutItemGeometry::ChangeLayoutItemGeometry(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Change Layout Item Geometry"), formWindow)
{
}

void ChangeLayoutItemGeometry::init(QWidget *widget, int row, int column, int rowspan, int colspan)
{
    m_widget = widget;
    Q_ASSERT(m_widget->parentWidget() != 0);

    QLayout *layout = LayoutInfo::managedLayout(formWindow()->core(), m_widget->parentWidget());
    Q_ASSERT(layout != 0);

    QGridLayout *grid = qobject_cast<QGridLayout*>(layout);
    Q_ASSERT(grid != 0);

    int itemIndex = grid->indexOf(m_widget);
    Q_ASSERT(itemIndex != -1);

    int current_row, current_column, current_rowspan, current_colspan;
    grid->getItemPosition(itemIndex, &current_row, &current_column, &current_rowspan, &current_colspan);

    m_oldInfo.setRect(current_column, current_row, current_colspan, current_rowspan);
    m_newInfo.setRect(column, row, colspan, rowspan);
}

void ChangeLayoutItemGeometry::changeItemPosition(const QRect &g)
{
    QLayout *layout = LayoutInfo::managedLayout(formWindow()->core(), m_widget->parentWidget());
    Q_ASSERT(layout != 0);

    QGridLayout *grid = qobject_cast<QGridLayout*>(layout);
    Q_ASSERT(grid != 0);

    int itemIndex = grid->indexOf(m_widget);
    Q_ASSERT(itemIndex != -1);

    QLayoutItem *item = grid->takeAt(itemIndex);
    delete item;

    add_to_grid_layout(grid, m_widget, g.top(), g.left(), g.height(), g.width());

    grid->invalidate();
    grid->activate();

    QLayoutSupport::createEmptyCells(grid);

    formWindow()->clearSelection(false);
    formWindow()->selectWidget(m_widget, true);
}

void ChangeLayoutItemGeometry::redo()
{
    changeItemPosition(m_newInfo);
}

void ChangeLayoutItemGeometry::undo()
{
    changeItemPosition(m_oldInfo);
}

// ---- InsertRowCommand ----
InsertRowCommand::InsertRowCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Insert Row"), formWindow)
{
}

void InsertRowCommand::init(QWidget *widget, int row)
{
    m_widget = widget;
    m_row = row;
}

void InsertRowCommand::redo()
{
}

void InsertRowCommand::undo()
{
}



// ---- ContainerWidgetCommand ----
ContainerWidgetCommand::ContainerWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{
}

ContainerWidgetCommand::~ContainerWidgetCommand()
{
}

QDesignerContainerExtension *ContainerWidgetCommand::containerExtension() const
{
    QExtensionManager *mgr = core()->extensionManager();
    return qt_extension<QDesignerContainerExtension*>(mgr, m_containerWidget);
}

void ContainerWidgetCommand::init(QWidget *containerWidget)
{
    m_containerWidget = containerWidget;

    if (QDesignerContainerExtension *c = containerExtension()) {
        m_index = c->currentIndex();
        m_widget = c->widget(m_index);
    }
}

void ContainerWidgetCommand::removePage()
{
    if (QDesignerContainerExtension *c = containerExtension()) {
        c->remove(m_index);

        m_widget->hide();
        m_widget->setParent(formWindow());
    }
}

void ContainerWidgetCommand::addPage()
{
    if (QDesignerContainerExtension *c = containerExtension()) {
        c->insertWidget(m_index, m_widget);

        m_widget->show();
        c->setCurrentIndex(m_index);
    }
}

// ---- DeleteContainerWidgetPageCommand ----
DeleteContainerWidgetPageCommand::DeleteContainerWidgetPageCommand(QDesignerFormWindowInterface *formWindow)
    : ContainerWidgetCommand(formWindow)
{
}

DeleteContainerWidgetPageCommand::~DeleteContainerWidgetPageCommand()
{
}

void DeleteContainerWidgetPageCommand::init(QWidget *containerWidget)
{
    ContainerWidgetCommand::init(containerWidget);
    setText(QApplication::translate("Command", "Delete Page"));
}

void DeleteContainerWidgetPageCommand::redo()
{
    removePage();
    cheapUpdate();
}

void DeleteContainerWidgetPageCommand::undo()
{
    addPage();
    cheapUpdate();
}

// ---- AddContainerWidgetPageCommand ----
AddContainerWidgetPageCommand::AddContainerWidgetPageCommand(QDesignerFormWindowInterface *formWindow)
    : ContainerWidgetCommand(formWindow)
{
}

AddContainerWidgetPageCommand::~AddContainerWidgetPageCommand()
{
}

void AddContainerWidgetPageCommand::init(QWidget *containerWidget)
{
    init(containerWidget, InsertBefore);
}

void AddContainerWidgetPageCommand::init(QWidget *containerWidget, InsertionMode mode)
{
    m_containerWidget = containerWidget;

    if (QDesignerContainerExtension *c = containerExtension()) {
        m_index = c->currentIndex();
        if (mode == InsertAfter)
            m_index++;
        m_widget = new QDesignerWidget(formWindow(), m_containerWidget);
        m_widget->setObjectName(QApplication::translate("Command", "page"));
        formWindow()->ensureUniqueObjectName(m_widget);

        setText(QApplication::translate("Command", "Insert Page"));

        QDesignerFormEditorInterface *core = formWindow()->core();
        core->metaDataBase()->add(m_widget);
    }
}

void AddContainerWidgetPageCommand::redo()
{
    addPage();
    cheapUpdate();
}

void AddContainerWidgetPageCommand::undo()
{
    removePage();
    cheapUpdate();
}

// ---- ChangeTableContentsCommand ----
ChangeTableContentsCommand::ChangeTableContentsCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Change Table Contents"), formWindow),
        m_oldColumnCount(0),
        m_newColumnCount(0),
        m_oldRowCount(0),
        m_newRowCount(0)
{

}

void ChangeTableContentsCommand::init(QTableWidget *tableWidget, QTableWidget *fromTableWidget)
{
    m_tableWidget = tableWidget;

    m_oldItemsState.clear();
    m_newItemsState.clear();
    m_oldHorizontalHeaderState.clear();
    m_newHorizontalHeaderState.clear();
    m_oldVerticalHeaderState.clear();
    m_newVerticalHeaderState.clear();

    m_oldColumnCount = tableWidget->columnCount();
    m_oldRowCount = tableWidget->rowCount();
    m_newColumnCount = fromTableWidget->columnCount();
    m_newRowCount = fromTableWidget->rowCount();

    for (int col = 0; col < m_oldColumnCount; col++) {
        QTableWidgetItem *item = tableWidget->horizontalHeaderItem(col);
        if (item) {
            QString text = item->text();
            QIcon icon = item->icon();
            if (!text.isEmpty() || !icon.isNull()) {
                m_oldHorizontalHeaderState[col] =
                            qMakePair<QString, QIcon>(text, icon);
            }
        }
    }

    for (int row = 0; row < m_oldRowCount; row++) {
        QTableWidgetItem *item = tableWidget->verticalHeaderItem(row);
        if (item) {
            QString text = item->text();
            QIcon icon = item->icon();
            if (!text.isEmpty() || !icon.isNull()) {
                m_oldVerticalHeaderState[row] =
                            qMakePair<QString, QIcon>(text, icon);
            }
        }
    }

    for (int col = 0; col < m_oldColumnCount; col++) {
        for (int row = 0; row < m_oldRowCount; row++) {
            QTableWidgetItem *item = tableWidget->item(row, col);
            if (item) {
                QString text = item->text();
                QIcon icon = item->icon();
                if (!text.isEmpty() || !icon.isNull()) {
                    m_oldItemsState[qMakePair<int, int>(row, col)] =
                                qMakePair<QString, QIcon>(text, icon);
                }
            }
        }
    }


    for (int col = 0; col < m_newColumnCount; col++) {
        QTableWidgetItem *item = fromTableWidget->horizontalHeaderItem(col);
        if (item) {
            QString text = item->text();
            QIcon icon = item->icon();
            if (!text.isEmpty() || !icon.isNull()) {
                m_newHorizontalHeaderState[col] =
                            qMakePair<QString, QIcon>(text, icon);
            }
        }
    }

    for (int row = 0; row < m_newRowCount; row++) {
        QTableWidgetItem *item = fromTableWidget->verticalHeaderItem(row);
        if (item) {
            QString text = item->text();
            QIcon icon = item->icon();
            if (!text.isEmpty() || !icon.isNull()) {
                m_newVerticalHeaderState[row] =
                            qMakePair<QString, QIcon>(text, icon);
            }
        }
    }

    for (int col = 0; col < m_newColumnCount; col++) {
        for (int row = 0; row < m_newRowCount; row++) {
            QTableWidgetItem *item = fromTableWidget->item(row, col);
            if (item) {
                QString text = item->text();
                QIcon icon = item->icon();
                if (!text.isEmpty() || !icon.isNull()) {
                    m_newItemsState[qMakePair<int, int>(row, col)] =
                                qMakePair<QString, QIcon>(text, icon);
                }
            }
        }
    }
}

void ChangeTableContentsCommand::changeContents(QTableWidget *tableWidget,
        int rowCount, int columnCount,
        const QMap<int, QPair<QString, QIcon> > &horizontalHeaderState,
        const QMap<int, QPair<QString, QIcon> > &verticalHeaderState,
        const QMap<QPair<int, int>, QPair<QString, QIcon> > &itemsState) const
{
    tableWidget->clear();

    tableWidget->setColumnCount(columnCount);
    QMap<int, QPair<QString, QIcon> >::ConstIterator itColumn =
                horizontalHeaderState.constBegin();
    while (itColumn != horizontalHeaderState.constEnd()) {
        int column = itColumn.key();
        QString text = itColumn.value().first;
        QIcon icon = itColumn.value().second;
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setText(text);
        item->setIcon(icon);
        tableWidget->setHorizontalHeaderItem(column, item);

        itColumn++;
    }

    tableWidget->setRowCount(rowCount);
    QMap<int, QPair<QString, QIcon> >::ConstIterator itRow =
                verticalHeaderState.constBegin();
    while (itRow != verticalHeaderState.constEnd()) {
        int row = itRow.key();
        QString text = itRow.value().first;
        QIcon icon = itRow.value().second;
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setText(text);
        item->setIcon(icon);
        tableWidget->setVerticalHeaderItem(row, item);

        itRow++;
    }

    QMap<QPair<int, int>, QPair<QString, QIcon> >::ConstIterator itItem =
                itemsState.constBegin();
    while (itItem != itemsState.constEnd()) {
        int row = itItem.key().first;
        int column = itItem.key().second;
        QString text = itItem.value().first;
        QIcon icon = itItem.value().second;
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setText(text);
        item->setIcon(icon);
        tableWidget->setItem(row, column, item);

        itItem++;
    }
}

void ChangeTableContentsCommand::redo()
{
    changeContents(m_tableWidget, m_newRowCount, m_newColumnCount,
                m_newHorizontalHeaderState, m_newVerticalHeaderState, m_newItemsState);
}

void ChangeTableContentsCommand::undo()
{
    changeContents(m_tableWidget, m_oldRowCount, m_oldColumnCount,
                m_oldHorizontalHeaderState, m_oldVerticalHeaderState, m_oldItemsState);
}

// ---- ChangeTreeContentsCommand ----
ChangeTreeContentsCommand::ChangeTreeContentsCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Change Tree Contents"), formWindow),
        m_oldHeaderItemState(0),
        m_newHeaderItemState(0),
        m_oldColumnCount(0),
        m_newColumnCount(0)
{

}

ChangeTreeContentsCommand::~ChangeTreeContentsCommand()
{
    clearState(m_oldItemsState, m_oldHeaderItemState);
    clearState(m_newItemsState, m_newHeaderItemState);
}

void ChangeTreeContentsCommand::init(QTreeWidget *treeWidget, QTreeWidget *fromTreeWidget)
{
    m_treeWidget = treeWidget;
    m_oldColumnCount = treeWidget->columnCount();
    m_newColumnCount = fromTreeWidget->columnCount();

    initState(m_oldItemsState, m_oldHeaderItemState, treeWidget);
    initState(m_newItemsState, m_newHeaderItemState, fromTreeWidget);
}

void ChangeTreeContentsCommand::redo()
{
    changeContents(m_treeWidget, m_newColumnCount, m_newItemsState, m_newHeaderItemState);
}

void ChangeTreeContentsCommand::undo()
{
    changeContents(m_treeWidget, m_oldColumnCount, m_oldItemsState, m_oldHeaderItemState);
}

void ChangeTreeContentsCommand::initState(QList<QTreeWidgetItem *> &itemsState,
                QTreeWidgetItem *&headerItemState, QTreeWidget *fromTreeWidget) const
{
    clearState(itemsState, headerItemState);

    for (int i = 0; i < fromTreeWidget->topLevelItemCount(); i++)
        itemsState.append(fromTreeWidget->topLevelItem(i)->clone());

    headerItemState = fromTreeWidget->headerItem()->clone();
}

void ChangeTreeContentsCommand::changeContents(QTreeWidget *treeWidget, int columnCount,
        const QList<QTreeWidgetItem *> &itemsState, const QTreeWidgetItem *headerItemState) const
{
    treeWidget->clear();
    treeWidget->setColumnCount(columnCount);

    treeWidget->setHeaderItem(headerItemState->clone());

    if (!columnCount)
        return;

    foreach (QTreeWidgetItem *item, itemsState)
        treeWidget->addTopLevelItem(item->clone());
}

void ChangeTreeContentsCommand::clearState(QList<QTreeWidgetItem *> &itemsState,
            QTreeWidgetItem *&headerItemState) const
{
    QListIterator<QTreeWidgetItem *> it(itemsState);
    while (it.hasNext())
        delete it.next();
    itemsState.clear();
    if (headerItemState)
        delete headerItemState;
    headerItemState = 0;
}

// ---- ChangeListContentsCommand ----
ChangeListContentsCommand::ChangeListContentsCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{

}

void ChangeListContentsCommand::init(QListWidget *listWidget,
                const QList<QPair<QString, QIcon> > &items)
{
    m_listWidget = listWidget;
    m_comboBox = 0;

    m_newItemsState = items;
    m_oldItemsState.clear();

    for (int i = 0; i < listWidget->count(); i++) {
        QListWidgetItem *item = listWidget->item(i);
        QString text = item->text();
        QIcon icon = item->icon();

        m_oldItemsState.append(qMakePair<QString, QIcon>(text, icon));
    }
}

void ChangeListContentsCommand::init(QComboBox *comboBox,
                const QList<QPair<QString, QIcon> > &items)
{
    m_listWidget = 0;
    m_comboBox = comboBox;

    m_newItemsState = items;
    m_oldItemsState.clear();

    for (int i = 0; i < comboBox->count(); i++) {
        QString text = comboBox->itemText(i);
        QIcon icon = comboBox->itemIcon(i);

        m_oldItemsState.append(qMakePair<QString, QIcon>(text, icon));
    }
}

void ChangeListContentsCommand::redo()
{
    if (m_listWidget)
        changeContents(m_listWidget, m_newItemsState);
    else if (m_comboBox)
        changeContents(m_comboBox, m_newItemsState);
}

void ChangeListContentsCommand::undo()
{
    if (m_listWidget)
        changeContents(m_listWidget, m_oldItemsState);
    else if (m_comboBox)
        changeContents(m_comboBox, m_oldItemsState);
}

void ChangeListContentsCommand::changeContents(QListWidget *listWidget,
        const QList<QPair<QString, QIcon> > &itemsState) const
{
    listWidget->clear();
    QListIterator<QPair<QString, QIcon> > it(itemsState);
    while (it.hasNext()) {
        QPair<QString, QIcon> pair = it.next();
        QListWidgetItem *item = new QListWidgetItem(pair.second, pair.first);
        listWidget->addItem(item);
    }
}

void ChangeListContentsCommand::changeContents(QComboBox *comboBox,
        const QList<QPair<QString, QIcon> > &itemsState) const
{
    comboBox->clear();
    QListIterator<QPair<QString, QIcon> > it(itemsState);
    while (it.hasNext()) {
        QPair<QString, QIcon> pair = it.next();
        comboBox->addItem(pair.second, pair.first);
        comboBox->setItemData(comboBox->count() - 1, pair.second);
    }
}

// ---- AddActionCommand ----

AddActionCommand::AddActionCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QLatin1String("Add action"), formWindow)
{
    m_action = 0;
}

void AddActionCommand::init(QAction *action)
{
    Q_ASSERT(m_action == 0);
    m_action = action;
}

void AddActionCommand::redo()
{
    core()->actionEditor()->setFormWindow(formWindow());
    core()->actionEditor()->manageAction(m_action);
}

void AddActionCommand::undo()
{
    core()->actionEditor()->setFormWindow(formWindow());
    core()->actionEditor()->unmanageAction(m_action);
}

// ---- RemoveActionCommand ----

RemoveActionCommand::RemoveActionCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QLatin1String("Remove action"), formWindow)
{
    m_action = 0;
}

template <typename T>
static RemoveActionCommand::ActionData findActionIn(QDesignerFormWindowInterface *formWindow,
                                                    QAction *action)
{
    RemoveActionCommand::ActionData result;

    QList<T*> widgetList = qFindChildren<T*>(formWindow->mainContainer());
    foreach (QWidget *widget, widgetList) {
        QList<QAction*> actionList = widget->actions();
        for (int i = 0; i < actionList.size(); ++i) {
            if (actionList.at(i) == action) {
                QAction *before = 0;
                if (i + 1 < actionList.size())
                    before = actionList.at(i + 1);
                result.append(RemoveActionCommand::ActionDataItem(before, widget));
                break;
            }
        }
    }

    return result;
}

void RemoveActionCommand::init(QAction *action)
{
    Q_ASSERT(m_action == 0);
    m_action = action;

    m_actionData = findActionIn<QMenu>(formWindow(), action);
    m_actionData += findActionIn<QToolBar>(formWindow(), action);
}

void RemoveActionCommand::redo()
{
    foreach (const ActionDataItem &item, m_actionData) {
        item.widget->removeAction(m_action);
    }
    core()->actionEditor()->setFormWindow(formWindow());
    core()->actionEditor()->unmanageAction(m_action);
}

void RemoveActionCommand::undo()
{
    core()->actionEditor()->setFormWindow(formWindow());
    core()->actionEditor()->manageAction(m_action);
    foreach (const ActionDataItem &item, m_actionData) {
        item.widget->insertAction(item.before, m_action);
    }
}

// ---- InsertActionIntoCommand ----

InsertActionIntoCommand::InsertActionIntoCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Add action"), formWindow),
    m_parentWidget(0), m_action(0), m_beforeAction(0)
{
}

void InsertActionIntoCommand::init(QWidget *parentWidget, QAction *action,
            QAction *beforeAction, bool update)
{
    Q_ASSERT(m_parentWidget == 0);
    Q_ASSERT(m_action == 0);

    m_parentWidget = parentWidget;
    m_action = action;
    m_beforeAction = beforeAction;
    m_update = update;
}

void InsertActionIntoCommand::redo()
{
    Q_ASSERT(m_action != 0);
    Q_ASSERT(m_parentWidget != 0);

    m_parentWidget->insertAction(m_beforeAction, m_action);

    if (m_update) {
        core()->propertyEditor()->setObject(m_action);
        cheapUpdate();
    }
}

void InsertActionIntoCommand::undo()
{
    Q_ASSERT(m_action != 0);
    Q_ASSERT(m_parentWidget != 0);

    if (QDesignerMenu *menu = qobject_cast<QDesignerMenu*>(m_parentWidget))
        menu->hideSubMenu();

    m_parentWidget->removeAction(m_action);

    if (m_update) {
        core()->propertyEditor()->setObject(m_parentWidget);
        cheapUpdate();
    }
}

// ---- RemoveActionFromCommand ----

RemoveActionFromCommand::RemoveActionFromCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Remove action"), formWindow),
    m_parentWidget(0), m_action(0), m_beforeAction(0)
{
}

void RemoveActionFromCommand::init(QWidget *parentWidget, QAction *action,
            QAction *beforeAction, bool update)
{
    Q_ASSERT(m_parentWidget == 0);
    Q_ASSERT(m_action == 0);

    m_parentWidget = parentWidget;
    m_action = action;
    m_beforeAction = beforeAction;
    m_update = update;
}

void RemoveActionFromCommand::redo()
{
    Q_ASSERT(m_action != 0);
    Q_ASSERT(m_parentWidget != 0);

    if (QDesignerMenu *menu = qobject_cast<QDesignerMenu*>(m_parentWidget))
        menu->hideSubMenu();
    m_parentWidget->removeAction(m_action);

    if (m_update) {
        core()->propertyEditor()->setObject(m_parentWidget);
        cheapUpdate();
    }
}

void RemoveActionFromCommand::undo()
{
    Q_ASSERT(m_action != 0);
    Q_ASSERT(m_parentWidget != 0);

    m_parentWidget->insertAction(m_beforeAction, m_action);

    if (m_update) {
        core()->propertyEditor()->setObject(m_action);
        cheapUpdate();
    }
}

// ---- AddMenuActionCommand ----
AddMenuActionCommand::AddMenuActionCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Add menu"), formWindow)
{
    m_action = 0;
    m_parent = 0;
}

void AddMenuActionCommand::init(QAction *action, QWidget *parent)
{
    Q_ASSERT(action->menu() != 0);
    m_action = action;
    m_parent = parent;
}

void AddMenuActionCommand::redo()
{
    core()->metaDataBase()->add(m_action);
    core()->metaDataBase()->add(m_action->menu());

    core()->propertyEditor()->setObject(m_action->menu());
    cheapUpdate();
}

void AddMenuActionCommand::undo()
{
    core()->metaDataBase()->remove(m_action->menu());
    core()->metaDataBase()->remove(m_action);

    core()->propertyEditor()->setObject(m_parent);
    cheapUpdate();
}

// ---- RemoveMenuActionCommand ----
RemoveMenuActionCommand::RemoveMenuActionCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Remove menu"), formWindow)
{
    m_action = 0;
    m_parent = 0;
}

void RemoveMenuActionCommand::init(QAction *action, QWidget *parent)
{
    Q_ASSERT(action->menu() != 0);
    m_action = action;
    m_parent = parent;
}

void RemoveMenuActionCommand::redo()
{
    m_action->menu()->setParent(0);
    core()->metaDataBase()->remove(m_action->menu());
    core()->metaDataBase()->remove(m_action);

    core()->propertyEditor()->setObject(m_parent);
    cheapUpdate();
}

void RemoveMenuActionCommand::undo()
{
    m_action->menu()->setParent(m_parent);
    core()->metaDataBase()->add(m_action);
    core()->metaDataBase()->add(m_action->menu());

    core()->propertyEditor()->setObject(m_action->menu());
    cheapUpdate();
}

// ---- CreateSubmenuCommand ----
CreateSubmenuCommand::CreateSubmenuCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Create submenu"), formWindow)
{
    m_action = 0;
    m_menu = 0;
}

void CreateSubmenuCommand::init(QDesignerMenu *menu, QAction *action)
{
    m_menu = menu;
    m_action = action;
}

void CreateSubmenuCommand::redo()
{
    m_menu->createRealMenuAction(m_action);
    cheapUpdate();
}

void CreateSubmenuCommand::undo()
{
    m_menu->removeRealMenu(m_action);
    cheapUpdate();
}

// ---- DeleteToolBarCommand ----
DeleteToolBarCommand::DeleteToolBarCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QApplication::translate("Command", "Delete Tool Bar"), formWindow)
{
}

void DeleteToolBarCommand::init(QToolBar *toolBar)
{
    m_toolBar = toolBar;
    m_mainWindow = qobject_cast<QMainWindow*>(toolBar->parentWidget());
}

void DeleteToolBarCommand::redo()
{
    if (m_mainWindow) {
        QDesignerContainerExtension *c;
        c = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), m_mainWindow);
        Q_ASSERT(c != 0);
        for (int i=0; i<c->count(); ++i) {
            if (c->widget(i) == m_toolBar) {
                c->remove(i);
                break;
            }
        }
    }

    core()->metaDataBase()->remove(m_toolBar);
    m_toolBar->hide();
    m_toolBar->setParent(formWindow());
    formWindow()->emitSelectionChanged();
}

void DeleteToolBarCommand::undo()
{
    if (m_mainWindow) {
        m_toolBar->setParent(m_mainWindow);
        QDesignerContainerExtension *c;
        c = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), m_mainWindow);

        c->addWidget(m_toolBar);

        core()->metaDataBase()->add(m_toolBar);
        m_toolBar->show();
        formWindow()->emitSelectionChanged();
    }
}


} // namespace qdesigner_internal
