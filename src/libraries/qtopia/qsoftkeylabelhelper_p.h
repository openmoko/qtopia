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

#ifndef QSOFTKEYLABELHELPERPRIVATE_H
#define QSOFTKEYLABELHELPERPRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
#include "qsoftkeylabelhelper.h"

class QAbstractSoftKeyLabelHelper;

class QSoftKeyLabelHelperPrivate : public QObject
{
    Q_OBJECT
public:
    QSoftKeyLabelHelperPrivate(QAbstractSoftKeyLabelHelper* parent);

    bool eventFilter(QObject* watched, QEvent *event);
    QAbstractSoftKeyLabelHelper* q;
};

#endif // QSOFTKEYLABELHELPERPRIVATE_H
