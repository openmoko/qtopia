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
#include "conversion.h"

#include <qtopia/calc/doubleinstruction.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qtopia/config.h>

#include <qlayout.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>
#include <qregexp.h>

double UnitConversionInstruction::from = 0;
double UnitConversionInstruction::to = 0;
char UnitConversionInstruction::tempFrom = 'n';
char UnitConversionInstruction::tempTo = 'n';

FormConversion::FormConversion(QWidget *parent) : QWidget(parent) {
    systemEngine->registerInstruction(new UnitConversionInstruction());

    setName( tr("Conversion") ); 
    //resize( 384, 476 );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, sizePolicy().hasHeightForWidth() ) );

    currentButton = 0;

    QVBoxLayout *vbl = new QVBoxLayout(this, 0, 3);
    QFont big(font());
    big.setWeight(QFont::Bold);

    QHBoxLayout *hbl = new QHBoxLayout(vbl);

    typeSelector = new QComboBox(this,"typeSelector");
    hbl->insertWidget(-1,typeSelector);

    PBC = new QPushButton(this,"PBC");
    PBC->setText(tr("CE/C"));
    hbl->insertWidget(-1,PBC);
    PBC->setFont(big);

    modeList = new QList<conversionMode>();
    conversionMode *nextMode;
    QWidget *w;
    QGridLayout *gl;
    conversionData *nextData;
    conversionStack = new QWidgetStack(this);

    // TODO temperature conversions are builtin
    nextMode = new conversionMode;
    nextMode->name = QString( tr("Temperature") );
    nextMode->dataList = new QList<conversionData>();
    typeSelector->insertItem(nextMode->name);
    w = new QWidget(this,nextMode->name);
    QHBoxLayout *gl2 = new QHBoxLayout(w, 0, 3);
    gl2->setAutoAdd(TRUE);
    conversionStack->addWidget(w,modeList->count());

    nextData = new conversionData;
    nextData->name = QString( tr("fahrenheit") );
    nextData->button = new QPushButton(w);
    nextData->button->setToggleButton(TRUE);
    nextData->button->setText(nextData->name);
    connect(nextData->button,SIGNAL(clicked()),this,SLOT(fahrenheitButtonClicked()));
    nextMode->dataList->append(nextData);

    nextData = new conversionData;
    nextData->name = QString( tr("celcius") );
    nextData->button = new QPushButton(w);
    nextData->button->setToggleButton(TRUE);
    nextData->button->setText(nextData->name);
    connect(nextData->button,SIGNAL(clicked()),this,SLOT(celciusButtonClicked()));
    nextMode->dataList->append(nextData);

    nextData = new conversionData;
    nextData->name = QString( tr("kelvin") );
    nextData->button = new QPushButton(w);
    nextData->button->setToggleButton(TRUE);
    nextData->button->setText(nextData->name);
    connect(nextData->button,SIGNAL(clicked()),this,SLOT(kelvinButtonClicked()));
    nextMode->dataList->append(nextData);

    modeList->append(nextMode);
    conversionStack->addWidget(w,modeList->count());
    // end setting up temperature conversion

    // read conversion data files
    QString path = QPEApplication::qpeDir() + "etc/calculator/";
    QDir directory;
    directory.setPath( path );
    QStringList fileList = directory.entryList("*.conf");

    if ( !fileList.count() ) 
        qDebug("No conversion data files are available.");
    else {
        QFile myFile;

        //iterate over found conf files
        for (uint i = 0; i < fileList.count(); i++) {
            QString fileName = fileList[i];
            Config cfg(path+fileName, Config::File);
            QStringList groups = cfg.allGroups();
            nextMode = new conversionMode;
            nextMode->dataList = new QList<conversionData>();
            w = new QWidget(this);
            gl = new QGridLayout(w,2,5);
            gl->setAutoAdd(TRUE);
            conversionStack->addWidget(w,modeList->count()); 
            for (uint j = 0; j < groups.count(); j++) {
                QString group = groups[j];
                if (group == "Translation")
                    continue;
                cfg.setGroup(group);
                if (group == "Name") {
                    nextMode->name = cfg.readEntry("Name");
                    typeSelector->insertItem(nextMode->name);
                } else {
                    nextData = new conversionData;
                    nextData->name = cfg.readEntry("Name");
                    nextData->factor = cfg.readEntry("Factor").toDouble();
                    if (nextData->factor == 1)
                       nextMode->defaultType = nextData;
                    nextMode->dataList->append(nextData);
                    
                    //create the button for this data conversion
                    QPushButton *pb = new QPushButton(w);
                    pb->setToggleButton(TRUE);
                    pb->setText(nextData->name);
                    connect(pb, SIGNAL(clicked()), this, SLOT(conversionButtonClicked()));
                    nextData->button = pb;
                }
            }
            modeList->append(nextMode);
            
        }
    }
    
    vbl->insertWidget(-1,conversionStack);

    connect (PBC, SIGNAL(clicked()), this, SLOT(CClicked()));
    connect (typeSelector, SIGNAL(activated(int)), this, SLOT(selectType(int)));

    // Remember last selection
    Config config("calculator"); // No tr
    config.setGroup("Conversion plugin");
    int lastView = config.readNumEntry("lastView",0);
    if (lastView > (int)modeList->count()+1 || lastView < 0)
	lastView = 0;

    typeSelector->setCurrentItem(lastView);
    conversionStack->raiseWidget(lastView);

    siw = new Type1DecimalInputWidget(this,"SIW");
    vbl->insertWidget(-1,siw);
}
FormConversion::~FormConversion() {
    Config config("calculator"); // No tr
    config.setGroup("Conversion plugin"); // No tr
    config.writeEntry("lastView", conversionStack->id(conversionStack->visibleWidget()));
}

