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

#ifndef CALCULATOR_SYSTEM_H
#define CALCULATOR_SYSTEM_H

#include <qtopia/qpeglobal.h>
#include <qstack.h>
#include <qlist.h>
#include <qlineedit.h>
#include <qlabel.h>

#include <qtopia/calc/instruction.h>

class iBraceOpen;
enum State { sStart, sAppend, sError };
enum Error { eError, eOutOfRange, eDivZero, eNotSolvable, eNonPositive,
    eNonInteger,eNotANumber,eInf,eNoDataFactory };

class QTOPIA_EXPORT Engine {
public:
    Engine();
    ~Engine();

    void registerInstruction(InstructionDescription *);
    void pushInstruction(InstructionDescription *);
    void pushInstruction2(QString);
    void immediateInstruction(QString);
    void evaluate();

    void dualReset();
    void softReset();
    void softReset2();
    void hardReset();
    void pushChar(char);
    void pushChar2(char);
    void delChar();
    void memoryRecall();
    void memorySave();
    void memorySave2();
    void memoryReset();

    void openBrace();
    void closeBrace();

    void setError(Error = eError);
    void setError(QString);
    void setDisplay(QLineEdit *);
    void setAccType(QString);
    void setAccType2(QString);
    QString getDisplay();
protected:
    friend class iBraceOpen;
    void decBraceCount(){braceCount--;};
private:
    InstructionDescription *resolveDescription(QString);
    Instruction *resolveInstruction(QString);
    Instruction *resolveInstruction(InstructionDescription *);

    void updateDisplay();
    void updateDisplay2();
    Data *evalStack(Data *,bool,int=0);
    void executeInstructionOnStack(QString, QStack<InstructionDescription>);
    void executeInstructionOnStack2(QString, QStack<Data>);
    int previousInstructionsPrecedence;
    inline bool checkState();

    QString currentType;
    State state;
    int braceCount;
    int decimalPlaces;
    bool secondaryReset;

    QStack<InstructionDescription> stack;
    QStack<QString> iStack;
    QStack<Data> dStack;

    QLabel *memMark,*kMark;
    Data *mem;
    Data *emptyDataCache;
    Data *acc;
    Data *k;
    InstructionDescription *kDesc;

    QList<InstructionDescription> list;
    QLineEdit *lcd;
};

#define QTOPIA_DEFINED_ENGINE
#include <qtopia/qtopiawinexport.h>

extern QTOPIA_EXPORT Engine *systemEngine;
#endif
