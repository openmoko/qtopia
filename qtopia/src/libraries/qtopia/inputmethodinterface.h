/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef INPUTMETHODINTERFACE_H
#define INPUTMETHODINTERFACE_H

#include <qtopia/qcom.h>

#include <qnamespace.h>
#include <qstring.h>

#ifndef QT_NO_COMPONENT
// {637A8A14-AF98-41DA-969A-2BD16ECDA8C7}
# ifndef IID_InputMethod
#  define IID_InputMethod QUuid( 0x637a8a14, 0xaf98, 0x41da, 0x96, 0x9a, 0x2b, 0xd1, 0x6e, 0xcd, 0xa8, 0xc7)
# endif
#endif

class QWidget;
class QPixmap;
class QObject;
class QWSInputMethod;
class QWSGestureMethod;

struct InputMethodInterface : public QUnknownInterface
{
    virtual QWidget *inputMethod( QWidget *parent, Qt::WFlags f ) = 0;
    virtual void resetState() = 0;
    virtual QPixmap *icon() = 0;
    virtual QString name() = 0;
    virtual void onKeyPress( QObject *receiver, const char *slot ) = 0;
};

// {70F0991C-8282-4625-A279-BD9D7D959FF6} 
#ifndef IID_ExtInputMethod
#define IID_ExtInputMethod QUuid( 0x70f0991c, 0x8282, 0x4625, 0xa2, 0x79, 0xbd, 0x9d, 0x7d, 0x95, 0x9f, 0xf6)
#endif

struct ExtInputMethodInterface : public QUnknownInterface
{
    //identifying functions.
    virtual QString name() = 0;
    virtual QPixmap *icon() = 0;

    // state managment.
    virtual void resetState() = 0;

    virtual QWidget *keyboardWidget( QWidget *parent, Qt::WFlags f ) = 0;
    // filenames, not menu names.
    virtual QStringList compatible() = 0;

    virtual QWSInputMethod *inputMethod() = 0;
    virtual QWidget *statusWidget( QWidget *parent, Qt::WFlags f )= 0;

    virtual void qcopReceive( const QCString &msg, const QByteArray &data )= 0;
};

#endif
