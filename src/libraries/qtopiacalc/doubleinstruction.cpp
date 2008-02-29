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
#include "doubleinstruction.h"
#ifdef ENABLE_INTEGER
#include "integerdata.h"
#endif
#ifdef ENABLE_FRACTION
#include "fractiondata.h"
#endif
#include "engine.h"

// Maths libraries
#include <qtopia/qmath.h>
#include <math.h>

// Base class automatic type casting
BaseDoubleInstructionDescription::BaseDoubleInstructionDescription()
    :InstructionDescription() {
    typeOne = typeTwo = type = "Double"; // No tr
}
Data *BaseDoubleInstruction::eval(Data *d) {
    doubleNum = (DoubleData *)num;
    Data *ret = doEval((DoubleData *)d);
    return ret;
}

// Factory
Data * iDoubleFactory::eval(Data * /* d */) {
    DoubleData *ret = new DoubleData();
    ret->clear();
    return ret;
}
DoubleFactory::DoubleFactory():BaseDoubleInstructionDescription() {
    instructionName = "Factory"; // No tr
};

// Copy
Data *iDoubleCopy::eval(Data *d) {
    DoubleData *ret = new DoubleData();
    ret->clear();
    ret->set(((DoubleData *)d)->get());
    return ret;
};
DoubleCopy::DoubleCopy():BaseDoubleInstructionDescription() {
    instructionName = "Copy"; // No tr
};

#ifdef ENABLE_INTEGER
Data * iConvertIntDouble::eval(Data *d) {
    DoubleData *ret = new DoubleData();;
    IntegerData *i = (IntegerData *)d;
    ret->set((double)i->get());
    return ret;
}
ConvertIntDouble::ConvertIntDouble():InstructionDescription() {
    instructionName = "Convert"; // No tr
    typeOne = "Int";
    typeTwo = "Double"; // No tr
}
#endif
#ifdef ENABLE_FRACTION
Data * iConvertFractionDouble::eval(Data *d) {
    DoubleData *ret = new DoubleData();
    FractionData *f = (FractionData *)d;
    if (!f->getDenominator()) {
	systemEngine->setError(eDivZero);
	ret->set(0);
    } else {
	double num = f->getNumerator();
	double den = f->getDenominator();
	double val = num / den;
	ret->set(val);
    }
    return ret;
}
ConvertFractionDouble::ConvertFractionDouble():InstructionDescription() {
    instructionName = "Convert"; // No tr
    typeOne = "Fraction"; // No tr
    typeTwo = "Double"; // No tr
}
#endif
// Mathematical functions
Data * iAddDoubleDouble::doEval(DoubleData *d) {
//qDebug("%d + %d", doubleNum->get(),d->get());
    d->set(doubleNum->get() + d->get());
//qDebug("= %d",d->get());
    return d;
}
Data * iSubtractDoubleDouble::doEval(DoubleData *d) {
    d->set(doubleNum->get() - d->get());
    return d;
}
Data * iMultiplyDoubleDouble::doEval(DoubleData *d) {
    d->set( doubleNum->get() * d->get() );
    return d;
}
Data * iDivideDoubleDouble::doEval(DoubleData *d) {
    d->set( doubleNum->get() / d->get() );
    return d;
}
Data * iDoublePow::doEval(DoubleData *d) {
    d->set( pow(doubleNum->get(),d->get()) );
    return d;
}
DoubleXRootY::DoubleXRootY():BaseDoubleInstructionDescription() {
    instructionName = "X root y"; // No tr
    precedence = 20;
}


