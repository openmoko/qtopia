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
#else
#include "../../plugins/calculator/simple/simple.h"
#endif

#include "calculator.h"
#include "engine.h"

QTOPIA_EXPORT Engine *systemEngine;

// Calculator class
Calculator::Calculator(
#ifdef QTEST
	int argc,char **argv,
#endif
	QWidget * p, const char *n,WFlags fl) : QWidget (p, n, fl)
{
    systemEngine = new Engine();
    // Load plugins
    calculatorLayout = new QVBoxLayout(this);
    LCD = new QLineEdit(this);
    QFont f = LCD->font();
    f.setPointSize(f.pointSize()*2);
    LCD->setFont(f);
    LCD->setReadOnly(TRUE);
    calculatorLayout->addWidget(LCD);
    systemEngine->setDisplay(LCD);

#ifndef QT_NO_COMPONENT
    QString path = QPEApplication::qpeDir() + "plugins/calculator";
#ifndef Q_OS_WIN32
    QDir dir (path, "lib*.so");
#else
    QDir dir (path, "*.dll");
#endif
    QStringList list = dir.entryList();

    pluginList = new QValueList<Plugin>();
    QStringList::Iterator it;
    for (it = list.begin(); it != list.end(); ++it) {
	CalculatorInterface *iface = 0;
	QLibrary *lib = new QLibrary(path + "/" + *it);
	Plugin plugin;
	if (lib->queryInterface(IID_Calc, (QUnknownInterface **)&iface) == QS_OK) {
		plugin.library = lib;
		plugin.interface = iface;
		pluginList->append(plugin);
	} else {
	    delete lib;
	}
    }

    if (!pluginList->count()) {
	LCD->setText("No plugins have been found");
    } else {
	LCD->setAlignment(Qt::AlignRight);
	if (pluginList->count() == 1) {
	    calculatorLayout->addWidget((*(pluginList->at(0))).interface->create(this));
	} else {
	    modeBox = new QComboBox(this);
	    calculatorLayout->addWidget(modeBox);

	    pluginWidgetStack = new QWidgetStack(this);
	    for (uint it2 = 0; it2 < pluginList->count(); it2++) {
		QValueList<Plugin>::Iterator current = pluginList->at(it2);
		(*current).widget = (*current).interface->create(this);
#ifdef QTEST
		QString tmp((*current).widget->name());
		for (int c=1;c<argc;c++) {
		    if (argv[c] == tmp) {
#endif
			modeBox->insertItem((*current).widget->name());
			pluginWidgetStack->addWidget((*current).widget,it2);
#ifdef QTEST
		    } else
		    delete (*current).widget;
		}
#endif
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
    FormSimple *si = new FormSimple(this,"Simple",0);
    calculatorLayout->addWidget(si);
#endif
}

Calculator::~Calculator()
{
#ifndef QT_NO_COMPONENT
    Config config("calculator");
    config.setGroup("View");
    config.writeEntry("lastView", modeBox->currentItem() );
#endif
    /* this was segfaulting on shutdown
    delete LCD;
    delete modeBox;
    delete pluginWidgetStack;
    delete systemEngine;
    delete pluginList;
    delete calculatorLayout;
    */
}

