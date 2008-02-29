/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "todopluginoptions.h"

#include <qtopia/config.h>

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qspinbox.h>

TodoPluginOptions::TodoPluginOptions(QWidget *parent, const char *name)
    : TodoOptionsBase(parent, name)
{
    readConfig();
}

void TodoPluginOptions::writeConfig()
{
    QButton *button = buttonGroup->selected();

    if ( button ) {
	Config config("todoplugin");
	config.setGroup("view");
	config.writeEntry("selection", buttonGroup->id(button) );
	config.writeEntry("days", dayBox->value() );
	config.writeEntry("limitcount", maxBox->value() );
	
	config.write();
    }
}

void TodoPluginOptions::readConfig()
{
    Config config("todoplugin");
    config.setGroup("view");
    int sel = config.readNumEntry("selection", 0);
    buttonGroup->setButton( sel );

    dayBox->setValue( config.readNumEntry("days", 1) );
    maxBox->setValue( config.readNumEntry("limitcount", 2) );
}