// Immediate
Data * iDoubleSin::doEval(DoubleData *d) {
    d->set( qSin(d->get()) );
    return d;
}
Data * iDoubleCos::doEval(DoubleData *d) {
    d->set( qCos(d->get()) );
    return d;
}
Data * iDoubleTan::doEval(DoubleData *d) {
    d->set( qTan(d->get()) );
    return d;
}
Data * iDoubleASin::doEval(DoubleData *d) {
    d->set( asin(d->get()) );
    return d;
}
Data * iDoubleACos::doEval(DoubleData *d) {
    d->set( acos(d->get()) );
    return d;
}
Data * iDoubleATan::doEval(DoubleData *d) {
    d->set( atan(d->get()) );
    return d;
}
Data * iDoubleLog::doEval(DoubleData *d) {
    d->set( log10(d->get()) );
    return d;
}
Data * iDoubleLn::doEval(DoubleData *d) {
    d->set( log(d->get()) );
    return d;
}
Data * iDoubleExp::doEval(DoubleData *d) {
    d->set( exp(d->get()) );
    return d;
}
Data * iDoubleOneOverX::doEval(DoubleData *d) {
    d->set( 1 / d->get() );
    return d;
}
Data * iDoubleFactorial::doEval(DoubleData *d) {
    if (d->get() < 0) {
	systemEngine->setError(eNonPositive);
    } else if ( d->get() > 180 ) {
	systemEngine->setError(eOutOfRange);
    } else if ( d->get() != int(d->get()) ) {
	systemEngine->setError(eNonInteger);
    } else {
	int count = (int)d->get();
	d->set(1);
	while (count) {
	    d->set(d->get()*count);
	    count--;
	}
    }
    return d;
}
Data * iDoubleSquareRoot::doEval(DoubleData *d) {
    d->set( sqrt(d->get()) );
    return d;
}
Data * iDoubleCubeRoot::doEval(DoubleData *d) {
#ifndef Q_OS_WIN32
    d->set( cbrt(d->get()) );
#else
    qDebug("Cubic root not available for WIN32");
#endif
    return d;
}
Data * iDoubleXRootY::doEval(DoubleData *d) {
    d->set( pow(doubleNum->get(), 1 / d->get()) );
    return d;
}

Data * iDoubleSquare::doEval(DoubleData *d) {
    d->set( d->get() * d->get() );
    return d;
}

Data * iDoubleNegate::doEval(DoubleData *d) {
    d->set( 0 - d->get() );
    return d;
}

AddDoubleDouble::AddDoubleDouble():BaseDoubleInstructionDescription() {
    instructionName = "Add"; // No tr
    precedence = 10;
}
SubtractDoubleDouble::SubtractDoubleDouble():BaseDoubleInstructionDescription() {
    instructionName = "Subtract"; // No tr
    precedence = 10;
}
MultiplyDoubleDouble::MultiplyDoubleDouble():BaseDoubleInstructionDescription() {
    instructionName = "Multiply"; // No tr
    precedence = 15;
}
DivideDoubleDouble::DivideDoubleDouble():BaseDoubleInstructionDescription() {
    instructionName = "Divide"; // No tr
    precedence = 15;
}
DoublePow::DoublePow():BaseDoubleInstructionDescription() {
    instructionName = "Pow";
    precedence = 20;
}
DoubleSin::DoubleSin():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "Sin";
}
DoubleCos::DoubleCos():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "Cos";
}
DoubleTan::DoubleTan():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "Tan"; // No tr
}
DoubleASin::DoubleASin():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "aSin";
}
DoubleACos::DoubleACos():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "aCos";
}
DoubleATan::DoubleATan():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "aTan";
}
DoubleLog::DoubleLog():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "Log";
}
DoubleLn::DoubleLn():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "Ln";
}
DoubleExp::DoubleExp():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "Exp";
}
DoubleOneOverX::DoubleOneOverX():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "One over x"; // No tr
}
DoubleFactorial::DoubleFactorial():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "Factorial"; // No tr
}
DoubleSquareRoot::DoubleSquareRoot():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "Square root"; // No tr
}
DoubleCubeRoot::DoubleCubeRoot():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "Cube root"; // No tr
}
DoubleSquare::DoubleSquare():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "Square"; // No tr
}
DoubleNegate::DoubleNegate():BaseDoubleInstructionDescription() {
    precedence = 0;
    argCount = 1;
    instructionName = "Negate"; // No tr
}
