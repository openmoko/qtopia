/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include "qkeymapper_p.h"
#include <qdebug.h>
#include <private/qevent_p.h>
#include <private/qlocale_p.h>


QKeyMapperPrivate::QKeyMapperPrivate()
{
    keyboardInputLocale = QLocale::system();
    keyboardInputDirection = Qt::RightToLeft;
}

QKeyMapperPrivate::~QKeyMapperPrivate()
{
    // clearMappings();
}

void QKeyMapperPrivate::clearMappings()
{
}

QList<int> QKeyMapperPrivate::possibleKeys(QKeyEvent *e)
{
    QList<int> result;
    if (e->key() && (e->key() != Qt::Key_unknown))
        result << int(e->key() + e->modifiers());
    else if (!e->text().isEmpty())
        result << int(e->text().at(0).unicode() + e->modifiers());
    return result;
}
