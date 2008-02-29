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

#ifndef SHEARER_P_H
#define SHEARER_P_H

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

#include "animator_p.h"
#include "parabola_p.h"

#include <QPixmap>

class Painter;
class SelectedItem;


class Shearer : public Animator
{
public:

    static const qreal DEFAULT_MAX_SHEAR_VAL;

    explicit Shearer(qreal maxShearVal = DEFAULT_MAX_SHEAR_VAL);

    void animate(QPainter *,SelectedItem *,qreal percent);

    // Returns description of this class for configuration purposes.
    static const QString &description() { return mDescription; }

private:

    static const QString mDescription;

    qreal maxShearVal;

    Parabola parabola;
};

#endif
