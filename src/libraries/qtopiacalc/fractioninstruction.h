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
#ifndef CALC_FRACTION_H
#define CALC_FRACTION_H

#include "instruction.h"
#include "fractiondata.h"

//Fraction instruction base
class QTOPIA_EXPORT BaseFractionInstruction:public Instruction {
public:
    BaseFractionInstruction():Instruction(){};
    ~BaseFractionInstruction(){};

    Data *eval(Data *);
    virtual Data *doEval(FractionData *){return new FractionData();};
protected:
    FractionData *fractionNum;
};
class QTOPIA_EXPORT BaseFractionInstructionDescription:public InstructionDescription {
public:
    BaseFractionInstructionDescription();
    ~BaseFractionInstructionDescription(){};
};

// Factory
class QTOPIA_EXPORT iFractionFactory:public BaseFractionInstruction {
public:
    iFractionFactory():BaseFractionInstruction(){};
    ~iFractionFactory(){};
    Data *eval(Data *);
};
class QTOPIA_EXPORT FractionFactory:public BaseFractionInstructionDescription {
public:FractionFactory();
    ~FractionFactory(){};
    Instruction *getInstruction(){return new iFractionFactory();};
};

// Copy
class QTOPIA_EXPORT iFractionCopy:public BaseFractionInstruction {
public:
    iFractionCopy():BaseFractionInstruction(){};
    ~iFractionCopy(){};
    Data *eval(Data *);
};
class QTOPIA_EXPORT FractionCopy:public BaseFractionInstructionDescription {
public:
    FractionCopy();
    ~FractionCopy(){};
    Instruction *getInstruction(){return new iFractionCopy();};
};

// Conversions
class QTOPIA_EXPORT iConvertDoubleFraction:public BaseFractionInstruction {
public:
    iConvertDoubleFraction():BaseFractionInstruction(){};
    ~iConvertDoubleFraction(){};
    Data *eval(Data *);
};
class QTOPIA_EXPORT ConvertDoubleFraction:public InstructionDescription {
public:
    ConvertDoubleFraction();
    ~ConvertDoubleFraction(){};
    Instruction *getInstruction(){return new iConvertDoubleFraction();};
};

// Mathematical functions
class QTOPIA_EXPORT iAddFractionFraction:public BaseFractionInstruction {
public:
    iAddFractionFraction(){};
    ~iAddFractionFraction(){};
    Data *doEval(FractionData *);
};
class QTOPIA_EXPORT AddFractionFraction:public BaseFractionInstructionDescription {
public:
    AddFractionFraction();
    ~AddFractionFraction(){};
    Instruction *getInstruction() {return new iAddFractionFraction();};
};

class QTOPIA_EXPORT iSubtractFractionFraction:public BaseFractionInstruction {
public:
    iSubtractFractionFraction (){};
    ~iSubtractFractionFraction (){};
    Data *doEval(FractionData *);
};
class QTOPIA_EXPORT SubtractFractionFraction:public BaseFractionInstructionDescription {
public:
    SubtractFractionFraction();
    ~SubtractFractionFraction(){};
    Instruction *getInstruction() {return new iSubtractFractionFraction();};
};

class QTOPIA_EXPORT iMultiplyFractionFraction:public BaseFractionInstruction {
public:
    iMultiplyFractionFraction (){};
    ~iMultiplyFractionFraction (){};
    Data *doEval(FractionData *);
};
class QTOPIA_EXPORT MultiplyFractionFraction:public BaseFractionInstructionDescription {
public:
    MultiplyFractionFraction();
    ~MultiplyFractionFraction(){};
    Instruction *getInstruction() {return new iMultiplyFractionFraction();};
};

class QTOPIA_EXPORT iDivideFractionFraction:public BaseFractionInstruction {
public:
    iDivideFractionFraction (){};
    ~iDivideFractionFraction (){};
    Data *doEval(FractionData *);
};
class QTOPIA_EXPORT DivideFractionFraction:public BaseFractionInstructionDescription {
public:
    DivideFractionFraction();
    ~DivideFractionFraction(){};
    Instruction *getInstruction() {return new iDivideFractionFraction();};
};

#endif
