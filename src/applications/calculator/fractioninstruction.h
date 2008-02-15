/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef CALC_FRACTION_H
#define CALC_FRACTION_H

#include "instruction.h"
#include "fractiondata.h"
#include "doubledata.h"

//Fraction instruction base
class BaseFractionInstruction:public Instruction {
public:
    BaseFractionInstruction();
    ~BaseFractionInstruction(){};

    virtual void eval();
    virtual void doEval(FractionData *,FractionData *){};
    virtual void doEval(FractionData *){};
};

// Factory
class iFractionFactory:public Instruction {
public:
    iFractionFactory();
    ~iFractionFactory(){};
    void eval();
};

// Copy
class iFractionCopy:public BaseFractionInstruction {
public:
    iFractionCopy();
    ~iFractionCopy(){};
    void eval();
};

// Conversions
class iConvertDoubleFraction:public Instruction {
public:
    iConvertDoubleFraction();
    ~iConvertDoubleFraction(){};
    void eval();
};

// Mathematical functions
class iAddFractionFraction:public BaseFractionInstruction {
public:
    iAddFractionFraction();
    ~iAddFractionFraction(){};
    void doEval(FractionData *,FractionData *);
};

class iSubtractFractionFraction:public BaseFractionInstruction {
public:
    iSubtractFractionFraction ();
    ~iSubtractFractionFraction (){};
    void doEval(FractionData *,FractionData *);
};

class iMultiplyFractionFraction:public BaseFractionInstruction {
public:
    iMultiplyFractionFraction ();
    ~iMultiplyFractionFraction (){};
    void doEval(FractionData *,FractionData *);
};

class iDivideFractionFraction:public BaseFractionInstruction {
public:
    iDivideFractionFraction ();
    ~iDivideFractionFraction (){};
    void doEval(FractionData *,FractionData *);
};

class iNegateFractionFraction:public BaseFractionInstruction {
public:
    iNegateFractionFraction ();
    ~iNegateFractionFraction (){};
    void doEval(FractionData *);
};

#endif
#endif //ENABLE_FRACTION
