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
#define QTOPIA_INTERNAL_LOADTRANSLATIONS
#include <qtopia/qpeapplication.h>
#include <qfont.h>
#include <qmessagebox.h>
#include <qtopia/global.h>

#ifndef QT_NO_TRANSLATION
#include <stdlib.h>
#endif

#ifndef QTOPIA_PHONE
#include <qvaluelist.h>
#include <qvbox.h>
#include <qdir.h>
#include <qtopia/config.h>
#include <qtopia/pluginloader.h>
#include <qwhatsthis.h>
#else
#include <qtopia/resource.h>
#include "../../plugins/calculator/phone/phone.h"
#include "../../plugins/calculator/simple/simple.h"
#endif

#include "calculator.h"
#include "engine.h"

QTOPIA_EXPORT Engine *systemEngine;

// Calculator class
Calculator::Calculator( QWidget * p, const char *n,WFlags fl) : QWidget (p, n, fl)
#ifndef QTOPIA_PHONE
,pluginWidgetStack(NULL), errorLabel(0)
#endif
{
    
#ifndef QT_NO_TRANSLATION
    QString libname = "libqtopiacalc";
    QPEApplication::loadTranslations(libname);
#endif

    QPEApplication::setInputMethodHint(this, QPEApplication::AlwaysOff);
    setCaption(tr("Calculator", "application header"));
    systemEngine = new Engine();
    calculatorLayout = new QVBoxLayout(this);
   
    LCD = new MyLcdDisplay(this);
    calculatorLayout->addWidget(LCD);

    systemEngine->setDisplay(LCD);

#ifndef QT_NO_CLIPBOARD
    cb = qApp->clipboard();
    //connect(cb, SIGNAL(dataChanged()),
    //   this, SLOT(clipboardChanged()));
#endif

    connect(systemEngine,SIGNAL(stackChanged()),
	LCD,SLOT(readStack()));

    // Load plugins
#ifndef QTOPIA_PHONE
    pluginList = new QValueList<CalculatorPlugin>();
    modeBox = new QComboBox(this);
    calculatorLayout->addWidget(modeBox);
    pluginWidgetStack = new QWidgetStack(this);
    connect (modeBox, SIGNAL(activated(int)), pluginWidgetStack, SLOT(raiseWidget(int)));
    calculatorLayout->addWidget(pluginWidgetStack);

    Config config("calculator"); // No tr
    config.setGroup("View"); // No tr
    lastView = config.readEntry("lastView", "Simple"); // No tr
    errorLabel = new QLabel(tr("<qt><center>No plugins have been found. "
                "You may enable plugins via the PluginManager.</center></qt>"), this, "errorLabel");
    errorLabel->setAlignment(Qt::AlignHCenter| Qt::AlignVCenter);
    calculatorLayout->addWidget(errorLabel);
#else
    if (!Global::mousePreferred()) {
        si = new FormPhone(this);
        QPEApplication::setInputMethodHint(si, QPEApplication::Number);
        connect(si, SIGNAL( close() ), this, SLOT( close() ) );
        si->installEventFilter(this);

        calculatorLayout->addWidget(si);

    } else {
       si = new FormSimple(this);
       calculatorLayout->addWidget(si);
    }
    ContextMenu * cmenu = new ContextMenu(this);

    QAction * a_copy = new QAction( tr( "Copy" ), Resource::loadIconSet( "copy" ),
            QString::null, 0, this, 0 );
    a_copy->setWhatsThis( tr("Copy the last result.") );
    connect( a_copy, SIGNAL( activated() ), this, SLOT( copy() ) );
    
    
    if (!Global::mousePreferred()) {
        QAction * a_clear = new QAction( tr( "Clear All" ), Resource::loadIconSet( "clearall" ),
                QString::null, 0, this, 0 );
        connect( a_clear, SIGNAL( activated() ), si, SLOT( clearAll() ) );
        a_clear->addTo(cmenu);
    }
    a_copy->addTo(cmenu);
#endif
}

