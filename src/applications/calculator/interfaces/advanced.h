/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#ifdef ENABLE_SCIENCE

#ifndef ADVANCEDIMPL_H
#define ADVANCEDIMPL_H

#include <QPushButton>
#include <QEvent>


#include "../engine.h"
#include "stdinputwidgets.h"


class QRadioButton;


class AdvancedButton : public QPushButton {
public:
    AdvancedButton(QWidget *p = 0) : QPushButton(p), inv(false) {
        fontChange( font() );
    }
    void fontChange( const QFont &f ) {
        QFontMetrics fm( f );
        int fontHeight = fm.height();
        setMinimumHeight( fontHeight * 2 );
        fontSize10 = f.pixelSize();
        if (fontSize10 == -1 )
            fontSize10 = f.pointSize();
        fontSize05 = fontSize10 / 2;
        ascent = fm.ascent();
    }

    virtual void advancedDrawLabel( QPainter *p, int x, int y ) = 0;
    void setInversed(bool i) { inv = i; update(); repaint();}

    void paintEvent(QPaintEvent *pe)
    {
        QPushButton::paintEvent(pe);
        QPainter p(this);

        int x = (width() - fontSize10) / 2;
        int y = ((height() - fontSize10) / 2) + ascent;
        if ( isDown() )
            x++, y++;
        advancedDrawLabel(&p, x, y);
    }

protected:
    bool inv;
    int fontSize05, fontSize10, ascent;
};


class FormAdvanced:public DecimalInputWidget{
    Q_OBJECT
public:
        FormAdvanced(QWidget * parent = 0);
        virtual ~FormAdvanced();

        QString interfaceName() { return QString(tr("Scientific")); };


protected:
        void init(int fromRow, int fromCol);
        void showEvent ( QShowEvent * );

private:
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

#endif //ADVANCEDIMPL_H
#endif //ENABLE_SCIENCE
