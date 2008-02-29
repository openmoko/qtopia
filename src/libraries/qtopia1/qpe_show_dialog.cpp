/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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


// This is #included by qtopia/qpeapplication.cpp and
//                      qtopia1/qpeapplication.cpp

static void qpe_show_dialog( QDialog* d, bool nomax )
{
    QSize sh = d->sizeHint();
    int w = QMAX(sh.width(),d->width());
    int h = QMAX(sh.height(),d->height());
    if ( !nomax
	    && ( w > qApp->desktop()->width()*3/4
		|| h > qApp->desktop()->height()*3/4 ) )
    {
	d->showMaximized();
    } else {
	d->resize(w,h);
	d->show();
    }
}

