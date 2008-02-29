/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
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
#ifndef APPSERVICES_H
#define APPSERVICES_H
#include "appservicesbase.h"
#include <qtopia/applnk.h>

class AppServices : public AppServicesBase
{ 
    Q_OBJECT

public:
    AppServices( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~AppServices();

    void done(int);

private slots:
    void loadState();
    void check(QListViewItem*);

private:
    AppLnkSet* allapps;
};

#endif // APPSERVICES_H
