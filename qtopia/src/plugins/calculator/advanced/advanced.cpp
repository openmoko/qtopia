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
#include "advanced.h"
#include <qtopia/calc/doubleinstruction.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qgrid.h>

#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>

static void drawRootOfX(QPainter *p, int x, int y, int fontSize)
{
    int fontSize05 = fontSize / 2;
    p->drawText(x+fontSize05-2,y,"x");
    p->drawLine(x-fontSize,y-fontSize05,x-fontSize05,y);
    p->drawLine(x-fontSize05,y,x,y-fontSize);
    p->drawLine(x,y-fontSize,x+fontSize+3,y-fontSize);
}
 
class PowerButton : public AdvancedButton {
public:
    PowerButton(QWidget *p, const QString &name) : AdvancedButton(p, name) { }
    void advancedDrawLabel( QPainter *p, int x, int y ) {
	if ( inv ) {
	    p->drawText(x - 5, y - fontSize05 - 2, "y");
	    drawRootOfX(p, x + fontSize05 - 2, y + 2, fontSize10);
	} else {
	    p->drawText(x, y, "x");
	    p->drawText(x + fontSize05, y - fontSize05, "y");
	}
    }
};

class SquareButton : public AdvancedButton {
public:
    SquareButton(QWidget *p, const QString &name) : AdvancedButton(p, name) { }
    void advancedDrawLabel( QPainter *p, int x, int y ) {
	if ( inv ) {
	    drawRootOfX(p, x, y, fontSize10);
	} else {
	    p->drawText(x, y, "x");
	    p->drawText(x+fontSize05, y-fontSize05, "2");
	}
    }
};
 
class LnButton : public AdvancedButton {
public:
    LnButton(QWidget *p, const QString &name) : AdvancedButton(p, name) { }
    void advancedDrawLabel( QPainter *p, int x, int y ) {
	if ( inv ) {
	    p->drawText(x, y, "e");
	    p->drawText(x+fontSize05, y-fontSize05, "x");
	} else {
	    p->drawText(x, y, "Ln");
	}
    }
};

class OneOverButton : public AdvancedButton {
public:
    OneOverButton(QWidget *p, const QString &name) : AdvancedButton(p, name) { }
    void advancedDrawLabel( QPainter *p, int x, int y ) {
	p->drawText(x + fontSize05, y - fontSize05, "1");
	int between = y + (fontSize05 / 2) - fontSize05;
	p->drawLine(x, between, x + fontSize10 + fontSize05, between);
	p->drawText(x + fontSize05, y + fontSize05, "x");
    }
};
 
void FormAdvanced::fontChange() {
    update();
    updateGeometry();
}

