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

#include <qtopia/resource.h>

FormAdvanced::FormAdvanced(QWidget *parent,const char *name,WFlags fl)
:QWidget(parent,name,fl) {
    if ( !name )
	setName( "Advanced" );
    QSizePolicy mySizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred,FALSE);

    QVBoxLayout *vbl = new QVBoxLayout(this);
    AdvancedLayout = new QGridLayout( vbl, 2, 4, 0, "AdvancedLayout");
    
    QFont fonty;
    int fontSize = fonty.pixelSize();
    if (fontSize == 12)
	fontSize = 14;

    // Row 1
    PBMPlus = new QToolButton(this,"PBM+");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBMPlus->setText(tr("M+"));
    AdvancedLayout->addWidget(PBMPlus,0,0);

    PBMR = new QToolButton(this,"PBMR");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBMR->setText(tr("MR"));
    AdvancedLayout->addWidget(PBMR,0,1);

    PBMC = new QToolButton(this,"PBMC");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBMC->setText(tr("MC"));
    AdvancedLayout->addWidget(PBMC,0,2);

    PBC = new QToolButton(this,"PBC");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBC->setText(tr("CE/C"));
    AdvancedLayout->addWidget(PBC,0,3);

    // Row 2
    PBASin = new QToolButton(this,"PBASin");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBASin->setText(tr("asin"));
    AdvancedLayout->addWidget(PBASin,1,0);

    PBACos = new QToolButton(this,"PBACos");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBACos->setText(tr("acos"));
    AdvancedLayout->addWidget(PBACos,1,1);

    PBATan = new QToolButton(this,"PBATan");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBATan->setText(tr("atan"));
    AdvancedLayout->addWidget(PBATan,1,2);

    PBDel = new QToolButton(this,"PBDel");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBDel->setText(tr("<-"));
    AdvancedLayout->addWidget(PBDel,1,3);

    // Row 3
    PBSin = new QToolButton(this,"PBSin");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBSin->setText(tr("sin"));
    AdvancedLayout->addWidget(PBSin,2,0);

    PBCos = new QToolButton(this,"PBCos");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBCos->setText(tr("cos"));
    AdvancedLayout->addWidget(PBCos,2,1);

    PBTan = new QToolButton(this,"PBTan");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBTan->setText(tr("tan"));
    AdvancedLayout->addWidget(PBTan,2,2);

    PBFactorial = new QToolButton(this,"PBTan");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBFactorial->setText(tr("n!"));
    AdvancedLayout->addWidget(PBFactorial,2,3);

    // Row 4
    PBSquareRoot = new QToolButton(this,"PBSquareRoot");
    PBMPlus->setSizePolicy(mySizePolicy);
    QString fontSizeName;
    fontSizeName.setNum(fontSize);
    fontSizeName.prepend("calculator/squareroot-");
    PBSquareRoot->setPixmap(Resource::findPixmap(fontSizeName));
    AdvancedLayout->addWidget(PBSquareRoot,3,0);

    PBXRootY = new QToolButton(this,"PBXRootY");
    PBMPlus->setSizePolicy(mySizePolicy);
    fontSizeName.setNum(fontSize);
    fontSizeName.prepend("calculator/xrooty-");
    PBXRootY->setPixmap(Resource::loadPixmap(fontSizeName));
    AdvancedLayout->addWidget(PBXRootY,3,1);

    PBLn = new QToolButton(this,"PBLn");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBLn->setText(tr("ln"));
    AdvancedLayout->addWidget(PBLn,3,2);

    PBNegate = new QToolButton(this,"PBNegate");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBNegate->setText(tr("+/-"));
    AdvancedLayout->addWidget(PBNegate,3,3);

    // Row 5
    PBSquare = new QToolButton(this,"PBSquare");
    PBMPlus->setSizePolicy(mySizePolicy);
    fontSizeName.setNum(fontSize);
    fontSizeName.prepend("calculator/xsquared-");
    PBSquare->setPixmap(Resource::findPixmap(fontSizeName));
    AdvancedLayout->addWidget(PBSquare,4,0);

    PBPow = new QToolButton(this,"PBPow");
    PBMPlus->setSizePolicy(mySizePolicy);
    fontSizeName.setNum(fontSize);
    fontSizeName.prepend("calculator/xpowy-");
    PBPow->setPixmap(Resource::loadPixmap(fontSizeName));
    AdvancedLayout->addWidget(PBPow,4,1);

    PBLog = new QToolButton(this,"PBLog");
    PBMPlus->setSizePolicy(mySizePolicy);
    fontSizeName.setNum(fontSize);
    fontSizeName.prepend("calculator/epowx-");
    PBLog->setPixmap(Resource::findPixmap(fontSizeName));
    AdvancedLayout->addWidget(PBLog,4,2);

    PBOneOverX = new QToolButton(this,"PBOneOverX");
    PBMPlus->setSizePolicy(mySizePolicy);
    fontSizeName.setNum(fontSize);
    fontSizeName.prepend("calculator/onebyx-");
    PBOneOverX->setPixmap(Resource::findPixmap(fontSizeName));
    AdvancedLayout->addWidget(PBOneOverX,4,3);

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
    systemEngine->setAccType("DOUBLE");
    QWidget::showEvent(e);
}
void FormAdvanced::MCClicked() {
    systemEngine->memoryReset();
}
void FormAdvanced::MRClicked() {
    systemEngine->memoryRecall();
}
void FormAdvanced::MPlusClicked() {
    systemEngine->memorySave();
}
void FormAdvanced::CClicked() {
    systemEngine->dualReset();
}

void FormAdvanced::aSinClicked() {
    systemEngine->pushInstruction(new DoubleASin());
}
void FormAdvanced::aCosClicked() {
    systemEngine->pushInstruction(new DoubleACos());
}
void FormAdvanced::aTanClicked() {
    systemEngine->pushInstruction(new DoubleATan());
}
void FormAdvanced::DelClicked() {
    systemEngine->delChar();
}

void FormAdvanced::SinClicked() {
    systemEngine->pushInstruction(new DoubleSin());
}
void FormAdvanced::CosClicked() {
    systemEngine->pushInstruction(new DoubleCos());
}
void FormAdvanced::TanClicked() {
    systemEngine->pushInstruction(new DoubleTan());
}
void FormAdvanced::OneOverXClicked() {
    systemEngine->pushInstruction(new DoubleOneOverX());
}
void FormAdvanced::LogClicked() {
    systemEngine->pushInstruction(new DoubleExp());
}
void FormAdvanced::LnClicked() {
    systemEngine->pushInstruction(new DoubleLn());
}
void FormAdvanced::FactorialClicked() {
    systemEngine->pushInstruction(new DoubleFactorial());
}
void FormAdvanced::NegateClicked() {
    systemEngine->pushInstruction(new DoubleNegate());
}
void FormAdvanced::SquareClicked() {
    systemEngine->pushInstruction(new DoubleSquare());
}
void FormAdvanced::PowClicked() {
    systemEngine->pushInstruction(new DoublePow());
}
void FormAdvanced::SquareRootClicked() {
    systemEngine->pushInstruction(new DoubleSquareRoot());
}
void FormAdvanced::XRootYClicked() {
    systemEngine->pushInstruction(new DoubleXRootY());
}
