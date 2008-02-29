/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
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
#include "conversion.h"

#include <qtopia/calc/doubleinstruction.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>

#include <qlayout.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>
#include <qregexp.h>

FormConversion::FormConversion(QWidget *parent,const char *name,WFlags fl)
:QWidget(parent,name,fl) {
    if ( !name )
	setName( "Conversion" );
    resize( 384, 476 );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, sizePolicy().hasHeightForWidth() ) );

    QVBoxLayout *vbl = new QVBoxLayout(this);
    QFont big(font());
    big.setWeight(QFont::Bold);

    // Row 1
    QHBoxLayout *hbl = new QHBoxLayout(vbl);

    PBMPlus = new QPushButton(this,"PBM+");
    PBMPlus->setText(tr("M+"));
    hbl->insertWidget(-1,PBMPlus);
    PBMPlus->setFont(big);

    PBMR = new QPushButton(this,"PBMR");
    PBMR->setText(tr("MR"));
    hbl->insertWidget(-1,PBMR);
    PBMR->setFont(big);

    PBMC = new QPushButton(this,"PBMC");
    PBMC->setText(tr("MC"));
    hbl->insertWidget(-1,PBMC);
    PBMC->setFont(big);

    PBDel = new QPushButton(this,"PBDel");
    PBDel->setText(tr("<-"));
    hbl->insertWidget(-1,PBDel);
    PBDel->setFont(big);

    PBC = new QPushButton(this,"PBC");
    PBC->setText(tr("CE/C"));
    hbl->insertWidget(-1,PBC);
    PBC->setFont(big);

    // Row 2
    typeSelector = new QComboBox(this,"typeSelector");
    vbl->insertWidget(1,typeSelector);

    // Row 3
    // read conversion data files
    conversionStack = new QWidgetStack(this);
    QString path = QPEApplication::qpeDir() + "etc/calculator/";
    QDir directory;
    directory.setPath(path);
    QStringList fileList = directory.entryList("*.cvt");

    if ( !fileList.count() ) {
	qDebug("No conversion data files available");
    } else {
	QFile myfile;
	modeList = new QList<conversionMode>();
	QString line;

	// iterate over found *.cvt files
	for (uint i = 0;i < fileList.count();i++) {
	    QString fileName = fileList[i];
	    QString fullFileName = path + fileName;
	    QFile myfile;
	    myfile.setName(fullFileName);
	    if ( !myfile.open( IO_Translate | IO_ReadOnly ) ) {
		qDebug("Cant open conversion data file %s",fullFileName.latin1());
	    } else {
		QTextStream ts(&myfile);
		conversionMode *nextMode = new conversionMode;
		// strip the .cvt extension from the file name
		nextMode->name = fileName.left(fileName.length() - 4);
		nextMode->dataList = new QList<conversionData>();
		typeSelector->insertItem(nextMode->name);
		// add a new widget to the widgetstack
		QWidget *w = new QWidget(this,nextMode->name);
		QGridLayout *gl = new QGridLayout(w,2,5);
		gl->setAutoAdd(TRUE);
		conversionStack->addWidget(w,modeList->count());
#ifdef QTEST
qDebug("MODE name = %s",nextMode->name.latin1());
#endif
		while ( !ts.eof() ) {
		    line = ts.readLine();
		    if (!line.contains("*") && line.contains(QRegExp("\\S"))) {
			conversionData *nextData = new conversionData;
			int pos = line.findRev(' ');
			nextData->name = line.left(pos);
			nextData->factor = line.right(line.length()-pos-1).toDouble();
#ifdef QTEST
qDebug("DATA name = %s",nextData->name.latin1());
qDebug("DATA string value = %s",line.right(line.length()-pos-1).latin1());
qDebug("DATA value = %f",nextData->factor);
#endif
			if (nextData->factor == 1) 
			    nextMode->defaultType = nextData;
			nextMode->dataList->append(nextData);
			// add a new button to the widget on the widgetstack
			QPushButton *pb = new QPushButton(w);
			pb->setToggleButton(TRUE);
			pb->setText(nextData->name);
			connect(pb,SIGNAL(clicked()),this,SLOT(conversionButtonClicked()));
		    }
		}
		modeList->append(nextMode);
		myfile.close();
	    }
	}
    }
    // TODO remember last selection
    conversionStack->raiseWidget(0);
    vbl->insertWidget(2,conversionStack);

    connect (PBMR, SIGNAL(clicked()), this, SLOT(MRClicked()));
    connect (PBMC, SIGNAL(clicked()), this, SLOT(MCClicked()));
    connect (PBMPlus, SIGNAL(clicked()), this, SLOT(MPlusClicked()));
    connect (PBC, SIGNAL(clicked()), this, SLOT(CClicked()));
    connect (PBDel, SIGNAL(clicked()), this, SLOT(DelClicked()));
    connect (typeSelector, SIGNAL(activated(int)), this, SLOT(selectType(int)));

    // Row 4
    siw = new Type1DecimalInputWidget(this,"SIW");
    vbl->insertWidget(3,siw);
}

void FormConversion::showEvent ( QShowEvent *e ) {
    sys.setAccType("DOUBLE");
    QWidget::showEvent(e);
}

void FormConversion::selectType(int i) {
    conversionStack->raiseWidget(i);
}
void FormConversion::conversionButtonClicked() {
    qDebug("cb clicked");
}

void FormConversion::MCClicked() {
    sys.memoryReset();
}
void FormConversion::MRClicked() {
    sys.memoryRecall();
}
void FormConversion::MPlusClicked() {
    sys.memorySave();
}
void FormConversion::CClicked() {
    sys.dualReset();
}
void FormConversion::DelClicked() {
    sys.delChar();
}

