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
#ifndef CALC_DOUBLEINSTRUCTION_H
#define CALC_DOUBLEINSTRUCTION_H

#include "instruction.h"
#include "doubledata.h"

// Double instruction base
class BaseDoubleInstruction:public Instruction {
public:
    BaseDoubleInstruction():Instruction(){};
    ~BaseDoubleInstruction(){};

    Data *eval(Data *);
    virtual Data *doEval(DoubleData *){return new DoubleData();};
protected:
    DoubleData *doubleNum;
};
class BaseDoubleInstructionDescription:public InstructionDescription {
public:
    BaseDoubleInstructionDescription();
    ~BaseDoubleInstructionDescription(){};
};

// Conversions
class iConvertDoubleDouble:public BaseDoubleInstruction {
public:
    iConvertDoubleDouble():BaseDoubleInstruction(){};
    ~iConvertDoubleDouble(){};
    Data *eval(Data *);
};
class ConvertDoubleDouble:public BaseDoubleInstructionDescription {
public:
    ConvertDoubleDouble();
    ~ConvertDoubleDouble(){};
    Instruction *getInstruction(){return new iConvertDoubleDouble();};
};

class iConvertIntDouble:public Instruction {
public:
    iConvertIntDouble():Instruction(){};
    ~iConvertIntDouble(){};
    Data *eval(Data *);
};
class ConvertIntDouble:public InstructionDescription {
public:
    ConvertIntDouble();
    ~ConvertIntDouble(){};
    Instruction *getInstruction() {return new iConvertIntDouble();};
};

class iConvertFractionDouble:public Instruction {
public:
    iConvertFractionDouble():Instruction(){};
    ~iConvertFractionDouble(){};
    Data *eval(Data *);
};
class ConvertFractionDouble:public InstructionDescription {
public:
    ConvertFractionDouble();
    ~ConvertFractionDouble(){};
    Instruction *getInstruction() {return new iConvertFractionDouble();};
};


// Mathematical functions
class iAddDoubleDouble:public BaseDoubleInstruction {
public:
    iAddDoubleDouble():BaseDoubleInstruction(){};
    ~iAddDoubleDouble(){};
    Data *doEval(DoubleData *);
};
class AddDoubleDouble:public BaseDoubleInstructionDescription {
public:
    AddDoubleDouble();
    ~AddDoubleDouble(){};
    Instruction *getInstruction() {return new iAddDoubleDouble();};
};

class iSubtractDoubleDouble:public BaseDoubleInstruction {
public:
    iSubtractDoubleDouble():BaseDoubleInstruction(){};
    ~iSubtractDoubleDouble(){};
    Data *doEval(DoubleData *);
};
class SubtractDoubleDouble:public BaseDoubleInstructionDescription {
public:
    SubtractDoubleDouble();
    ~SubtractDoubleDouble(){};
    Instruction *getInstruction() {return new iSubtractDoubleDouble();};
};

class iMultiplyDoubleDouble:public BaseDoubleInstruction {
public:
    iMultiplyDoubleDouble():BaseDoubleInstruction(){};
    ~iMultiplyDoubleDouble(){};
    Data *doEval(DoubleData *);
};
class MultiplyDoubleDouble:public BaseDoubleInstructionDescription {
public:
    MultiplyDoubleDouble();
    ~MultiplyDoubleDouble(){};
    Instruction *getInstruction() {return new iMultiplyDoubleDouble();};
};

class iDivideDoubleDouble:public BaseDoubleInstruction {
public:
    iDivideDoubleDouble():BaseDoubleInstruction(){};
    ~iDivideDoubleDouble(){};
Data *doEval(DoubleData *);
};
class DivideDoubleDouble:public BaseDoubleInstructionDescription {
public:
    DivideDoubleDouble();
    ~DivideDoubleDouble(){};
    Instruction *getInstruction() {return new iDivideDoubleDouble();};
};

class iDoublePow:public BaseDoubleInstruction {
public:
    iDoublePow():BaseDoubleInstruction(){};
    ~iDoublePow(){};
    Data *doEval(DoubleData *);
};
class DoublePow:public BaseDoubleInstructionDescription {
public:
    DoublePow();
    ~DoublePow(){};
    Instruction *getInstruction() {return new iDoublePow();};
};

// Immediate
class iDoubleSin:public BaseDoubleInstruction {
public:
    iDoubleSin():BaseDoubleInstruction(){};
    ~iDoubleSin(){};
    Data *doEval(DoubleData *);
};
class DoubleSin:public BaseDoubleInstructionDescription {
public:
    DoubleSin();
    ~DoubleSin(){};
    Instruction *getInstruction() {return new iDoubleSin();};
};

class iDoubleCos:public BaseDoubleInstruction {
public:
    iDoubleCos():BaseDoubleInstruction(){};
    ~iDoubleCos(){};
    Data *doEval(DoubleData *);
};
class DoubleCos:public BaseDoubleInstructionDescription {
public:
    DoubleCos();
    ~DoubleCos(){};
    Instruction *getInstruction() {return new iDoubleCos();};
};

class iDoubleTan:public BaseDoubleInstruction {
public:
    iDoubleTan():BaseDoubleInstruction(){};
    ~iDoubleTan(){};
    Data *doEval(DoubleData *);
};
class DoubleTan:public BaseDoubleInstructionDescription {
public:
    DoubleTan();
    ~DoubleTan(){};
    Instruction *getInstruction() {return new iDoubleTan();};
};
class iDoubleASin:public BaseDoubleInstruction {
public:
    iDoubleASin():BaseDoubleInstruction(){};
    ~iDoubleASin(){};
    Data *doEval(DoubleData *);
};
class DoubleASin:public BaseDoubleInstructionDescription {
public:
    DoubleASin();
    ~DoubleASin(){};
    Instruction *getInstruction() {return new iDoubleASin();};
};

