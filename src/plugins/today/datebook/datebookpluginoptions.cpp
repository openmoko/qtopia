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

#include "datebookpluginoptions.h"

#include <qspinbox.h>

#include <qtopia/config.h>

DatebookPluginOptions::DatebookPluginOptions(QWidget *parent, const char *name)
    : DatebookOptionsBase(parent, name)
{
    readConfig();
}

void DatebookPluginOptions::writeConfig()
{
    Config config("datebookplugin");
    config.setGroup("view");
    config.writeEntry("days", dayBox->value() );
    config.writeEntry("limit", maxBox->value() );
    config.write();
}

void DatebookPluginOptions::readConfig()
{
    Config config("datebookplugin");
    config.setGroup("view");
    dayBox->setValue( config.readNumEntry("days", 1) );
    maxBox->setValue( config.readNumEntry("limit", 3) );
}

