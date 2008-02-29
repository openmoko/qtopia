/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#ifndef TODOCATEGORYSELECTOR_H
#define TODOCATEGORYSELECTOR_H

#include "qtopia/qcategoryselector.h"

/* Todo List wide category scope variable */
static const char * TodoCategoryScope = "Todo List"; // No tr

/* Simple class used to set the scope of our QCategorySelectors when we create them
   from .ui files.  Also see the datebook/datebookcategoryselector.h file.

   This could go away if we could set the scope of a QCategorySelector after
   we create it, but currently that is a large change.
 */

class TodoCategorySelector : public QCategorySelector
{
    public:
        TodoCategorySelector(QWidget *parent = 0)
            : QCategorySelector(TodoCategoryScope, QCategorySelector::Editor | QCategorySelector::DialogView, parent)
            {
            }

};

#endif

