/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
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
#include "advanced.h"
#include <qtopia/calc/doubleinstruction.h>
#include <qlayout.h>

#include <qpe/resource.h>

FormAdvanced::FormAdvanced(QWidget *parent,const char *name,WFlags fl)
:QWidget(parent,name,fl) {
    if ( !name )
	setName( "Advanced" );
    resize( 384, 476 );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, sizePolicy().hasHeightForWidth() ) );

    QVBoxLayout *vbl = new QVBoxLayout(this);
    AdvancedLayout = new QGridLayout( vbl, 2, 4, 0, "AdvancedLayout");

    // Row 1
    PBMPlus = new QToolButton(this,"PBM+");
    PBMPlus->setText(tr("M+"));
    AdvancedLayout->addWidget(PBMPlus,0,0);

    PBMR = new QToolButton(this,"PBMR");
    PBMR->setText(tr("MR"));
    AdvancedLayout->addWidget(PBMR,0,1);

    PBMC = new QToolButton(this,"PBMC");
    PBMC->setText(tr("MC"));
    AdvancedLayout->addWidget(PBMC,0,2);

    PBC = new QToolButton(this,"PBC");
    PBC->setText(tr("CE/C"));
    AdvancedLayout->addWidget(PBC,0,3);

    // Row 2
    PBASin = new QToolButton(this,"PBASin");
    PBASin->setText(tr("asin"));
    AdvancedLayout->addWidget(PBASin,1,0);

    PBACos = new QToolButton(this,"PBACos");
    PBACos->setText(tr("acos"));
    AdvancedLayout->addWidget(PBACos,1,1);

    PBATan = new QToolButton(this,"PBATan");
    PBATan->setText(tr("atan"));
    AdvancedLayout->addWidget(PBATan,1,2);

    PBDel = new QToolButton(this,"PBDel");
    PBDel->setText(tr("<-"));
    AdvancedLayout->addWidget(PBDel,1,3);

    // Row 3
    PBSin = new QToolButton(this,"PBSin");
    PBSin->setText(tr("sin"));
    AdvancedLayout->addWidget(PBSin,2,0);

    PBCos = new QToolButton(this,"PBCos");
    PBCos->setText(tr("cos"));
    AdvancedLayout->addWidget(PBCos,2,1);

    PBTan = new QToolButton(this,"PBTan");
    PBTan->setText(tr("tan"));
    AdvancedLayout->addWidget(PBTan,2,2);

    PBOneOverX = new QToolButton(this,"PBOneOverX");
    PBOneOverX->setPixmap(Resource::findPixmap("calculator/onebyx"));
    AdvancedLayout->addWidget(PBOneOverX,2,3);

    // Row 4
    PBLog = new QToolButton(this,"PBLog");
    PBLog->setText(tr("log"));
    AdvancedLayout->addWidget(PBLog,3,0);

    PBLn = new QToolButton(this,"PBLn");
    PBLn->setText(tr("ln"));
    AdvancedLayout->addWidget(PBLn,3,1);

    PBFactorial = new QToolButton(this,"PBFactorial");
    PBFactorial->setText(tr("x!"));
    AdvancedLayout->addWidget(PBFactorial,3,2);

    PBNegate = new QToolButton(this,"PBNegate");
    PBNegate->setText(tr("+/-"));
    AdvancedLayout->addWidget(PBNegate,3,3);

    // Row 5
    PBSquare = new QToolButton(this,"PBSquare");
    PBSquare->setPixmap(Resource::findPixmap("calculator/xsquared"));
    AdvancedLayout->addWidget(PBSquare,4,0);

    PBPow = new QToolButton(this,"PBPow");
    PBPow->setPixmap(Resource::loadPixmap("calculator/xpowy"));
    AdvancedLayout->addWidget(PBPow,4,1);

    PBSquareRoot = new QToolButton(this,"PBSquareRoot");
    PBSquareRoot->setPixmap(Resource::findPixmap("calculator/squareroot"));
    AdvancedLayout->addWidget(PBSquareRoot,4,2);

    PBXRootY = new QToolButton(this,"PBXRootY");
    PBXRootY->setPixmap(Resource::loadPixmap("calculator/xrooty"));
    AdvancedLayout->addWidget(PBXRootY,4,3);

    connect (PBMR, SIGNAL(clicked()), this, SLOT(MRClicked()));
    connect (PBMC, SIGNAL(clicked()), this, SLOT(MCClicked()));
    connect (PBMPlus, SIGNAL(clicked()), this, SLOT(MPlusClicked()));
    connect (PBC, SIGNAL(clicked()), this, SLOT(CClicked()));

    connect (PBASin, SIGNAL(clicked()), this, SLOT(aSinClicked()));
    connect (PBACos, SIGNAL(clicked()), this, SLOT(aCosClicked()));
    connect (PBATan, SIGNAL(clicked()), this, SLOT(aTanClicked()));
    connect (PBDel, SIGNAL(clicked()), this, SLOT(DelClicked()));

    connect (PBSin, SIGNAL(clicked()), this, SLOT(SinClicked()));
    connect (PBCos, SIGNAL(clicked()), this, SLOT(CosClicked()));
    connect (PBTan, SIGNAL(clicked()), this, SLOT(TanClicked()));
    connect (PBOneOverX, SIGNAL(clicked()), this, SLOT(OneOverXClicked()));

    connect (PBLog, SIGNAL(clicked()), this, SLOT(LogClicked()));
    connect (PBLn, SIGNAL(clicked()), this, SLOT(LnClicked()));
    connect (PBFactorial, SIGNAL(clicked()), this, SLOT(FactorialClicked()));
    connect (PBNegate, SIGNAL(clicked()), this, SLOT(NegateClicked()));

    connect (PBSquare, SIGNAL(clicked()), this, SLOT(SquareClicked()));
    connect (PBPow, SIGNAL(clicked()), this, SLOT(PowClicked()));
    connect (PBSquareRoot, SIGNAL(clicked()), this, SLOT(SquareRootClicked()));
    connect (PBXRootY, SIGNAL(clicked()), this, SLOT(XRootYClicked()));

    siw = new Type1DecimalInputWidget(this,"SIW");
    vbl->addWidget(siw);
}

