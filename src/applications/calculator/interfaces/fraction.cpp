/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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
#ifdef ENABLE_FRACTION

#include <QDesktopWidget>

#include <qtopiaapplication.h>

#include "../doubleinstruction.h"
#include "fraction.h"

FormFraction::FormFraction(QWidget *parent, Qt::WFlags fl)
        :FractionInputWidget(parent,fl) {

    setObjectName( "Fraction" ); // No tr

    InputWidgetLayout = new QGridLayout(this);
    InputWidgetLayout->setSpacing( 3 );
    InputWidgetLayout->setMargin( 0 );

    QSizePolicy mySizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    //FractionInputWidget *siw = new FractionInputWidget(this);
    setSizePolicy(mySizePolicy);
    init(0,0);
}

void FormFraction::init(int fromRow, int fromCol)
{
    //FractionInputWidget::init(fromRow, fromCol);

    QRect screenRect = QtopiaApplication::desktop()->availableGeometry();
    bool portrait = screenRect.width() < screenRect.height();
    if ( portrait ) {  //portrait mode
        FractionInputWidget::init(fromRow+1, fromCol);
        //grid->addWidget(siw, 1, 0, 4, 4);
    } else { //landscape mode
        FractionInputWidget::init(fromRow, fromCol);
        //grid->addWidget(siw, 0, 0, 4, 4);
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
    PBCE->setText(tr("CE"));

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

#endif //ENABLE_FRACTION
