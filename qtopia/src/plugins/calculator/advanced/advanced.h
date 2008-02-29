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

#ifndef ADVANCEDIMPL_H
#define ADVANCEDIMPL_H

#include <qpushbutton.h>

#include <qtopia/calc/engine.h>
#include <qtopia/calc/stdinputwidgets.h>


class QRadioButton;


class AdvancedButton : public QPushButton {
public:
    AdvancedButton(QWidget *p, const QString &name) : QPushButton(p, name) {
	fontChange( font() );
    }
    void fontChange( const QFont &f ) {
	QFontMetrics fm( f );
	int fontHeight = fm.height();
	setMinimumHeight( fontHeight * 2 );
	fontSize10 = f.pixelSize();
	fontSize05 = fontSize10 / 2;
	ascent = fm.ascent();
    }
    void drawButtonLabel( QPainter *p ) {
	int x = (width() - fontSize10) / 2;
	int y = ((height() - fontSize10) / 2) + ascent;
	if ( isDown() )
	    x++, y++;
	advancedDrawLabel(p, x, y);
    }
    virtual void advancedDrawLabel( QPainter *p, int x, int y ) = 0;
    void setInversed(bool i) { inv = i; repaint(); }
protected:
    bool inv;
    int fontSize05, fontSize10, ascent;
};


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
	QPushButton *PBPi, *PBE;
	QPushButton *PBSin,*PBCos,*PBTan,*PBOneOverX;
	QPushButton *PBLog,*PBFactorial;
	AdvancedButton *PBLn,*PBSquare,*PBPow;
        QPushButton *PBInverse;

        QRadioButton *degree, *radians, *gradians;
        bool IsInverse;

private slots:
	void DelClicked();
	void CClicked();
	void MRClicked();
	void MPlusClicked();
	void MCClicked();
	void SinDegClicked();
	void CosDegClicked();
	void TanDegClicked();
	void SinRadClicked();
	void CosRadClicked();
	void TanRadClicked();
	void SinGraClicked();
	void CosGraClicked();
	void TanGraClicked();
	void OneOverXClicked();
	void LnClicked();
	void FactorialClicked();
	void SquareClicked();
	void PowClicked();
	void PiClicked();
        void EClicked();
        void LogClicked();
        void InverseClicked();
        void DegreeMode(bool);
        void RadiansMode(bool);
        void GradiansMode(bool);
};

#endif
