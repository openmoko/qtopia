/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.1, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "promotiontaskmenu_p.h"
#include "qdesigner_promotiondialog_p.h"
#include "widgetfactory_p.h"
#include "metadatabase_p.h"
#include "widgetdatabase_p.h"
#include "qdesigner_command_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormWindowCursorInterface>
#include <QtDesigner/QDesignerLanguageExtension>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QExtensionManager>

#include <QtGui/QAction>
#include <QtGui/QWidget>
#include <QtGui/QMenu>
#include <QtCore/QSignalMapper>
#include <QtCore/qdebug.h>

static QAction *separatorAction(QObject *parent)
{
    QAction *rc = new  QAction(parent);
    rc->setSeparator(true);
    return rc;
}

namespace qdesigner_internal {

PromotionTaskMenu::PromotionTaskMenu(QWidget *widget,Mode mode, QObject *parent) :
    QObject(parent),
    m_mode(mode),
    m_widget(widget),
    m_promotionMapper(0),
    m_globalEditAction(new QAction(tr("Promoted widgets..."), this)),
    m_EditPromoteToAction(new QAction(tr("Promote to ..."), this)),
    m_promoteLabel(tr("Promote to")),
    m_demoteLabel(tr("Demote to %1"))
{
    connect(m_globalEditAction, SIGNAL(triggered()), this, SLOT(slotEditPromotedWidgets()));
    connect(m_EditPromoteToAction, SIGNAL(triggered()), this, SLOT(slotEditPromoteTo()));
}

void PromotionTaskMenu::setWidget(QWidget *widget)
{
    m_widget = widget;
}

void PromotionTaskMenu::setPromoteLabel(const QString &promoteLabel)
{
    m_promoteLabel = promoteLabel;
}

void PromotionTaskMenu::setEditPromoteToLabel(const QString &promoteEditLabel)
{
    m_EditPromoteToAction->setText(promoteEditLabel);
}

void PromotionTaskMenu::setDemoteLabel(const QString &demoteLabel)
{
    m_demoteLabel = demoteLabel;
}

PromotionTaskMenu::PromotionState  PromotionTaskMenu::createPromotionActions(QDesignerFormWindowInterface *formWindow)
{
    // clear out old
    if (!m_promotionActions.empty()) {
        qDeleteAll(m_promotionActions);
        m_promotionActions.clear();
    }
    // No promotion of main container
    if (formWindow->mainContainer() == m_widget)
        return NotApplicable;

    // Check for a homogenous selection
    const PromotionSelectionList promotionSelection = promotionSelectionList(formWindow);

    if (promotionSelection.empty())
        return NoHomogenousSelection;

    QDesignerFormEditorInterface *core = formWindow->core();
    // if it is promoted: demote only.
    if (isPromoted(formWindow->core(), m_widget)) {
        const QString label = m_demoteLabel.arg( promotedExtends(core , m_widget));
        QAction *demoteAction = new QAction(label, this);
        connect(demoteAction, SIGNAL(triggered()), this, SLOT(slotDemoteFromCustomWidget()));
        m_promotionActions.push_back(demoteAction);
        return CanDemote;
    }
    // figure out candidates
    const QString baseClassName = WidgetFactory::classNameOf(core,  m_widget);
    const WidgetDataBaseItemList candidates = promotionCandidates(core->widgetDataBase(), baseClassName );
    if (candidates.empty()) {
        // Is this thing promotable at all?
        return QDesignerPromotionDialog::baseClassNames(core->promotion()).contains(baseClassName) ?  CanPromote : NotApplicable;
    }
    // Set up a signal mapper to associate class names
    if (!m_promotionMapper) {
        m_promotionMapper = new QSignalMapper(this);
        connect(m_promotionMapper, SIGNAL(mapped(QString)), this, SLOT(slotPromoteToCustomWidget(QString)));
    }

    QMenu *candidatesMenu = new QMenu();
    // Create a sub menu
    const WidgetDataBaseItemList::const_iterator cend = candidates.constEnd();
    // Set up actions and map class names
    for (WidgetDataBaseItemList::const_iterator it = candidates.constBegin(); it != cend; ++it) {
        const QString customClassName = (*it)->name();
        QAction *action = new QAction((*it)->name(), this);
        connect(action, SIGNAL(triggered()), m_promotionMapper, SLOT(map()));
        m_promotionMapper->setMapping(action, customClassName);
        candidatesMenu->addAction(action);
    }
    // Sub menu action
    QAction *subMenuAction = new QAction(m_promoteLabel, this);
    subMenuAction->setMenu(candidatesMenu);
    m_promotionActions.push_back(subMenuAction);
    return CanPromote;
}

void PromotionTaskMenu::addActions(unsigned separatorFlags, ActionList &actionList)
{
    addActions(formWindow(), separatorFlags, actionList);
}

void PromotionTaskMenu::addActions(QDesignerFormWindowInterface *fw, unsigned flags,
                                   ActionList &actionList)
{
    Q_ASSERT(m_widget);
    const int previousSize = actionList.size();
    const PromotionState promotionState = createPromotionActions(fw);

    // Promotion candidates/demote
    actionList += m_promotionActions;

    // Edit action depending on context
    switch (promotionState) {
    case  CanPromote:
        actionList += m_EditPromoteToAction;
        break;
    default:
        if (!(flags & SuppressGlobalEdit))
            actionList += m_globalEditAction;
        break;
    }
    // Add separators if required
    if (actionList.size() > previousSize) {
        if (flags &  LeadingSeparator)
            actionList.insert(previousSize, separatorAction(this));
        if (flags & TrailingSeparator)
            actionList += separatorAction(this);
    }
}

void  PromotionTaskMenu::addActions(QDesignerFormWindowInterface *fw, unsigned flags, QMenu *menu)
{
    ActionList actionList;
    addActions(fw, flags, actionList);
    menu->addActions(actionList);
}

void  PromotionTaskMenu::addActions(unsigned flags, QMenu *menu)
{
    addActions(formWindow(), flags, menu);
}

void PromotionTaskMenu::promoteTo(QDesignerFormWindowInterface *fw, const QString &customClassName)
{
    Q_ASSERT(m_widget);
    PromoteToCustomWidgetCommand *cmd = new PromoteToCustomWidgetCommand(fw);
    cmd->init(promotionSelectionList(fw), customClassName);
    fw->commandHistory()->push(cmd);
}


void  PromotionTaskMenu::slotPromoteToCustomWidget(const QString &customClassName)
{
    promoteTo(formWindow(), customClassName);
}

void PromotionTaskMenu::slotDemoteFromCustomWidget()
{
    QDesignerFormWindowInterface *fw = formWindow();
    const PromotionSelectionList promotedWidgets = promotionSelectionList(fw);
    Q_ASSERT(!promotedWidgets.empty() && isPromoted(fw->core(), promotedWidgets.front()));

    // ### use the undo stack
    DemoteFromCustomWidgetCommand *cmd = new DemoteFromCustomWidgetCommand(fw);
    cmd->init(promotedWidgets);
    fw->commandHistory()->push(cmd);
}

void PromotionTaskMenu::slotEditPromoteTo()
{
    Q_ASSERT(m_widget);
    // Check whether invoked over a promotable widget
    QDesignerFormWindowInterface *fw = formWindow();
    QDesignerFormEditorInterface *core = fw->core();
    const QString base_class_name = WidgetFactory::classNameOf(core, m_widget);
    Q_ASSERT(QDesignerPromotionDialog::baseClassNames(core->promotion()).contains(base_class_name));
    // Show over promotable widget
    QString promoteToClassName;
    QDialog *promotionEditor = 0;
    if (QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*>(core->extensionManager(), core))
        promotionEditor = lang->createPromotionDialog(core, base_class_name, &promoteToClassName, fw);
    if (!promotionEditor)
        promotionEditor = new QDesignerPromotionDialog(core, fw, base_class_name, &promoteToClassName);
    if (promotionEditor->exec() == QDialog::Accepted && !promoteToClassName.isEmpty()) {
        promoteTo(fw, promoteToClassName);
    }
    delete promotionEditor;
}

void PromotionTaskMenu::slotEditPromotedWidgets()
{
    // Global context, show over non-promotable widget
    QDesignerFormWindowInterface *fw = formWindow();
    if (!fw)
        return;
    editPromotedWidgets(fw->core(), fw);
}

PromotionTaskMenu::PromotionSelectionList PromotionTaskMenu::promotionSelectionList(QDesignerFormWindowInterface *formWindow) const
{
    // In multi selection mode, check for a homogenous selection (same class, same promotion state)
    // and return the list if this is the case. Also make sure m_widget
    // is the last widget in the list so that it is re-selected as the last
    // widget by the promotion commands.

    PromotionSelectionList rc;

    if ( m_mode ==  ModeMultiSelection) {
        const QString className = m_widget->metaObject()->className();
        const bool promoted = isPromoted(formWindow->core(), m_widget);

        if (QDesignerFormWindowCursorInterface *cursor = formWindow->cursor()) {
            const int selectedWidgetCount = cursor->selectedWidgetCount();
            for (int i=0; i < selectedWidgetCount; i++) {
                QWidget *w = cursor->selectedWidget(i);
                // Check, put  m_widget last
                if (w != m_widget) {
                    if (w->metaObject()->className() != className || isPromoted(formWindow->core(), w) !=  promoted) {
                        return PromotionSelectionList();
                    }
                    rc.push_back(w);
                }
            }
        }
    }

    rc.push_back(m_widget);
    return rc;
}

QDesignerFormWindowInterface *PromotionTaskMenu::formWindow() const
{
    QDesignerFormWindowInterface *result = QDesignerFormWindowInterface::findFormWindow(m_widget);
    Q_ASSERT(result != 0);
    return result;
}

void PromotionTaskMenu::editPromotedWidgets(QDesignerFormEditorInterface *core, QWidget* parent) {
    QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*>(core->extensionManager(), core);
    // Show over non-promotable widget
    QDialog *promotionEditor =  0;
    if (lang)
        lang->createPromotionDialog(core, parent);
    if (!promotionEditor)
        promotionEditor = new QDesignerPromotionDialog(core, parent);
    promotionEditor->exec();
    delete promotionEditor;
}
} // namespace qdesigner_internal
