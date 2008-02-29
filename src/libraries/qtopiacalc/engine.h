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
#ifndef NEW_STYLE_DISPLAY
#include <qlineedit.h>
#endif
#include <qlabel.h>

#include <qtopia/calc/instruction.h>

class iBraceOpen;
enum State { sStart, sAppend, sError };
enum Error { eError, eOutOfRange, eDivZero, eNotSolvable, eNonPositive,
    eNonInteger,eNotANumber,eInf,eNoDataFactory,eNegInf };

class QTOPIA_EXPORT Engine {
public:
    Engine();
    ~Engine();

    void registerInstruction(InstructionDescription *);
#ifdef NEW_STYLE_STACK
    void pushInstruction(QString);
    void immediateInstruction(QString);
#else
    void pushInstruction(InstructionDescription *);
#endif
    void evaluate();

    void dualReset();
    void softReset(bool = true);
    void hardReset();
    void pushChar(char);
    void push(QString);
    void delChar();
    void memoryRecall();
    void memorySave();
    void memoryReset();

    void openBrace();
    void closeBrace();

    void setError(Error = eError);
    void setError(QString);
#ifdef NEW_STYLE_DISPLAY
    void setDisplay(QLabel *);
#else
    void setDisplay(QLineEdit *);
#endif
    void setAccType(QString);
    QString getDisplay();
protected:
    friend class iBraceOpen;
    void decBraceCount(){braceCount--;};
private:
    InstructionDescription *resolveDescription(QString);
    Instruction *resolveInstruction(QString);
    Instruction *resolveInstruction(InstructionDescription *);

    void updateDisplay();
    Data *evalStack(Data *,bool,int=0);
#ifdef NEW_STYLE_STACK
    void executeInstructionOnStack(QString, QStack<Data>);
#else
    void executeInstructionOnStack(QString, QStack<InstructionDescription>);
#endif
    int previousInstructionsPrecedence;
    inline bool checkState();

    QString currentType;
    State state;
    int braceCount;
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
#ifdef NEW_STYLE_DISPLAY
    QLabel *lcd;
    QPainter *lcdPainter;
#else
    QLineEdit *lcd;
#endif
};

#define QTOPIA_DEFINED_ENGINE
#include <qtopia/qtopiawinexport.h>

extern QTOPIA_EXPORT Engine *systemEngine;
#endif
