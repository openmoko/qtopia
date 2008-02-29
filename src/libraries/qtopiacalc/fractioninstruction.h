/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef CALC_FRACTION_H
#define CALC_FRACTION_H

#include "instruction.h"
#include "fractiondata.h"

//Fraction instruction base
class BaseFractionInstruction:public Instruction {
public:
    BaseFractionInstruction():Instruction(){};
    ~BaseFractionInstruction(){};

    Data *eval(Data *);
    virtual Data *doEval(FractionData *){return new FractionData();};
protected:
    FractionData *fractionNum;
};
class BaseFractionInstructionDescription:public InstructionDescription {
public:
    BaseFractionInstructionDescription();
    ~BaseFractionInstructionDescription(){};
};

// Conversions
class iConvertFractionFraction:public BaseFractionInstruction {
public:
    iConvertFractionFraction():BaseFractionInstruction(){};
    ~iConvertFractionFraction(){};
    Data *eval(Data *);
};
class ConvertFractionFraction:public BaseFractionInstructionDescription {
public:
    ConvertFractionFraction();
    ~ConvertFractionFraction(){};
    Instruction *getInstruction(){return new iConvertFractionFraction();};
};
class iConvertDoubleFraction:public BaseFractionInstruction {
public:
    iConvertDoubleFraction():BaseFractionInstruction(){};
    ~iConvertDoubleFraction(){};
    Data *eval(Data *);
};
class ConvertDoubleFraction:public InstructionDescription {
public:
    ConvertDoubleFraction();
    ~ConvertDoubleFraction(){};
    Instruction *getInstruction(){return new iConvertDoubleFraction();};
};

// Mathematical functions
class iAddFractionFraction:public BaseFractionInstruction {
public:
    iAddFractionFraction(){};
    ~iAddFractionFraction(){};
    Data *doEval(FractionData *);
};
class AddFractionFraction:public BaseFractionInstructionDescription {
public:
    AddFractionFraction();
    ~AddFractionFraction(){};
    Instruction *getInstruction() {return new iAddFractionFraction();};
};

class iSubtractFractionFraction:public BaseFractionInstruction {
public:
    iSubtractFractionFraction (){};
    ~iSubtractFractionFraction (){};
    Data *doEval(FractionData *);
};
class SubtractFractionFraction:public BaseFractionInstructionDescription {
public:
    SubtractFractionFraction();
    ~SubtractFractionFraction(){};
    Instruction *getInstruction() {return new iSubtractFractionFraction();};
};

class iMultiplyFractionFraction:public BaseFractionInstruction {
public:
    iMultiplyFractionFraction (){};
    ~iMultiplyFractionFraction (){};
    Data *doEval(FractionData *);
};
class MultiplyFractionFraction:public BaseFractionInstructionDescription {
public:
    MultiplyFractionFraction();
    ~MultiplyFractionFraction(){};
    Instruction *getInstruction() {return new iMultiplyFractionFraction();};
};

class iDivideFractionFraction:public BaseFractionInstruction {
public:
    iDivideFractionFraction (){};
    ~iDivideFractionFraction (){};
    Data *doEval(FractionData *);
};
class DivideFractionFraction:public BaseFractionInstructionDescription {
public:
    DivideFractionFraction();
    ~DivideFractionFraction(){};
    Instruction *getInstruction() {return new iDivideFractionFraction();};
};

#endif
