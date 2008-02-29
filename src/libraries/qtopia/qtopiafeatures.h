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

#ifndef _QTOPIAFEATURES_H_
#define _QTOPIAFEATURES_H_

#include <qtopiaglobal.h>
class QString;

class QTOPIA_EXPORT QtopiaFeatures
{
public:
    static void setFeature(const QString &);
    static void removeFeature(const QString &);
    static bool hasFeature(const QString &);
};

#endif // _QTOPIAFEATURES_H_