Calculator::~Calculator()
{
#ifndef QTOPIA_PHONE
    if (modeBox->count() > 0 ) {
	Config config("calculator"); // No tr
	config.setGroup("View"); // No tr
	config.writeEntry("lastView", modeBox->currentText() ); // No tr
    }
    delete pluginList;
#endif
    delete LCD;
    delete systemEngine;
}

void Calculator::keyPressEvent(QKeyEvent *e) {
    int key = e->key();

#ifndef QT_NO_CLIPBOARD
    if (e->state() & ControlButton) {
	switch (key) {
	    case Qt::Key_C:
		copy();
		return;
	    case Qt::Key_V:
		paste();
		return;
	    case Qt::Key_X:
		cut();
		return;
	}
    }
#endif

    switch (key) {
	// backspace
	case Qt::Key_Backtab:
	case Qt::Key_Backspace:
        case Qt::Key_Delete:
            if (!Global::mousePreferred()) {
#ifdef QTOPIA_PHONE
    	        sendKeyEvent(si, Key_Delete, TRUE);
	        sendKeyEvent(si, Key_Delete, FALSE);
#else
                ;
#endif
            } else
	        systemEngine->delChar();
	    break;
	// evaluate
	case Qt::Key_Enter:
	case Qt::Key_Return:
	case Qt::Key_Equal:
	    systemEngine->evaluate();           //void paste();
//   void cut();
	    break;
	// basic mathematical keys
	case Qt::Key_Plus:
	    systemEngine->pushInstruction("Add"); // No tr
	    break;
	case Qt::Key_Asterisk:
	    systemEngine->pushInstruction("Multiply"); // No tr
	    break;
	case Qt::Key_Minus:
	    systemEngine->pushInstruction("Subtract"); // No tr
	    break;
	case Qt::Key_Slash:
	    systemEngine->pushInstruction("Divide"); // No tr
	    break;
	case Qt::Key_ParenLeft:
	    systemEngine->openBrace();
	    break;
	case Qt::Key_ParenRight:
	    systemEngine->closeBrace();
	    break;
	default:
	    QChar qc = e->text().at(0);
	    if ( qc.isPrint() && !qc.isSpace() ) {
		systemEngine->push(qc.latin1());
		e->accept();
		return;
	    }
	    QWidget::keyPressEvent(e);
   }
}

void Calculator::copy() {
    cb->setText(systemEngine->getDisplay());
}

void Calculator::cut() {
    copy();
    systemEngine->softReset();
}

void Calculator::paste() {
    QString t = cb->text();
    qWarning(QString("paste: %1").arg(t));
    if (!t.isEmpty()) {
	for (int i=0; (uint)i<t.length(); i++) {
	    switch (t[i].latin1()) {
		case '=':
		    systemEngine->evaluate();
		    break;
		case '+':
		    systemEngine->pushInstruction("Add"); // No tr
		    break;
		case '*':
		    systemEngine->pushInstruction("Multiply"); // No tr
		    break;
		case '-':
		    systemEngine->pushInstruction("Subtract"); // No tr
		    break;
		case '/':
		    systemEngine->pushInstruction("Divide"); // No tr
		    break;
		case '(':
		    systemEngine->openBrace();
		    break;
		case ')':
		    systemEngine->closeBrace();
		    break;
		default:
		    if ( t[i].isPrint() && !t[i].isSpace() ) {
			systemEngine->push(t[i].latin1());
		    }
	    }
	}
    }
}

#ifdef QTOPIA_PHONE
bool Calculator::eventFilter(QObject *o, QEvent *e) {
#ifndef QT_NO_QWS_IM
    if (e->type() == QEvent::IMEnd) {
        QIMEvent *ie = (QIMEvent *) e;
        QChar symbol = ie->text()[0].latin1();
        int key;
        switch (symbol) {
            case '+':
                sendKeyEvent(o, Key_Plus, TRUE);
                break;
            case '-':
                sendKeyEvent(o, Key_Minus, TRUE);
                break;
            case '/':
                sendKeyEvent(o, Key_Slash, TRUE);
                break;
            case '*':
                sendKeyEvent(o, Key_Asterisk, TRUE);
                break;
            case '=':
                sendKeyEvent(o, Key_Enter, TRUE);
                break;
            case '(':
                systemEngine->openBrace();
                break;
            case ')':
                systemEngine->closeBrace();
                break;
            case '.': 
            case ',':
                sendKeyEvent(o, Key_Period, TRUE);
               break; 
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                key = Key_0 + symbol.digitValue();
                sendKeyEvent(o, (Qt::Key)key, TRUE);
                sendKeyEvent(o, (Qt::Key)key, FALSE);
                break;
            default:
                return QWidget::eventFilter(o,e);
                break;
                
        }
        ie->accept();
        return TRUE;
    }     

#endif
    return QWidget::eventFilter(o,e);
}   
#endif

