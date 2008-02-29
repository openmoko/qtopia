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

#include "q3wizard_container.h"
#include <Qt3Support/Q3Wizard>

#include <QtCore/qdebug.h>

Q3WizardContainer::Q3WizardContainer(Q3Wizard *wizard, QObject *parent)
    : QObject(parent),
      m_wizard(wizard)
{}

int Q3WizardContainer::count() const
{
    return m_wizard->pageCount();
}

QWidget *Q3WizardContainer::widget(int index) const
{
    Q_ASSERT(index != -1);
    return m_wizard->page(index);
}

int Q3WizardContainer::currentIndex() const
{
    if (m_wizard->currentPage() == 0 && m_wizard->pageCount())
        m_wizard->showPage(widget(0));

    return m_wizard->indexOf(m_wizard->currentPage());
}

void Q3WizardContainer::setCurrentIndex(int index)
{
    m_wizard->showPage(widget(index));
}

void Q3WizardContainer::addWidget(QWidget *widget)
{
    m_wizard->addPage(widget, tr("Page"));
}

void Q3WizardContainer::insertWidget(int index, QWidget *widget)
{
    m_wizard->insertPage(widget, tr("Page"), index);
}

void Q3WizardContainer::remove(int index)
{
    m_wizard->removePage(widget(index));
}

Q3WizardContainerFactory::Q3WizardContainerFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *Q3WizardContainerFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerContainerExtension))
        return 0;

    if (Q3Wizard *w = qobject_cast<Q3Wizard*>(object))
        return new Q3WizardContainer(w, parent);

    return 0;
}

