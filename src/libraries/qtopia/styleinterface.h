/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
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

#ifndef STYLEINTERFACE_H
#define STYLEINTERFACE_H

#include <qpe/qcom.h>

#ifndef QT_NO_COMPONENT
// {17AF792C-E461-49A9-9B71-068B9A8DDAE4} 
#ifndef IID_Style
#define IID_Style QUuid( 0x17af792c, 0xe461, 0x49a9, 0x9b, 0x71, 0x06, 0x8b, 0x9a, 0x8d, 0xda, 0xe4)
#endif
#endif

class QStyle;

struct StyleInterface : public QUnknownInterface
{
    virtual QStyle *style() = 0;
    virtual QString name() const = 0;
};

#endif