FormAdvanced::FormAdvanced(QWidget *parent) :QWidget(parent), IsInverse(FALSE){
    QSizePolicy mySizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred,FALSE);

    QWidget *d = QPEApplication::desktop();
    bool portrait;
    siw = new Type1DecimalInputWidget(this,"SIW"); //number keys
    siw->setSizePolicy(mySizePolicy);
    QWhatsThis::add(this, tr("The scientific calculator plugin provides the most comprehensive " 
                "interface. Many buttons have a second mode which can be activated via <b>INV</b>."));

    QGrid * grid = new QGrid(3, QGrid::Horizontal, this);
    grid->setMargin( 6 );
    grid->setSizePolicy(mySizePolicy);
    degree = new QRadioButton(tr("Degree"),grid);
    QWhatsThis::add(degree, tr("Calculate in degree mode"));
    radians = new QRadioButton(tr("Radians"),grid);
    QWhatsThis::add(radians, tr("Calculate in radian mode"));
    gradians = new QRadioButton(tr("Gradians"), grid);
    QWhatsThis::add(gradians, tr("Calculate in gradian mode"));
    degree->setChecked(TRUE);

    connect(degree, SIGNAL(toggled(bool)), this, SLOT(DegreeMode(bool)));
    connect(radians, SIGNAL(toggled(bool)), this, SLOT(RadiansMode(bool)));
    connect(gradians, SIGNAL(toggled(bool)), this, SLOT(GradiansMode(bool)));
    QGridLayout *AdvancedLayout;

    if ( d->width() < d->height() ) { //portrait
        AdvancedLayout = new QGridLayout( this, 10, 4, 0, 3 );
        AdvancedLayout->addMultiCellWidget(grid, 0, 0, 0, 3);
        AdvancedLayout->addMultiCellWidget(siw, 6, 9, 0, 3);
        portrait = TRUE;
    } else { //landscape
        AdvancedLayout = new QGridLayout( this, 5, 8, 0, 3);
        AdvancedLayout->addMultiCellWidget(grid, 0, 0, 0, 7);
        AdvancedLayout->addMultiCellWidget(siw, 1, 4, 4, 7);
        portrait = FALSE;
    }
    
    
    PBMPlus = new QPushButton(this,"PBM+");
    PBMPlus->setSizePolicy(mySizePolicy);
    PBMPlus->setText(tr("M+"));

    PBMR = new QPushButton(this,"PBMR");
    PBMR->setSizePolicy(mySizePolicy);
    PBMR->setText(tr("MR"));

    PBMC = new QPushButton(this,"PBMC");
    PBMC->setSizePolicy(mySizePolicy);
    PBMC->setText(tr("MC"));

    PBC = new QPushButton(this,"PBC");
    PBC->setSizePolicy(mySizePolicy);
    PBC->setText(tr("CE/C"));

    if (portrait) {
        AdvancedLayout->addWidget(PBMPlus, 1, 0);
        AdvancedLayout->addWidget(PBMR, 1, 1);
        AdvancedLayout->addWidget(PBMC, 1, 2);
        AdvancedLayout->addWidget(PBC, 1, 3);
    } else {
        AdvancedLayout->addWidget(PBMPlus, 3, 3);
        AdvancedLayout->addWidget(PBMR, 4, 3);
        AdvancedLayout->addWidget(PBMC, 2, 3);
        AdvancedLayout->addWidget(PBC, 1, 3);
    }
    
    PBPi = new QPushButton(this,"PBPi");
    PBPi->setSizePolicy(mySizePolicy);
    PBPi->setText(tr("Pi"));
    
    PBE = new QPushButton(this, "PBE");
    PBE->setSizePolicy(mySizePolicy);
    PBE->setText(tr("e"));
    
    PBSin = new QPushButton(this,"PBSin");
    PBSin->setSizePolicy(mySizePolicy);
    PBSin->setText(tr("Sin"));
    
    PBCos = new QPushButton(this,"PBCos");
    PBCos->setSizePolicy(mySizePolicy);
    PBCos->setText(tr("Cos"));
       
    PBTan = new QPushButton(this,"PBTan");
    PBTan->setSizePolicy(mySizePolicy);
    PBTan->setText(tr("Tan"));

    PBFactorial = new QPushButton(this,"PBFactorial");
    PBFactorial->setSizePolicy(mySizePolicy);
    PBFactorial->setText(tr("n!"));

    PBLn = (AdvancedButton*)new LnButton(this,"PBLn");
    PBLn->setSizePolicy(mySizePolicy);

    PBInverse = new QPushButton(this, "PBInverse");
    PBInverse->setSizePolicy(mySizePolicy);
    PBInverse->setText(tr("INV", "inverse mode"));

    PBSquare = (AdvancedButton*)new SquareButton(this,"PBSquare");
    PBSquare->setSizePolicy(mySizePolicy);

    PBPow = (AdvancedButton*)new PowerButton(this,"PBPow");
    PBPow->setSizePolicy(mySizePolicy);

    PBOneOverX = (QPushButton*)new OneOverButton(this,"PBOneOverX");
    PBOneOverX->setSizePolicy(mySizePolicy);

    PBLog = new QPushButton(this, "PBLog10");
    PBLog->setSizePolicy(mySizePolicy);
    PBLog->setText(tr("Log"));

    if (portrait) {
        AdvancedLayout->addWidget(PBPi, 2, 2);
        AdvancedLayout->addWidget(PBSin, 3, 0);
        AdvancedLayout->addWidget(PBCos, 3, 1);
        AdvancedLayout->addWidget(PBTan, 3, 2);
        AdvancedLayout->addWidget(PBFactorial, 2, 0);
        AdvancedLayout->addWidget(PBLn, 4, 3);
        AdvancedLayout->addWidget(PBInverse, 2, 3);
        AdvancedLayout->addWidget(PBE, 2, 1);
        AdvancedLayout->addWidget(PBSquare, 4, 0);
        AdvancedLayout->addWidget(PBPow, 4, 1);
        AdvancedLayout->addWidget(PBLog, 3, 3);
        AdvancedLayout->addWidget(PBOneOverX, 4, 2);
 
    } else {
        AdvancedLayout->addWidget(PBPi, 1, 1);
        AdvancedLayout->addWidget(PBSin, 2, 0);
        AdvancedLayout->addWidget(PBCos, 2, 1);
        AdvancedLayout->addWidget(PBTan, 2, 2);
        AdvancedLayout->addWidget(PBFactorial, 4, 2);
        AdvancedLayout->addWidget(PBLn, 3, 1);
        AdvancedLayout->addWidget(PBInverse, 1, 2);
        AdvancedLayout->addWidget(PBE, 1, 0);
        AdvancedLayout->addWidget(PBSquare, 4, 0);
        AdvancedLayout->addWidget(PBPow, 4, 1);
        AdvancedLayout->addWidget(PBLog, 3, 0);
        AdvancedLayout->addWidget(PBOneOverX, 3, 2);
    }

    connect (PBMR, SIGNAL(clicked()), this, SLOT(MRClicked()));
    connect (PBMC, SIGNAL(clicked()), this, SLOT(MCClicked()));
    connect (PBMPlus, SIGNAL(clicked()), this, SLOT(MPlusClicked()));
    connect (PBC, SIGNAL(clicked()), this, SLOT(CClicked()));

    connect (PBPi, SIGNAL(clicked()), this, SLOT(PiClicked()));
    connect (PBE, SIGNAL(clicked()), this, SLOT(EClicked()));
    connect (PBInverse, SIGNAL(clicked()), this, SLOT(InverseClicked()));

    connect (PBSin, SIGNAL(clicked()), this, SLOT(SinDegClicked()));
    connect (PBCos, SIGNAL(clicked()), this, SLOT(CosDegClicked()));
    connect (PBTan, SIGNAL(clicked()), this, SLOT(TanDegClicked()));
    connect (PBOneOverX, SIGNAL(clicked()), this, SLOT(OneOverXClicked()));

    connect (PBLn, SIGNAL(clicked()), this, SLOT(LnClicked()));
    connect (PBLog, SIGNAL(clicked()), this, SLOT(LogClicked()));
    connect (PBFactorial, SIGNAL(clicked()), this, SLOT(FactorialClicked()));

    connect (PBSquare, SIGNAL(clicked()), this, SLOT(SquareClicked()));
    connect (PBPow, SIGNAL(clicked()), this, SLOT(PowClicked()));
}

