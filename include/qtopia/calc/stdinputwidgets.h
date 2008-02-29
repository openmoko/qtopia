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
#ifndef STDINPUTWIDGETS_H
#define STDINPUTWIDGETS_H

#include <qlayout.h>
#include <qtopia/calc/engine.h>
#include <qtoolbutton.h>
#include <qtopia/calc/doubleinstruction.h>
#ifdef ENABLE_FRACTION
#include <qtopia/calc/fractioninstruction.h>
#endif

class QTOPIA_EXPORT Type1BaseInputWidget : public QWidget
{
    Q_OBJECT

public:
    Type1BaseInputWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Type1BaseInputWidget(){};

    QGridLayout* Type1BaseInputWidgetLayout;
public slots:
    void val0Clicked ();
    void val1Clicked ();
    void val2Clicked ();
    void val3Clicked ();
    void val4Clicked ();
    void val5Clicked ();
    void val6Clicked ();
    void val7Clicked ();
    void val8Clicked ();
    void val9Clicked ();
    void evalClicked();
    void addClicked ();
    void subClicked ();
    void mulClicked ();
    void divClicked ();
    void negClicked ();
    void bsClicked ();


protected:
    QToolButton* PB0,*PB1,*PB2,*PB3,*PB4,*PB5,*PB6,*PB7,*PB8,*PB9;
    QToolButton* PBPlus;
    QToolButton* PBMinus;
    QToolButton* PBTimes;
    QToolButton* PBDiv;
    QToolButton* PBEval;
    QToolButton* PBNegate;
    QToolButton* PBBS;
};

class QTOPIA_EXPORT Type1DecimalInputWidget : public Type1BaseInputWidget
{
    Q_OBJECT

public:
    Type1DecimalInputWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Type1DecimalInputWidget(){};

public slots:
    void decimalClicked ();

private:
    QToolButton* PBDecimal;
};


class QTOPIA_EXPORT Type1FractionInputWidget : public Type1BaseInputWidget
{
    Q_OBJECT

public:
    Type1FractionInputWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Type1FractionInputWidget(){};

public slots:
    void fractionClicked ();

private:
    QToolButton* PBFraction;
};

class QTOPIA_EXPORT Type2InputWidget:public QWidget {
    Q_OBJECT
public:
    Type2InputWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Type2InputWidget(){};

protected:
    QGridLayout *Type2InputWidgetLayout;

public slots:
    void val0Clicked ();
    void val1Clicked ();
    void val2Clicked ();
    void val3Clicked ();
    void val4Clicked ();
    void val5Clicked ();
    void val6Clicked ();
    void val7Clicked ();
    void val8Clicked ();
    void val9Clicked ();

    void addClicked ();
    void subClicked ();
    void mulClicked ();
    void divClicked ();
    void evalClicked();
    void decimalClicked();

private:
    QToolButton* PB0,*PB1,*PB2,*PB3,*PB4,*PB5,*PB6,*PB7,*PB8,*PB9;
    QToolButton* PBPlus;
    QToolButton* PBMinus;
    QToolButton* PBTimes;
    QToolButton* PBDiv;
    QToolButton* PBEval;
    QToolButton* PBDecimal;
};

#endif
