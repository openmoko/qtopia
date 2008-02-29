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

#include <qapplication.h>
#include <qstringlist.h>
#include <qlabel.h>
#include "composeimpl.h"
#include "composeim.h"

/*
   Constructs the ComposeImpl
*/
ComposeImpl::ComposeImpl()
    : input(0), icn(0), statWid(0), ref(0)
{
}

/*
   Destroys the ComposeImpl
*/
ComposeImpl::~ComposeImpl()
{
    if (input)
	delete input;
    if (icn)
	delete icn;
    if (statWid)
	delete statWid;
}

/*
   If uuid is a valid id for this plugin, sets \a iface to point to the
   Input Method provided and returns QS_OK.  Otherwise sets iface to 0
   and returns QS_FALSE
*/
QRESULT ComposeImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_ExtInputMethod )
	*iface = this;
    else
	return QS_FALSE;

    (*iface)->addRef();
    return QS_OK;
}

/*
   Exports the interface so can be loaded by Qtopia
*/
Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( ComposeImpl )
}

/*
   Returns 0 for a composing input method plugin.
*/
QWidget *ComposeImpl::keyboardWidget( QWidget *, Qt::WFlags )
{
    return 0;
}


/*
   Returns the QWSInputMethod provided by the plugin.
*/
QWSInputMethod *ComposeImpl::inputMethod( )
{
    if ( !input )
	input = new ComposeIM( );
    return input;
}


/*
   Resets the state of the input method.
*/
void ComposeImpl::resetState()
{
    if ( input )
	input->reset();
}


/*
   Returns the icon for the input method plugin
*/
QPixmap *ComposeImpl::icon()
{
    /* XPM */
    static const char * pix_xpm[] = {
	"16 13 3 1",
	" 	c #FFFFFFFFFFFF",
	"#	c #000000000000",
	".	c #FFFFFFFFFFFF",
	"                ",
	"      ####      ",
	"    ##....##    ",
	"   #........#   ",
	"   #........#   ",
	"  #...####...#  ",
	"  #..#.......#  ",
	"  #..#.......#  ",
	"  #..#.......#  ",
	"   #..####..#   ",
	"   #........#   ",
	"    ##....##    ",
	"      ####      "};

    if (!icn)
	icn = new QPixmap( pix_xpm );
    return icn;
}


/*
   Returns the name of the input method plugin
*/
QString ComposeImpl::name()
{
    return qApp->translate( "InputMethods", "Compose" );
}

/*
For a composing input method, the widget returned by statusWidget()
will be placed in the taskbar when the input method is selected. This
widget is typically used to display status, and can also be used to
let the user interact with the input method.
*/
QWidget *ComposeImpl::statusWidget( QWidget *parent, Qt::WFlags flags )
{
    if (!statWid) {
	statWid = new QLabel( parent, 0, flags );
	statWid->setPixmap(*icon());
    }
    return statWid;
}



/*
The compatible() function can be used to say that this input method is
only compatible with certain other input methods. In this case, there
are no restrictions.
*/
QStringList ComposeImpl::compatible( )
{
    return QStringList();
}


/*
In qcopReceive(), we get notified when there is an event on the
inputmethod channel.
*/
void ComposeImpl::qcopReceive( const QCString &, const QByteArray & )
{
    //process QCop event
}


