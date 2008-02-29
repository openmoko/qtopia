/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

ContactSourceDialog::ContactSourceDialog(QWidget *parent)
    : QPimSourceDialog(parent), contactModel(0)
{
    setWindowTitle(tr("Show Contacts From", "e.g. Show Contacts From Phone/SIM Card"));

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

void ContactSourceDialog::setPimModel(QPimModel *m)
{
    contactModel = qobject_cast<QContactModel*>(m);
    Q_ASSERT(contactModel);
    QPimSourceDialog::setPimModel(m);
}

void ContactSourceDialog::importActiveSim()
{
    if(contactModel)
        contactModel->mirrorAll(contactModel->simSource(), contactModel->defaultSource());
}

void ContactSourceDialog::exportActiveSim()
{
    if (contactModel) {
        if (!contactModel->mirrorAll(contactModel->defaultSource(), contactModel->simSource())) {
            QMessageBox::warning(this, tr("Contacts"),
                tr("<qt>Could not export contacts to SIM Card.  Please ensure sufficient"
                    " space is available on SIM Card.</qt>"));
        }
    }
}

