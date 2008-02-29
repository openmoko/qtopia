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

#ifndef ADVANCEDIMPL_H
#define ADVANCEDIMPL_H

#include <qpushbutton.h>

#include <qtopia/calc/engine.h>
#include <qtopia/calc/stdinputwidgets.h>

class FormAdvanced:public QWidget {
    Q_OBJECT
public:
	FormAdvanced(QWidget *);
	~FormAdvanced();

public slots:
        void showEvent ( QShowEvent * );

protected:
	void fontChange();

private:
	Type1DecimalInputWidget *siw;
	QPushButton *PBDel,*PBC;
	QPushButton *PBMR,*PBMC,*PBMPlus;
	QPushButton *PBSin,*PBCos,*PBTan,*PBOneOverX;
	QPushButton *PBASin,*PBACos,*PBATan;
	QPushButton *PBLog,*PBLn,*PBFactorial,*PBNegate;
	QPushButton *PBSquare,*PBPow,*PBSquareRoot,*PBXRootY;
	QGridLayout* AdvancedLayout;

	QPixmap *PMPow,*PMLog,*PMSquare,*PMXRootY,*PMOneOverX,
	*PMSquareRoot;

	void updatePictures();

private slots:
	void DelClicked();
	void CClicked();
	void MRClicked();
	void MPlusClicked();
	void MCClicked();
	void aSinClicked();
	void aCosClicked();
	void aTanClicked();
	void SinClicked();
	void CosClicked();
	void TanClicked();
	void OneOverXClicked();
	void LogClicked();
	void LnClicked();
	void FactorialClicked();
	void NegateClicked();
	void SquareClicked();
	void PowClicked();
	void SquareRootClicked();
	void XRootYClicked();
};

#endif