FormAdvanced::~FormAdvanced() {
    delete PBPi;
    delete PBC;
    delete PBMR;
    delete PBMC;
    delete PBMPlus;
    delete PBSin;
    delete PBCos;
    delete PBTan;
    delete PBOneOverX;
    delete PBLn;
    delete PBFactorial;
    delete PBSquare;
    delete PBPow;
    delete PBLog;
    delete degree;
    delete gradians;
    delete radians;
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

void FormAdvanced::DelClicked() {
    systemEngine->delChar();
}

void FormAdvanced::SinDegClicked() {
    if (IsInverse) {
        systemEngine->pushInstruction("aSinDeg"); // no tr
        InverseClicked();
    }
    else
        systemEngine->pushInstruction("SinDeg"); // No tr
}
void FormAdvanced::CosDegClicked() {
    if (IsInverse) {
        systemEngine->pushInstruction("aCosDeg"); // no tr
        InverseClicked();
    }
    else
        systemEngine->pushInstruction("CosDeg"); // No tr
}
void FormAdvanced::TanDegClicked() {
    if (IsInverse) {
        systemEngine->pushInstruction("aTanDeg"); // no tr
        InverseClicked();
    }
    else
        systemEngine->pushInstruction("TanDeg"); // No tr
}
void FormAdvanced::SinRadClicked() {
    if (IsInverse) {
        systemEngine->pushInstruction("aSinRad"); // no tr
        InverseClicked();
    }
    else
        systemEngine->pushInstruction("SinRad"); // No tr
}
void FormAdvanced::CosRadClicked() {
    if (IsInverse) {
        systemEngine->pushInstruction("aCosRad"); // no tr
        InverseClicked();
    }
    else
        systemEngine->pushInstruction("CosRad"); // No tr
}
void FormAdvanced::TanRadClicked() {
    if (IsInverse) {
        systemEngine->pushInstruction("aTanRad"); // no tr
        InverseClicked();
    }
    else
        systemEngine->pushInstruction("TanRad"); // No tr
}
void FormAdvanced::SinGraClicked() {
    if (IsInverse) {
        systemEngine->pushInstruction("aSinGra"); // no tr
        InverseClicked();
    }
    else
        systemEngine->pushInstruction("SinGra"); // No tr
}
void FormAdvanced::CosGraClicked() {
    if (IsInverse) {
        systemEngine->pushInstruction("aCosGra"); // no tr
        InverseClicked();
    }
    else
        systemEngine->pushInstruction("CosGra"); // No tr
}
void FormAdvanced::TanGraClicked() {
    if (IsInverse) {
        systemEngine->pushInstruction("aTanGra"); // no tr
        InverseClicked();
    }
    else
        systemEngine->pushInstruction("TanGra"); // No tr
}
void FormAdvanced::OneOverXClicked() {
    systemEngine->pushInstruction("One over x"); // No tr
}
void FormAdvanced::LnClicked() {
    if (IsInverse) {
        systemEngine->pushInstruction("Exp"); //no tr
        InverseClicked();
    }
    else
        systemEngine->pushInstruction("Ln"); // No tr
}
void FormAdvanced::LogClicked() {
    systemEngine->pushInstruction("Log"); //no tr
}
void FormAdvanced::FactorialClicked() {
    systemEngine->pushInstruction("Factorial"); // No tr
}
void FormAdvanced::SquareClicked() {
    if (IsInverse) {
        systemEngine->pushInstruction("Square root"); // No tr
        InverseClicked();
    }
    else
        systemEngine->pushInstruction("Square"); // No tr
}
void FormAdvanced::PowClicked() {
    if (IsInverse) {
        systemEngine->pushInstruction("X root Y"); //no tr
        InverseClicked();
    }
    else
        systemEngine->pushInstruction("Pow"); // No tr
}
void FormAdvanced::PiClicked() {
    systemEngine->push(QString("3.14159265358979323846")); // No tr
}
void FormAdvanced::EClicked() {
    systemEngine->push(QString("2.71828182845904523536")); // No tr
}
void FormAdvanced::InverseClicked() {
    IsInverse = ! IsInverse;
    if (IsInverse) {
        PBSin->setText(tr("ASin"));
        PBCos->setText(tr("ACos"));
        PBTan->setText(tr("ATan"));
    } else {
        PBSin->setText(tr("Sin"));
        PBCos->setText(tr("Cos"));
        PBTan->setText(tr("Tan"));
    }

    PBSquare->setInversed( IsInverse );
    PBPow->setInversed( IsInverse );
    PBLn->setInversed( IsInverse );
}

void FormAdvanced::DegreeMode(bool on) {
    if (on) {
        radians->setChecked(FALSE);
        gradians->setChecked(FALSE);
        PBSin->disconnect();
        PBCos->disconnect();
        PBTan->disconnect();
        connect (PBSin, SIGNAL(clicked()), this, SLOT(SinDegClicked()));
        connect (PBCos, SIGNAL(clicked()), this, SLOT(CosDegClicked()));
        connect (PBTan, SIGNAL(clicked()), this, SLOT(TanDegClicked()));
    }
}
void FormAdvanced::RadiansMode(bool on) {
    if (on) {
        degree->setChecked(FALSE);
        gradians->setChecked(FALSE);
        PBSin->disconnect();
        PBCos->disconnect();
        PBTan->disconnect();
        connect (PBSin, SIGNAL(clicked()), this, SLOT(SinRadClicked()));
        connect (PBCos, SIGNAL(clicked()), this, SLOT(CosRadClicked()));
        connect (PBTan, SIGNAL(clicked()), this, SLOT(TanRadClicked()));
    }
}
void FormAdvanced::GradiansMode(bool on) {
    if (on) {
        radians->setChecked(FALSE);
        degree->setChecked(FALSE);
        PBSin->disconnect();
        PBCos->disconnect();
        PBTan->disconnect();
        connect (PBSin, SIGNAL(clicked()), this, SLOT(SinGraClicked()));
        connect (PBCos, SIGNAL(clicked()), this, SLOT(CosGraClicked()));
        connect (PBTan, SIGNAL(clicked()), this, SLOT(TanGraClicked()));
    }
}
