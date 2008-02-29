/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3textview.h"

#ifndef QT_NO_TEXTVIEW

/*! 
  \class Q3TextView
  \brief The Q3TextView class provides a rich text viewer.

  \compat

  This class wraps a read-only \l Q3TextEdit.
  Use a \l Q3TextEdit instead, and call setReadOnly(true)
  to disable editing.
*/

/*! \internal */

Q3TextView::Q3TextView(const QString& text, const QString& context,
                      QWidget *parent, const char *name)
    : Q3TextEdit(text, context, parent, name)
{
    setReadOnly(true);
}

/*! \internal */

Q3TextView::Q3TextView(QWidget *parent, const char *name)
    : Q3TextEdit(parent, name)
{
    setReadOnly(true);
}

/*! \internal */

Q3TextView::~Q3TextView()
{
}

#endif
