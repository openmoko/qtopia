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

#ifndef CALCULATOR_SYSTEM_H
#define CALCULATOR_SYSTEM_H

#include <qtopia/qpeglobal.h>
#include <qstack.h>
#include <qlist.h>
#include <qlabel.h>

#include <qtopia/calc/instruction.h>
#include <qtopia/calc/display.h>

class iBraceOpen;
enum EngineState { sStart, sAppend, sError };
enum Error { eError, eOutOfRange, eDivZero, eNotSolvable, eNonPositive,
    eNonInteger,eNotANumber,eInf,eNoDataFactory,eNegInf, eSurpassLimits };
enum ResetState { drNone, drSoft, drHard };

class QTOPIA_EXPORT Engine:public QObject {
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

    void setError(Error = eError, bool resetStack = TRUE);
    void setError(QString, bool resetStack = TRUE);
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
    void doEvalStack(int=0,bool=FALSE);
    void evalStack(int=0,bool=FALSE);
    void executeInstructionOnStack(QString);
    void executeInstructionOnStack(Instruction *);

    void changeState(EngineState);
    void changeResetState(ResetState);

    int previousInstructionsPrecedence;

    
    EngineState state;
    ResetState drs;
#ifdef QTOPIA_PHONE
public:
#endif
    QStack<QString> iStack;
    QStack<Data> dStack;
private:
    QStack <Data> tmpDStack;

    QString currentType,wantedType;
    int braceCount;

    QLabel *memMark,*kMark;
    Data *mem;
    Data *recoveredDStack;
    Instruction *kDesc;

    QList<Instruction> list;
    MyLcdDisplay *lcd;
};

#define QTOPIA_DEFINED_ENGINE
#include <qtopia/qtopiawinexport.h>

extern QTOPIA_EXPORT Engine *systemEngine;
#endif
