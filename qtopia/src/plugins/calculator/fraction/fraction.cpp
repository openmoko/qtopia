/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qtopia/calc/doubleinstruction.h>
#include <qtopia/qpeapplication.h>
#include "fraction.h"

FormFraction::FormFraction(QWidget *parent,const char *name,WFlags fl)
	:QWidget(parent,name,fl) {
    if ( !name )
	setName( "Fraction" ); // No tr

    bool portrait;
    QWidget *d = QPEApplication::desktop();
    QGridLayout *grid;

    QSizePolicy mySizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred,FALSE);
    
    Type1FractionInputWidget *siw = new Type1FractionInputWidget(this,"SIW");
    siw->setSizePolicy(mySizePolicy);

    if ( d->width() < d->height() ) {  //portrait mode
        grid = new QGridLayout(this, 5, 4, 0 , 3, "FractionLayout");
        grid->addMultiCellWidget(siw, 1, 4, 0, 3);
        portrait = TRUE;
    } else { //landscape mode
        grid = new QGridLayout(this, 4, 5, 0, 3,  "FractionLayout");
        grid->addMultiCellWidget(siw, 0, 3, 0, 3);
        portrait = FALSE;
    }
    
    
    QFont big(font());
    big.setPointSize(QMIN(big.pointSize()*2, 18));

    QPushButton *PBMPlus = new QPushButton(this,"PBM+");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBMPlus->setText(tr("M+"));
    PBMPlus->setFont(big);

    QPushButton *PBMC = new QPushButton(this,"PBMC");
    PBMC->setSizePolicy(mySizePolicy);
    PBMC->setText(tr("MC"));
    PBMC->setFont(big);

    QPushButton *PBMR = new QPushButton(this,"PBMR");
    PBMR->setSizePolicy(mySizePolicy);
    PBMR->setText(tr("MR"));
    PBMR->setFont(big);

    QPushButton *PBCE = new QPushButton(this,"PBCE");
    PBCE->setSizePolicy(mySizePolicy);
    PBCE->setText(tr("CE/C"));
    PBCE->setFont(big);

    if (portrait) {
        grid->addWidget(PBCE, 0, 3);
        grid->addWidget(PBMC, 0, 1);
        grid->addWidget(PBMR, 0, 2);
        grid->addWidget(PBMPlus, 0, 0);
    } else {
        grid->addWidget(PBCE, 0, 4);
        grid->addWidget(PBMC, 1, 4);
        grid->addWidget(PBMR, 2, 4);
        grid->addWidget(PBMPlus, 3, 4);
    }

    connect (PBCE, SIGNAL(clicked()), this, SLOT(CEClicked()));
    connect (PBMR, SIGNAL(clicked()), this, SLOT(MRClicked()));
    connect (PBMC, SIGNAL(clicked()), this, SLOT(MCClicked()));
    connect (PBMPlus, SIGNAL(clicked()), this, SLOT(MPlusClicked()));
}

void FormFraction::showEvent ( QShowEvent *e ) {
    systemEngine->setAccType("Fraction"); // No tr
    
    QWidget::showEvent(e);
}

void FormFraction::CEClicked() {
    systemEngine->dualReset();
}
void FormFraction::MCClicked() {
    systemEngine->memoryReset();
}
void FormFraction::MRClicked() {
    systemEngine->memoryRecall();
}
void FormFraction::MPlusClicked() {
    systemEngine->memorySave();
}

