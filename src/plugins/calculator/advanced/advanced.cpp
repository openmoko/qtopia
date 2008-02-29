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
#include <qpixmap.h>
#include <qpainter.h>

#include <qtopia/resource.h>

void FormAdvanced::fontChange() {
    updatePictures();
    update();
    updateGeometry();
}

void FormAdvanced::updatePictures() {
    QFont big(font());
    big.setPointSize(big.pointSize()*3/2);
    QColor bg = PBMPlus->backgroundColor();

    int fontSize = big.pixelSize();
    int fontSize05 = fontSize / 2;
    int fontSize10 = fontSize;
    int fontSize15 = fontSize + fontSize05;
    int fontSize20 = fontSize * 2;
    int fontSize25 = fontSize20 + fontSize05;

    int pixSize = fontSize25;

    PMPow = new QPixmap(pixSize,pixSize);
    PMPow->fill(bg);
    QPainter p(PMPow);
    p.setPen(black);
    p.setFont(big);
    p.drawText(fontSize10,fontSize20,"x");
    p.drawText(fontSize15,fontSize15,"y");
    p.end();
    PBPow->setPixmap(*PMPow);

    PMLog = new QPixmap(pixSize,pixSize);
    PMLog->fill(bg);
    p.begin(PMLog);
    p.setPen(black);
    p.setFont(big);
    p.drawText(fontSize10,fontSize20,"e");
    p.drawText(fontSize15,fontSize15,"x");
    p.end();
    PBLog->setPixmap(*PMLog);

    PMSquare = new QPixmap(pixSize,pixSize);
    PMSquare->fill(bg);
    p.begin(PMSquare);
    p.setPen(black);
    p.setFont(big);
    p.drawText(fontSize10,fontSize20,"x");
    p.drawText(fontSize15,fontSize15,"2");
    p.end();
    PBSquare->setPixmap(*PMSquare);

    PMXRootY = new QPixmap(pixSize,pixSize);
    PMXRootY->fill(bg);
    p.begin(PMXRootY);
    p.setPen(black);
    p.setFont(big);
    p.drawText(fontSize15,fontSize20,"x");
    p.drawText(fontSize05,fontSize,"y");
    p.drawLine(fontSize05,fontSize15,fontSize10,fontSize20);
    p.drawLine(fontSize10,fontSize20,fontSize15,fontSize10);
    p.drawLine(fontSize15,fontSize10,fontSize25,fontSize10);
    p.end();
    PBXRootY->setPixmap(*PMXRootY);

    PMOneOverX = new QPixmap(pixSize,pixSize);
    PMOneOverX->fill(bg);
    p.begin(PMOneOverX);
    p.setPen(black);
    p.setFont(big);
    p.drawText(fontSize10,fontSize10,"1");
    int between = (fontSize10 + fontSize15) / 2;
    p.drawLine(fontSize05,between,fontSize20,between);
    p.drawText(fontSize10,fontSize20,"x");
    p.end();
    PBOneOverX->setPixmap(*PMOneOverX);

    PMSquareRoot = new QPixmap(pixSize,pixSize);
    PMSquareRoot->fill(bg);
    p.begin(PMSquareRoot);
    p.setPen(black);
    p.setFont(big);
    p.drawText(fontSize15,fontSize20,"x");
    p.drawLine(fontSize05,fontSize15,fontSize10,fontSize20);
    p.drawLine(fontSize10,fontSize20,fontSize15,fontSize10);
    p.drawLine(fontSize15,fontSize10,fontSize25,fontSize10);
    p.end();
    PBSquareRoot->setPixmap(*PMSquareRoot);

    delete PMPow;
    delete PMLog;
    delete PMSquare;
    delete PMXRootY;
    delete PMOneOverX;
    delete PMSquareRoot;
} 

