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

#ifndef CONVERSIONIMPL_H
#define CONVERSIONIMPL_H

#include <qtopia/calc/engine.h>
#include <qtopia/calc/stdinputwidgets.h>

#include <qpushbutton.h>
#include <qwidgetstack.h>
#include <qcombobox.h>

struct conversionData {
    QString name;
    double factor;
};

struct conversionMode {
    QString name;
    QList<conversionData> *dataList;
    conversionData *defaultType;
};

class FormConversion:public QWidget {
    Q_OBJECT
public:
    FormConversion(QWidget *,const char*,WFlags);
    ~FormConversion(){};

    public slots:
	void showEvent ( QShowEvent * );

private:
    Engine sys;
    Type1DecimalInputWidget *siw;
    QPushButton *PBDel,*PBC;
    QPushButton *PBMR,*PBMC,*PBMPlus;
    QList<conversionMode> *modeList;
    QComboBox *typeSelector;
    QWidgetStack *conversionStack;

public slots:
    void selectType(int);
    void conversionButtonClicked();

private slots:
    void DelClicked();
    void CClicked();
    void MRClicked();
    void MPlusClicked();
    void MCClicked();
};

#endif
