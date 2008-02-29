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

#ifndef CONVERSIONIMPL_H
#define CONVERSIONIMPL_H

#include <qtopia/calc/engine.h>
#include <qtopia/calc/doubleinstruction.h>
#include <qtopia/calc/stdinputwidgets.h>

#include <qpushbutton.h>
#include <qwidgetstack.h>
#include <qcombobox.h>

class UnitConversionInstruction : public BaseDoubleInstruction {
public:
    UnitConversionInstruction():BaseDoubleInstruction(){
	name = "UnitConversion";
	retType = type = "Double";
	argCount = 1;
	precedence = 0;
    };
    ~UnitConversionInstruction(){};

   void doEvalI(DoubleData *acc) {
	DoubleData *result = new DoubleData();
	if (tempFrom != 'n' && tempTo != 'n') {
	    double tmp = acc->get();

	    if (tempFrom == 'k')
		tmp -= 273;
	    if (tempFrom == 'f')
		tmp = (tmp - 32) * 5 / 9;
	    if (tempTo == 'k')
		tmp += 273;
	    if (tempTo == 'f')
		tmp = tmp / 5 * 9 + 32;

	    result->set(tmp);
	} else {
	    result->set(acc->get() * to / from);
	}
	systemEngine->putData(result);
    };

    static double from;
    static double to;
    static char tempFrom;
    static char tempTo;
};

struct conversionData {
    QString name;
    double factor;
    QPushButton *button;
};

struct conversionMode {
    QString name;
    QList<conversionData> *dataList;
    conversionData *defaultType;
};

class FormConversion:public QWidget {
    Q_OBJECT
public:
    FormConversion(QWidget *);
    ~FormConversion();

public slots:
    void showEvent ( QShowEvent * );
    void celciusButtonClicked ();
    void fahrenheitButtonClicked ();
    void kelvinButtonClicked ();

private:
    Type1DecimalInputWidget *siw;
    QPushButton *PBC;
    QList<conversionMode> *modeList;
    QComboBox *typeSelector;
    QWidgetStack *conversionStack;

    double currentFactor, newFactor;
    QPushButton *currentButton, *newButton;

    void doConversion();

public slots:
    void selectType(int);
    void conversionButtonClicked();

private slots:
    void CClicked();
};

#endif