FormAdvanced::FormAdvanced(QWidget *parent,const char *name,WFlags fl)
:QWidget(parent,name,fl) {
    QSizePolicy mySizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred,FALSE);

    QVBoxLayout *vbl = new QVBoxLayout(this);
    AdvancedLayout = new QGridLayout( vbl, 2, 4, 0, "AdvancedLayout");

    QFont fonty;
    int fontSize = fonty.pixelSize();
    if (fontSize == 12)
	fontSize = 14;

    // Row 1
    PBMPlus = new QPushButton(this,"PBM+");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBMPlus->setText(tr("M+"));
    AdvancedLayout->addWidget(PBMPlus,0,0);

    PBMR = new QPushButton(this,"PBMR");
    PBMR->setSizePolicy(mySizePolicy);
    PBMR->setText(tr("MR"));
    AdvancedLayout->addWidget(PBMR,0,1);

    PBMC = new QPushButton(this,"PBMC");
    PBMC->setSizePolicy(mySizePolicy);
    PBMC->setText(tr("MC"));
    AdvancedLayout->addWidget(PBMC,0,2);

    PBC = new QPushButton(this,"PBC");
    PBC->setSizePolicy(mySizePolicy);
    PBC->setText(tr("CE/C"));
    AdvancedLayout->addWidget(PBC,0,3);

    // Row 2
    PBASin = new QPushButton(this,"PBASin");
    PBASin->setSizePolicy(mySizePolicy);
    PBASin->setText(tr("asin"));
    AdvancedLayout->addWidget(PBASin,1,0);

    PBACos = new QPushButton(this,"PBACos");
    PBACos->setSizePolicy(mySizePolicy);
    PBACos->setText(tr("acos"));
    AdvancedLayout->addWidget(PBACos,1,1);

    PBATan = new QPushButton(this,"PBATan");
    PBATan->setSizePolicy(mySizePolicy);
    PBATan->setText(tr("atan"));
    AdvancedLayout->addWidget(PBATan,1,2);

    PBDel = new QPushButton(this,"PBDel");
    PBDel->setSizePolicy(mySizePolicy);
    PBDel->setText(tr("<-"));
    AdvancedLayout->addWidget(PBDel,1,3);

    // Row 3
    PBSin = new QPushButton(this,"PBSin");
    PBSin->setSizePolicy(mySizePolicy);
    PBSin->setText(tr("sin"));
    AdvancedLayout->addWidget(PBSin,2,0);

    PBCos = new QPushButton(this,"PBCos");
    PBCos->setSizePolicy(mySizePolicy);
    PBCos->setText(tr("cos"));
    AdvancedLayout->addWidget(PBCos,2,1);

    PBTan = new QPushButton(this,"PBTan");
    PBTan->setSizePolicy(mySizePolicy);
    PBTan->setText(tr("tan"));
    AdvancedLayout->addWidget(PBTan,2,2);

    PBFactorial = new QPushButton(this,"PBTan");
    PBFactorial->setSizePolicy(mySizePolicy);
    PBFactorial->setText(tr("n!"));
    AdvancedLayout->addWidget(PBFactorial,2,3);

    // Row 4
    PBSquareRoot = new QPushButton(this,"PBSquareRoot");
    PBSquareRoot->setSizePolicy(mySizePolicy);
    AdvancedLayout->addWidget(PBSquareRoot,3,0);

    PBXRootY = new QPushButton(this,"PBXRootY");
    PBXRootY->setSizePolicy(mySizePolicy);
    AdvancedLayout->addWidget(PBXRootY,3,1);

    PBLn = new QPushButton(this,"PBLn");
    PBLn->setSizePolicy(mySizePolicy);
    PBLn->setText(tr("ln"));
    AdvancedLayout->addWidget(PBLn,3,2);

    PBNegate = new QPushButton(this,"PBNegate");
    PBNegate->setSizePolicy(mySizePolicy);
    PBNegate->setText(tr("+/-"));
    AdvancedLayout->addWidget(PBNegate,3,3);

    // Row 5
    PBSquare = new QPushButton(this,"PBSquare");
    PBSquare->setSizePolicy(mySizePolicy);
    AdvancedLayout->addWidget(PBSquare,4,0);

    PBPow = new QPushButton(this,"PBPow");
    PBPow->setSizePolicy(mySizePolicy);
    AdvancedLayout->addWidget(PBPow,4,1);

    PBLog = new QPushButton(this,"PBLog");
    PBLog->setSizePolicy(mySizePolicy);
    PBLog->setText(tr("log"));
    AdvancedLayout->addWidget(PBLog,4,2);

    PBOneOverX = new QPushButton(this,"PBOneOverX");
    PBOneOverX->setSizePolicy(mySizePolicy);
    AdvancedLayout->addWidget(PBOneOverX,4,3);

    updatePictures();

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

FormAdvanced::~FormAdvanced() {
    delete PBDel;
    delete PBC;
    delete PBMR;
    delete PBMC;
    delete PBMPlus;
    delete PBSin;
    delete PBCos;
    delete PBTan;
    delete PBOneOverX;
    delete PBLog;
    delete PBLn;
    delete PBFactorial;
    delete PBNegate;
    delete PBSquare;
    delete PBPow;
    delete PBSquareRoot;
    delete PBXRootY;

    delete siw;
    delete AdvancedLayout;
}

void FormAdvanced::showEvent ( QShowEvent *e ) {
    systemEngine->setAccType("Double"); // No tr
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
