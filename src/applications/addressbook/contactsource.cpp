/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <contactsource.h>
#include <qcontactmodel.h>
#include <qpimsourcemodel.h>

#include <qsoftmenubar.h>

#include <QListView>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDebug>

ContactSourceDialog::ContactSourceDialog(QWidget *parent)
    : QDialog(parent), contactModel(0)
{
    setWindowTitle(tr("Show Contacts From", "e.g. Show Contacts From Phone/SIM Card"));

    view = new QListView;
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(view);
    setLayout(layout);

    model = new QPimSourceModel(this);
    view->setModel(model);

    QMenu* contextMenu = QSoftMenuBar::menuFor(this);
    QAction *actionCopyFromSim = new QAction(QIcon(":icon/sync"), tr("Import from SIM"), this);
    actionCopyFromSim->setWhatsThis(tr("Copy all entries from the SIM card to the Phone"));

    connect(actionCopyFromSim, SIGNAL(triggered()), this, SLOT(importActiveSim()));

    QAction *actionCopyToSim = new QAction(QIcon(":icon/sync"), tr("Export to SIM"), this);
    actionCopyToSim->setWhatsThis(tr("<qt>Copy all currently shown entries from the Phone to the SIM card. "
                "Entries not shown due to category filtering will not be copied.</qt>"));
    connect(actionCopyToSim, SIGNAL(triggered()), this, SLOT(exportActiveSim()));

    contextMenu->addAction(actionCopyFromSim);
    contextMenu->addAction(actionCopyToSim);
}

ContactSourceDialog::~ContactSourceDialog()
{
}

void ContactSourceDialog::setContactModel(QContactModel *m)
{
    contactModel = m;
    model->setContexts(contactModel->contexts());
    model->setCheckedSources(contactModel->visibleSources());
}

void ContactSourceDialog::importActiveSim()
{
    contactModel->mirrorAll(contactModel->simSource(), contactModel->phoneSource());
}

void ContactSourceDialog::exportActiveSim()
{
    if (!contactModel->mirrorAll(contactModel->phoneSource(), contactModel->simSource())) {
        QMessageBox::warning(this, tr("Contacts"),
                tr("<qt>Could not export contacts to SIM Card.  Please ensure sufficient"
                    " space is available on SIM Card.</qt>"));
    }
}

void ContactSourceDialog::accept()
{
    contactModel->setVisibleSources(model->checkedSources());
    QDialog::accept();
}
