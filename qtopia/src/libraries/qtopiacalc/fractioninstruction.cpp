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
#include "fractioninstruction.h"
#include "doubledata.h"

// Data type
void BaseFractionInstruction::eval() {
    FractionData *acc = (FractionData *)systemEngine->getData();
    if (argCount == 1)
	doEval(acc);
    else {
	FractionData *num = (FractionData *)systemEngine->getData();
	doEval(acc,num);
	delete num;
    }
    delete acc;
}
BaseFractionInstruction::BaseFractionInstruction()
    : Instruction() {
	type = retType = "Fraction"; // No tr
    }

// Factory
void iFractionFactory::eval() {
    FractionData *ret = new FractionData();
    ret->clear();
    systemEngine->putData(ret);
}
iFractionFactory::iFractionFactory():Instruction() {
    type = retType = "Fraction"; // No tr
    name = "Factory"; // No tr
};

// Copy
void iFractionCopy::eval() {
    FractionData *src = (FractionData *)systemEngine->getData();
    FractionData *tgt = new FractionData();
    tgt->clear();
    tgt->set(((FractionData *)src)->getNumerator(),
	    ((FractionData *)src)->getDenominator());
    systemEngine->putData(src);
    systemEngine->putData(tgt);
};
iFractionCopy::iFractionCopy():BaseFractionInstruction() {
    name = "Copy"; // No tr
    retType = type = "Fraction"; // No tr
    argCount = 1;
};

// Conversion functions
void iConvertDoubleFraction::eval() {

    FractionData *ret = new FractionData();
    double target,tmp, diff;
    int64 upper,lower;
    
    DoubleData *doubleD = (DoubleData *)systemEngine->getData();
    target = doubleD->get();
    QString formattedOutput = doubleD->getFormattedOutput();
    bool ok=FALSE;
    if (formattedOutput.contains('.') == 0) {
	ret->set(formattedOutput.toInt(&ok),1);
    }
    int maxCycles = 0;
    if (!ok) {
	diff = upper = lower = 1;
	tmp = 0;
	while ( (diff > 0.0000001 || diff < -0.0000001) &&  maxCycles < 100000) { // limited precision
            maxCycles++;
	    if (tmp < target)
		upper++;
	    else {
		lower++;
		upper = (int64) target * lower;
	    }
	    tmp =  (double) upper / (double) lower;
            diff = tmp-target;
	}
	ret->set(upper,lower);
    }
    if (ok || maxCycles < 100000) {
        systemEngine->putData(ret);
        delete doubleD;
    } else {
        delete ret;
        systemEngine->putData(doubleD); // put data back on stack for recover
        systemEngine->setError(eOutOfRange, FALSE);
    }
}
iConvertDoubleFraction::iConvertDoubleFraction():Instruction() {
    name = "Convert"; // No tr
    retType = "Fraction"; // No tr
    type = "Double"; // No tr
}

// Mathematical functions
void iAddFractionFraction::doEval (FractionData *f,FractionData *fractionNum) {
    FractionData *result = new FractionData();
    int64 nn,nd,fn,fd;
    nn = fractionNum->getNumerator();
    nd = fractionNum->getDenominator();
    fn = f->getNumerator();
    fd = f->getDenominator();
    result->set(nn * fd + nd * fn, nd * fd);
    systemEngine->putData(result);
}
void iSubtractFractionFraction::doEval (FractionData *f,FractionData *fractionNum) {
    FractionData *result = new FractionData();
    int64 nn,nd,fn,fd;
    nn = fractionNum->getNumerator();
    nd = fractionNum->getDenominator();
    fn = f->getNumerator();
    fd = f->getDenominator();
    result->set(nn * fd - nd * fn, nd * fd);
    systemEngine->putData(result);
}
void iMultiplyFractionFraction::doEval (FractionData *f,FractionData *fractionNum) {
    FractionData *result = new FractionData();
    int64 nn = fractionNum->getNumerator();
    int64 nd = fractionNum->getDenominator();
    int64 fn = f->getNumerator();
    int64 fd = f->getDenominator();
    result->set(nn * fn, nd * fd);
    systemEngine->putData(result);
}
void iDivideFractionFraction::doEval (FractionData *f,FractionData *fractionNum) {
    FractionData *result = new FractionData();
    int64 nn = fractionNum->getNumerator();
    int64 nd = fractionNum->getDenominator();
    int64 fn = f->getNumerator();
    int64 fd = f->getDenominator();
    if (nd*fn == 0) {
        systemEngine->setError(eDivZero);
        return;
    }
    result->set(nn * fd, nd * fn);
    systemEngine->putData(result);
}

void iNegateFractionFraction::doEval (FractionData *f) {
    FractionData *result = new FractionData();
    result->set( -f->getNumerator(), f->getDenominator() );
    result->setEdited(TRUE);
    systemEngine->putData(result);
}

iAddFractionFraction::iAddFractionFraction():BaseFractionInstruction() {
    name = "Add"; // No tr
    precedence = 10;
    displayString = "+";
    argCount = 2;
}
iSubtractFractionFraction::iSubtractFractionFraction():BaseFractionInstruction() {
    name = "Subtract"; // No tr
    precedence = 10;
    displayString = "-";
    argCount = 2;
}
iMultiplyFractionFraction::iMultiplyFractionFraction():BaseFractionInstruction() {
    name = "Multiply"; // No tr
    precedence = 15;
    displayString = "x";
    argCount = 2;
}
iDivideFractionFraction::iDivideFractionFraction():BaseFractionInstruction() {
    name = "Divide"; // No tr
    precedence = 15;
    displayString = "/";
    argCount = 2;
}
iNegateFractionFraction::iNegateFractionFraction():BaseFractionInstruction() {
    name = "Negate"; // No tr
    precedence = 0;
    argCount = 1;
}
