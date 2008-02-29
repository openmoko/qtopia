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
#include "popupimpl.h"
#include "popupim.h"
#include <qpixmap.h>
#include <qapplication.h>

/*
   Constructs the PopupIMImpl.
*/
PopupIMImpl::PopupIMImpl()
    : input(0), icn(0), ref(0)
{
}

/*
   Destroys the PopupIMImpl
*/
PopupIMImpl::~PopupIMImpl()
{
    if (input)
	delete input;
    if (icn)
	delete icn;
}

/*
   If uuid is a valid id for this plugin, sets \a iface to point to the
   Input Method provided and returns QS_OK.  Otherwise sets iface to 0
   and returns QS_FALSE
*/
QRESULT PopupIMImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
        *iface = this;
    else if ( uuid == IID_InputMethod )
        *iface = this;
    else
	return QS_FALSE;

    (*iface)->addRef();
    return QS_OK;
}

/*
   Returns the widget used in the Popup Input Method.
   If the widget is not yet created, constructs the input method widget as a
   child of \a parent with flags \a f.  Otherwise returns a pointer
    to the previously created input method widget.
*/
QWidget *PopupIMImpl::inputMethod( QWidget *parent, Qt::WFlags f )
{
    if ( !input )
	input = new PopupIM( parent, "SimpleInput", f );
    return input;
}

/*
   Returns the name of the input method plugin
*/
QString PopupIMImpl::name()
{
    return qApp->translate( "InputMethods", "SimpleInput" );
}

/*
   Returns the icon for the input method plugin
*/
QPixmap *PopupIMImpl::icon()
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
	"   #..####..#   ",
	"  #...#...#..#  ",
	"  #...#...#..#  ",
	"  #...####...#  ",
	"  #...#......#  ",
	"   #..#.....#   ",
	"   #........#   ",
	"    ##....##    ",
	"      ####      "};

    if ( !icn )
	icn = new QPixmap( (const char **)pix_xpm );
    return icn;
}

/*
   Connects the signal for key press events from the popup input method
   to the slot provided by Qtopia server.
*/
void PopupIMImpl::onKeyPress( QObject *receiver, const char *slot )
{
    if ( input )
        QObject::connect( input, SIGNAL(keyPress(ushort,ushort,ushort,bool,bool)), receiver, slot );
}

/*
   Exports the interface so can be loaded by Qtopia
*/
Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( PopupIMImpl )
}
