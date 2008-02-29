/**********************************************************************
** Copyright (C) 2000-2003 Trolltech AS.  All rights reserved.
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
#ifndef APPLICATIONINTERFACE_H
#define APPLICATIONINTERFACE_H

#include <qstringlist.h>
#include <qtopia/qcom.h>

#ifndef QT_NO_COMPONENT
// {07E15B48-B947-4334-B866-D2AD58157D8C} 
#ifndef IID_QtopiaApplication
#define IID_QtopiaApplication QUuid( 0x07e15b48, 0xb947, 0x4334, 0xb8, 0x66, 0xd2, 0xad, 0x58, 0x15, 0x7d, 0x8c)
#endif
#endif

struct ApplicationInterface : public QUnknownInterface
{
public:
    virtual QWidget *createMainWindow( const QString &appName, QWidget *parent=0, const char *name=0, Qt::WFlags f=0 ) = 0;
    virtual QStringList applications() const = 0;
};

#endif
