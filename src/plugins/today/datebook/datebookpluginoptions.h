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



#ifndef DATEBOOK_PLUGIN_OPTIONS_H
#define DATEBOOK_PLUGIN_OPTIONS_H

#include <qtopia/qpeglobal.h>
#include <qwidget.h>

#include "datebookoptionsbase.h"

class DatebookPluginOptions : public DatebookOptionsBase
{
    Q_OBJECT
public:
    DatebookPluginOptions(QWidget *parent, const char *name);

    void writeConfig();

private:
    void readConfig();
};

#endif
