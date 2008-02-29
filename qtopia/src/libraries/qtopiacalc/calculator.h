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

#ifndef CALC_H
#define CALC_H

#include <qlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qwidgetstack.h>

#include <qtopia/calc/plugininterface.h>
#include <qtopia/calc/engine.h>

#ifndef QT_NO_CLIPBOARD
#include <qclipboard.h>
#endif

#ifndef QTOPIA_PHONE
#if QT_VERSION >= 0x030000
#include <private/qcomlibrary_p.h>
#define QLibrary QComLibrary
#else
#include <qtopia/qlibrary.h>
#endif

struct CalculatorPlugin {
    CalculatorInterface *interface;
    QWidget *widget;
    bool isShown;
};
#endif

class QTOPIA_EXPORT Calculator:public QWidget {
Q_OBJECT
public:
    Calculator ( QWidget * p = 0, const char *n = 0, WFlags fl=0);
    ~Calculator ();

protected:
    void keyPressEvent(QKeyEvent *e);
#ifdef QTOPIA_PHONE
    bool eventFilter(QObject *o, QEvent *e);
#endif
    void showEvent(QShowEvent *);
#ifndef QTOPIA_PHONE
    void hideEvent(QHideEvent *);
#endif
   
private:
   void loadPlugins();
#ifdef QTOPIA_PHONE
   void sendKeyEvent(QObject *o, Qt::Key key, bool pressed);
#endif
   
       
   QVBoxLayout *calculatorLayout;
#ifndef QTOPIA_PHONE
   QWidgetStack *pluginWidgetStack;
   QComboBox *modeBox;
   QValueList<CalculatorPlugin> *pluginList;
   QString lastView;
   QLabel *errorLabel;
#else
   QWidget *si;
#endif
   
   MyLcdDisplay *LCD;
#ifndef QT_NO_CLIPBOARD
   QClipboard *cb;
   void paste();
   void cut();
private slots:
   void copy();
#endif
};

#endif