class iDoubleACos:public BaseDoubleInstruction {
public:
    iDoubleACos():BaseDoubleInstruction(){};
    ~iDoubleACos(){};
    Data *doEval(DoubleData *);
};
class DoubleACos:public BaseDoubleInstructionDescription {
public:
    DoubleACos();
    ~DoubleACos(){};
    Instruction *getInstruction() {return new iDoubleACos();};
};

class iDoubleATan:public BaseDoubleInstruction {
public:
    iDoubleATan():BaseDoubleInstruction(){};
    ~iDoubleATan(){};
    Data *doEval(DoubleData *);
};
class DoubleATan:public BaseDoubleInstructionDescription {
public:
    DoubleATan();
    ~DoubleATan(){};
    Instruction *getInstruction() {return new iDoubleATan();};
};


class iDoubleLog:public BaseDoubleInstruction {
public:
    iDoubleLog():BaseDoubleInstruction(){};
    ~iDoubleLog(){};
    Data *doEval(DoubleData *);
};
class DoubleLog:public BaseDoubleInstructionDescription {
public:
    DoubleLog();
    ~DoubleLog(){};
    Instruction *getInstruction() {return new iDoubleLog();};
};

class iDoubleLn:public BaseDoubleInstruction {
public:
    iDoubleLn():BaseDoubleInstruction(){};
    ~iDoubleLn(){};
    Data *doEval(DoubleData *);
};
class DoubleLn:public BaseDoubleInstructionDescription {
public:
    DoubleLn();
    ~DoubleLn(){};
    Instruction *getInstruction() {return new iDoubleLn();};
};

class iDoubleExp:public BaseDoubleInstruction {
public:
    iDoubleExp():BaseDoubleInstruction(){};
    ~iDoubleExp(){};
    Data *doEval(DoubleData *);
};
class DoubleExp:public BaseDoubleInstructionDescription {
public:
    DoubleExp();
    ~DoubleExp(){};
    Instruction *getInstruction() {return new iDoubleExp();};
};

class iDoubleOneOverX:public BaseDoubleInstruction {
public:
    iDoubleOneOverX():BaseDoubleInstruction(){};
    ~iDoubleOneOverX(){};
    Data *doEval(DoubleData *);
};
class DoubleOneOverX:public BaseDoubleInstructionDescription {
public:
    DoubleOneOverX();
    ~DoubleOneOverX(){};
    Instruction *getInstruction() {return new iDoubleOneOverX();};
};

class iDoubleFactorial:public BaseDoubleInstruction {
public:
    iDoubleFactorial():BaseDoubleInstruction(){};
    ~iDoubleFactorial(){};
    Data *doEval(DoubleData *);
};
class DoubleFactorial:public BaseDoubleInstructionDescription {
public:
    DoubleFactorial();
    ~DoubleFactorial(){};
    Instruction *getInstruction() {return new iDoubleFactorial();};
};

class iDoubleSquareRoot:public BaseDoubleInstruction {
public:
    iDoubleSquareRoot():BaseDoubleInstruction(){};
    ~iDoubleSquareRoot(){};
    Data *doEval(DoubleData *);
};
class DoubleSquareRoot:public BaseDoubleInstructionDescription {
public:
    DoubleSquareRoot();
    ~DoubleSquareRoot(){};
    Instruction *getInstruction() {return new iDoubleSquareRoot();};
};

class iDoubleCubeRoot:public BaseDoubleInstruction {
public:
    iDoubleCubeRoot():BaseDoubleInstruction(){};
    ~iDoubleCubeRoot(){};
    Data *doEval(DoubleData *);
};
class DoubleCubeRoot:public BaseDoubleInstructionDescription {
public:
    DoubleCubeRoot();
    ~DoubleCubeRoot(){};
    Instruction *getInstruction() {return new iDoubleCubeRoot();};
};

class iDoubleXRootY:public BaseDoubleInstruction {
public:
    iDoubleXRootY():BaseDoubleInstruction(){};
    ~iDoubleXRootY(){};
    Data *doEval(DoubleData *);
};
class DoubleXRootY:public BaseDoubleInstructionDescription {
public:
    DoubleXRootY();
    ~DoubleXRootY(){};
    Instruction *getInstruction() {return new iDoubleXRootY();};
};

class iDoubleSquare:public BaseDoubleInstruction {
public:
    iDoubleSquare():BaseDoubleInstruction(){};
    ~iDoubleSquare(){};
    Data *doEval(DoubleData *);
};
class DoubleSquare:public BaseDoubleInstructionDescription {
public:
    DoubleSquare();
    ~DoubleSquare(){};
    Instruction *getInstruction() {return new iDoubleSquare();};
};

class iDoubleNegate:public BaseDoubleInstruction {
public:
    iDoubleNegate():BaseDoubleInstruction(){};
    ~iDoubleNegate(){};
    Data *doEval(DoubleData *);
};
class DoubleNegate:public BaseDoubleInstructionDescription {
public:
    DoubleNegate();
    ~DoubleNegate(){};
    Instruction *getInstruction() {return new iDoubleNegate();};
};


#endif
