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
#ifndef CALC_DOUBLEINSTRUCTION_H
#define CALC_DOUBLEINSTRUCTION_H

#include "instruction.h"
#include "doubledata.h"
#include "engine.h"


// Double instruction base
class QTOPIA_EXPORT BaseDoubleInstruction:public Instruction {
public:
    BaseDoubleInstruction():Instruction(){};
    ~BaseDoubleInstruction(){};

    Data *eval(Data *);
    virtual Data *doEval(DoubleData *){return new DoubleData();};
protected:
    DoubleData *doubleNum;
};
class QTOPIA_EXPORT BaseDoubleInstructionDescription:public InstructionDescription {
public:
    BaseDoubleInstructionDescription();
    ~BaseDoubleInstructionDescription(){};
};

// Factory 
class QTOPIA_EXPORT iDoubleFactory:public BaseDoubleInstruction {
public:
    iDoubleFactory():BaseDoubleInstruction(){};
    ~iDoubleFactory(){};
    Data *eval(Data *);
};
class QTOPIA_EXPORT DoubleFactory:public BaseDoubleInstructionDescription {
public:DoubleFactory();
    ~DoubleFactory(){};
    Instruction *getInstruction(){return new iDoubleFactory();};
};

// Copy
class QTOPIA_EXPORT iDoubleCopy:public BaseDoubleInstruction {
public:
    iDoubleCopy():BaseDoubleInstruction(){};
    ~iDoubleCopy(){};
    Data *eval(Data *);
};
class QTOPIA_EXPORT DoubleCopy:public BaseDoubleInstructionDescription {
public:
    DoubleCopy();
    ~DoubleCopy(){};
    Instruction *getInstruction(){return new iDoubleCopy();};
};

#ifdef ENABLE_INTEGER
class QTOPIA_EXPORT iConvertIntDouble:public Instruction {
public:
    iConvertIntDouble():Instruction(){};
    ~iConvertIntDouble(){};
    Data *eval(Data *);
};
class QTOPIA_EXPORT ConvertIntDouble:public InstructionDescription {
public:
    ConvertIntDouble();
    ~ConvertIntDouble(){};
    Instruction *getInstruction() {return new iConvertIntDouble();};
};
#endif
#ifdef ENABLE_FRACTION
class QTOPIA_EXPORT iConvertFractionDouble:public Instruction {
public:
    iConvertFractionDouble():Instruction(){};
    ~iConvertFractionDouble(){};
    Data *eval(Data *);
};
class QTOPIA_EXPORT ConvertFractionDouble:public InstructionDescription {
public:
    ConvertFractionDouble();
    ~ConvertFractionDouble(){};
    Instruction *getInstruction() {return new iConvertFractionDouble();};
};
#endif

