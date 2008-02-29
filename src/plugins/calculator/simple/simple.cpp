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
    resize( 384, 476 );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, sizePolicy().hasHeightForWidth() ) );

    SimpleLayout = new QGridLayout( this, 2, 4, 0, 0, "SimpleLayout");
    QFont big(font());
    big.setPointSize(big.pointSize()*2);

    PBMPlus = new QPushButton(this,"PBM+");
    PBMPlus->setText(tr("M+"));
    PBMPlus->setFont(big);
    SimpleLayout->addWidget(PBMPlus,0,0);

    PBMC = new QPushButton(this,"PBMC");
    PBMC->setText(tr("MC"));
    PBMC->setFont(big);
    SimpleLayout->addWidget(PBMC,0,1);

    PBMR = new QPushButton(this,"PBMR");
    PBMR->setText(tr("MR"));
    PBMR->setFont(big);
    SimpleLayout->addWidget(PBMR,0,2);

    PBCE = new QPushButton(this,"PBCE");
    PBCE->setText(tr("CE/C"));
    PBCE->setFont(big);
    SimpleLayout->addWidget(PBCE,0,3);

    connect (PBCE, SIGNAL(clicked()), this, SLOT(CEClicked()));
    connect (PBMR, SIGNAL(clicked()), this, SLOT(MRClicked()));
    connect (PBMC, SIGNAL(clicked()), this, SLOT(MCClicked()));
    connect (PBMPlus, SIGNAL(clicked()), this, SLOT(MPlusClicked()));

    siw = new Type1DecimalInputWidget(this,"SIW");
    SimpleLayout->addMultiCellWidget(siw,1,1,0,3);
}

void FormSimple::showEvent ( QShowEvent *e ) {
    sys.setAccType(new DoubleData());
    QWidget::showEvent(e);
}

void FormSimple::CEClicked() {
    sys.dualReset();
}
void FormSimple::MCClicked() {
    sys.memoryReset();
}
void FormSimple::MRClicked() {
    sys.memoryRecall();
}
void FormSimple::MPlusClicked() {
    sys.memorySave();
}

