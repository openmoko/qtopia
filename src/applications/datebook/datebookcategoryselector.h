/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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
#ifndef DATEBOOKCATEGORYSELECTOR_H
#define DATEBOOKCATEGORYSELECTOR_H

#include "qtopia/qcategoryselector.h"


/* Datebook wide category scope variable */
static const char * DateBookCategoryScope = "Calendar"; //No tr

/* Simple class used to set the scope of our QCategorySelectors when we create them
   from .ui files.  Also see the todo/todocategoryselector.h file.

   This could go away if we could set the scope of a QCategorySelector after
   we create it, but currently that is a large change.
 */

class DateBookCategorySelector : public QCategorySelector
{
    public:
        DateBookCategorySelector(QWidget *parent = 0)
            : QCategorySelector(DateBookCategoryScope, QCategorySelector::Editor | QCategorySelector::DialogView, parent)
            {
            }

};

#endif

