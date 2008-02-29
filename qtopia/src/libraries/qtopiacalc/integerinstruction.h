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
#ifndef CALC_INTEGER_H
#define CALC_INTEGER_H

#include <qtopia/calc/instruction.h>
#include <qtopia/calc/integerdata.h>

// Integer instruction base
class QTOPIA_EXPORT BaseIntegerInstruction : public Instruction {
public:
    BaseIntegerInstruction():Instruction(){};
    ~BaseIntegerInstruction(){};

    Data *eval(Data*);
    Data *doEval(IntegerData *i){return i;};
protected:
    IntegerData *integerNum;
};

// Mathematical functions
class QTOPIA_EXPORT IntegerAdd : public BaseIntegerInstruction {
public:
    IntegerAdd():BaseIntegerInstruction(){};
    ~IntegerAdd(){};
    Data *doEval(IntegerData *);
};
class QTOPIA_EXPORT IntegerSub : public BaseIntegerInstruction {
public:
    IntegerSub():BaseIntegerInstruction(){};
    ~IntegerSub(){};
    Data *doEval(IntegerData *);
};
class QTOPIA_EXPORT IntegerMul : public BaseIntegerInstruction {
public:
    IntegerMul():BaseIntegerInstruction(){};
    ~IntegerMul(){};
    Data *doEval(IntegerData *);
};
class QTOPIA_EXPORT IntegerDiv : public BaseIntegerInstruction {
public:
    IntegerDiv():BaseIntegerInstruction(){};
    ~IntegerDiv(){};
    Data *doEval(IntegerData *);
};

#endif
