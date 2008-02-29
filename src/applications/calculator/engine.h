/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef CALCULATOR_SYSTEM_H
#define CALCULATOR_SYSTEM_H

#include <qtopiaglobal.h>
#include <qstack.h>
#include <qlist.h>
#include <qlabel.h>

#include "instruction.h"
#include "display.h"

class iBraceOpen;
enum State { sStart, sAppend, sError };
enum Error { eError, eOutOfRange, eDivZero, eNotSolvable, eNonPositive,
    eNonInteger,eNotANumber,eInf,eNoDataFactory,eNegInf, eSurpassLimits };
enum ResetState { drNone, drSoft, drHard };

class Engine:public QObject {
    Q_OBJECT
public:
    Engine();
    ~Engine();

    void registerInstruction(Instruction*);
    void pushInstruction(QString);
    void evaluate();

    Data *getData();
    void putData(Data *);

    void dualReset();
    void softReset();
    void hardReset();
    void push(char);
    void push(QString);
    void delChar();
    void memoryRecall();
    void memorySave();
    void memoryReset();

    void openBrace();
    void closeBrace();

    void setError(Error = eError, bool resetStack = true);
    void setError(QString, bool resetStack = true);
    void setDisplay(MyLcdDisplay *);
    void setAccType(QString);
    QString getDisplay();

#ifdef QTOPIA_PHONE
    int numOps();
    bool error();
#endif

signals:
    void stackChanged();
    void dualResetStateChanged(ResetState);


protected:
    friend class iBraceOpen;
    friend class iBraceOpenImpl;
    friend class MyLcdDisplay;

    void incBraceCount();
    bool checkState();
    QString errorString;
    Instruction *resolve(QString);

private:
    void doEvalStack(int=0,bool=false);
    void evalStack(int=0,bool=false);
    void executeInstructionOnStack(QString);
    void executeInstructionOnStack(Instruction *);

    void changeState(State);
    void changeResetState(ResetState);

    int previousInstructionsPrecedence;


    State state;
    ResetState drs;
#ifdef QTOPIA_PHONE
public:
#endif
    QStack<QString*> iStack;
    QStack<Data*> dStack;
private:
    QStack <Data*> tmpDStack;

    QString currentType,wantedType;
    int braceCount;

    QLabel *memMark,*kMark;
    Data *mem;
    Data *recoveredDStack;
    Instruction *kDesc;

    QList<Instruction*> list;
    MyLcdDisplay *lcd;
};

extern Engine *systemEngine;
#endif //CALCULATOR_SYSTEM_H
