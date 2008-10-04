/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "testtimeedit.h"
#include "testwidgetslog.h"

#include <QTimeEdit>


TestTimeEdit::TestTimeEdit(QObject* _q)
    : TestDateTimeEdit(_q), q(qobject_cast<QTimeEdit*>(_q))
{ TestWidgetsLog(); }

QString TestTimeEdit::text() const
{
    TestWidgetsLog();
    return q->time().toString(q->displayFormat());
}

bool TestTimeEdit::canWrap(QObject *o)
{ return qobject_cast<QTimeEdit*>(o); }

