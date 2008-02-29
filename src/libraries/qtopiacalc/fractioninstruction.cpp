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
#include "fractioninstruction.h"
#include "doubledata.h"

// Automatic type casting
Data *BaseFractionInstruction::eval(Data *d) {
    fractionNum = (FractionData *)num;
    Data *ret = doEval((FractionData *)d);
    FractionData *fRet = (FractionData *)ret;
    fRet->set(fRet->getNumerator(),fRet->getDenominator());
    return ret;
}
BaseFractionInstructionDescription::BaseFractionInstructionDescription()
    : InstructionDescription() {
	typeOne = typeTwo = "FRACTION";
    }

// Conversion functions
Data * iConvertFractionFraction::eval(Data *d) {
    FractionData *ret = new FractionData();
    ret->set(((FractionData *)d)->getNumerator(),
	    ((FractionData *)d)->getDenominator());
    return ret;
}
ConvertFractionFraction::ConvertFractionFraction():BaseFractionInstructionDescription() {
    instructionName = "CONVERT";
}
Data * iConvertDoubleFraction::eval(Data *d) {
    FractionData *ret = new FractionData();
    double target,tmp,upper,lower;
    target = ((DoubleData *)d)->get();
    if (target == 0)
	ret->set(0,1);
    else {
	upper = lower = 1;
	tmp = 0;
	while (tmp != target) {
	    if (tmp < target)
		upper++;
	    else {
		lower++;
		upper = target * lower;
	    }
	    tmp = upper / lower;
	}
	ret->set(upper,lower);
    }
    return ret;
}
ConvertDoubleFraction::ConvertDoubleFraction():InstructionDescription() {
    instructionName = "CONVERT";
    typeOne = "DOUBLE";
    typeTwo = "FRACTION";
}

// Mathematical functions
Data * iAddFractionFraction::doEval (FractionData *f) {
    int nn,nd,fn,fd;
    nn = fractionNum->getNumerator();
    nd = fractionNum->getDenominator();
    fn = f->getNumerator();
    fd = f->getDenominator();
    fractionNum->set(nn * fd + nd * fn, nd * fd);
    return fractionNum;
}
Data * iSubtractFractionFraction::doEval (FractionData *f) {
    int nn = fractionNum->getNumerator();
    int nd = fractionNum->getDenominator();
    int fn = f->getNumerator();
    int fd = f->getDenominator();
    fractionNum->set(nn * fd - nd * fn, nd * fd);
    return fractionNum;
}
Data * iMultiplyFractionFraction::doEval (FractionData *f) {
    int nn = fractionNum->getNumerator();
    int nd = fractionNum->getDenominator();
    int fn = f->getNumerator();
    int fd = f->getDenominator();
    fractionNum->set(nn * fn, nd * fd);
    return fractionNum;
}
Data * iDivideFractionFraction::doEval (FractionData *f) {
    int nn = fractionNum->getNumerator();
    int nd = fractionNum->getDenominator();
    int fn = f->getNumerator();
    int fd = f->getDenominator();
    fractionNum->set(nn * fd, nd * fn);
    return fractionNum;
}

AddFractionFraction::AddFractionFraction():BaseFractionInstructionDescription() {
    instructionName = "ADD";
    precedence = 10;
}
SubtractFractionFraction::SubtractFractionFraction():BaseFractionInstructionDescription() {
    instructionName = "SUBTRACT";
    precedence = 10;
}
MultiplyFractionFraction::MultiplyFractionFraction():BaseFractionInstructionDescription() {
    instructionName = "MULTIPLY";
    precedence = 15;
}
DivideFractionFraction::DivideFractionFraction():BaseFractionInstructionDescription() {
    instructionName = "DIVIDE";
    precedence = 15;
}