void Calculator::showEvent(QShowEvent *)
{
    systemEngine->memoryReset();
    systemEngine->dualReset();
    Global::hideInputMethod();
    loadPlugins();
}

#ifndef QTOPIA_PHONE
void Calculator::hideEvent(QHideEvent *)
{
    lastView = modeBox->currentText();
}
#endif

#ifdef QTOPIA_PHONE
void Calculator::sendKeyEvent(QObject *o, Qt::Key key, bool pressed) 
{
    QKeyEvent *ke;
    if (pressed)
        ke = new QKeyEvent(QEvent::KeyPress, key, key, 0, QString(QChar(key)));
    else    
        ke = new QKeyEvent(QEvent::KeyRelease, key, key, 0, QString(QChar(key)));
    QPEApplication::sendEvent(o, ke);
}
#endif

void Calculator::loadPlugins() 
{
#ifndef QTOPIA_PHONE
    QValueList<CalculatorPlugin>::Iterator current;

    for (uint it = 0; it<pluginList->count(); it++) {
        current = pluginList->at(it);
        (*current).isShown= FALSE;
    }
    
    PluginLoader loader("calculator");
    QStringList list = loader.list();
    QStringList::Iterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        CalculatorInterface *iface = 0;
        CalculatorPlugin plugin;
        if (loader.queryInterface(*it,IID_Calc, (QUnknownInterface **)&iface) == QS_OK) {
            bool found = FALSE; 
            for (uint index = 0; index<pluginList->count(); index++) {
                current = pluginList->at(index);
                if ((*current).interface->pluginName() == iface->pluginName()) {
                    (*current).isShown = TRUE;
                    found = TRUE;
                    break;
                }
            }
            if (!found) {
                plugin.interface = iface;
                plugin.isShown = TRUE;
                plugin.widget = plugin.interface->create(this);
                pluginList->append(plugin);
            }
        }
    }

    //remove plugins not enabled anymore
    for (current = pluginList->begin(); current != pluginList->end(); ++current) {
        if (!(*current).isShown) {
            pluginWidgetStack->removeWidget((*current).widget);
            delete (*current).widget;
            (*current).interface->release();
            current = pluginList->remove(current);
            current--;
        }
    }

    errorLabel->hide();
    uint count = pluginList->count();
    if (!count) {
        LCD->hide();
        modeBox->hide();
        pluginWidgetStack->hide();
        errorLabel->show();
    } else if (count == 1) {
        LCD->show();
        modeBox->hide();
        pluginWidgetStack->show();
        current = pluginList->at(0);
        int id = pluginWidgetStack->id((*current).widget);
        if (id == -1) { //add at the beginn
            id = 0;
            pluginWidgetStack->addWidget((*current).widget,0);
        }
        pluginWidgetStack->raiseWidget( id );
    } else {
        modeBox->clear();
        modeBox->show();
        pluginWidgetStack->show();
        LCD->show();

        int lastPlugin = 0;
 
        for (uint index = 0; index < pluginList->count(); index++) {
            current = pluginList->at(index);
            pluginWidgetStack->removeWidget((*current).widget);
        }
        for (uint index = 0; index < pluginList->count(); index++) {
            current = pluginList->at(index);
            QString plName = (*current).interface->pluginName();
            modeBox->insertItem(plName);
            if (lastView == plName)
                lastPlugin = index;
            pluginWidgetStack->addWidget((*current).widget, index);
        }
        
        modeBox->setCurrentItem( lastPlugin );
        pluginWidgetStack->raiseWidget( lastPlugin );
    }
#endif
}
