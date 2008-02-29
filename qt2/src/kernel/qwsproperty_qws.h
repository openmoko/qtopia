/****************************************************************************
** $Id: qt/src/kernel/qwsproperty_qws.h   2.3.12   edited 2005-10-27 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QWSPROPERTY_H
#define QWSPROPERTY_H

#ifndef QT_H
#include "qwscommand_qws.h"
#endif // QT_H

/*********************************************************************
 *
 * Class: QWSPropertyManager
 *
 *********************************************************************/

#ifndef QT_NO_QWS_PROPERTIES

class QWSPropertyManagerData;

class QWSPropertyManager
{
public:
    enum Mode {
	PropReplace = 0,
	PropPrepend,
	PropAppend
    };

    // pre-defined properties
    enum Atom {
	PropSelection = 0
    };
    
    QWSPropertyManager();
    ~QWSPropertyManager();

    bool setProperty( int winId, int property, int mode, const char *data, int len );
    bool hasProperty( int winId, int property );
    bool removeProperty( int winId, int property );
    bool addProperty( int winId, int property );
    bool getProperty( int winId, int property, char *&data, int &len );
    bool removeProperties( int winId );

private:
    QWSPropertyManagerData* d;
};

#endif //QT_NO_QWS_PROPERTIES
#endif //QWSPROPERTY_H
