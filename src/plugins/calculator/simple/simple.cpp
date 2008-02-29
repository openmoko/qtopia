/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qtopia/calc/doubleinstruction.h>
#include "simple.h"


FormSimple::FormSimple(QWidget *parent,const char *name,WFlags fl)
	:QWidget(parent,name,fl) {
    if ( !name )
	setName( "Simple" );
    QSizePolicy mySizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred,FALSE);

    QVBoxLayout *vbl = new QVBoxLayout(this);
    QHBoxLayout *SimpleLayout = new QHBoxLayout( vbl, 0, "SimpleLayout");
    QFont big(font());
    big.setPointSize(big.pointSize()*2);

    QPushButton *PBMPlus = new QPushButton(this,"PBM+");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBMPlus->setText(tr("M+"));
    PBMPlus->setFont(big);
    SimpleLayout->addWidget(PBMPlus);

    QPushButton *PBMC = new QPushButton(this,"PBMC");
    PBMC->setSizePolicy(mySizePolicy);
    PBMC->setText(tr("MC"));
    PBMC->setFont(big);
    SimpleLayout->addWidget(PBMC);

    QPushButton *PBMR = new QPushButton(this,"PBMR");
    PBMR->setSizePolicy(mySizePolicy);
    PBMR->setText(tr("MR"));
    PBMR->setFont(big);
    SimpleLayout->addWidget(PBMR);

    QPushButton *PBCE = new QPushButton(this,"PBCE");
    PBCE->setSizePolicy(mySizePolicy);
    PBCE->setText(tr("CE/C"));
    PBCE->setFont(big);
    SimpleLayout->addWidget(PBCE);

    connect (PBCE, SIGNAL(clicked()), this, SLOT(CEClicked()));
    connect (PBMR, SIGNAL(clicked()), this, SLOT(MRClicked()));
    connect (PBMC, SIGNAL(clicked()), this, SLOT(MCClicked()));
    connect (PBMPlus, SIGNAL(clicked()), this, SLOT(MPlusClicked()));

    Type1DecimalInputWidget *siw = new Type1DecimalInputWidget(this,"SIW");
    siw->setSizePolicy(mySizePolicy);
    vbl->addWidget(siw);
    vbl->setStretchFactor(siw,4);
}

void FormSimple::showEvent ( QShowEvent *e ) {
    systemEngine->setAccType("DOUBLE");
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

