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

#include <qtopiaapplication.h>

#include <QDesktopWidget>

#include "simple.h"
#include "../doubleinstruction.h"


FormSimple::FormSimple(QWidget *parent) : DecimalInputWidget(parent) {

    setObjectName( tr("Simple") );

    InputWidgetLayout = new QGridLayout(this);
    InputWidgetLayout->setSpacing( 3 );
    InputWidgetLayout->setMargin( 0 );

    init(0, 0);
}


void FormSimple::init(int fromRow, int fromCol)
{
    QRect screenRect = QtopiaApplication::desktop()->availableGeometry();
    bool portrait = screenRect.width() < screenRect.height();
    if ( portrait ) {
        DecimalInputWidget::init(fromRow+1, fromCol);
    } else {
        DecimalInputWidget::init(fromRow, fromCol);
    }

    QPushButton *PBMPlus = new QPushButton(this);
    PBMPlus->setSizePolicy(sizePolicy());
    PBMPlus->setText(tr("M+"));

    QPushButton *PBMC = new QPushButton(this);
    PBMC->setSizePolicy(sizePolicy());
    PBMC->setText(tr("MC"));

    QPushButton *PBMR = new QPushButton(this);
    PBMR->setSizePolicy(sizePolicy());
    PBMR->setText(tr("MR"));

    QPushButton *PBCE = new QPushButton(this);
    PBCE->setSizePolicy(sizePolicy());
    PBCE->setText(tr("CE/C"));

#ifdef QTOPIA_PHONE
    PBMPlus->setFocusPolicy(Qt::NoFocus);
    PBMC->setFocusPolicy(Qt::NoFocus);
    PBMR->setFocusPolicy(Qt::NoFocus);
    PBCE->setFocusPolicy(Qt::NoFocus);
#endif

    if (portrait) {
        InputWidgetLayout->addWidget(PBCE, 0, 3);
        InputWidgetLayout->addWidget(PBMC, 0, 1);
        InputWidgetLayout->addWidget(PBMR, 0, 2);
        InputWidgetLayout->addWidget(PBMPlus, 0, 0);
    } else {
        InputWidgetLayout->addWidget(PBCE, 0, 4);
        InputWidgetLayout->addWidget(PBMC, 1, 4);
        InputWidgetLayout->addWidget(PBMR, 2, 4);
        InputWidgetLayout->addWidget(PBMPlus, 3, 4);
    }

    connect (PBCE, SIGNAL(clicked()), this, SLOT(CEClicked()));
    connect (PBMR, SIGNAL(clicked()), this, SLOT(MRClicked()));
    connect (PBMC, SIGNAL(clicked()), this, SLOT(MCClicked()));
    connect (PBMPlus, SIGNAL(clicked()), this, SLOT(MPlusClicked()));
}


void FormSimple::showEvent ( QShowEvent *e ) {
    systemEngine->setAccType("Double"); // No tr

    QWidget::showEvent(e);
}

void FormSimple::CEClicked() {
    systemEngine->dualReset();
}
void FormSimple::MCClicked() {
    systemEngine->memoryReset();
}
void FormSimple::MRClicked() {
    systemEngine->memoryRecall();
}
void FormSimple::MPlusClicked() {
    systemEngine->memorySave();
}

