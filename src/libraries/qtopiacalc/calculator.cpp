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
#include <qtopia/qpeapplication.h>
#include <qfont.h>

#ifndef QT_NO_COMPONENT
#include <qvaluelist.h>
#include <qdir.h>
#include <qtopia/config.h>
#include <qtopia/pluginloader.h>
#else
#include "../../plugins/calculator/simple/simple.h"
#endif

#ifndef NEW_STYLE_STACK
#include "doubleinstruction.h"
#endif

#include "calculator.h"
#include "engine.h"

QTOPIA_EXPORT Engine *systemEngine;

// Calculator class
Calculator::Calculator( QWidget * p, const char *n,WFlags fl) : QWidget (p, n, fl),
	pluginWidgetStack(NULL)
{
    QPEApplication::setInputMethodHint(this, QPEApplication::AlwaysOff);
    setCaption(tr("Calculator"));
    pluginList = new QValueList<CalculatorPlugin>();
    modeBox = 0;
    systemEngine = new Engine();
    calculatorLayout = new QVBoxLayout(this);
#ifdef NEW_STYLE_DISPLAY
    LCD = new QLabel(this);
#else
    LCD = new QLineEdit(this);
    LCD->setFocusPolicy(QWidget::NoFocus);
    QFont f = LCD->font();
    f.setPointSize(f.pointSize()*2);
    LCD->setFont(f);
    LCD->setReadOnly(TRUE);
#endif
    calculatorLayout->addWidget(LCD);
    systemEngine->setDisplay(LCD);

    // Load plugins
#ifndef QT_NO_COMPONENT
    if ( !pluginList->count() ) {
	PluginLoader loader( "calculator" ); // No tr
	QStringList list = loader.list();
	QStringList::Iterator it;
	for (it = list.begin(); it != list.end(); ++it) {
	    CalculatorInterface *iface = 0;
	    CalculatorPlugin plugin;
	    if (loader.queryInterface(*it,IID_Calc, (QUnknownInterface **)&iface) == QS_OK) {
		    plugin.interface = iface;
		    pluginList->append(plugin);
	    }
	}
    }

    if (!pluginList->count()) {
	LCD->setText(qApp->translate("Calculator","No plugins have been found"));
    } else {
	LCD->setAlignment(Qt::AlignRight);
	QValueList<CalculatorPlugin>::Iterator current;
	// Only one plugin, dont use the modeBox
	if (pluginList->count() == 1) {
	    current = pluginList->at(0);
	    (*current).widget = (*current).interface->create(this);
	    calculatorLayout->addWidget((*current).widget);
	// Many plugins, use the modeBox
	} else {
	    modeBox = new QComboBox(this);
	    calculatorLayout->addWidget(modeBox);

	    pluginWidgetStack = new QWidgetStack(this);
	    for (uint it2 = 0; it2 < pluginList->count(); it2++) {
		current = pluginList->at(it2);
		(*current).widget = (*current).interface->create(this);
		modeBox->insertItem((*current).interface->pluginName());
		pluginWidgetStack->addWidget((*current).widget,it2);
	    }
	    calculatorLayout->addWidget(pluginWidgetStack);
	    connect (modeBox, SIGNAL(activated(int)), pluginWidgetStack, SLOT(raiseWidget(int)));

	    Config config("calculator");
	    config.setGroup("View");
	    int lastView = config.readNumEntry("lastView", 0);
	    if ( lastView > modeBox->count() || lastView < 0)
		lastView = 0;
	    modeBox->setCurrentItem( lastView );
	    pluginWidgetStack->raiseWidget( lastView );
	}
    }
#else
    LCD->setAlignment(Qt::AlignRight);
    FormSimple *si = new FormSimple(this);
    calculatorLayout->addWidget(si);
#endif
}

Calculator::~Calculator()
{
    if (modeBox) {
	Config config("calculator");
	config.setGroup("View");
	config.writeEntry("lastView", modeBox->currentItem() );
	delete modeBox;
	delete pluginWidgetStack;
    }
    delete pluginList;
    delete LCD;
    delete systemEngine;
    delete calculatorLayout;
}

void Calculator::keyPressEvent(QKeyEvent *e) {
    int key = e->key();
    switch (key) {
	// backspace
	case Qt::Key_Backtab:
	case Qt::Key_Backspace:
	case Qt::Key_Delete:
	    systemEngine->delChar();
	    break;
	// evaluate
	case Qt::Key_Enter:
	case Qt::Key_Return:
	case Qt::Key_Equal:
	    systemEngine->evaluate();
	    break;
#ifndef NEW_STYLE_STACK
	// basic mathematical keys
	case Qt::Key_Plus:
	    systemEngine->pushInstruction(new AddDoubleDouble());
	    break;
	case Qt::Key_Asterisk:
	    systemEngine->pushInstruction(new MultiplyDoubleDouble());
	    break;
	case Qt::Key_Minus:
	    systemEngine->pushInstruction(new SubtractDoubleDouble());
	    break;
	case Qt::Key_Slash:
	    systemEngine->pushInstruction(new DivideDoubleDouble());
	    break;
#endif
	default:
	    QChar qc = e->text().at(0);
	    if ( qc.isPrint() && !qc.isSpace() )
		systemEngine->pushChar(qc.latin1());
    }
}
