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

#include "exceptiondialog.h"

#include <qtopia/qpeapplication.h>


ExceptionDialog::ExceptionDialog( QWidget *parent, const char *name, bool modal, WFlags f )
    : ExceptionDialogBase( parent, name, modal, f )
{
}

ExceptionDialog::Button ExceptionDialog::exec()
{
    mButton = Cancel;
#ifdef QTOPIA_PHONE
    int ret = QPEApplication::execDialog( this );
#else
    int ret = ExceptionDialogBase::exec();
#endif
    if ( ret )
	return mButton;
    else
	return Cancel;
}

ExceptionDialog::Button ExceptionDialog::result()
{
    return mButton;
}

void ExceptionDialog::buttonClicked( int button )
{
    //if ( button == btnGroup->id( btnCurrent ) ) {
    switch ( button ) {
	case 0:
	    mButton = Current;
	    accept();
	    break;
	case 1:
	    mButton = All;
	    accept();
	    break;
	case 2:
	    mButton = Following;
	    accept();
	    break;
	default:
	    mButton = Cancel;
	    reject();
	    break;
    }
}