// Mathematical functions
class QTOPIA_EXPORT iAddDoubleDouble:public BaseDoubleInstruction {
public:
    iAddDoubleDouble():BaseDoubleInstruction(){};
    ~iAddDoubleDouble(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT AddDoubleDouble:public BaseDoubleInstructionDescription {
public:
    AddDoubleDouble();
    ~AddDoubleDouble(){};
    Instruction *getInstruction() {return new iAddDoubleDouble();};
};

class QTOPIA_EXPORT iSubtractDoubleDouble:public BaseDoubleInstruction {
public:
    iSubtractDoubleDouble():BaseDoubleInstruction(){};
    ~iSubtractDoubleDouble(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT SubtractDoubleDouble:public BaseDoubleInstructionDescription {
public:
    SubtractDoubleDouble();
    ~SubtractDoubleDouble(){};
    Instruction *getInstruction() {return new iSubtractDoubleDouble();};
};

class QTOPIA_EXPORT iMultiplyDoubleDouble:public BaseDoubleInstruction {
public:
    iMultiplyDoubleDouble():BaseDoubleInstruction(){};
    ~iMultiplyDoubleDouble(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT MultiplyDoubleDouble:public BaseDoubleInstructionDescription {
public:
    MultiplyDoubleDouble();
    ~MultiplyDoubleDouble(){};
    Instruction *getInstruction() {return new iMultiplyDoubleDouble();};
};

class QTOPIA_EXPORT iDivideDoubleDouble:public BaseDoubleInstruction {
public:
    iDivideDoubleDouble():BaseDoubleInstruction(){};
    ~iDivideDoubleDouble(){};
Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DivideDoubleDouble:public BaseDoubleInstructionDescription {
public:
    DivideDoubleDouble();
    ~DivideDoubleDouble(){};
    Instruction *getInstruction() {return new iDivideDoubleDouble();};
};

class QTOPIA_EXPORT iDoublePow:public BaseDoubleInstruction {
public:
    iDoublePow():BaseDoubleInstruction(){};
    ~iDoublePow(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoublePow:public BaseDoubleInstructionDescription {
public:
    DoublePow();
    ~DoublePow(){};
    Instruction *getInstruction() {return new iDoublePow();};
};

// Immediate
class QTOPIA_EXPORT iDoubleSin:public BaseDoubleInstruction {
public:
    iDoubleSin():BaseDoubleInstruction(){};
    ~iDoubleSin(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleSin:public BaseDoubleInstructionDescription {
public:
    DoubleSin();
    ~DoubleSin(){};
    Instruction *getInstruction() {return new iDoubleSin();};
};

class QTOPIA_EXPORT iDoubleCos:public BaseDoubleInstruction {
public:
    iDoubleCos():BaseDoubleInstruction(){};
    ~iDoubleCos(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleCos:public BaseDoubleInstructionDescription {
public:
    DoubleCos();
    ~DoubleCos(){};
    Instruction *getInstruction() {return new iDoubleCos();};
};

class QTOPIA_EXPORT iDoubleTan:public BaseDoubleInstruction {
public:
    iDoubleTan():BaseDoubleInstruction(){};
    ~iDoubleTan(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleTan:public BaseDoubleInstructionDescription {
public:
    DoubleTan();
    ~DoubleTan(){};
    Instruction *getInstruction() {return new iDoubleTan();};
};
class QTOPIA_EXPORT iDoubleASin:public BaseDoubleInstruction {
public:
    iDoubleASin():BaseDoubleInstruction(){};
    ~iDoubleASin(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleASin:public BaseDoubleInstructionDescription {
public:
    DoubleASin();
    ~DoubleASin(){};
    Instruction *getInstruction() {return new iDoubleASin();};
};

class QTOPIA_EXPORT iDoubleACos:public BaseDoubleInstruction {
public:
    iDoubleACos():BaseDoubleInstruction(){};
    ~iDoubleACos(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleACos:public BaseDoubleInstructionDescription {
public:
    DoubleACos();
    ~DoubleACos(){};
    Instruction *getInstruction() {return new iDoubleACos();};
};

class QTOPIA_EXPORT iDoubleATan:public BaseDoubleInstruction {
public:
    iDoubleATan():BaseDoubleInstruction(){};
    ~iDoubleATan(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleATan:public BaseDoubleInstructionDescription {
public:
    DoubleATan();
    ~DoubleATan(){};
    Instruction *getInstruction() {return new iDoubleATan();};
};


class QTOPIA_EXPORT iDoubleLog:public BaseDoubleInstruction {
public:
    iDoubleLog():BaseDoubleInstruction(){};
    ~iDoubleLog(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleLog:public BaseDoubleInstructionDescription {
public:
    DoubleLog();
    ~DoubleLog(){};
    Instruction *getInstruction() {return new iDoubleLog();};
};

class QTOPIA_EXPORT iDoubleLn:public BaseDoubleInstruction {
public:
    iDoubleLn():BaseDoubleInstruction(){};
    ~iDoubleLn(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleLn:public BaseDoubleInstructionDescription {
public:
    DoubleLn();
    ~DoubleLn(){};
    Instruction *getInstruction() {return new iDoubleLn();};
};

class QTOPIA_EXPORT iDoubleExp:public BaseDoubleInstruction {
public:
    iDoubleExp():BaseDoubleInstruction(){};
    ~iDoubleExp(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleExp:public BaseDoubleInstructionDescription {
public:
    DoubleExp();
    ~DoubleExp(){};
    Instruction *getInstruction() {return new iDoubleExp();};
};

class QTOPIA_EXPORT iDoubleOneOverX:public BaseDoubleInstruction {
public:
    iDoubleOneOverX():BaseDoubleInstruction(){};
    ~iDoubleOneOverX(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleOneOverX:public BaseDoubleInstructionDescription {
public:
    DoubleOneOverX();
    ~DoubleOneOverX(){};
    Instruction *getInstruction() {return new iDoubleOneOverX();};
};

class QTOPIA_EXPORT iDoubleFactorial:public BaseDoubleInstruction {
public:
    iDoubleFactorial():BaseDoubleInstruction(){};
    ~iDoubleFactorial(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleFactorial:public BaseDoubleInstructionDescription {
public:
    DoubleFactorial();
    ~DoubleFactorial(){};
    Instruction *getInstruction() {return new iDoubleFactorial();};
};

class QTOPIA_EXPORT iDoubleSquareRoot:public BaseDoubleInstruction {
public:
    iDoubleSquareRoot():BaseDoubleInstruction(){};
    ~iDoubleSquareRoot(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleSquareRoot:public BaseDoubleInstructionDescription {
public:
    DoubleSquareRoot();
    ~DoubleSquareRoot(){};
    Instruction *getInstruction() {return new iDoubleSquareRoot();};
};

class QTOPIA_EXPORT iDoubleCubeRoot:public BaseDoubleInstruction {
public:
    iDoubleCubeRoot():BaseDoubleInstruction(){};
    ~iDoubleCubeRoot(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleCubeRoot:public BaseDoubleInstructionDescription {
public:
    DoubleCubeRoot();
    ~DoubleCubeRoot(){};
    Instruction *getInstruction() {return new iDoubleCubeRoot();};
};

class QTOPIA_EXPORT iDoubleXRootY:public BaseDoubleInstruction {
public:
    iDoubleXRootY():BaseDoubleInstruction(){};
    ~iDoubleXRootY(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleXRootY:public BaseDoubleInstructionDescription {
public:
    DoubleXRootY();
    ~DoubleXRootY(){};
    Instruction *getInstruction() {return new iDoubleXRootY();};
};

class QTOPIA_EXPORT iDoubleSquare:public BaseDoubleInstruction {
public:
    iDoubleSquare():BaseDoubleInstruction(){};
    ~iDoubleSquare(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleSquare:public BaseDoubleInstructionDescription {
public:
    DoubleSquare();
    ~DoubleSquare(){};
    Instruction *getInstruction() {return new iDoubleSquare();};
};

class QTOPIA_EXPORT iDoubleNegate:public BaseDoubleInstruction {
public:
    iDoubleNegate():BaseDoubleInstruction(){};
    ~iDoubleNegate(){};
    Data *doEval(DoubleData *);
};
class QTOPIA_EXPORT DoubleNegate:public BaseDoubleInstructionDescription {
public:
    DoubleNegate();
    ~DoubleNegate(){};
    Instruction *getInstruction() {return new iDoubleNegate();};
};


#endif
