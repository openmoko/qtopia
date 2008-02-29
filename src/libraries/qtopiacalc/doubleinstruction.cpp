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
#include "integerdata.h"
#include "fractiondata.h"
#include "engine.h"

// Maths libraries
#include <qpe/qmath.h>
#include <math.h>

// Automatic type casting
BaseDoubleInstructionDescription::BaseDoubleInstructionDescription()
    :InstructionDescription() {
    typeOne = typeTwo = "DOUBLE";
}
Data *BaseDoubleInstruction::eval(Data *d) {
    doubleNum = (DoubleData *)num;
    Data *ret = doEval((DoubleData *)d);
    return ret;
}

// Conversion functions
Data * iConvertDoubleDouble::eval(Data *d) {
    DoubleData *ret = new DoubleData();
    ret->set(((DoubleData *)d)->get());
    return ret;
}
ConvertDoubleDouble::ConvertDoubleDouble():BaseDoubleInstructionDescription() {
    instructionName = "CONVERT";
};
Data * iConvertIntDouble::eval(Data *d) {
    DoubleData *ret = new DoubleData();;
    IntegerData *i = (IntegerData *)d;
    ret->set((double)i->get());
    return ret;
}
ConvertIntDouble::ConvertIntDouble():InstructionDescription() {
    instructionName = "CONVERT";
    typeOne = "INT";
    typeTwo = "DOUBLE";
}
Data * iConvertFractionDouble::eval(Data *d) {
    DoubleData *ret = new DoubleData();
    FractionData *f = (FractionData *)d;
    if (!f->getDenominator()) {
	Engine i;
	i.setError(eDivZero);
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
    instructionName = "CONVERT";
    typeOne = "FRACTION";
    typeTwo = "DOUBLE";
}

// Mathematical functions
Data * iAddDoubleDouble::doEval(DoubleData *d) {
    d->set(doubleNum->get() + d->get());
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
	Engine i;
	i.setError(eNonPositive);
    } else if ( d->get() > 180 ) {
	Engine i;
	i.setError(eOutOfRange);
    } else if ( d->get() != int(d->get()) ) {
	Engine i;
	i.setError(eNonInteger);
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
    d->set( cbrt(d->get()) );
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
    instructionName = "ADD";
    precedence = 10;
}
SubtractDoubleDouble::SubtractDoubleDouble():BaseDoubleInstructionDescription() {
    instructionName = "SUBTRACT";
    precedence = 10;
}
MultiplyDoubleDouble::MultiplyDoubleDouble():BaseDoubleInstructionDescription() {
    instructionName = "MULTIPLY";
    precedence = 15;
}
DivideDoubleDouble::DivideDoubleDouble():BaseDoubleInstructionDescription() {
    instructionName = "DIVIDE";
    precedence = 15;
}
DoublePow::DoublePow():BaseDoubleInstructionDescription() {
    instructionName = "POW";
    precedence = 20;
}
DoubleSin::DoubleSin():BaseDoubleInstructionDescription() {
    instructionName = "SIN";
}
DoubleCos::DoubleCos():BaseDoubleInstructionDescription() {
    instructionName = "COS";
}
DoubleTan::DoubleTan():BaseDoubleInstructionDescription() {
    instructionName = "TAN";
}
DoubleASin::DoubleASin():BaseDoubleInstructionDescription() {
    instructionName = "ASIN";
}
DoubleACos::DoubleACos():BaseDoubleInstructionDescription() {
    instructionName = "ACOS";
}
DoubleATan::DoubleATan():BaseDoubleInstructionDescription() {
    instructionName = "ATAN";
}
DoubleLog::DoubleLog():BaseDoubleInstructionDescription() {
    instructionName = "LOG";
}
DoubleLn::DoubleLn():BaseDoubleInstructionDescription() {
    instructionName = "LN";
}
DoubleExp::DoubleExp():BaseDoubleInstructionDescription() {
    instructionName = "EXP";
}
DoubleOneOverX::DoubleOneOverX():BaseDoubleInstructionDescription() {
    instructionName = "ONEOVERX";
}
DoubleFactorial::DoubleFactorial():BaseDoubleInstructionDescription() {
    instructionName = "XFACTORIAL";
}
DoubleSquareRoot::DoubleSquareRoot():BaseDoubleInstructionDescription() {
    instructionName = "SQUAREROOT";
}
DoubleCubeRoot::DoubleCubeRoot():BaseDoubleInstructionDescription() {
    instructionName = "CUBEROOT";
}
DoubleXRootY::DoubleXRootY():BaseDoubleInstructionDescription() {
    instructionName = "XROOTY";
    precedence = 20;
}

DoubleSquare::DoubleSquare():BaseDoubleInstructionDescription() {
    instructionName = "SQUARE";
}
DoubleNegate::DoubleNegate():BaseDoubleInstructionDescription() {
    instructionName = "NEGATE";
}