void FormAdvanced::showEvent ( QShowEvent *e ) {
    sys.setAccType(new DoubleData());
    QWidget::showEvent(e);
}
void FormAdvanced::MCClicked() {
    sys.memoryReset();
}
void FormAdvanced::MRClicked() {
    sys.memoryRecall();
}
void FormAdvanced::MPlusClicked() {
    sys.memorySave();
}
void FormAdvanced::CClicked() {
    sys.dualReset();
}

void FormAdvanced::aSinClicked() {
    sys.pushInstruction(new DoubleASin());
}
void FormAdvanced::aCosClicked() {
    sys.pushInstruction(new DoubleACos());
}
void FormAdvanced::aTanClicked() {
    sys.pushInstruction(new DoubleATan());
}
void FormAdvanced::DelClicked() {
    sys.delChar();
}

void FormAdvanced::SinClicked() {
    sys.pushInstruction(new DoubleSin());
}
void FormAdvanced::CosClicked() {
    sys.pushInstruction(new DoubleCos());
}
void FormAdvanced::TanClicked() {
    sys.pushInstruction(new DoubleTan());
}
void FormAdvanced::OneOverXClicked() {
    sys.pushInstruction(new DoubleOneOverX());
}
void FormAdvanced::LogClicked() {
    sys.pushInstruction(new DoubleLog());
}
void FormAdvanced::LnClicked() {
    sys.pushInstruction(new DoubleLn());
}
void FormAdvanced::FactorialClicked() {
    sys.pushInstruction(new DoubleFactorial());
}
void FormAdvanced::NegateClicked() {
    sys.pushInstruction(new DoubleNegate());
}
void FormAdvanced::SquareClicked() {
    sys.pushInstruction(new DoubleSquare());
}
void FormAdvanced::PowClicked() {
    sys.pushInstruction(new DoublePow());
}
void FormAdvanced::SquareRootClicked() {
    sys.pushInstruction(new DoubleSquareRoot());
}
void FormAdvanced::XRootYClicked() {
    sys.pushInstruction(new DoubleXRootY());
}
