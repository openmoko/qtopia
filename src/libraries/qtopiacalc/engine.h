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

#ifndef CALCULATOR_SYSTEM_H
#define CALCULATOR_SYSTEM_H

#include <qstack.h>
#include <qlist.h>
#include <qlineedit.h>
#include <qlabel.h>

#include <qtopia/calc/instruction.h>

class iBraceOpen;
enum State { sStart, sAppend, sError };
enum Error { eError, eOutOfRange, eDivZero, eNotSolvable, eNonPositive,
    eNonInteger,eNotANumber,eInf };
class Engine {
public:
    Engine();
    ~Engine();

    void registerInstruction(InstructionDescription * = 0);
    Instruction *resolveInstruction(InstructionDescription &);
    void pushInstruction(InstructionDescription *);
    void evaluate();

    void dualReset();
    void softReset();
    void hardReset();
    void pushChar(char);
    void delChar();
    void memoryRecall();
    void memorySave();
    void memoryReset();
    void updateDisplay();

    void openBrace();
    void closeBrace();

    void setError(Error);
    void setError(QString);
    void setDisplay(QLineEdit *);
    void setAcc(Data *);
    void setAccType(Data *);
    Data *getAcc();

    Data *convert(Data *from, Data *to);
protected:
    Data *evalStack(Data *,bool,int=0);
    static State state;
    static Data *mem;
    static Data *acc;
    static Data *k;
    static InstructionDescription *kDesc;
    static int myCounter;
    static int braceCount;
    static int decimalPlaces;
    static QStack<InstructionDescription> stack;
    static QList<InstructionDescription> list;
    static QLineEdit *lcd;
    static QLabel *memMark,*kMark;
    static bool secondaryReset;
    friend class iBraceOpen;

    // nasty
    static int previousInstructionsPrecedence;
};

#endif
