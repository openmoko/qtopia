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
#ifndef CALC_DOUBLEINSTRUCTION_H
#define CALC_DOUBLEINSTRUCTION_H

#include <qtopia/calc/instruction.h>
#include <qtopia/calc/doubledata.h>
#include <qtopia/calc/engine.h>


// Double instruction base
class QTOPIA_EXPORT BaseDoubleInstruction:public Instruction {
public:
    BaseDoubleInstruction();
    ~BaseDoubleInstruction();

    virtual void eval();
    virtual void doEval(DoubleData *,DoubleData *);
    virtual void doEvalI(DoubleData *);
};

// Factory 
class QTOPIA_EXPORT iDoubleFactory:public Instruction {
public:
    iDoubleFactory();
    ~iDoubleFactory(){};
    void eval();
};

// Copy
class QTOPIA_EXPORT iDoubleCopy:public Instruction {
public:
    iDoubleCopy();
    ~iDoubleCopy(){};
    void eval();
};

#ifdef ENABLE_INTEGER
class QTOPIA_EXPORT iConvertIntDouble:public Instruction {
public:
    iConvertIntDouble();
    ~iConvertIntDouble(){};
    Data *eval(Data *);
};
#endif
#ifdef ENABLE_FRACTION
class QTOPIA_EXPORT iConvertFractionDouble:public Instruction {
public:
    iConvertFractionDouble();
    ~iConvertFractionDouble(){};
    void eval();
};
#endif

// Mathematical functions
class QTOPIA_EXPORT iAddDoubleDouble:public BaseDoubleInstruction {
public:
    iAddDoubleDouble();
    ~iAddDoubleDouble(){};
    void doEval(DoubleData *,DoubleData *);
};

class QTOPIA_EXPORT iSubtractDoubleDouble:public BaseDoubleInstruction {
public:
    iSubtractDoubleDouble();
    ~iSubtractDoubleDouble(){};
    void doEval(DoubleData *,DoubleData *);
};

class QTOPIA_EXPORT iMultiplyDoubleDouble:public BaseDoubleInstruction {
public:
    iMultiplyDoubleDouble();
    ~iMultiplyDoubleDouble(){};
    void doEval(DoubleData *,DoubleData *);
};

class QTOPIA_EXPORT iDivideDoubleDouble:public BaseDoubleInstruction {
public:
    iDivideDoubleDouble();
    ~iDivideDoubleDouble(){};
    void doEval(DoubleData *,DoubleData *);
};

// Normal functions
class QTOPIA_EXPORT iDoublePow:public BaseDoubleInstruction {
public:
    iDoublePow();
    ~iDoublePow(){};
    void doEval(DoubleData *,DoubleData *);
};

class QTOPIA_EXPORT iDoubleXRootY:public BaseDoubleInstruction {
public:
    iDoubleXRootY();
    ~iDoubleXRootY(){};
    void doEval(DoubleData *,DoubleData *);
};

// Immediate
class QTOPIA_EXPORT iDoubleSinRad:public BaseDoubleInstruction {
public:
    iDoubleSinRad();
    ~iDoubleSinRad(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleSinDeg:public BaseDoubleInstruction {
public:
    iDoubleSinDeg();
    ~iDoubleSinDeg(){};
    void doEvalI(DoubleData *);
};
class QTOPIA_EXPORT iDoubleSinGra:public BaseDoubleInstruction {
public:
    iDoubleSinGra();
    ~iDoubleSinGra(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleCosRad:public BaseDoubleInstruction {
public:
    iDoubleCosRad();
    ~iDoubleCosRad(){};
    void doEvalI(DoubleData *);
};
class QTOPIA_EXPORT iDoubleCosDeg:public BaseDoubleInstruction {
public:
    iDoubleCosDeg();
    ~iDoubleCosDeg(){};
    void doEvalI(DoubleData *);
};
class QTOPIA_EXPORT iDoubleCosGra:public BaseDoubleInstruction {
public:
    iDoubleCosGra();
    ~iDoubleCosGra(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleTanRad:public BaseDoubleInstruction {
public:
    iDoubleTanRad();
    ~iDoubleTanRad(){};
    void doEvalI(DoubleData *);
};
class QTOPIA_EXPORT iDoubleTanDeg:public BaseDoubleInstruction {
public:
    iDoubleTanDeg();
    ~iDoubleTanDeg(){};
    void doEvalI(DoubleData *);
};
class QTOPIA_EXPORT iDoubleTanGra:public BaseDoubleInstruction {
public:
    iDoubleTanGra();
    ~iDoubleTanGra(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleASinRad:public BaseDoubleInstruction {
public:
    iDoubleASinRad();
    ~iDoubleASinRad(){};
    void doEvalI(DoubleData *);
};
class QTOPIA_EXPORT iDoubleASinDeg:public BaseDoubleInstruction {
public:
    iDoubleASinDeg();
    ~iDoubleASinDeg(){};
    void doEvalI(DoubleData *);
};
class QTOPIA_EXPORT iDoubleASinGra:public BaseDoubleInstruction {
public:
    iDoubleASinGra();
    ~iDoubleASinGra(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleACosRad:public BaseDoubleInstruction {
public:
    iDoubleACosRad();
    ~iDoubleACosRad(){};
    void doEvalI(DoubleData *);
};
class QTOPIA_EXPORT iDoubleACosDeg:public BaseDoubleInstruction {
public:
    iDoubleACosDeg();
    ~iDoubleACosDeg(){};
    void doEvalI(DoubleData *);
};
class QTOPIA_EXPORT iDoubleACosGra:public BaseDoubleInstruction {
public:
    iDoubleACosGra();
    ~iDoubleACosGra(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleATanRad:public BaseDoubleInstruction {
public:
    iDoubleATanRad();
    ~iDoubleATanRad(){};
    void doEvalI(DoubleData *);
};
class QTOPIA_EXPORT iDoubleATanDeg:public BaseDoubleInstruction {
public:
    iDoubleATanDeg();
    ~iDoubleATanDeg(){};
    void doEvalI(DoubleData *);
};
class QTOPIA_EXPORT iDoubleATanGra:public BaseDoubleInstruction {
public:
    iDoubleATanGra();
    ~iDoubleATanGra(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleLog:public BaseDoubleInstruction {
public:
    iDoubleLog();
    ~iDoubleLog(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleLn:public BaseDoubleInstruction {
public:
    iDoubleLn();
    ~iDoubleLn(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleExp:public BaseDoubleInstruction {
public:
    iDoubleExp();
    ~iDoubleExp(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleOneOverX:public BaseDoubleInstruction {
public:
    iDoubleOneOverX();
    ~iDoubleOneOverX(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleFactorial:public BaseDoubleInstruction {
public:
    iDoubleFactorial();
    ~iDoubleFactorial(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleSquareRoot:public BaseDoubleInstruction {
public:
    iDoubleSquareRoot();
    ~iDoubleSquareRoot(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleCubeRoot:public BaseDoubleInstruction {
public:
    iDoubleCubeRoot();
    ~iDoubleCubeRoot(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleSquare:public BaseDoubleInstruction {
public:
    iDoubleSquare();
    ~iDoubleSquare(){};
    void doEvalI(DoubleData *);
};

class QTOPIA_EXPORT iDoubleNegate:public BaseDoubleInstruction {
public:
    iDoubleNegate();
    ~iDoubleNegate(){};
    void doEvalI(DoubleData *);
};


#endif
