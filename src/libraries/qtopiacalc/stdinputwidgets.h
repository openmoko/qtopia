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
#ifndef STDINPUTWIDGETS_H
#define STDINPUTWIDGETS_H

#include <qlayout.h>
#include "engine.h"
#include <qtoolbutton.h>
#include "doubleinstruction.h"
#ifdef ENABLE_FRACTION
#include "fractioninstruction.h"
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

protected:
    QToolButton* PB0,*PB1,*PB2,*PB3,*PB4,*PB5,*PB6,*PB7,*PB8,*PB9;
    QToolButton* PBPlus;
    QToolButton* PBMinus;
    QToolButton* PBTimes;
    QToolButton* PBDiv;
    QToolButton* PBEval;
};

class QTOPIA_EXPORT Type1DecimalInputWidget : public Type1BaseInputWidget
{
    Q_OBJECT

public:
    Type1DecimalInputWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Type1DecimalInputWidget(){};

public slots:
    void decimalClicked ();

    void addClicked ();
    void subClicked ();
    void mulClicked ();
    void divClicked ();

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

    void addClicked ();
    void subClicked ();
    void mulClicked ();
    void divClicked ();

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