void FormConversion::showEvent ( QShowEvent *e ) {
    systemEngine->setAccType("Double"); // No tr
    
    QWidget::showEvent(e);
}

void FormConversion::selectType(int i) {
    UnitConversionInstruction::tempFrom = UnitConversionInstruction::tempTo = 'n';

    int currentMode = conversionStack->id(conversionStack->visibleWidget());
    QList<conversionData> dataList = *(modeList->at(currentMode))->dataList;

    for (unsigned int c = 0;c < dataList.count(); c++) {
	QPushButton *b = (*(dataList.at(c))).button;
	if (b->isOn())
	    b->toggle();
    }
    currentButton = 0;
    conversionStack->raiseWidget(i);
}

void FormConversion::conversionButtonClicked() {
    doConversion();
}

void FormConversion::doConversion() {
    int currentMode = conversionStack->id(conversionStack->visibleWidget());
    QList<conversionData> dataList = *(modeList->at(currentMode))->dataList;

    bool newButtonFound = FALSE;
    for (unsigned int c = 0;c < dataList.count(); c++) {
	QPushButton *b = (*(dataList.at(c))).button;
	if (b->isOn()) {
	    if (b == currentButton) {
		b->toggle();
	    } else {
		newButton = b;
		newFactor = (*(dataList.at(c))).factor;
		newButtonFound = TRUE;
	    }
	}
    }
    if (newButtonFound) {
	if (currentButton) { // temp convert ignores these anyway
	    UnitConversionInstruction::from = currentFactor;
	    UnitConversionInstruction::to = newFactor;
	    systemEngine->pushInstruction("UnitConversion");
	}
	currentButton = newButton;
	currentFactor = newFactor;
    } else { // current button toggled off
	currentButton = 0;
    }
}

void FormConversion::kelvinButtonClicked() {
    UnitConversionInstruction::tempFrom = UnitConversionInstruction::tempTo;
    UnitConversionInstruction::tempTo = 'k';
    doConversion();
}
void FormConversion::celciusButtonClicked() {
    UnitConversionInstruction::tempFrom = UnitConversionInstruction::tempTo;
    UnitConversionInstruction::tempTo = 'c';
    doConversion();
}
void FormConversion::fahrenheitButtonClicked() {
    UnitConversionInstruction::tempFrom = UnitConversionInstruction::tempTo;
    UnitConversionInstruction::tempTo = 'f';
    doConversion();
}

void FormConversion::CClicked() {
    systemEngine->hardReset();
}

